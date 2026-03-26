/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "toolkit/threading/Thread.hpp"
#include <toolkit/platforms/FileSystem.hpp>

#if TK_WINDOWS

#include "Win32System.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/utils/Convertor.hpp"
#include "toolkit/base/Math.hpp"
#include "toolkit/types/DateTime.hpp"
#include "toolkit/types/Path.hpp"

#define NOMINMAX
#include <Windows.h>
#include <ShlObj.h>
#include <ShellAPI.h>
#include <KnownFolders.h>
#include "Win32FileSystem.hpp"

namespace toolkit
{

    const TkDateTime WindowsEpoch(1970, 1, 1);

#define WIN32_INIT_BUFFER(path, buffer)     \
    StringAsUTF16 u16_##buffer{path.get()}; \
    StringAsTerminated<> buffer(u16_##buffer.get(), path.length())

    FileHandle TkFileSystem::openFile(OpenFileParams &params)
    {
        // Note: FileMode, FileAccess and FileShare enums are mapping directly to the Win32 types
        StringAsUTF16<MAX_PATH> pathU16{params.path.get()};
        auto handle = ::CreateFileW(pathU16.get(), (DWORD)params.access, (DWORD)params.share, nullptr, (DWORD)params.mode, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (handle == INVALID_HANDLE_VALUE)
        {
            LogError("[FileSys] Error:%s", Win32System::getLastErrMsg());
            return nullptr;
        }

        return (void *)handle;
    }

    void TkFileSystem::closeFile(FileHandle handle)
    {
        if (!handle)
            return;
        ::CloseHandle(handle);
        handle = nullptr;
    }

    bool TkFileSystem::readFile(FileHandle handle, void *buffer, u32 bytesToRead, u32 *bytesRead)
    {
        DWORD tmp;
        if (::ReadFile(handle, buffer, bytesToRead, &tmp, nullptr))
        {
            if (bytesRead)
                *bytesRead = tmp;
            return true;
        }

        if (bytesRead)
            *bytesRead = 0;
        return false;
    }

    bool TkFileSystem::writeFile(FileHandle handle, const void *buffer, u32 bytesToWrite, u32 *bytesWritten)
    {
        DWORD tmp;
        if (::WriteFile(handle, buffer, bytesToWrite, &tmp, nullptr))
        {
            if (bytesWritten)
                *bytesWritten = tmp;
            return true;
        }

        if (bytesWritten)
            *bytesWritten = 0;
        return false;
    }

    u32 TkFileSystem::getFileSize(FileHandle handle)
    {
        ::LARGE_INTEGER result;
        ::GetFileSizeEx(handle, &result);
        return (u32)result.QuadPart;
    }

    TkDateTime TkFileSystem::getFileLastWrite(FileHandle handle)
    {
        ::FILETIME lpLastWriteTime;
        GetFileTime(handle, nullptr, nullptr, &lpLastWriteTime);

        ::SYSTEMTIME lpSystemTime;
        FileTimeToSystemTime(&lpLastWriteTime, &lpSystemTime);

        return TkDateTime{lpSystemTime.wYear, lpSystemTime.wMonth, lpSystemTime.wDay, lpSystemTime.wHour, lpSystemTime.wMinute, lpSystemTime.wSecond, lpSystemTime.wMilliseconds};
    }

    u32 TkFileSystem::getFilePos(FileHandle handle)
    {
        return SetFilePointer(handle, 0, nullptr, FILE_CURRENT);
    }

    void TkFileSystem::setFilePos(FileHandle handle, u32 seek)
    {
        SetFilePointer(handle, seek, nullptr, FILE_BEGIN);
    }

    bool TkFileSystem::isFileOpened(FileHandle handle)
    {
        return handle != nullptr;
    }

    bool TkFileSystem::makeDir(const TkStringView path)
    {
        StringAsUTF16<MAX_PATH> pathU16{path.get()};
        StringAsTerminated<> buffer(pathU16.get(), pathU16.length());
        const DWORD fileAttributes = GetFileAttributesW(buffer);
        if (fileAttributes != INVALID_FILE_ATTRIBUTES)
            return (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 || (fileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;

        const int result = SHCreateDirectoryExW(nullptr, buffer, nullptr);
        return result == ERROR_SUCCESS || result == ERROR_FILE_EXISTS || result == ERROR_ALREADY_EXISTS;
    }

    bool TkFileSystem::deleteDir(const TkStringView path, bool deleteContents)
    {
        // If need to delete contents first
        if (deleteContents)
        {
            TkString pattern = {path};
            pattern = pattern / '*';

            WIN32_FIND_DATAW info;
            StringAsUTF16<MAX_PATH> u16Pattern{pattern.get()};
            HANDLE hFind = FindFirstFileW(u16Pattern.get(), &info);

            if (hFind != INVALID_HANDLE_VALUE)
            {
                do
                {
                    StringAsUTF8<MAX_PATH> u8Name{info.cFileName};

                    if (TkStringUtils::compareIStrings(u8Name.get(), ".") == 0 ||
                        TkStringUtils::compareIStrings(u8Name.get(), "..") == 0)
                    {
                        continue;
                    }

                    TkString childPath = {path};
                    childPath = childPath / u8Name.get();

                    if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        // Recursively delete sub directory
                        if (!deleteDir(*childPath, true))
                        {
                            FindClose(hFind);
                            return false;
                        }
                    }
                    else
                    {
                        StringAsUTF16<MAX_PATH> u16Child{childPath.get()};
                        if (!DeleteFileW(u16Child.get()))
                        {
                            FindClose(hFind);
                            return false;
                        }
                    }
                } while (FindNextFileW(hFind, &info));

                DWORD err = GetLastError();
                FindClose(hFind);

                if (err != ERROR_NO_MORE_FILES)
                    return false;
            }
            else
            {
                // Directory does not exist → failure
                if (GetLastError() != ERROR_FILE_NOT_FOUND)
                    return false;
            }
        }

        // Remove the directory itself
        StringAsUTF16<MAX_PATH> u16Path{path.get()};
        if (!RemoveDirectoryW(u16Path.get()))
            return false;

        return true;
    }

    bool TkFileSystem::isDirExist(const TkStringView path)
    {
        WIN32_INIT_BUFFER(path, buffer);
        const DWORD attr = ::GetFileAttributesW(buffer);
        if (attr == INVALID_FILE_ATTRIBUTES)
            return false;
        return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    static bool getFilesFromDirectoryTop(
        TkArray<TkString> &results,
        const TkString &directory,
        const char *searchPattern)
    {
        WIN32_FIND_DATAW info;
        TkString pattern = directory / searchPattern;

        StringAsUTF16<MAX_PATH> patterU16{pattern.get()};
        HANDLE hFind = ::FindFirstFileW(patterU16.get(), &info);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            DWORD err = ::GetLastError();
            return err == ERROR_FILE_NOT_FOUND;
        }

        do
        {
            StringAsUTF8<MAX_PATH> cFileName{info.cFileName};
            if (TkStringUtils::compareIStrings(cFileName.get(), ".") == 0 ||
                TkStringUtils::compareIStrings(cFileName.get(), "..") == 0)
                continue;

            if ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                results.add(directory / cFileName.get());
            }
        } while (::FindNextFileW(hFind, &info));

        DWORD err = ::GetLastError();
        ::FindClose(hFind);

        return err == ERROR_NO_MORE_FILES;
    }

    static bool getFilesFromDirectoryAll(
        TkArray<TkString> &results,
        const TkString &directory,
        const char *searchPattern)
    {
        if (!getFilesFromDirectoryTop(results, directory, searchPattern))
            return false;

        WIN32_FIND_DATAW info;
        TkString pattern = directory / '*';

        StringAsUTF16<MAX_PATH> patterU16{pattern.get()};
        HANDLE hFind = ::FindFirstFileW(patterU16.get(), &info);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            DWORD err = ::GetLastError();
            return err == ERROR_FILE_NOT_FOUND;
        }

        do
        {
            StringAsUTF8<MAX_PATH> cFileName{info.cFileName};
            if (TkStringUtils::compareIStrings(cFileName.get(), ".") == 0 ||
                TkStringUtils::compareIStrings(cFileName.get(), "..") == 0)
                continue;

            if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                TkString subDir = directory / cFileName.get();
                if (!getFilesFromDirectoryAll(results, subDir, searchPattern))
                {
                    ::FindClose(hFind);
                    return false;
                }
            }
        } while (::FindNextFileW(hFind, &info));

        DWORD err = ::GetLastError();
        ::FindClose(hFind);

        return err == ERROR_NO_MORE_FILES;
    }

    bool TkFileSystem::listDir(TkArray<TkString> &results, const TkString &path, const char *searchPattern, bool recursive)
    {
        if (!recursive)
            return getFilesFromDirectoryTop(results, path, searchPattern);
        return getFilesFromDirectoryAll(results, path, searchPattern);
    }

    bool TkFileSystem::listChildren(TkArray<TkString> &results, const TkString &path)
    {
        WIN32_FIND_DATAW info;
        TkString pattern = path / '*';

        StringAsUTF16<MAX_PATH> patterU16{pattern.get()};
        HANDLE handle = ::FindFirstFileW(patterU16.get(), &info);
        if (handle == INVALID_HANDLE_VALUE)
        {
            DWORD err = ::GetLastError();
            return err == ERROR_FILE_NOT_FOUND;
        }

        do
        {
            StringAsUTF8<MAX_PATH> cFileName{info.cFileName};
            if (TkStringUtils::compareIStrings(cFileName.get(), ".") == 0 ||
                TkStringUtils::compareIStrings(cFileName.get(), "..") == 0)
                continue;

            if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                results.add(path / cFileName.get());
            }
        } while (::FindNextFileW(handle, &info));

        DWORD err = ::GetLastError();
        ::FindClose(handle);

        return err == ERROR_NO_MORE_FILES;
    }

    bool TkFileSystem::deleteFile(const TkStringView path)
    {
        WIN32_INIT_BUFFER(path, buffer);
        return ::DeleteFileW(buffer) != 0;
    }

    bool TkFileSystem::isFileExist(const TkStringView path)
    {
        WIN32_INIT_BUFFER(path, buffer);
        DWORD attr = ::GetFileAttributesW(buffer);

        if (attr == INVALID_FILE_ATTRIBUTES)
            return false;

        return (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    psize TkFileSystem::getFileSize(const TkStringView path)
    {
        WIN32_INIT_BUFFER(path, buffer);
        WIN32_FILE_ATTRIBUTE_DATA info;
        if (!!GetFileAttributesExW(buffer, GetFileExInfoStandard, &info))
        {
            if ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                ::LARGE_INTEGER li;
                li.HighPart = info.nFileSizeHigh;
                li.LowPart = info.nFileSizeLow;
                return li.QuadPart;
            }
        }
        return 0;
    }

    bool TkFileSystem::isReadOnly(const TkStringView path)
    {
        WIN32_INIT_BUFFER(path, buffer);
        DWORD attr = ::GetFileAttributesW(buffer);

        if (attr == INVALID_FILE_ATTRIBUTES)
            return false;

        return (attr & FILE_ATTRIBUTE_READONLY) != 0;
    }

    bool TkFileSystem::setReadOnly(const TkStringView path, bool isReadOnly)
    {
        WIN32_INIT_BUFFER(path, buffer);

        DWORD attr = isReadOnly
                         ? FILE_ATTRIBUTE_READONLY
                         : FILE_ATTRIBUTE_NORMAL;

        return ::SetFileAttributesW(buffer, attr) != 0;
    }

    bool TkFileSystem::moveFile(const TkStringView dst, const TkStringView src, bool overwrite)
    {
        DWORD flags = MOVEFILE_COPY_ALLOWED |
                      (overwrite ? MOVEFILE_REPLACE_EXISTING : 0);

        WIN32_INIT_BUFFER(dst, bufferDst);
        WIN32_INIT_BUFFER(src, bufferSrc);

        if (dst.length() == src.length() &&
            TkStringUtils::compareIStrings(*dst, *src) == 0)
        {
            TkString tmp;
            getTempDirPath(tmp);
            StringAsUTF16<MAX_PATH> tmpU16{tmp.get()};

            if (!::MoveFileExW(bufferSrc, tmpU16.get(), MOVEFILE_REPLACE_EXISTING))
                return false;

            return ::MoveFileExW(tmpU16.get(), bufferDst, flags) != 0;
        }

        return ::MoveFileExW(bufferSrc, bufferDst, flags) != 0;
    }

    bool TkFileSystem::copyFile(const TkStringView dst, const TkStringView src)
    {
        WIN32_INIT_BUFFER(dst, bufferDst);
        WIN32_INIT_BUFFER(src, bufferSrc);

        return ::CopyFileW(bufferSrc, bufferDst, FALSE) != 0;
    }

    bool TkFileSystem::copyDir(const TkStringView dst, const TkStringView src, bool recursive)
    {
        if (!isDirExist(src))
        {
            LogError("[FileSys] copyDir source does not exist. src=%s", src.get());
            return false;
        }

        if (TkPath::isSame(dst, src))
        {
            LogError("[FileSys] copyDir source and destination are the same. path=%s", src.get());
            return false;
        }

        if (!isDirExist(dst))
        {
            if (!TkFileSystem::makeDir(dst))
            {
                LogError("[FileSys] copyDir failed to create destination directory. dst=%s", dst.get());
                return false;
            }
        }

        ::WIN32_FIND_DATAW info;
        TkString pattern = TkString(src) / '*';
        StringAsUTF16<MAX_PATH> patternU16{pattern.get()};

        HANDLE hFind = ::FindFirstFileW(patternU16.get(), &info);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            DWORD err = ::GetLastError();
            return err == ERROR_FILE_NOT_FOUND;
        }

        do
        {
            StringAsUTF8<MAX_PATH> cFileName{info.cFileName};
            if (TkStringUtils::compareStrings(cFileName.get(), ".") == 0 ||
                TkStringUtils::compareStrings(cFileName.get(), "..") == 0)
                continue;

            TkString srcPath = TkString(src) / cFileName.get();
            TkString dstPath = TkString(dst) / cFileName.get();

            if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (recursive)
                {
                    if (!copyDir(dstPath, srcPath, true))
                    {
                        LogError("[FileSys] copyDir failed to copy child directory. src=%s dst=%s", srcPath.get(), dstPath.get());
                        ::FindClose(hFind);
                        return false;
                    }
                }
            }
            else
            {
                if (!copyFile(dstPath, srcPath))
                {
                    LogError("[FileSys] copyDir failed to copy file. src=%s dst=%s err=%s", srcPath.get(), dstPath.get(), Win32System::getLastErrMsg());
                    ::FindClose(hFind);
                    return false;
                }
            }
        } while (::FindNextFileW(hFind, &info));

        DWORD err = ::GetLastError();
        ::FindClose(hFind);

        return err == ERROR_NO_MORE_FILES;
    }

    TkDateTime TkFileSystem::getLastEdit(const TkStringView path)
    {
        TkDateTime result = TkDateTime::minValue();
        WIN32_INIT_BUFFER(path, buffer);
        ::WIN32_FILE_ATTRIBUTE_DATA data;
        if (!!GetFileAttributesExW(buffer, GetFileExInfoStandard, &data))
        {
            SYSTEMTIME lpSystemTime;
            FileTimeToSystemTime(&data.ftLastWriteTime, &lpSystemTime);
            result = TkDateTime(lpSystemTime.wYear, lpSystemTime.wMonth, lpSystemTime.wDay, lpSystemTime.wHour, lpSystemTime.wMinute, lpSystemTime.wSecond, lpSystemTime.wMilliseconds);
        }
        return result;
    }

    // Convert the UNIX style line endings into DOS style (from \n into \r\n)
    static void convertLineEndingsToDos(const TkStringView t1, TkArray<wchar> &output)
    {
        // Prepare output (add some space for \r characters, just guess ~1%)
        StringAsUTF16<MAX_PATH> text{t1.get()};
        output.clear();
        output.ensureCapacity(TkMath::ceilToInt((float)text.length() * 1.01f));
        // Perform conversion
        auto readPtr = text.get();
        for (i32 i = 0; i < text.length(); i++)
        {
            // Try to find LF
            if (*readPtr == '\n')
            {
                // Check if next isn't CR
                if (*(readPtr + 1) != '\r')
                {
                    // Append CR
                    output.add('\r');
                }
            }
            // Copy character
            output.add(*readPtr++);
        }
    }

    void TkFileSystem::getDirPath(const SpecialFolder type, TkString &result)
    {
        ::KNOWNFOLDERID rfid;
        switch (type)
        {
        case SpecialFolder::kDesktop:
            rfid = FOLDERID_Desktop;
            break;
        case SpecialFolder::kDocuments:
            rfid = FOLDERID_Documents;
            break;
        case SpecialFolder::kPictures:
            rfid = FOLDERID_Pictures;
            break;
        case SpecialFolder::kAppData:
            rfid = FOLDERID_RoamingAppData;
            break;
        case SpecialFolder::kLocalAppData:
            rfid = FOLDERID_LocalAppData;
            break;
        case SpecialFolder::kProgramData:
            rfid = FOLDERID_ProgramData;
            break;
        case SpecialFolder::kTemporary:
        {
            wchar buffer[MAX_PATH];
            if (GetTempPathW(MAX_PATH, buffer))
            {
                StringAsUTF8<MAX_PATH> u8Buffer{buffer};
                result = u8Buffer.get();
                TkPath::normalize(result);
            }
            return;
        }
        }

        PWSTR path = nullptr;
        HRESULT hr = SHGetKnownFolderPath(rfid, 0, nullptr, &path);
        if (SUCCEEDED(hr))
        {
            StringAsUTF8<MAX_PATH> u8Buffer{path};
            result = u8Buffer.get();
            TkPath::normalize(result);
        }

        CoTaskMemFree(path);
    }

    bool Win32FileSystem::moveToTrashBin(const TkStringView path)
    {
        TK_ASSERT(path.length() < MAX_PATH);
        wchar pathNullNull[MAX_PATH + 1];
        StringAsUTF16<MAX_PATH> pathU16{path.get()};
        TkMemory::memoryCopy(pathNullNull, pathU16.get(), sizeof(wchar) * pathU16.length() + 1);
        pathNullNull[pathU16.length()] = 0;
        pathNullNull[pathU16.length() + 1] = 0;

        ::SHFILEOPSTRUCT op;
        op.hwnd = nullptr;
        op.wFunc = FO_DELETE;
        op.pFrom = pathNullNull;
        op.pTo = nullptr;
        op.fFlags = FOF_ALLOWUNDO | FOF_NO_UI;
        op.fAnyOperationsAborted = FALSE;
        op.hNameMappings = nullptr;
        op.lpszProgressTitle = nullptr;

        return ::SHFileOperation(&op) == 0;
    }

    TkThread *Win32FileWatcher::s_thread = nullptr;
    bool Win32FileWatcher::s_threadActive = false;
    ::HANDLE Win32FileWatcher::s_ioHandle = INVALID_HANDLE_VALUE;
    TkMutex Win32FileWatcher::s_mtx {};
    TkStaticArray<Win32FileWatcher *, 16> Win32FileWatcher::s_watchers {};

    FileWatcherHandle TkFileSystem::openFileWatcher(const TkString &directory, bool withSubDirs)
    {
        auto *ptr = TkMemory::newObject<Win32FileWatcher>(TkMemory::getDefault());
        ptr->path = directory;
        ptr->withSubDirs = withSubDirs;

        TkMemory::memoryClear(&ptr->overlapped, sizeof(ptr->overlapped));

        // Create directory handle for events handling
        StringAsUTF16<MAX_PATH> pth{directory.get()};
        ptr->directoryHandle = ::CreateFileW(
            pth.get(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            nullptr);
        if (ptr->directoryHandle == INVALID_HANDLE_VALUE)
        {
            LogError("[FileWatcher] Win32 file handle create failed %s", Win32System::getLastErrMsg());
            TkMemory::deletePtr(ptr);
            return nullptr;
        }

        // Register
        Win32FileWatcher::s_mtx.lock();
        Win32FileWatcher::s_watchers.add(ptr);
        if (!Win32FileWatcher::s_thread)
        {
            Win32FileWatcher::s_ioHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);
            Win32FileWatcher::s_threadActive = true;
            Win32FileWatcher::s_thread = TkThread::start(Win32FileWatcher::run, "FileWatcher Thread", ThreadPriority::kBelowNormal);
        }
        CreateIoCompletionPort(ptr->directoryHandle, Win32FileWatcher::s_ioHandle, 0, 1);
        Win32FileWatcher::s_mtx.unlock();

        // Initialize filesystem events tracking
        ptr->readDirectoryChanges();

        return (FileWatcherHandle)ptr;
    }

    void TkFileSystem::closeFileWatcher(FileWatcherHandle handle)
    {
        if (!handle) 
            TK_ASSERT("Handle is nullptr");
        auto ptr = (Win32FileWatcher *)handle;
        Win32FileWatcher::s_mtx.lock();
        Win32FileWatcher::s_watchers.remove(ptr);
        Win32FileWatcher::s_mtx.unlock();

        ptr->stopNow = true;

        if (ptr->directoryHandle != INVALID_HANDLE_VALUE)
        {
            CancelIoEx(ptr->directoryHandle, (OVERLAPPED *)&ptr->overlapped);

            const HANDLE handle = ptr->directoryHandle;
            ptr->directoryHandle = INVALID_HANDLE_VALUE;
            WaitForSingleObjectEx(handle, 0, true);

            CloseHandle(handle);
        }

        while (TSync::atomicRead(&ptr->activeCallbacks) > 0)
            TkThread::sleep(1);

        Win32FileWatcher::s_mtx.lock();
        if (Win32FileWatcher::s_watchers.isEmpty() && Win32FileWatcher::s_thread)
        {
            Win32FileWatcher::s_threadActive = false;
            Win32FileWatcher::s_mtx.unlock();
            PostQueuedCompletionStatus(Win32FileWatcher::s_ioHandle, 0, 0, nullptr);
            Win32FileWatcher::s_thread->join();
            Win32FileWatcher::s_mtx.lock();
            TkThread::destroy(Win32FileWatcher::s_thread);
            Win32FileWatcher::s_thread = nullptr;
            CloseHandle(Win32FileWatcher::s_ioHandle);
            Win32FileWatcher::s_ioHandle = INVALID_HANDLE_VALUE;
        }
        Win32FileWatcher::s_mtx.unlock();

        TkMemory::deletePtr(TkMemory::getDefault(), ptr);
    }

    TkAction<const TkString &, FileAction> &TkFileSystem::getEvent(FileWatcherHandle handle)
    {
        if (!handle) 
            TK_ASSERT("Handle is nullptr");
        auto ptr = (Win32FileWatcher *)handle;
        return ptr->event;
    }

    bool TkFileSystem::isEnabled(FileWatcherHandle handle)
    {
        return handle != nullptr;
    }

    i32 Win32FileWatcher::run()
    {
        DWORD numBytes = 0;
        LPOVERLAPPED overlapped = nullptr;
        ULONG_PTR compKey = 0;
        while (s_threadActive)
        {
            if (GetQueuedCompletionStatus(s_ioHandle, &numBytes, &compKey, &overlapped, INFINITE) && overlapped && numBytes != 0)
            {
                Win32FileWatcher *targetWatcher = nullptr;
                s_mtx.lock();
                for (auto watcher : s_watchers)
                {
                    if ((OVERLAPPED *)&watcher->overlapped == overlapped)
                    {
                        targetWatcher = watcher;
                        TSync::interlockedIncrement(&targetWatcher->activeCallbacks);
                        break;
                    }
                }
                s_mtx.unlock();

                if (targetWatcher)
                {
                    targetWatcher->notificationCompletion();
                    TSync::interlockedDecrement(&targetWatcher->activeCallbacks);
                }
            }
        }
        return 0;
    }

    void Win32FileWatcher::readDirectoryChanges()
    {
        BOOL result = ReadDirectoryChangesW(
            directoryHandle,
            Buffer,
            bufferSize,
            withSubDirs ? TRUE : FALSE,
            FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_FILE_NAME,
            nullptr,
            (OVERLAPPED *)&overlapped,
            nullptr);
        if (!result)
        {
            LogError("[FileWatcher] Win32 file handle create failed %s", Win32System::getLastErrMsg());
            Sleep(1);
        }
    }

    void Win32FileWatcher::notificationCompletion()
    {
        // Process notifications
        auto notify = (FILE_NOTIFY_INFORMATION *)Buffer;
        do
        {
            // Convert action type
            FileAction action;
            switch (notify->Action)
            {
            case FILE_ACTION_RENAMED_NEW_NAME:
            case FILE_ACTION_RENAMED_OLD_NAME:
                action = FileAction::kRename;
                break;
            case FILE_ACTION_ADDED:
                action = FileAction::kCreate;
                break;
            case FILE_ACTION_REMOVED:
                action = FileAction::kDelete;
                break;
            case FILE_ACTION_MODIFIED:
                action = FileAction::kModify;
                break;
            default:
                action = FileAction::kUnknown;
                break;
            }
            if (action != FileAction::kUnknown)
            {
                // Build path
                StringAsUTF8<MAX_PATH> file{notify->FileName};
                TkString path(file.get(), notify->FileNameLength / sizeof(WCHAR));
                path = this->path / path;

                // Send event without holding the global watchers lock.
                event(path, action);
            }

            // Move to the next notify
            notify = (FILE_NOTIFY_INFORMATION *)((byte *)notify + notify->NextEntryOffset);
        } while (notify->NextEntryOffset != 0);

        // Get the new read issued as fast as possible
        if (!stopNow)
        {
            readDirectoryChanges();
        }
    }
}

#endif
