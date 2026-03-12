/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

#include <toolkit/collection/Array.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/platforms/FileSystem.hpp>
#include <toolkit/platforms/Network.hpp>
#include <toolkit/types/DateTime.hpp>
#include <toolkit/types/UUID.hpp>
#include <toolkit/types/Path.hpp>
#include <toolkit/base/Encoding.hpp>

namespace
{
    TkString makeTempTestDir()
    {
        return System::getTempDir() / TkString::format("ToolkitPlatform_%s", TkUuid::create().toString(UuidFormatType::kN).get());
    }

    struct TempDirScope
    {
        TkString path;

        TempDirScope()
            : path(makeTempTestDir())
        {
            EXPECT_TRUE(TkFileSystem::makeDir(path));
        }

        ~TempDirScope()
        {
            TkFileSystem::deleteDir(path, true);
        }
    };
}

class PlatformApiTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        System::init();
        System::beforeRun();
    }

    static void TearDownTestSuite()
    {
        System::beforeQuit();
        System::quit();
    }
};

TEST_F(PlatformApiTest, SystemMemoryAllocation)
{
    void *reserved = System::reserveMemory(4096);
    ASSERT_NE(reserved, nullptr);

    System::commitMemory(reserved, 4096);
    byte *bytes = static_cast<byte *>(reserved);
    bytes[0] = 0x12;
    bytes[4095] = 0x34;
    EXPECT_EQ(bytes[0], 0x12);
    EXPECT_EQ(bytes[4095], 0x34);
    System::releaseMemory(reserved, 4096);

    void *aligned = System::allocAligned(256, 64);
    ASSERT_NE(aligned, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(aligned) % 64u, 0u);
    System::free(aligned);
}

TEST_F(PlatformApiTest, SystemTimeAndPaths)
{
    EXPECT_GT(System::getTimeSeconds(), 0.0);
    EXPECT_GT(System::getTimeCycles(), 0ull);
    EXPECT_GT(System::getClockFrequency(), 0ull);

    TimeStats local{};
    TimeStats utc{};
    System::getSystemTime(local);
    System::getUTCTime(utc);

    EXPECT_GE(local.month, 1);
    EXPECT_LE(local.month, 12);
    EXPECT_GE(local.day, 1);
    EXPECT_LE(local.day, 31);
    EXPECT_GE(utc.month, 1);
    EXPECT_LE(utc.month, 12);

    TkString execPath = System::getExecPath();
    EXPECT_FALSE(execPath.isEmpty());
    EXPECT_TRUE(TkFileSystem::isFileExist(execPath));

    TkString tempDir = System::getTempDir();
    EXPECT_FALSE(tempDir.isEmpty());
    EXPECT_TRUE(TkFileSystem::isDirExist(tempDir));
}

TEST_F(PlatformApiTest, SystemWorkDirAndEnvVar)
{
    TempDirScope tmp;
    const TkString original = System::getWorkDir();

    ASSERT_TRUE(System::setWorkDir(tmp.path));
    EXPECT_TRUE(TkPath::isSame(System::getWorkDir(), tmp.path));
    EXPECT_TRUE(System::setWorkDir(original));
    EXPECT_TRUE(TkPath::isSame(System::getWorkDir(), original));

    const TkString key = "TK_PLATFORM_API_TEST_VAR";
    const TkString value = TkUuid::create().toString(UuidFormatType::kN);
    ASSERT_TRUE(System::setEnvVar(key, value));

    TkString readValue;
    ASSERT_TRUE(System::getEnvVar(key, readValue));
    EXPECT_EQ(readValue, value);
}

TEST_F(PlatformApiTest, SystemLoadLibraryAndSymbol)
{
    void *lib = System::loadLib("kernel32.dll");
    ASSERT_NE(lib, nullptr);

    void *symbol = System::getLibSymbol(lib, "GetCurrentThreadId");
    EXPECT_NE(symbol, nullptr);

    System::freeLib(lib);
}

TEST_F(PlatformApiTest, FileSystemOpenWriteReadAndAttributes)
{
    TempDirScope tmp;
    const TkString filePath = tmp.path / "sample.txt";
    const TkString movedPath = tmp.path / "moved.txt";
    const TkString copiedPath = tmp.path / "copied.txt";
    const TkString text = "platform api text";

    OpenFileParams createParams{filePath, FileMode::kCreateAlways, FileAccess::kReadWrite, FileShare::kReadWrite};
    FileHandle handle = TkFileSystem::openFile(createParams);
    ASSERT_NE(handle, nullptr);
    ASSERT_TRUE(TkFileSystem::writeText(handle, text, Encoding::kUTF8));
    EXPECT_GT(TkFileSystem::getFileSize(handle), 0u);
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
    TkFileSystem::closeFile(handle);

    EXPECT_FALSE(TkFileSystem::isReadOnly(filePath));
    EXPECT_TRUE(TkFileSystem::setReadOnly(filePath, true));
    EXPECT_TRUE(TkFileSystem::isReadOnly(filePath));
    EXPECT_TRUE(TkFileSystem::setReadOnly(filePath, false));
    EXPECT_FALSE(TkFileSystem::isReadOnly(filePath));

    EXPECT_TRUE(TkFileSystem::copyFile(copiedPath, filePath));
    EXPECT_TRUE(TkFileSystem::isFileExist(copiedPath));

    EXPECT_TRUE(TkFileSystem::moveFile(movedPath, filePath, true));
    EXPECT_TRUE(TkFileSystem::isFileExist(movedPath));
    EXPECT_FALSE(TkFileSystem::isFileExist(filePath));
}

TEST_F(PlatformApiTest, FileSystemDirectoryListingAndCopy)
{
    TempDirScope tmp;
    TempDirScope dst;
    const TkString childDir = tmp.path / "child";
    const TkString nestedFile = childDir / "nested.txt";
    const TkString copyDir = dst.path / "copy";

    ASSERT_TRUE(TkFileSystem::makeDir(childDir));

    OpenFileParams params{nestedFile, FileMode::kCreateAlways, FileAccess::kWrite, FileShare::kReadWrite};
    FileHandle handle = TkFileSystem::openFile(params);
    ASSERT_NE(handle, nullptr);
    ASSERT_TRUE(TkFileSystem::writeText(handle, "nested", Encoding::kUTF8));
    TkFileSystem::closeFile(handle);

    TkArray<TkString> children;
    ASSERT_TRUE(TkFileSystem::listChildren(children, tmp.path));
    EXPECT_TRUE(children.contains(childDir));

    TkArray<TkString> files;
    ASSERT_TRUE(TkFileSystem::listDir(files, tmp.path, "*.txt", true));
    EXPECT_TRUE(files.contains(nestedFile));

    ASSERT_TRUE(TkFileSystem::copyDir(copyDir, tmp.path, true));
    EXPECT_TRUE(TkFileSystem::isDirExist(copyDir));
    EXPECT_TRUE(TkFileSystem::isFileExist(copyDir / "child" / "nested.txt"));
    EXPECT_GT(TkFileSystem::getFileSize(copyDir / "child" / "nested.txt"), 0u);
}

TEST_F(PlatformApiTest, FileSystemWatcherSmoke)
{
    TempDirScope tmp;

    FileWatcherHandle watcher = TkFileSystem::openFileWatcher(tmp.path, false);
    ASSERT_NE(watcher, nullptr);
    EXPECT_TRUE(TkFileSystem::isEnabled(watcher));

    TkFileSystem::closeFileWatcher(watcher);
}

TEST_F(PlatformApiTest, NetworkSocketAndGroupSmoke)
{
    NetEndPoint bindEndPoint;
    ASSERT_TRUE(TkNetwork::createEndPoint("", "0", NetIPVersion::kIPv4, bindEndPoint));

    NetSocket *socket = TkNetwork::newSocket(NetProtocol::kUdp, NetIPVersion::kIPv4);
    ASSERT_NE(socket, nullptr);
    ASSERT_TRUE(TkNetwork::bind(socket, bindEndPoint));

    NetSocketGroup group{};
    ASSERT_TRUE(TkNetwork::newSocketGroup(4, &group));

    const i32 index = TkNetwork::addSocket(&group, socket);
    ASSERT_GE(index, 0);
    EXPECT_EQ(group.count, 1u);

    NetSocket out{};
    ASSERT_TRUE(TkNetwork::getSocket(&group, index, &out));
    EXPECT_EQ(out.protocol, NetProtocol::kUdp);
    EXPECT_EQ(*(u64 *)out.data, *(u64 *)socket->data);

    EXPECT_TRUE(TkNetwork::removeSocket(&group, socket));
    EXPECT_EQ(group.count, 0u);

    TkNetwork::clearGroup(&group);
    EXPECT_TRUE(TkNetwork::destroySocketGroup(&group));
    EXPECT_TRUE(TkNetwork::destroySocket(socket));
}

TEST_F(PlatformApiTest, NetworkCreateEndpointRejectsInvalidAddress)
{
    NetEndPoint endPoint;
    EXPECT_FALSE(TkNetwork::createEndPoint("256.256.256.256", "99999", NetIPVersion::kIPv4, endPoint, false));
}
