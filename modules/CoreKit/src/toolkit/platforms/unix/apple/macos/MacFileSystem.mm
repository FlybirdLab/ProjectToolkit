/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.h"

#if TK_MACOS

#include <toolkit/platforms/FileSystem.hpp>
#include "../AppleUtils.h"

#include <toolkit/base/Function.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/types/String.hpp>

#include <CoreServices/CoreServices.h>

namespace toolkit
{
    namespace
    {
        struct MacFileWatcher
        {
            TkString directory;
            bool withSubDirs = false;
            bool enabled = false;
            CFRunLoopRef runLoop = nullptr;
            FSEventStreamRef stream = nullptr;
            TkAction<const TkString&, FileAction> event;
        };

        TkString trimTrailingSlash(const TkString& path)
        {
            TkString result(path);
            while (result.length() > 1 && result.endsWith('/'))
                result.resize(result.length() - 1);
            return result;
        }

        bool isDirectEventPath(const TkString& directory, const TkString& path)
        {
            const TkString root = trimTrailingSlash(directory);
            if (path == root)
                return true;
            if (path.length() <= root.length() + 1)
                return false;
            if (path.find(root) != 0 || path[root.length()] != '/')
                return false;

            const TkString relative = path.substring(root.length() + 1);
            return relative.find('/') == -1;
        }

        FileAction getFileAction(FSEventStreamEventFlags flags)
        {
            if (flags & kFSEventStreamEventFlagItemRemoved)
                return FileAction::kDelete;
            if (flags & kFSEventStreamEventFlagItemRenamed)
                return FileAction::kRename;
            if (flags & kFSEventStreamEventFlagItemCreated)
                return FileAction::kCreate;
            if (flags & kFSEventStreamEventFlagItemModified)
                return FileAction::kModify;
            if (flags & kFSEventStreamEventFlagItemInodeMetaMod)
                return FileAction::kModify;
            if (flags & kFSEventStreamEventFlagItemChangeOwner)
                return FileAction::kModify;
            if (flags & kFSEventStreamEventFlagItemXattrMod)
                return FileAction::kModify;
            return FileAction::kUnknown;
        }

        void directoryWatchCallback(ConstFSEventStreamRef, void* watcherPtr, size_t eventCount, void* eventPaths, const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId[])
        {
            MacFileWatcher* watcher = reinterpret_cast<MacFileWatcher*>(watcherPtr);
            if (!watcher || !watcher->enabled)
                return;

            CFArrayRef pathArray = reinterpret_cast<CFArrayRef>(eventPaths);
            for (size_t eventIndex = 0; eventIndex < eventCount; ++eventIndex)
            {
                const FSEventStreamEventFlags flags = eventFlags[eventIndex];
                if (!(flags & kFSEventStreamEventFlagItemIsFile) && !(flags & kFSEventStreamEventFlagItemIsDir))
                    continue;

                const FileAction action = getFileAction(flags);
                if (action == FileAction::kUnknown)
                    continue;

                CFStringRef eventPath = reinterpret_cast<CFStringRef>(CFArrayGetValueAtIndex(pathArray, static_cast<CFIndex>(eventIndex)));
                const TkString resolvedPath = AppleUtils::toString(eventPath);
                if (resolvedPath.isEmpty())
                    continue;
                if (!watcher->withSubDirs && !isDirectEventPath(watcher->directory, resolvedPath))
                    continue;

                watcher->event(resolvedPath, action);
            }
        }

        void releaseWatcher(MacFileWatcher* watcher)
        {
            if (!watcher)
                return;

            watcher->enabled = false;
            if (watcher->stream)
            {
                FSEventStreamStop(watcher->stream);
                if (watcher->runLoop)
                    FSEventStreamUnscheduleFromRunLoop(watcher->stream, watcher->runLoop, kCFRunLoopDefaultMode);
                FSEventStreamInvalidate(watcher->stream);
                FSEventStreamRelease(watcher->stream);
                watcher->stream = nullptr;
            }
            if (watcher->runLoop)
            {
                CFRelease(watcher->runLoop);
                watcher->runLoop = nullptr;
            }
            TkMemory::deletePtr(TkMemory::getDefault(), watcher);
        }
    }

    FileWatcherHandle TkFileSystem::openFileWatcher(const TkString& directory, bool withSubDirs)
    {
        if (directory.isEmpty() || !isDirExist(directory))
            return kInvalidFileWatcherHandle;

        MacFileWatcher* watcher = TkMemory::newObject<MacFileWatcher>(TkMemory::getDefault());
        watcher->directory = trimTrailingSlash(directory);
        watcher->withSubDirs = withSubDirs;

        CFStringRef path = AppleUtils::toCFString(directory);
        if (!path)
        {
            releaseWatcher(watcher);
            return kInvalidFileWatcherHandle;
        }

        const void* values[] = {path};
        CFArrayRef pathsToWatch = CFArrayCreate(nullptr, values, 1, &kCFTypeArrayCallBacks);
        CFRelease(path);
        if (!pathsToWatch)
        {
            releaseWatcher(watcher);
            return kInvalidFileWatcherHandle;
        }

        FSEventStreamContext context{};
        context.version = 0;
        context.info = watcher;
        context.retain = nullptr;
        context.release = nullptr;
        context.copyDescription = nullptr;

        constexpr CFAbsoluteTime kLatencySeconds = 0.2;
        watcher->stream = FSEventStreamCreate(
            nullptr,
            &directoryWatchCallback,
            &context,
            pathsToWatch,
            kFSEventStreamEventIdSinceNow,
            kLatencySeconds,
            kFSEventStreamCreateFlagUseCFTypes | kFSEventStreamCreateFlagNoDefer | kFSEventStreamCreateFlagFileEvents);
        CFRelease(pathsToWatch);

        if (!watcher->stream)
        {
            releaseWatcher(watcher);
            return kInvalidFileWatcherHandle;
        }

        watcher->runLoop = CFRunLoopGetCurrent();
        CFRetain(watcher->runLoop);
        FSEventStreamScheduleWithRunLoop(watcher->stream, watcher->runLoop, kCFRunLoopDefaultMode);

        if (!FSEventStreamStart(watcher->stream))
        {
            LogError("[FileWatcher] FSEventStreamStart failed for %s", directory.getText());
            releaseWatcher(watcher);
            return kInvalidFileWatcherHandle;
        }

        watcher->enabled = true;
        return reinterpret_cast<FileWatcherHandle>(watcher);
    }

    void TkFileSystem::closeFileWatcher(FileWatcherHandle handle)
    {
        releaseWatcher(reinterpret_cast<MacFileWatcher*>(handle));
    }

    TkAction<const TkString&, FileAction>& TkFileSystem::getEvent(FileWatcherHandle handle)
    {
        static TkAction<const TkString&, FileAction> emptyEvent;
        if (!handle)
            return emptyEvent;
        MacFileWatcher* watcher = reinterpret_cast<MacFileWatcher*>(handle);
        return watcher->event;
    }

    bool TkFileSystem::isEnabled(FileWatcherHandle handle)
    {
        if (!handle)
            return false;
        MacFileWatcher* watcher = reinterpret_cast<MacFileWatcher*>(handle);
        return watcher->enabled;
    }
}

#endif
