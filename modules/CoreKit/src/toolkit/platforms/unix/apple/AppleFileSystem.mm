/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "AppleFileSystem.h"

#if TK_APPLE

#include <toolkit/collection/Array.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/types/DateTime.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/TimeSpan.hpp>
#include <toolkit/utils/StringUtils.hpp>

#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace toolkit
{
    namespace
    {
        TkDateTime dateTimeFromUnixTime(time_t value)
        {
            return TkDateTime(1970, 1, 1) + TkTimeSpan(0, 0, 0, static_cast<i32>(value));
        }

        bool isDirectoryPath(const char* path)
        {
            struct stat info{};
            return path && ::stat(path, &info) == 0 && S_ISDIR(info.st_mode);
        }

        bool isFilePath(const char* path)
        {
            struct stat info{};
            return path && ::stat(path, &info) == 0 && S_ISREG(info.st_mode);
        }

        bool deletePathTree(const char* path)
        {
            DIR* dir = ::opendir(path);
            if (!dir)
                return false;

            bool ok = true;
            dirent* entry = nullptr;
            while ((entry = ::readdir(dir)) != nullptr)
            {
                if (::strcmp(entry->d_name, ".") == 0 || ::strcmp(entry->d_name, "..") == 0)
                    continue;

                char child[PATH_MAX];
                ::snprintf(child, sizeof(child), "%s/%s", path, entry->d_name);

                struct stat info{};
                if (::lstat(child, &info) != 0)
                {
                    ok = false;
                    break;
                }

                if (S_ISDIR(info.st_mode))
                    ok = deletePathTree(child);
                else
                    ok = ::unlink(child) == 0;
                if (!ok)
                    break;
            }

            ::closedir(dir);
            return ok && ::rmdir(path) == 0;
        }

        bool matchesPattern(const char* name, const char* pattern)
        {
            if (!pattern || pattern[0] == 0 || ::strcmp(pattern, "*") == 0)
                return true;

            const i32 nameLength = TkStringUtils::length(name);
            const i32 patternLength = TkStringUtils::length(pattern);
            if (pattern[0] == '*' && patternLength > 1)
            {
                const i32 suffixLength = patternLength - 1;
                return nameLength >= suffixLength && TkStringUtils::compareStrings(name + nameLength - suffixLength, pattern + 1, suffixLength) == 0;
            }
            return TkStringUtils::compareStrings(name, pattern) == 0;
        }

        bool listFiles(TkArray<TkString>& results, const char* path, const char* pattern, bool recursive)
        {
            DIR* dir = ::opendir(path);
            if (!dir)
                return false;

            bool ok = true;
            dirent* entry = nullptr;
            while ((entry = ::readdir(dir)) != nullptr)
            {
                if (::strcmp(entry->d_name, ".") == 0 || ::strcmp(entry->d_name, "..") == 0)
                    continue;

                char child[PATH_MAX];
                ::snprintf(child, sizeof(child), "%s/%s", path, entry->d_name);

                struct stat info{};
                if (::lstat(child, &info) != 0)
                {
                    ok = false;
                    break;
                }

                if (S_ISREG(info.st_mode) && matchesPattern(entry->d_name, pattern))
                    results.add(TkString(child));
                else if (recursive && S_ISDIR(info.st_mode))
                    ok = listFiles(results, child, pattern, recursive);

                if (!ok)
                    break;
            }

            ::closedir(dir);
            return ok;
        }
    }

    bool TkFileSystem::makeDir(const TkStringView path)
    {
        const TkString pathString(path);
        if (pathString.isEmpty())
            return false;
        if (isDirectoryPath(pathString.getText()))
            return true;

        const i32 slashIndex = pathString.findLast('/');
        if (slashIndex > 0)
        {
            const TkString parent = pathString.left(slashIndex);
            if (parent.hasChars() && !isDirectoryPath(parent.getText()) && !makeDir(parent))
                return false;
        }

        return ::mkdir(pathString.getText(), 0755) == 0 || errno == EEXIST;
    }

    bool TkFileSystem::deleteDir(const TkStringView path, bool deleteContents)
    {
        const TkString pathString(path);
        if (!isDirectoryPath(pathString.getText()))
            return false;
        return deleteContents ? deletePathTree(pathString.getText()) : ::rmdir(pathString.getText()) == 0;
    }

    bool TkFileSystem::isDirExist(const TkStringView path)
    {
        return isDirectoryPath(TkString(path).getText());
    }

    bool TkFileSystem::listDir(TkArray<TkString>& results, const TkString& path, const char* searchPattern, bool recursive)
    {
        return listFiles(results, path.getText(), searchPattern, recursive);
    }

    bool TkFileSystem::listChildren(TkArray<TkString>& results, const TkString& path)
    {
        DIR* dir = ::opendir(path.getText());
        if (!dir)
            return false;

        bool ok = true;
        dirent* entry = nullptr;
        while ((entry = ::readdir(dir)) != nullptr)
        {
            if (::strcmp(entry->d_name, ".") == 0 || ::strcmp(entry->d_name, "..") == 0)
                continue;

            char child[PATH_MAX];
            ::snprintf(child, sizeof(child), "%s/%s", path.getText(), entry->d_name);
            if (isDirectoryPath(child))
                results.add(TkString(child));
        }

        ::closedir(dir);
        return ok;
    }

    bool TkFileSystem::copyDir(const TkStringView dst, const TkStringView src, bool recursive)
    {
        const TkString dstString(dst);
        const TkString srcString(src);
        if (!isDirectoryPath(srcString.getText()))
            return false;
        if (!makeDir(dst))
            return false;

        DIR* dir = ::opendir(srcString.getText());
        if (!dir)
            return false;

        bool ok = true;
        dirent* entry = nullptr;
        while ((entry = ::readdir(dir)) != nullptr)
        {
            if (::strcmp(entry->d_name, ".") == 0 || ::strcmp(entry->d_name, "..") == 0)
                continue;

            const TkString srcPath = srcString / entry->d_name;
            const TkString dstPath = dstString / entry->d_name;
            struct stat info{};
            if (::lstat(srcPath.getText(), &info) != 0)
            {
                ok = false;
                break;
            }

            if (S_ISDIR(info.st_mode))
            {
                if (recursive)
                    ok = copyDir(dstPath, srcPath, true);
            }
            else if (S_ISREG(info.st_mode))
            {
                ok = copyFile(dstPath, srcPath);
            }

            if (!ok)
                break;
        }

        ::closedir(dir);
        return ok;
    }

    bool TkFileSystem::deleteFile(const TkStringView path)
    {
        return ::unlink(TkString(path).getText()) == 0;
    }

    bool TkFileSystem::isFileExist(const TkStringView path)
    {
        return isFilePath(TkString(path).getText());
    }

    psize TkFileSystem::getFileSize(const TkStringView path)
    {
        struct stat info{};
        const TkString pathString(path);
        if (::stat(pathString.getText(), &info) != 0 || S_ISDIR(info.st_mode))
            return 0;
        return static_cast<psize>(info.st_size);
    }

    bool TkFileSystem::isReadOnly(const TkStringView path)
    {
        const TkString pathString(path);
        if (::access(pathString.getText(), W_OK) == -1)
            return errno == EACCES;
        return false;
    }

    bool TkFileSystem::setReadOnly(const TkStringView path, bool isReadOnly)
    {
        const TkString pathString(path);
        struct stat info{};
        if (::stat(pathString.getText(), &info) != 0)
            return false;
        if (isReadOnly)
            info.st_mode &= ~S_IWUSR;
        else
            info.st_mode |= S_IWUSR;
        return ::chmod(pathString.getText(), info.st_mode) == 0;
    }

    bool TkFileSystem::moveFile(const TkStringView dst, const TkStringView src, bool overwrite)
    {
        const TkString dstString(dst);
        const TkString srcString(src);
        if (!overwrite && isFilePath(dstString.getText()))
            return false;
        if (overwrite)
            ::unlink(dstString.getText());
        if (::rename(srcString.getText(), dstString.getText()) == 0)
            return true;

        if (errno == EXDEV && copyFile(dst, src))
        {
            ::unlink(srcString.getText());
            return true;
        }
        return false;
    }

    bool TkFileSystem::copyFile(const TkStringView dst, const TkStringView src)
    {
        const TkString dstString(dst);
        const TkString srcString(src);
        const i32 srcFile = ::open(srcString.getText(), O_RDONLY);
        if (srcFile < 0)
            return false;

        i32 dstFile = ::open(dstString.getText(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (dstFile < 0)
        {
            ::close(srcFile);
            return false;
        }

        bool ok = true;
        char buffer[4096];
        ssize_t readSize = 0;
        while ((readSize = ::read(srcFile, buffer, sizeof(buffer))) > 0)
        {
            char* ptr = buffer;
            ssize_t remaining = readSize;
            while (remaining > 0)
            {
                const ssize_t written = ::write(dstFile, ptr, static_cast<size_t>(remaining));
                if (written < 0)
                {
                    if (errno == EINTR)
                        continue;
                    ok = false;
                    break;
                }
                remaining -= written;
                ptr += written;
            }
            if (!ok)
                break;
        }
        if (readSize < 0)
            ok = false;

        if (::close(dstFile) != 0)
            ok = false;
        ::close(srcFile);
        return ok;
    }

    TkDateTime TkFileSystem::getLastEdit(const TkStringView path)
    {
        struct stat info{};
        const TkString pathString(path);
        if (::stat(pathString.getText(), &info) != 0)
            return TkDateTime::minValue();
        return dateTimeFromUnixTime(info.st_mtime);
    }

    void TkFileSystem::getDirPath(const SpecialFolder type, TkString& result)
    {
        const char* homeEnv = ::getenv("HOME");
        const TkString home = homeEnv && *homeEnv ? TkString(homeEnv) : TkString("/");
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
        case SpecialFolder::kLocalAppData:
            result = home / "Library/Caches";
            break;
        case SpecialFolder::kProgramData:
            result = home / "Library/Application Support";
            break;
        case SpecialFolder::kTemporary:
        default:
            result = System::getTempDir();
            break;
        }
    }

    void TkFileSystem::getTempDirPath(TkString& tmpPath)
    {
        tmpPath = System::getTempDir();
    }

#if !TK_MACOS
    FileWatcherHandle TkFileSystem::openFileWatcher(const TkString& directory, bool withSubDirs)
    {
        (void)directory;
        (void)withSubDirs;
        return kInvalidFileWatcherHandle;
    }

    void TkFileSystem::closeFileWatcher(FileWatcherHandle)
    {
    }

    TkAction<const TkString&, FileAction>& TkFileSystem::getEvent(FileWatcherHandle)
    {
        static TkAction<const TkString&, FileAction> empty;
        return empty;
    }

    bool TkFileSystem::isEnabled(FileWatcherHandle)
    {
        return false;
    }
#endif
}

#endif
