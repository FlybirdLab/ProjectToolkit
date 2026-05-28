/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/platforms/FileSystem.hpp>

#if TK_LINUX

#include <toolkit/base/Assert.hpp>
#include <toolkit/base/Function.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/threading/Sync.hpp>
#include <toolkit/threading/Thread.hpp>
#include <toolkit/types/DateTime.hpp>
#include <toolkit/types/Path.hpp>
#include <toolkit/types/TimeSpan.hpp>
#include <toolkit/utils/StringUtils.hpp>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace toolkit
{
    static const TkDateTime UnixEpoch(1970, 1, 1);

    static bool isDotDir(const char *name)
    {
        return TkStringUtils::compareStrings(name, ".") == 0 || TkStringUtils::compareStrings(name, "..") == 0;
    }

    static TkDateTime toDateTime(time_t value)
    {
        return UnixEpoch + TkTimeSpan(static_cast<i64>(value) * TkTimeSpan::TicksPerSecond);
    }

    static bool deleteDirTree(const TkStringView path)
    {
        const TkString root(path);
        DIR *dir = opendir(root.getText());
        if (!dir)
            return false;

        bool ok = true;
        dirent *entry = nullptr;
        while ((entry = readdir(dir)) != nullptr)
        {
            if (isDotDir(entry->d_name))
                continue;

            TkString child = root / entry->d_name;
            struct stat info {};
            if (lstat(child.getText(), &info) != 0)
            {
                ok = false;
                break;
            }

            if (S_ISDIR(info.st_mode))
            {
                if (!deleteDirTree(child))
                {
                    ok = false;
                    break;
                }
            }
            else if (unlink(child.getText()) != 0)
            {
                ok = false;
                break;
            }
        }
        closedir(dir);
        return ok && rmdir(root.getText()) == 0;
    }

    static bool matchPattern(const char *name, const char *searchPattern)
    {
        return !searchPattern || searchPattern[0] == '\0' || TkStringUtils::compareStrings(searchPattern, "*") == 0 || fnmatch(searchPattern, name, 0) == 0;
    }

    static bool listFilesTop(TkArray<TkString> &results, const TkString &path, const char *searchPattern)
    {
        DIR *dir = opendir(path.getText());
        if (!dir)
            return false;

        dirent *entry = nullptr;
        while ((entry = readdir(dir)) != nullptr)
        {
            if (isDotDir(entry->d_name))
                continue;

            TkString child = path / entry->d_name;
            struct stat info {};
            if (lstat(child.getText(), &info) != 0)
                continue;

            if (S_ISREG(info.st_mode) && matchPattern(entry->d_name, searchPattern))
                results.add(child);
        }

        closedir(dir);
        return true;
    }

    static bool listFilesAll(TkArray<TkString> &results, const TkString &path, const char *searchPattern)
    {
        if (!listFilesTop(results, path, searchPattern))
            return false;

        DIR *dir = opendir(path.getText());
        if (!dir)
            return false;

        bool ok = true;
        dirent *entry = nullptr;
        while ((entry = readdir(dir)) != nullptr)
        {
            if (isDotDir(entry->d_name))
                continue;

            TkString child = path / entry->d_name;
            struct stat info {};
            if (lstat(child.getText(), &info) != 0)
                continue;

            if (S_ISDIR(info.st_mode) && !listFilesAll(results, child, searchPattern))
            {
                ok = false;
                break;
            }
        }

        closedir(dir);
        return ok;
    }

    FileHandle TkFileSystem::openFile(OpenFileParams &params)
    {
        i32 flags = 0;
        const u32 access = static_cast<u32>(params.access);
        if (access == static_cast<u32>(FileAccess::kReadWrite))
            flags |= O_RDWR;
        else if (access == static_cast<u32>(FileAccess::kWrite))
            flags |= O_WRONLY;
        else
            flags |= O_RDONLY;

        switch (params.mode)
        {
        case FileMode::kCreateNew:
            flags |= O_CREAT | O_EXCL;
            break;
        case FileMode::kCreateAlways:
            flags |= O_CREAT | O_TRUNC;
            break;
        case FileMode::kOpenExisting:
            break;
        case FileMode::kOpenAlways:
            flags |= O_CREAT;
            break;
        case FileMode::kTruncateExisting:
            flags |= O_TRUNC;
            break;
        }

        const TkString path(params.path);
        const i32 fd = open(path.getText(), flags, 0666);
        if (fd < 0)
        {
            LogError("[FileSys] Error:%s", strerror(errno));
            return kInvalidFileHandle;
        }
        return reinterpret_cast<FileHandle>(static_cast<intptr>(fd + 1));
    }

    void TkFileSystem::closeFile(FileHandle handle)
    {
        if (!isFileOpened(handle))
            return;
        close(static_cast<i32>(reinterpret_cast<intptr>(handle)) - 1);
    }

    bool TkFileSystem::readFile(FileHandle handle, void *buffer, u32 bytesToRead, u32 *bytesRead)
    {
        if (!isFileOpened(handle))
            return false;
        const ssize_t result = read(static_cast<i32>(reinterpret_cast<intptr>(handle)) - 1, buffer, bytesToRead);
        if (result < 0)
        {
            if (bytesRead)
                *bytesRead = 0;
            return false;
        }
        if (bytesRead)
            *bytesRead = static_cast<u32>(result);
        return true;
    }

    bool TkFileSystem::writeFile(FileHandle handle, const void *buffer, u32 bytesToWrite, u32 *bytesWritten)
    {
        if (!isFileOpened(handle))
            return false;
        const ssize_t result = write(static_cast<i32>(reinterpret_cast<intptr>(handle)) - 1, buffer, bytesToWrite);
        if (result < 0)
        {
            if (bytesWritten)
                *bytesWritten = 0;
            return false;
        }
        if (bytesWritten)
            *bytesWritten = static_cast<u32>(result);
        return true;
    }

    u32 TkFileSystem::getFileSize(FileHandle handle)
    {
        if (!isFileOpened(handle))
            return 0;
        struct stat info {};
        if (fstat(static_cast<i32>(reinterpret_cast<intptr>(handle)) - 1, &info) != 0)
            return 0;
        return static_cast<u32>(info.st_size);
    }

    TkDateTime TkFileSystem::getFileLastWrite(FileHandle handle)
    {
        if (!isFileOpened(handle))
            return TkDateTime::minValue();
        struct stat info {};
        if (fstat(static_cast<i32>(reinterpret_cast<intptr>(handle)) - 1, &info) != 0)
            return TkDateTime::minValue();
        return toDateTime(info.st_mtime);
    }

    u32 TkFileSystem::getFilePos(FileHandle handle)
    {
        if (!isFileOpened(handle))
            return 0;
        return static_cast<u32>(lseek(static_cast<i32>(reinterpret_cast<intptr>(handle)) - 1, 0, SEEK_CUR));
    }

    void TkFileSystem::setFilePos(FileHandle handle, u32 seek)
    {
        if (isFileOpened(handle))
            lseek(static_cast<i32>(reinterpret_cast<intptr>(handle)) - 1, seek, SEEK_SET);
    }

    bool TkFileSystem::isFileOpened(FileHandle handle)
    {
        return handle != kInvalidFileHandle;
    }

    bool TkFileSystem::makeDir(const TkStringView path)
    {
        if (isDirExist(path))
            return true;

        TkString dir(path);
        TkPath::normalize(dir);
        const i32 slashIndex = dir.findLast('/');
        if (slashIndex > 0)
        {
            const TkString parent(dir.getText(), slashIndex);
            if (parent.hasChars() && !isDirExist(parent) && !makeDir(parent))
                return false;
        }

        return mkdir(dir.getText(), 0755) == 0 || errno == EEXIST;
    }

    bool TkFileSystem::deleteDir(const TkStringView path, bool deleteContents)
    {
        const TkString p(path);
        return deleteContents ? deleteDirTree(p) : rmdir(p.getText()) == 0;
    }

    bool TkFileSystem::isDirExist(const TkStringView path)
    {
        struct stat info {};
        const TkString p(path);
        return stat(p.getText(), &info) == 0 && S_ISDIR(info.st_mode);
    }

    bool TkFileSystem::listDir(TkArray<TkString> &results, const TkString &path, const char *searchPattern, bool recursive)
    {
        return recursive ? listFilesAll(results, path, searchPattern) : listFilesTop(results, path, searchPattern);
    }

    bool TkFileSystem::listChildren(TkArray<TkString> &results, const TkString &path)
    {
        DIR *dir = opendir(path.getText());
        if (!dir)
            return false;

        dirent *entry = nullptr;
        while ((entry = readdir(dir)) != nullptr)
        {
            if (isDotDir(entry->d_name))
                continue;

            TkString child = path / entry->d_name;
            struct stat info {};
            if (lstat(child.getText(), &info) == 0 && S_ISDIR(info.st_mode))
                results.add(child);
        }

        closedir(dir);
        return true;
    }

    bool TkFileSystem::deleteFile(const TkStringView path)
    {
        const TkString p(path);
        return unlink(p.getText()) == 0;
    }

    bool TkFileSystem::isFileExist(const TkStringView path)
    {
        struct stat info {};
        const TkString p(path);
        return stat(p.getText(), &info) == 0 && S_ISREG(info.st_mode);
    }

    psize TkFileSystem::getFileSize(const TkStringView path)
    {
        struct stat info {};
        const TkString p(path);
        if (stat(p.getText(), &info) != 0 || S_ISDIR(info.st_mode))
            return 0;
        return static_cast<psize>(info.st_size);
    }

    bool TkFileSystem::isReadOnly(const TkStringView path)
    {
        const TkString p(path);
        return access(p.getText(), W_OK) != 0 && errno == EACCES;
    }

    bool TkFileSystem::setReadOnly(const TkStringView path, bool isReadOnly)
    {
        struct stat info {};
        const TkString p(path);
        if (stat(p.getText(), &info) != 0)
            return false;
        mode_t mode = info.st_mode;
        if (isReadOnly)
            mode &= ~S_IWUSR;
        else
            mode |= S_IWUSR;
        return chmod(p.getText(), mode) == 0;
    }

    bool TkFileSystem::moveFile(const TkStringView dst, const TkStringView src, bool overwrite)
    {
        const TkString dstPath(dst);
        const TkString srcPath(src);
        if (!overwrite && isFileExist(dstPath))
            return false;
        if (overwrite)
            unlink(dstPath.getText());
        if (rename(srcPath.getText(), dstPath.getText()) == 0)
            return true;
        if (errno == EXDEV && copyFile(dstPath, srcPath))
        {
            unlink(srcPath.getText());
            return true;
        }
        return false;
    }

    bool TkFileSystem::copyFile(const TkStringView dst, const TkStringView src)
    {
        const TkString srcPath(src);
        const TkString dstPath(dst);
        const i32 srcFile = open(srcPath.getText(), O_RDONLY);
        if (srcFile < 0)
            return false;

        const i32 dstFile = open(dstPath.getText(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (dstFile < 0)
        {
            close(srcFile);
            return false;
        }

        bool ok = true;
        char buffer[4096];
        ssize_t readSize = 0;
        while ((readSize = read(srcFile, buffer, sizeof(buffer))) > 0)
        {
            char *ptr = buffer;
            while (readSize > 0)
            {
                const ssize_t writeSize = write(dstFile, ptr, static_cast<size_t>(readSize));
                if (writeSize < 0)
                {
                    if (errno == EINTR)
                        continue;
                    ok = false;
                    break;
                }
                readSize -= writeSize;
                ptr += writeSize;
            }
            if (!ok)
                break;
        }
        if (readSize < 0)
            ok = false;

        close(dstFile);
        close(srcFile);
        return ok;
    }

    bool TkFileSystem::copyDir(const TkStringView dst, const TkStringView src, bool recursive)
    {
        if (!isDirExist(src) || TkPath::isSame(dst, src))
            return false;
        if (!isDirExist(dst) && !makeDir(dst))
            return false;

        DIR *dir = opendir(TkString(src).getText());
        if (!dir)
            return false;

        bool ok = true;
        dirent *entry = nullptr;
        while ((entry = readdir(dir)) != nullptr)
        {
            if (isDotDir(entry->d_name))
                continue;

            const TkString srcPath = TkString(src) / entry->d_name;
            const TkString dstPath = TkString(dst) / entry->d_name;
            struct stat info {};
            if (lstat(srcPath.getText(), &info) != 0)
                continue;

            if (S_ISDIR(info.st_mode))
            {
                if (recursive && !copyDir(dstPath, srcPath, true))
                {
                    ok = false;
                    break;
                }
            }
            else if (!copyFile(dstPath, srcPath))
            {
                ok = false;
                break;
            }
        }

        closedir(dir);
        return ok;
    }

    TkDateTime TkFileSystem::getLastEdit(const TkStringView path)
    {
        struct stat info {};
        const TkString p(path);
        if (stat(p.getText(), &info) != 0)
            return TkDateTime::minValue();
        return toDateTime(info.st_mtime);
    }

    void TkFileSystem::getDirPath(const SpecialFolder type, TkString &result)
    {
        passwd *pwd = getpwuid(getuid());
        const TkString home = pwd && pwd->pw_dir ? TkString(pwd->pw_dir) : System::getWorkDir();
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
            TkString dataHome;
            result = System::getEnvVar("XDG_DATA_HOME", dataHome) ? dataHome : home / ".local/share";
            break;
        }
        case SpecialFolder::kProgramData:
            result = "/usr/share";
            break;
        case SpecialFolder::kTemporary:
            result = System::getTempDir();
            break;
        }
    }

    struct LinuxFileWatcher
    {
        TkString root;
        bool withSubDirs = false;
        bool closing = false;
        i64 activeCallbacks = 0;
        TkAction<const TkString &, FileAction> event;
        TkArray<i32> watches;
        TkArray<TkString> watchPaths;
    };

    namespace LinuxFileWatcherService
    {
        static TkMutex s_mutex;
        static TkArray<LinuxFileWatcher *> s_watchers;
        static i32 s_inotify = -1;
        static TkThread *s_thread = nullptr;
        static bool s_active = false;

        static LinuxFileWatcher *findWatcher(i32 wd, TkString *path)
        {
            for (LinuxFileWatcher *watcher : s_watchers)
            {
                for (i32 i = 0; i < watcher->watches.count(); ++i)
                {
                    if (watcher->watches[i] == wd)
                    {
                        if (path)
                            *path = watcher->watchPaths[i];
                        return watcher;
                    }
                }
            }
            return nullptr;
        }

        static void addWatch(LinuxFileWatcher *watcher, const TkString &path)
        {
            const i32 wd = inotify_add_watch(s_inotify, path.getText(), IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
            if (wd < 0)
                return;
            watcher->watches.add(wd);
            watcher->watchPaths.add(path);
        }

        static void addSubDirWatches(LinuxFileWatcher *watcher, const TkString &path)
        {
            DIR *dir = opendir(path.getText());
            if (!dir)
                return;

            dirent *entry = nullptr;
            while ((entry = readdir(dir)) != nullptr)
            {
                if (isDotDir(entry->d_name))
                    continue;

                TkString child = path / entry->d_name;
                struct stat info {};
                if (lstat(child.getText(), &info) == 0 && S_ISDIR(info.st_mode))
                {
                    addWatch(watcher, child);
                    addSubDirWatches(watcher, child);
                }
            }
            closedir(dir);
        }

        static i32 run()
        {
            static constexpr i32 bufferSize = 16 * 1024;
            char buffer[bufferSize];

            while (s_active)
            {
                fd_set readSet;
                FD_ZERO(&readSet);
                FD_SET(s_inotify, &readSet);
                timeval timeout {};
                timeout.tv_usec = 10000;

                const i32 selectResult = select(s_inotify + 1, &readSet, nullptr, nullptr, &timeout);
                if (selectResult <= 0)
                    continue;

                const ssize_t length = read(s_inotify, buffer, sizeof(buffer));
                if (length <= 0)
                    continue;

                ssize_t offset = 0;
                while (offset < length)
                {
                    auto *event = reinterpret_cast<inotify_event *>(buffer + offset);
                    TkString parent;
                    LinuxFileWatcher *watcher = nullptr;

                    s_mutex.lock();
                    watcher = findWatcher(event->wd, &parent);
                    if (watcher && !watcher->closing)
                        TSync::interlockedIncrement(&watcher->activeCallbacks);
                    else
                        watcher = nullptr;
                    s_mutex.unlock();

                    if (watcher && event->len > 0)
                    {
                        FileAction action = FileAction::kUnknown;
                        if (event->mask & (IN_CREATE | IN_MOVED_TO))
                            action = FileAction::kCreate;
                        else if (event->mask & (IN_DELETE | IN_MOVED_FROM))
                            action = FileAction::kDelete;
                        else if (event->mask & IN_MODIFY)
                            action = FileAction::kModify;

                        const TkString path = parent / event->name;
                        if (action != FileAction::kUnknown)
                            watcher->event(path, action);

                        if (watcher->withSubDirs && (event->mask & IN_ISDIR) && (event->mask & (IN_CREATE | IN_MOVED_TO)))
                        {
                            s_mutex.lock();
                            addWatch(watcher, path);
                            addSubDirWatches(watcher, path);
                            s_mutex.unlock();
                        }
                    }
                    if (watcher)
                        TSync::interlockedDecrement(&watcher->activeCallbacks);

                    offset += sizeof(inotify_event) + event->len;
                }
            }
            return 0;
        }
    }

    FileWatcherHandle TkFileSystem::openFileWatcher(const TkString &directory, bool withSubDirs)
    {
        using namespace LinuxFileWatcherService;

        if (!isDirExist(directory))
            return kInvalidFileWatcherHandle;

        s_mutex.lock();
        if (s_inotify < 0)
        {
            s_inotify = inotify_init1(IN_NONBLOCK);
            if (s_inotify < 0)
            {
                s_mutex.unlock();
                return kInvalidFileWatcherHandle;
            }
            s_active = true;
            s_thread = TkThread::start(run, "FileWatcher Thread", ThreadPriority::kBelowNormal);
        }

        auto *watcher = TkMemory::newObject<LinuxFileWatcher>(TkMemory::getDefault());
        watcher->root = directory;
        watcher->withSubDirs = withSubDirs;
        addWatch(watcher, directory);
        if (withSubDirs)
            addSubDirWatches(watcher, directory);
        s_watchers.add(watcher);
        s_mutex.unlock();

        return reinterpret_cast<FileWatcherHandle>(watcher);
    }

    void TkFileSystem::closeFileWatcher(FileWatcherHandle handle)
    {
        using namespace LinuxFileWatcherService;

        if (!handle)
            return;

        auto *watcher = reinterpret_cast<LinuxFileWatcher *>(handle);
        s_mutex.lock();
        watcher->closing = true;
        for (i32 wd : watcher->watches)
            inotify_rm_watch(s_inotify, wd);
        s_watchers.remove(watcher);
        const bool shouldStop = s_watchers.isEmpty() && s_thread != nullptr;
        if (shouldStop)
            s_active = false;
        s_mutex.unlock();

        while (TSync::atomicRead(&watcher->activeCallbacks) > 0)
            TkThread::sleep(1);

        if (shouldStop)
        {
            s_thread->join();
            TkThread::destroy(s_thread);
            s_thread = nullptr;
            close(s_inotify);
            s_inotify = -1;
        }

        TkMemory::deletePtr(TkMemory::getDefault(), watcher);
    }

    TkAction<const TkString &, FileAction> &TkFileSystem::getEvent(FileWatcherHandle handle)
    {
        TK_ASSERT(handle);
        return reinterpret_cast<LinuxFileWatcher *>(handle)->event;
    }

    bool TkFileSystem::isEnabled(FileWatcherHandle handle)
    {
        return handle != nullptr;
    }
}

#endif
