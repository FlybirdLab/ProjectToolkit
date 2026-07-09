/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

#if TK_ANDROID

#include <toolkit/base/Function.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/types/DateTime.hpp>
#include <toolkit/types/Path.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/TimeSpan.hpp>
#include <toolkit/utils/StringUtils.hpp>

#include <android/asset_manager.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct AAssetManager AAssetManager;

extern "C" TK_API void TkAndroidSetAssetManager(AAssetManager *manager);

namespace toolkit
{
    void setAndroidAssetManager(AAssetManager *manager);
    AAssetManager *getAndroidAssetManager();

    enum class AndroidFileHandleType : u8
    {
        kUnix,
        kAsset,
    };

    struct AndroidFileHandle
    {
        AndroidFileHandleType type = AndroidFileHandleType::kUnix;
        i32 fd = -1;
        AAsset *asset = nullptr;
    };

    AAssetManager *s_assetManager = nullptr;

    void setAndroidAssetManager(AAssetManager *manager)
    {
        s_assetManager = manager;
    }

    AAssetManager *getAndroidAssetManager()
    {
        return s_assetManager;
    }

    TkDateTime dateTimeFromUnixTime(time_t value)
    {
        return TkDateTime(1970, 1, 1) + TkTimeSpan(0, 0, 0, static_cast<i32>(value));
    }

    TkString toString(TkStringView path)
    {
        return TkString(path);
    }

    AndroidFileHandle *toHandle(FileHandle handle)
    {
        return reinterpret_cast<AndroidFileHandle *>(handle);
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

    bool normalizeAssetPath(TkStringView path, TkString &result)
    {
        result = path;
        TkPath::normalize(result);
        while (result.startsWith("./"))
            result = result.substring(2);
        while (result.startsWith('/'))
            result = result.substring(1);
        return result.hasChars();
    }

    AAsset *openAsset(TkStringView path, int mode)
    {
        if (s_assetManager == nullptr)
            return nullptr;

        TkString assetPath;
        if (!normalizeAssetPath(path, assetPath))
            return nullptr;

        return AAssetManager_open(s_assetManager, assetPath.getText(), mode);
    }

    bool assetExists(TkStringView path)
    {
        AAsset *asset = openAsset(path, AASSET_MODE_UNKNOWN);
        if (asset == nullptr)
            return false;
        AAsset_close(asset);
        return true;
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
        FileHandle srcHandle = nullptr;
        OpenFileParams srcParams{.path = src, .mode = FileMode::kOpenExisting, .access = FileAccess::kRead, .share = FileShare::kRead};
        srcHandle = TkFileSystem::openFile(srcParams);
        if (srcHandle == nullptr)
            return false;

        const i32 dstFd = ::open(dst.getText(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
        if (dstFd == -1)
        {
            TkFileSystem::closeFile(srcHandle);
            return false;
        }

        bool success = true;
        byte buffer[64 * 1024];
        for (;;)
        {
            u32 readBytes = 0;
            if (!TkFileSystem::readFile(srcHandle, buffer, sizeof(buffer), &readBytes))
            {
                success = false;
                break;
            }
            if (readBytes == 0)
                break;
            if (!writeAll(dstFd, buffer, static_cast<ssize_t>(readBytes)))
            {
                success = false;
                break;
            }
        }

        if (::close(dstFd) != 0)
            success = false;
        TkFileSystem::closeFile(srcHandle);
        return success;
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
        return (statPath(path, info) && isRegularFileMode(info.st_mode)) || assetExists(path);
    }

    psize TkFileSystem::getFileSize(const TkStringView path)
    {
        struct stat info{};
        if (statPath(path, info))
            return isDirectoryMode(info.st_mode) ? 0 : static_cast<psize>(info.st_size);

        AAsset *asset = openAsset(path, AASSET_MODE_UNKNOWN);
        if (asset == nullptr)
            return 0;
        const psize size = static_cast<psize>(AAsset_getLength64(asset));
        AAsset_close(asset);
        return size;
    }

    bool TkFileSystem::isReadOnly(const TkStringView path)
    {
        const TkString text(path);
        struct stat info{};
        if (::stat(text.getText(), &info) == 0)
            return ::access(text.getText(), W_OK) == -1 && (errno == EACCES || errno == EROFS);
        return assetExists(path);
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
        const char *externalEnv = ::getenv("EXTERNAL_STORAGE");
        const char *dataEnv = ::getenv("ANDROID_DATA");
        const TkString home = homeEnv && *homeEnv ? homeEnv : "/data/local/tmp";
        const TkString external = externalEnv && *externalEnv ? externalEnv : home;
        const TkString data = dataEnv && *dataEnv ? dataEnv : "/data/data";

        switch (type)
        {
        case SpecialFolder::kDesktop:
        case SpecialFolder::kDocuments:
            result = external / "Documents";
            break;
        case SpecialFolder::kPictures:
            result = external / "Pictures";
            break;
        case SpecialFolder::kAppData:
        case SpecialFolder::kLocalAppData:
        case SpecialFolder::kProgramData:
            result = data;
            break;
        case SpecialFolder::kTemporary:
        {
            const char *tmp = ::getenv("TMPDIR");
            result = tmp && *tmp ? tmp : "/data/local/tmp";
            break;
        }
        }

        TkPath::normalize(result);
    }

    FileHandle TkFileSystem::openFile(OpenFileParams &params)
    {
        const bool wantsWrite = (static_cast<u32>(params.access) & static_cast<u32>(FileAccess::kWrite)) != 0;
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

        const i32 fd = ::open(params.path.getText(), flags, 0644);
        if (fd != -1)
        {
            auto *handle = TkMemory::newObject<AndroidFileHandle>(TkMemory::getDefault());
            handle->type = AndroidFileHandleType::kUnix;
            handle->fd = fd;
            return reinterpret_cast<FileHandle>(handle);
        }

        if (!wantsWrite && params.mode == FileMode::kOpenExisting)
        {
            AAsset *asset = openAsset(params.path, AASSET_MODE_RANDOM);
            if (asset != nullptr)
            {
                auto *handle = TkMemory::newObject<AndroidFileHandle>(TkMemory::getDefault());
                handle->type = AndroidFileHandleType::kAsset;
                handle->asset = asset;
                return reinterpret_cast<FileHandle>(handle);
            }
        }

        LogError("[FileSys] open failed path=%s errno=%i", params.path.getText(), errno);
        return nullptr;
    }

    void TkFileSystem::closeFile(FileHandle handle)
    {
        AndroidFileHandle *file = toHandle(handle);
        if (!file)
            return;

        if (file->type == AndroidFileHandleType::kAsset)
        {
            if (file->asset)
                AAsset_close(file->asset);
        }
        else if (file->fd >= 0)
        {
            ::close(file->fd);
        }
        TkMemory::deletePtr(TkMemory::getDefault(), file);
    }

    bool TkFileSystem::readFile(FileHandle handle, void *buffer, u32 bytesToRead, u32 *bytesRead)
    {
        AndroidFileHandle *file = toHandle(handle);
        if (!file)
        {
            if (bytesRead)
                *bytesRead = 0;
            return false;
        }

        ssize_t result = 0;
        do
        {
            result = file->type == AndroidFileHandleType::kAsset
                         ? AAsset_read(file->asset, buffer, bytesToRead)
                         : ::read(file->fd, buffer, bytesToRead);
        } while (result < 0 && errno == EINTR);

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
        AndroidFileHandle *file = toHandle(handle);
        if (!file || file->type == AndroidFileHandleType::kAsset)
        {
            if (bytesWritten)
                *bytesWritten = 0;
            return false;
        }

        ssize_t result = 0;
        do
        {
            result = ::write(file->fd, buffer, bytesToWrite);
        } while (result < 0 && errno == EINTR);

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
        AndroidFileHandle *file = toHandle(handle);
        if (!file)
            return 0;
        if (file->type == AndroidFileHandleType::kAsset)
            return static_cast<u32>(AAsset_getLength64(file->asset));

        struct stat info{};
        return ::fstat(file->fd, &info) == 0 ? static_cast<u32>(info.st_size) : 0;
    }

    TkDateTime TkFileSystem::getFileLastWrite(FileHandle handle)
    {
        AndroidFileHandle *file = toHandle(handle);
        if (!file || file->type == AndroidFileHandleType::kAsset)
            return TkDateTime::minValue();

        struct stat info{};
        return ::fstat(file->fd, &info) == 0 ? dateTimeFromUnixTime(info.st_mtime) : TkDateTime::minValue();
    }

    u32 TkFileSystem::getFilePos(FileHandle handle)
    {
        AndroidFileHandle *file = toHandle(handle);
        if (!file)
            return 0;
        if (file->type == AndroidFileHandleType::kAsset)
            return static_cast<u32>(AAsset_seek64(file->asset, 0, SEEK_CUR));
        return static_cast<u32>(::lseek(file->fd, 0, SEEK_CUR));
    }

    void TkFileSystem::setFilePos(FileHandle handle, u32 seek)
    {
        AndroidFileHandle *file = toHandle(handle);
        if (!file)
            return;
        if (file->type == AndroidFileHandleType::kAsset)
            AAsset_seek64(file->asset, static_cast<off64_t>(seek), SEEK_SET);
        else
            ::lseek(file->fd, seek, SEEK_SET);
    }

    bool TkFileSystem::isFileOpened(FileHandle handle)
    {
        AndroidFileHandle *file = toHandle(handle);
        return file != nullptr && (file->type == AndroidFileHandleType::kAsset ? file->asset != nullptr : file->fd >= 0);
    }

    FileWatcherHandle TkFileSystem::openFileWatcher(const TkString &directory, bool withSubDirs)
    {
        (void)directory;
        (void)withSubDirs;
        return nullptr;
    }

    void TkFileSystem::closeFileWatcher(FileWatcherHandle handle)
    {
        (void)handle;
    }

    TkAction<const TkString &, FileAction> &TkFileSystem::getEvent(FileWatcherHandle handle)
    {
        (void)handle;
        static TkAction<const TkString &, FileAction> emptyEvent;
        return emptyEvent;
    }

    bool TkFileSystem::isEnabled(FileWatcherHandle handle)
    {
        (void)handle;
        return false;
    }
}

extern "C" TK_API void TkAndroidSetAssetManager(AAssetManager *manager)
{
    toolkit::setAndroidAssetManager(manager);
}

#endif
