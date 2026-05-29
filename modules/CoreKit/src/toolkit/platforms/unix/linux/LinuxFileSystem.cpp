/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/platforms/FileSystem.hpp>

#if TK_LINUX

#include <toolkit/base/Function.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/threading/Sync.hpp>
#include <toolkit/threading/Thread.hpp>
#include <toolkit/types/DateTime.hpp>
#include <toolkit/types/Path.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/TimeSpan.hpp>
#include <toolkit/utils/StringUtils.hpp>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <poll.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace toolkit
{

    constexpr i32 kWatcherBufferSize = 32 * 1024;
    constexpr u32 kWatcherMask =
        IN_ATTRIB |
        IN_CLOSE_WRITE |
        IN_CREATE |
        IN_DELETE |
        IN_DELETE_SELF |
        IN_MODIFY |
        IN_MOVED_FROM |
        IN_MOVED_TO |
        IN_MOVE_SELF;

    struct LinuxWatchEntry
    {
        i32 descriptor = -1;
        TkString path;
    };

    struct LinuxFileWatcher
    {
        TkString path;
        bool withSubDirs = false;
        bool enabled = false;
        TkAction<const TkString &, FileAction> event;
        TkArray<LinuxWatchEntry> entries;
        volatile i64 activeCallbacks = 0;
    };

    TkMutex s_watcherMutex;
    TkArray<LinuxFileWatcher *> s_watchers;
    TkThread *s_watcherThread = nullptr;
    bool s_watcherThreadActive = false;
    i32 s_inotifyFd = -1;

    TkDateTime dateTimeFromUnixTime(time_t value)
    {
        return TkDateTime(1970, 1, 1) + TkTimeSpan(0, 0, 0, static_cast<i32>(value));
    }

    TkString toString(TkStringView path)
    {
        return TkString(path);
    }

    bool statPath(TkStringView path, struct stat &info)
    {
        const TkString text(path);
        return ::stat(text.getText(), &info) == 0;
    }

    bool lstatPath(TkStringView path, struct stat &info)
    {
        const TkString text(path);
        return ::lstat(text.getText(), &info) == 0;
    }

    bool isDotDir(const char *name)
    {
        return TkStringUtils::compareStrings(name, ".") == 0 || TkStringUtils::compareStrings(name, "..") == 0;
    }

    bool isRegularFileMode(mode_t mode)
    {
        return S_ISREG(mode);
    }

    bool isDirectoryMode(mode_t mode)
    {
        return S_ISDIR(mode);
    }

    bool makeDirRecursive(TkStringView path)
    {
        const TkString target(path);
        if (target.isEmpty())
            return false;
        if (TkFileSystem::isDirExist(target))
            return true;

        TkString current;
        const char *text = target.getText();
        for (i32 i = 0; i < target.length(); ++i)
        {
            current += text[i];
            if ((text[i] == '/' || text[i] == '\\') && current.length() > 1 && !TkFileSystem::isDirExist(current))
            {
                if (::mkdir(current.getText(), 0755) != 0 && errno != EEXIST)
                    return false;
            }
        }

        return ::mkdir(target.getText(), 0755) == 0 || errno == EEXIST;
    }

    bool deleteDirTree(TkStringView path)
    {
        const TkString root(path);
        DIR *dir = ::opendir(root.getText());
        if (!dir)
            return false;

        dirent *entry = nullptr;
        while ((entry = ::readdir(dir)) != nullptr)
        {
            if (isDotDir(entry->d_name))
                continue;

            const TkString child = root / entry->d_name;
            struct stat info{};
            if (!lstatPath(child, info))
            {
                ::closedir(dir);
                return false;
            }

            if (isDirectoryMode(info.st_mode))
            {
                if (!deleteDirTree(child))
                {
                    ::closedir(dir);
                    return false;
                }
            }
            else if (::unlink(child.getText()) != 0)
            {
                ::closedir(dir);
                return false;
            }
        }

        ::closedir(dir);
        return ::rmdir(root.getText()) == 0;
    }

    bool listFilesTop(TkArray<TkString> &results, const TkString &directory, const char *searchPattern)
    {
        DIR *dir = ::opendir(directory.getText());
        if (!dir)
            return errno == ENOENT;

        const char *pattern = searchPattern && *searchPattern ? searchPattern : "*";
        dirent *entry = nullptr;
        while ((entry = ::readdir(dir)) != nullptr)
        {
            if (isDotDir(entry->d_name))
                continue;
            if (::fnmatch(pattern, entry->d_name, 0) != 0)
                continue;

            const TkString child = directory / entry->d_name;
            struct stat info{};
            if (statPath(child, info) && isRegularFileMode(info.st_mode))
                results.add(child);
        }

        ::closedir(dir);
        return true;
    }

    bool listFilesRecursive(TkArray<TkString> &results, const TkString &directory, const char *searchPattern)
    {
        if (!listFilesTop(results, directory, searchPattern))
            return false;

        DIR *dir = ::opendir(directory.getText());
        if (!dir)
            return errno == ENOENT;

        dirent *entry = nullptr;
        while ((entry = ::readdir(dir)) != nullptr)
        {
            if (isDotDir(entry->d_name))
                continue;

            const TkString child = directory / entry->d_name;
            struct stat info{};
            if (statPath(child, info) && isDirectoryMode(info.st_mode))
            {
                if (!listFilesRecursive(results, child, searchPattern))
                {
                    ::closedir(dir);
                    return false;
                }
            }
        }

        ::closedir(dir);
        return true;
    }

    bool writeAll(i32 fd, const byte *data, ssize_t size)
    {
        while (size > 0)
        {
            const ssize_t written = ::write(fd, data, static_cast<size_t>(size));
            if (written < 0)
            {
                if (errno == EINTR)
                    continue;
                return false;
            }
            if (written == 0)
                return false;
            data += written;
            size -= written;
        }
        return true;
    }

    bool copyFileImpl(const TkString &dst, const TkString &src)
    {
        const i32 srcFd = ::open(src.getText(), O_RDONLY | O_CLOEXEC);
        if (srcFd == -1)
            return false;

        struct stat info{};
        const mode_t mode = ::fstat(srcFd, &info) == 0 ? (info.st_mode & 0777) : 0666;
        const i32 dstFd = ::open(dst.getText(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, mode);
        if (dstFd == -1)
        {
            ::close(srcFd);
            return false;
        }

        bool success = true;
        byte buffer[64 * 1024];
        for (;;)
        {
            const ssize_t readBytes = ::read(srcFd, buffer, sizeof(buffer));
            if (readBytes == 0)
                break;
            if (readBytes < 0)
            {
                if (errno == EINTR)
                    continue;
                success = false;
                break;
            }
            if (!writeAll(dstFd, buffer, readBytes))
            {
                success = false;
                break;
            }
        }

        if (::close(dstFd) != 0)
            success = false;
        ::close(srcFd);
        return success;
    }

    bool hasWatchLocked(const LinuxFileWatcher *watcher, const TkString &path)
    {
        for (const LinuxWatchEntry &entry : watcher->entries)
        {
            if (entry.path == path)
                return true;
        }
        return false;
    }

    bool addWatchLocked(LinuxFileWatcher *watcher, const TkString &path)
    {
        if (!watcher || !watcher->enabled || s_inotifyFd == -1 || hasWatchLocked(watcher, path))
            return false;

        const i32 descriptor = ::inotify_add_watch(s_inotifyFd, path.getText(), kWatcherMask);
        if (descriptor == -1)
            return false;

        watcher->entries.add({descriptor, path});
        return true;
    }

    void addWatchRecursiveLocked(LinuxFileWatcher *watcher, const TkString &path)
    {
        if (!addWatchLocked(watcher, path))
            return;

        if (!watcher->withSubDirs)
            return;

        DIR *dir = ::opendir(path.getText());
        if (!dir)
            return;

        dirent *entry = nullptr;
        while ((entry = ::readdir(dir)) != nullptr)
        {
            if (isDotDir(entry->d_name))
                continue;

            const TkString child = path / entry->d_name;
            struct stat info{};
            if (statPath(child, info) && isDirectoryMode(info.st_mode))
                addWatchRecursiveLocked(watcher, child);
        }

        ::closedir(dir);
    }

    LinuxFileWatcher *findWatcherLocked(i32 descriptor, TkString &directory)
    {
        for (LinuxFileWatcher *watcher : s_watchers)
        {
            for (const LinuxWatchEntry &entry : watcher->entries)
            {
                if (entry.descriptor == descriptor)
                {
                    directory = entry.path;
                    return watcher;
                }
            }
        }
        return nullptr;
    }

    FileAction getAction(const inotify_event &event)
    {
        if (event.mask & (IN_MOVED_FROM | IN_MOVED_TO | IN_MOVE_SELF))
            return FileAction::kRename;
        if (event.mask & (IN_CREATE))
            return FileAction::kCreate;
        if (event.mask & (IN_DELETE | IN_DELETE_SELF))
            return FileAction::kDelete;
        if (event.mask & (IN_MODIFY | IN_CLOSE_WRITE | IN_ATTRIB))
            return FileAction::kModify;
        return FileAction::kUnknown;
    }

    void processWatcherEvent(const inotify_event &event)
    {
        if (event.len == 0)
            return;

        LinuxFileWatcher *watcher = nullptr;
        TkString directory;
        TkString path;
        const FileAction action = getAction(event);
        if (action == FileAction::kUnknown)
            return;

        s_watcherMutex.lock();
        watcher = findWatcherLocked(event.wd, directory);
        if (watcher && watcher->enabled)
        {
            path = directory / event.name;
            TSync::interlockedIncrement(&watcher->activeCallbacks);
        }
        else
        {
            watcher = nullptr;
        }
        s_watcherMutex.unlock();

        if (!watcher)
            return;

        if ((event.mask & IN_ISDIR) && (event.mask & (IN_CREATE | IN_MOVED_TO)) && watcher->withSubDirs)
        {
            s_watcherMutex.lock();
            addWatchRecursiveLocked(watcher, path);
            s_watcherMutex.unlock();
        }

        watcher->event(path, action);
        TSync::interlockedDecrement(&watcher->activeCallbacks);
    }

    i32 watcherThreadMain()
    {
        byte buffer[kWatcherBufferSize];
        while (s_watcherThreadActive)
        {
            pollfd fd{};
            fd.fd = s_inotifyFd;
            fd.events = POLLIN;
            const i32 pollResult = ::poll(&fd, 1, 100);
            if (pollResult <= 0)
                continue;

            for (;;)
            {
                const ssize_t length = ::read(s_inotifyFd, buffer, sizeof(buffer));
                if (length < 0)
                {
                    if (errno == EINTR)
                        continue;
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                        break;
                    LogError("[FileWatcher] inotify read failed errno=%i", errno);
                    break;
                }
                if (length == 0)
                    break;

                ssize_t offset = 0;
                while (offset + static_cast<ssize_t>(sizeof(inotify_event)) <= length)
                {
                    const inotify_event *event = reinterpret_cast<const inotify_event *>(buffer + offset);
                    processWatcherEvent(*event);
                    offset += static_cast<ssize_t>(sizeof(inotify_event)) + event->len;
                }
            }
        }
        return 0;
    }

    bool ensureWatcherThreadLocked()
    {
        if (s_inotifyFd == -1)
        {
            s_inotifyFd = ::inotify_init();
            if (s_inotifyFd == -1)
            {
                LogError("[FileWatcher] inotify_init failed errno=%i", errno);
                return false;
            }
            const i32 statusFlags = ::fcntl(s_inotifyFd, F_GETFL, 0);
            if (statusFlags != -1)
                ::fcntl(s_inotifyFd, F_SETFL, statusFlags | O_NONBLOCK);
            const i32 descriptorFlags = ::fcntl(s_inotifyFd, F_GETFD, 0);
            if (descriptorFlags != -1)
                ::fcntl(s_inotifyFd, F_SETFD, descriptorFlags | FD_CLOEXEC);
        }

        if (!s_watcherThread)
        {
            s_watcherThreadActive = true;
            s_watcherThread = TkThread::start(watcherThreadMain, "FileWatcher Thread", ThreadPriority::kBelowNormal);
        }
        return s_watcherThread != nullptr;
    }

    bool TkFileSystem::makeDir(const TkStringView path)
    {
        return makeDirRecursive(path);
    }

    bool TkFileSystem::deleteDir(const TkStringView path, bool deleteContents)
    {
        const TkString text(path);
        if (!deleteContents)
            return ::rmdir(text.getText()) == 0;
        return deleteDirTree(text);
    }

    bool TkFileSystem::isDirExist(const TkStringView path)
    {
        struct stat info{};
        return statPath(path, info) && isDirectoryMode(info.st_mode);
    }

    bool TkFileSystem::listDir(TkArray<TkString> &results, const TkString &path, const char *searchPattern, bool recursive)
    {
        if (recursive)
            return listFilesRecursive(results, path, searchPattern);
        return listFilesTop(results, path, searchPattern);
    }

    bool TkFileSystem::listChildren(TkArray<TkString> &results, const TkString &path)
    {
        DIR *dir = ::opendir(path.getText());
        if (!dir)
            return errno == ENOENT;

        dirent *entry = nullptr;
        while ((entry = ::readdir(dir)) != nullptr)
        {
            if (isDotDir(entry->d_name))
                continue;

            const TkString child = path / entry->d_name;
            struct stat info{};
            if (statPath(child, info) && isDirectoryMode(info.st_mode))
                results.add(child);
        }

        ::closedir(dir);
        return true;
    }

    bool TkFileSystem::copyDir(const TkStringView dst, const TkStringView src, bool recursive)
    {
        if (!isDirExist(src) || TkPath::isSame(dst, src))
            return false;

        if (!isDirExist(dst) && !makeDir(dst))
            return false;

        const TkString srcDir(src);
        const TkString dstDir(dst);
        DIR *dir = ::opendir(srcDir.getText());
        if (!dir)
            return false;

        bool success = true;
        dirent *entry = nullptr;
        while ((entry = ::readdir(dir)) != nullptr)
        {
            if (isDotDir(entry->d_name))
                continue;

            const TkString srcPath = srcDir / entry->d_name;
            const TkString dstPath = dstDir / entry->d_name;
            struct stat info{};
            if (!statPath(srcPath, info))
            {
                success = false;
                break;
            }

            if (isDirectoryMode(info.st_mode))
            {
                if (recursive && !copyDir(dstPath, srcPath, true))
                {
                    success = false;
                    break;
                }
            }
            else if (isRegularFileMode(info.st_mode) && !copyFile(dstPath, srcPath))
            {
                success = false;
                break;
            }
        }

        ::closedir(dir);
        return success;
    }

    bool TkFileSystem::deleteFile(const TkStringView path)
    {
        const TkString text(path);
        return ::unlink(text.getText()) == 0;
    }

    bool TkFileSystem::isFileExist(const TkStringView path)
    {
        struct stat info{};
        return statPath(path, info) && isRegularFileMode(info.st_mode);
    }

    psize TkFileSystem::getFileSize(const TkStringView path)
    {
        struct stat info{};
        if (!statPath(path, info) || isDirectoryMode(info.st_mode))
            return 0;
        return static_cast<psize>(info.st_size);
    }

    bool TkFileSystem::isReadOnly(const TkStringView path)
    {
        const TkString text(path);
        if (::access(text.getText(), W_OK) == -1)
            return errno == EACCES || errno == EROFS;
        return false;
    }

    bool TkFileSystem::setReadOnly(const TkStringView path, bool isReadOnly)
    {
        const TkString text(path);
        struct stat info{};
        if (::stat(text.getText(), &info) != 0)
            return false;

        mode_t mode = info.st_mode;
        if (isReadOnly)
            mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
        else
            mode |= S_IWUSR;
        return ::chmod(text.getText(), mode) == 0;
    }

    bool TkFileSystem::moveFile(const TkStringView dst, const TkStringView src, bool overwrite)
    {
        if (TkPath::isSame(dst, src))
            return true;

        const TkString dstPath(dst);
        const TkString srcPath(src);
        if (!overwrite && isFileExist(dstPath))
            return false;

        if (overwrite)
            ::unlink(dstPath.getText());

        if (::rename(srcPath.getText(), dstPath.getText()) == 0)
            return true;

        if (errno != EXDEV)
            return false;

        if (!copyFile(dstPath, srcPath))
            return false;
        return deleteFile(srcPath);
    }

    bool TkFileSystem::copyFile(const TkStringView dst, const TkStringView src)
    {
        if (TkPath::isSame(dst, src))
            return true;
        return copyFileImpl(toString(dst), toString(src));
    }

    TkDateTime TkFileSystem::getLastEdit(const TkStringView path)
    {
        struct stat info{};
        return statPath(path, info) ? dateTimeFromUnixTime(info.st_mtime) : TkDateTime::minValue();
    }

    void TkFileSystem::getDirPath(const SpecialFolder type, TkString &result)
    {
        const char *homeEnv = ::getenv("HOME");
        const TkString home = homeEnv && *homeEnv ? homeEnv : "/";

        switch (type)
        {
        case SpecialFolder::kDesktop:
            result = home / "Desktop";
            break;
        case SpecialFolder::kDocuments:
            result = home / "Documents";
            break;
        case SpecialFolder::kPictures:
            result = home / "Pictures";
            break;
        case SpecialFolder::kAppData:
            result = "/usr/share";
            break;
        case SpecialFolder::kLocalAppData:
        {
            const char *dataHome = ::getenv("XDG_DATA_HOME");
            if (dataHome && *dataHome)
                result = dataHome;
            else
                result = home / ".local/share";
            break;
        }
        case SpecialFolder::kProgramData:
            result = "/usr/share";
            break;
        case SpecialFolder::kTemporary:
        {
            const char *tmp = ::getenv("TMPDIR");
            result = tmp && *tmp ? tmp : "/tmp";
            break;
        }
        }

        TkPath::normalize(result);
    }

    FileWatcherHandle TkFileSystem::openFileWatcher(const TkString &directory, bool withSubDirs)
    {
        if (!isDirExist(directory))
            return nullptr;

        LinuxFileWatcher *watcher = TkMemory::newObject<LinuxFileWatcher>(TkMemory::getDefault());
        watcher->path = directory;
        watcher->withSubDirs = withSubDirs;
        watcher->enabled = true;

        s_watcherMutex.lock();
        if (!ensureWatcherThreadLocked())
        {
            s_watcherMutex.unlock();
            TkMemory::deletePtr(TkMemory::getDefault(), watcher);
            return nullptr;
        }
        addWatchRecursiveLocked(watcher, directory);
        if (watcher->entries.isEmpty())
        {
            s_watcherMutex.unlock();
            TkMemory::deletePtr(TkMemory::getDefault(), watcher);
            return nullptr;
        }
        s_watchers.add(watcher);
        s_watcherMutex.unlock();

        return reinterpret_cast<FileWatcherHandle>(watcher);
    }

    void TkFileSystem::closeFileWatcher(FileWatcherHandle handle)
    {
        if (!handle)
            return;

        LinuxFileWatcher *watcher = reinterpret_cast<LinuxFileWatcher *>(handle);
        s_watcherMutex.lock();
        watcher->enabled = false;
        s_watchers.remove(watcher);
        for (const LinuxWatchEntry &entry : watcher->entries)
        {
            if (entry.descriptor != -1 && s_inotifyFd != -1)
                ::inotify_rm_watch(s_inotifyFd, entry.descriptor);
        }
        watcher->entries.clear();

        const bool shouldStopThread = s_watchers.isEmpty() && s_watcherThread;
        if (shouldStopThread)
            s_watcherThreadActive = false;
        s_watcherMutex.unlock();

        while (TSync::atomicRead(&watcher->activeCallbacks) > 0)
            TkThread::sleep(1);

        if (shouldStopThread)
        {
            s_watcherThread->join();
            s_watcherMutex.lock();
            TkThread::destroy(s_watcherThread);
            s_watcherThread = nullptr;
            if (s_inotifyFd != -1)
            {
                ::close(s_inotifyFd);
                s_inotifyFd = -1;
            }
            s_watcherMutex.unlock();
        }

        TkMemory::deletePtr(TkMemory::getDefault(), watcher);
    }

    TkAction<const TkString &, FileAction> &TkFileSystem::getEvent(FileWatcherHandle handle)
    {
        static TkAction<const TkString &, FileAction> emptyEvent;
        if (!handle)
            return emptyEvent;
        LinuxFileWatcher *watcher = reinterpret_cast<LinuxFileWatcher *>(handle);
        return watcher->event;
    }

    bool TkFileSystem::isEnabled(FileWatcherHandle handle)
    {
        return handle != nullptr;
    }
}

#endif
