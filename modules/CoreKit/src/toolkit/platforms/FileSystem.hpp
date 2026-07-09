/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/base/Encoding.hpp>
#include <toolkit/types/StringView.hpp>

namespace toolkit
{
    enum class SpecialFolder : u8
    {
        kDesktop,
        kDocuments,
        kPictures,
        kAppData,
        kLocalAppData,
        kProgramData,
        kTemporary,
    };

    enum class FileMode : u32
    {
        kCreateNew = 1,
        kCreateAlways = 2,
        kOpenExisting = 3,
        kOpenAlways = 4,
        kTruncateExisting = 5,
    };

    enum class FileAccess : u32
    {
        kRead = 0x80000000,
        kWrite = 0x40000000,
        kReadWrite = kRead | kWrite,
    };

    enum class FileShare : u32
    {
        kNone = 0x00000000,
        kRead = 0x00000001,
        kWrite = 0x00000002,
        kDelete = 0x00000004,
        kReadWrite = kRead | kWrite,
        kAll = kReadWrite | kDelete,
    };

    struct OpenFileParams
    {
        const TkStringView path;
        FileMode mode = FileMode::kOpenExisting;
        FileAccess access = FileAccess::kRead;
        FileShare share = FileShare::kRead;
    };

    enum class FileAction
    {
        kUnknown = 0,
        kCreate = 1,
        kDelete = 2,
        kModify = 4,
        kRename = 8,
    };

    typedef void *FileHandle;
    static constexpr FileHandle kInvalidFileHandle = nullptr;

    typedef void *FileWatcherHandle;
    static constexpr FileWatcherHandle kInvalidFileWatcherHandle = nullptr;

    class TkFileSystem
    {
    public:
        // Create a new directory.
        static bool makeDir(const TkStringView path);
        // Delete an existing directory.
        static bool deleteDir(const TkStringView path, bool deleteContents = true);
        // Check if directory exists.
        static bool isDirExist(const TkStringView path);
        // Find the files that match the specified search pattern in the specified directory
        static bool listDir(TkArray<TkString> &results, const TkString &path, const char *searchPattern = "*", bool recursive = false);
        // Find the directories that are inside the specified directory.
        static bool listChildren(TkArray<TkString> &results, const TkString &path);
        // Copy the directory
        static bool copyDir(const TkStringView dst, const TkStringView src, bool recursive = true);
        // Get the size of the directory (in bytes) defined by size of all files contained by it.
        static psize getDirSize(const TkStringView path);

    public:
        // Delete an existing file
        static bool deleteFile(const TkStringView path);
        // Check if a given file exists
        static bool isFileExist(const TkStringView path);
        // Get the size of the file (in bytes)
        static psize getFileSize(const TkStringView path);
        // Check if file is read-only
        static bool isReadOnly(const TkStringView path);
        // Set file as read-only
        static bool setReadOnly(const TkStringView path, bool isReadOnly);
        // Move the file
        static bool moveFile(const TkStringView dst, const TkStringView src, bool overwrite = false);
        // Copy the file
        static bool copyFile(const TkStringView dst, const TkStringView src);
        // Get last time when file has been modified (in UTC)
        static TkDateTime getLastEdit(const TkStringView path);
        // Get the special folder path
        static void getDirPath(const SpecialFolder type, TkString &result);
        // Get the file path to the temporary file that can be created and used.
        static void getTempDirPath(TkString &tmpPath);

    public:
        // Open a file as FileHandle to process
        static FileHandle openFile(OpenFileParams &params);
        // Open a file as FileHandle to process
        static FileHandle openFile(TkStringView path) {
            OpenFileParams p {.path = path};
            return openFile(p);
        }
        // Close a existing file handle
        static void closeFile(FileHandle);
        // Read data from a file.
        static bool readFile(FileHandle handle, void *buffer, u32 bytesToRead, u32 *bytesRead = nullptr);
        // Write data to a file.
        static bool writeFile(FileHandle handle, const void *buffer, u32 bytesToWrite, u32 *bytesWritten = nullptr);

        // Get size of the file (in bytes).
        static u32 getFileSize(FileHandle);
        // Retrieve the date and time that a file was last modified (in UTC).
        static TkDateTime getFileLastWrite(FileHandle);
        // Get current position of the file pointer.
        static u32 getFilePos(FileHandle);
        // Set new position of the file pointer.
        static void setFilePos(FileHandle, u32 seek);
        // Return true if file is opened.
        static bool isFileOpened(FileHandle);

        // Read data from file to byte array
        static bool readAsBytes(FileHandle, byte *data, i32 length);
        // Read data from file to byte array
        static bool readAsBytes(FileHandle, TkArray<byte> &data);
        // Read data from file to byte array
        static bool readAsBytes(FileHandle, TkDataCollection<byte> &data);
        // Read data from file to string
        static bool readAsText(FileHandle, TkString &data);
        // Write data to file from byte array
        static bool writeBytes(FileHandle, const byte *data, i32 length);
        // Write data to file from byte array
        static bool writeBytes(FileHandle, const TkArray<byte> &data);
        // Write data to file from string
        static bool writeText(FileHandle, const TkString &data, Encoding encoding);
        // Write data to file from string
        static bool writeText(FileHandle, const TkStringBuilder &data, Encoding encoding);
        // Write data to file from string
        static bool writeText(FileHandle, const char *data, i32 length, Encoding encoding);

    public:
        // Create a file watcher to monitor file modifications
        static FileWatcherHandle openFileWatcher(const TkString &directory, bool withSubDirs);
        // Close a file watcher
        static void closeFileWatcher(FileWatcherHandle);
        // Get the event binder of file watcher
        static TkAction<const TkString &, FileAction> &getEvent(FileWatcherHandle);
        // Is file watcher enabled
        static bool isEnabled(FileWatcherHandle);
    };
}
