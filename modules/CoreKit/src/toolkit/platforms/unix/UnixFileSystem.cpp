/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/platforms/FileSystem.hpp>

#if TK_POSIX && !TK_ANDROID

#include <toolkit/debug/Logger.hpp>
#include <toolkit/types/DateTime.hpp>
#include <toolkit/types/TimeSpan.hpp>

#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace toolkit
{
    static TkDateTime dateTimeFromUnixTime(time_t value)
    {
        return TkDateTime(1970, 1, 1) + TkTimeSpan(0, 0, 0, static_cast<i32>(value));
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
