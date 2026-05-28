/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/platforms/FileSystem.hpp>

#if TK_POSIX

#include <toolkit/collection/Array.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/threading/Sync.hpp>
#include <toolkit/types/DateTime.hpp>
#include <toolkit/types/Path.hpp>
#include <toolkit/types/TimeSpan.hpp>

#include <cerrno>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if TK_LINUX
#include <sys/inotify.h>
#include <sys/select.h>
#endif

namespace toolkit
{
    static TkDateTime dateTimeFromUnixTime(time_t value)
    {
        return TkDateTime(1970, 1, 1) + TkTimeSpan(0, 0, 0, static_cast<i32>(value));
    }

    static bool statPath(const TkStringView path, struct stat &info)
    {
        return ::stat(path.get(), &info) == 0;
    }

    static bool isDotDir(const char *name)
    {
        return TkStringUtils::compareStrings(name, ".") == 0 || TkStringUtils::compareStrings(name, "..") == 0;
    }

    static bool makeDirRecursive(const TkStringView path)
    {
        if (path.isEmpty())
            return false;
        if (TkFileSystem::isDirExist(path))
            return true;

        TkString current;
        const char *text = path.get();
        for (i32 i = 0; i < path.length(); ++i)
        {
            current += text[i];
            if ((text[i] == '/' || text[i] == '\\') && current.length() > 1 && !TkFileSystem::isDirExist(current))
            {
                if (::mkdir(current.get(), 0755) != 0 && errno != EEXIST)
                    return false;
            }
        }
        return ::mkdir(path.get(), 0755) == 0 || errno == EEXIST;
    }

    FileHandle TkFileSystem::openFile(OpenFileParams &params)
    {
        i32 flags = O_CLOEXEC;
        const auto access = static_cast<u32>(params.access);
        if (access == static_cast<u32>(FileAccess::kRead))
            flags |= O_RDONLY;
        else if (access == static_cast<u32>(FileAccess::kWrite))
            flags |= O_WRONLY;
        else
            flags |= O_RDWR;

        switch (params.mode)
        {
        case FileMode::kCreateAlways:
            flags |= O_CREAT | O_TRUNC;
            break;
        case FileMode::kCreateNew:
            flags |= O_CREAT | O_EXCL;
            break;
        case FileMode::kOpenAlways:
            flags |= O_CREAT;
            break;
        case FileMode::kTruncateExisting:
            flags |= O_TRUNC;
            break;
        case FileMode::kOpenExisting:
        default:
            break;
        }

        const i32 fd = ::open(params.path.get(), flags, 0644);
        if (fd == -1)
        {
            LogError("[FileSys] open failed path=%s errno=%i", params.path.get(), errno);
            return nullptr;
        }
        return reinterpret_cast<FileHandle>(static_cast<intptr>(fd + 1));
    }

    static i32 toFd(FileHandle handle)
    {
        return static_cast<i32>(reinterpret_cast<intptr>(handle) - 1);
    }

    void TkFileSystem::closeFile(FileHandle handle)
    {
        if (handle)
            ::close(toFd(handle));
    }

    bool TkFileSystem::readFile(FileHandle handle, void *buffer, u32 bytesToRead, u32 *bytesRead)
    {
        const ssize_t result = ::read(toFd(handle), buffer, bytesToRead);
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
        const ssize_t result = ::write(toFd(handle), buffer, bytesToWrite);
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
        struct stat info{};
        return ::fstat(toFd(handle), &info) == 0 ? static_cast<u32>(info.st_size) : 0;
    }

    TkDateTime TkFileSystem::getFileLastWrite(FileHandle handle)
    {
        struct stat info{};
        return ::fstat(toFd(handle), &info) == 0 ? dateTimeFromUnixTime(info.st_mtime) : TkDateTime::minValue();
    }

    u32 TkFileSystem::getFilePos(FileHandle handle)
    {
        return static_cast<u32>(::lseek(toFd(handle), 0, SEEK_CUR));
    }

    void TkFileSystem::setFilePos(FileHandle handle, u32 seek)
    {
        ::lseek(toFd(handle), seek, SEEK_SET);
    }

    bool TkFileSystem::isFileOpened(FileHandle handle)
    {
        return handle != nullptr;
    }
}

#endif
