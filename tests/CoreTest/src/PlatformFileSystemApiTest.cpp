/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "PlatformTestCommon.hpp"

#include <toolkit/threading/Thread.hpp>
#include <toolkit/collection/DataCollection.hpp>
#include <atomic>

class PlatformFileSystemApiTest : public PlatformTestBase
{
};

TEST_F(PlatformFileSystemApiTest, DirectoryAndSpecialFolderApis)
{
    TempDirScope tmp;
    TempDirScope dst;
    const TkString childDir = tmp.path / "child";
    const TkString nestedFile = childDir / "nested.txt";
    const TkString copyDir = dst.path / "copy";

    ASSERT_TRUE(TkFileSystem::makeDir(childDir));
    ASSERT_TRUE(writePlatformTestFile(nestedFile, "nested"));

    EXPECT_TRUE(TkFileSystem::isDirExist(tmp.path));
    EXPECT_EQ(TkFileSystem::getDirSize(childDir), TkFileSystem::getFileSize(nestedFile));

    TkArray<TkString> children;
    ASSERT_TRUE(TkFileSystem::listChildren(children, tmp.path));
    EXPECT_TRUE(children.contains(childDir));

    TkArray<TkString> files;
    ASSERT_TRUE(TkFileSystem::listDir(files, tmp.path, "*.txt", true));
    EXPECT_TRUE(files.contains(nestedFile));

    ASSERT_TRUE(TkFileSystem::copyDir(copyDir, tmp.path, true));
    EXPECT_TRUE(TkFileSystem::isDirExist(copyDir));
    EXPECT_TRUE(TkFileSystem::isFileExist(copyDir / "child" / "nested.txt"));

    TkString localAppDataPath;
    TkFileSystem::getDirPath(SpecialFolder::kLocalAppData, localAppDataPath);
    EXPECT_FALSE(localAppDataPath.isEmpty());

    TkString tempPath;
    TkFileSystem::getDirPath(SpecialFolder::kTemporary, tempPath);
    EXPECT_TRUE(TkFileSystem::isDirExist(tempPath));

    TkString generatedTempPath;
    TkFileSystem::getTempDirPath(generatedTempPath);
    EXPECT_FALSE(generatedTempPath.isEmpty());
    EXPECT_TRUE(TkPath::isSame(TkPath::getDir(generatedTempPath), System::getTempDir()));
}

TEST_F(PlatformFileSystemApiTest, OpenReadWriteMoveCopyAndDeleteApis)
{
    TempDirScope tmp;
    const TkString filePath = tmp.path / "sample.txt";
    const TkString copiedPath = tmp.path / "copied.txt";
    const TkString movedPath = tmp.path / "moved.txt";
    const TkString text = "platform api text";

    OpenFileParams createParams{filePath, FileMode::kCreateAlways, FileAccess::kReadWrite, FileShare::kReadWrite};
    FileHandle handle = TkFileSystem::openFile(createParams);
    ASSERT_NE(handle, nullptr);

    const byte rawBytes[] = {0x10, 0x20, 0x30, 0x40};
    ASSERT_TRUE(TkFileSystem::writeBytes(handle, rawBytes, static_cast<i32>(sizeof(rawBytes))));
    EXPECT_EQ(TkFileSystem::getFilePos(handle), sizeof(rawBytes));

    TkFileSystem::setFilePos(handle, 0);
    byte readBytes[4] = {};
    ASSERT_TRUE(TkFileSystem::readAsBytes(handle, readBytes, static_cast<i32>(sizeof(readBytes))));
    EXPECT_EQ(std::memcmp(readBytes, rawBytes, sizeof(rawBytes)), 0);

    TkFileSystem::closeFile(handle);

    handle = TkFileSystem::openFile(createParams);
    ASSERT_NE(handle, nullptr);
    ASSERT_TRUE(TkFileSystem::writeText(handle, text, Encoding::kUTF8));
    EXPECT_GT(TkFileSystem::getFileSize(handle), 0u);
    EXPECT_TRUE(TkFileSystem::isFileOpened(handle));
    TkFileSystem::closeFile(handle);

    EXPECT_TRUE(TkFileSystem::isFileExist(filePath));
    EXPECT_EQ(TkFileSystem::getFileSize(filePath), text.length());

    OpenFileParams readParams{filePath, FileMode::kOpenExisting, FileAccess::kRead, FileShare::kReadWrite};
    handle = TkFileSystem::openFile(readParams);
    ASSERT_NE(handle, nullptr);

    TkString readText;
    ASSERT_TRUE(TkFileSystem::readAsText(handle, readText));
    EXPECT_EQ(readText, text);
    EXPECT_GT(TkFileSystem::getFileLastWrite(handle).getYear(), 2000);

    TkArray<byte> byteArray;
    TkFileSystem::setFilePos(handle, 0);
    ASSERT_TRUE(TkFileSystem::readAsBytes(handle, byteArray));
    EXPECT_EQ(byteArray.count(), text.length());

    TkDataCollection<byte> byteCollection;
    TkFileSystem::setFilePos(handle, 0);
    ASSERT_TRUE(TkFileSystem::readAsBytes(handle, byteCollection));
    EXPECT_EQ(byteCollection.length(), text.length());
    TkFileSystem::closeFile(handle);

    EXPECT_FALSE(TkFileSystem::isReadOnly(filePath));
    EXPECT_TRUE(TkFileSystem::setReadOnly(filePath, true));
    EXPECT_TRUE(TkFileSystem::isReadOnly(filePath));
    EXPECT_TRUE(TkFileSystem::setReadOnly(filePath, false));
    EXPECT_FALSE(TkFileSystem::isReadOnly(filePath));

    EXPECT_GT(TkFileSystem::getLastEdit(filePath).getYear(), 2000);

    EXPECT_TRUE(TkFileSystem::copyFile(copiedPath, filePath));
    EXPECT_TRUE(TkFileSystem::isFileExist(copiedPath));

    EXPECT_TRUE(TkFileSystem::moveFile(movedPath, filePath, true));
    EXPECT_TRUE(TkFileSystem::isFileExist(movedPath));
    EXPECT_FALSE(TkFileSystem::isFileExist(filePath));

    EXPECT_TRUE(TkFileSystem::deleteFile(copiedPath));
    EXPECT_FALSE(TkFileSystem::isFileExist(copiedPath));
}

TEST_F(PlatformFileSystemApiTest, FileWatcherSmoke)
{
    TempDirScope tmp;
    const TkString watchFile = tmp.path / "watch.txt";

    FileWatcherHandle watcher = TkFileSystem::openFileWatcher(tmp.path, false);
    ASSERT_NE(watcher, nullptr);
    EXPECT_TRUE(TkFileSystem::isEnabled(watcher));

    std::atomic<int> eventCount = 0;
    auto &event = TkFileSystem::getEvent(watcher);
    event.bind([&](const TkString &path, FileAction) {
        if (TkPath::isSame(path, watchFile))
            ++eventCount;
    });

    ASSERT_TRUE(writePlatformTestFile(watchFile, "watch"));

    for (i32 i = 0; i < 50 && eventCount.load() == 0; ++i)
        TkThread::sleep(10);

    EXPECT_GT(eventCount.load(), 0);
    TkFileSystem::closeFileWatcher(watcher);
}
