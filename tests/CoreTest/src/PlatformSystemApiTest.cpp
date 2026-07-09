/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "PlatformTestCommon.hpp"

#include <toolkit/platforms/ProcessInitArgs.hpp>
#include <toolkit/collection/HashMap.hpp>
#include <toolkit/base/Math.hpp>

class PlatformSystemApiTest : public PlatformTestBase
{
};

TEST_F(PlatformSystemApiTest, MemoryAllocationApis)
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

    void *allocated = System::alloc(128);
    ASSERT_NE(allocated, nullptr);
    static_cast<byte *>(allocated)[0] = 0x56;

    void *reallocated = System::reallocAligned(allocated, 128, 512, 64);
    ASSERT_NE(reallocated, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(reallocated) % 64u, 0u);
    EXPECT_EQ(static_cast<byte *>(reallocated)[0], 0x56);
    System::free(reallocated);

    void *aligned = System::allocAligned(256, 64);
    ASSERT_NE(aligned, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(aligned) % 64u, 0u);
    System::free(aligned);
}

TEST_F(PlatformSystemApiTest, MetadataTimeAndIdentityApis)
{
    EXPECT_FALSE(System::getOpSysName().isEmpty());

    const TkVersion version = System::getOpSysVersion();
    EXPECT_GT(version.major(), 0);

    const CPUStats cpu = System::getCPUStats();
    EXPECT_GT(cpu.logicalCoreCount, 0u);
    EXPECT_GT(cpu.pageSize, 0u);
    EXPECT_GT(cpu.cacheLineSize, 0u);

    const MemoryStats memory = System::getMemoryStats();
    EXPECT_GT(memory.physicalMemSize, 0u);
    EXPECT_GT(memory.virtualMemSize, 0u);

    const ProcessMemoryStats processMemory = System::getProcessMemoryStats();
    EXPECT_GT(processMemory.usedPhysicalSize, 0u);

    EXPECT_GT(System::getCurrentProcessId(), 0u);
    EXPECT_GT(System::getTimeSeconds(), 0.0);
    EXPECT_GT(System::getTimeCycles(), 0ull);
    EXPECT_GT(System::getClockFrequency(), 0ull);

    TimeStats local{};
    TimeStats utc{};
    System::getSystemTime(local);
    System::getUTCTime(utc);
    EXPECT_GE(local.month, 1);
    EXPECT_LE(local.month, 12);
    EXPECT_GE(utc.month, 1);
    EXPECT_LE(utc.month, 12);

    const TkString execPath = System::getExecPath();
    EXPECT_FALSE(execPath.isEmpty());
    EXPECT_TRUE(TkFileSystem::isFileExist(execPath));

    const TkString tempDir = System::getTempDir();
    EXPECT_FALSE(tempDir.isEmpty());
    EXPECT_TRUE(TkFileSystem::isDirExist(tempDir));

    const TkUuid deviceId = System::getUniqueDeviceId();
    EXPECT_NE(deviceId, TkUuid::Empty);

    TkUuid created = TkUuid::Empty;
    System::createUuid(created);
    EXPECT_NE(created, TkUuid::Empty);
}

TEST_F(PlatformSystemApiTest, WorkDirEnvAndProcessApis)
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

    TkHashMap<TkString, TkString> envVars;
    System::getEnvVars(envVars);
    EXPECT_TRUE(envVars.containsKey(key));
    EXPECT_EQ(envVars[key], value);

    ProcessInitArgs args;
    args.file = "cmd.exe";
    args.args = "/c echo %TK_PLATFORM_CHILD%";
    args.workDir = tmp.path;
    args.logOutput = false;
    args.saveOutput = true;
    args.hiddenWindow = true;
    args.waitForQuit = true;
    args.shellExecute = false;
    args.envArgs.add(TkString("TK_PLATFORM_CHILD"), TkString("child_process_value"));

    const i32 result = System::newProcess(args);
    EXPECT_EQ(result, 0);

    const TkString output(args.output.get(), args.output.count());
    EXPECT_TRUE(output.contains("child_process_value", false));
}

TEST_F(PlatformSystemApiTest, LibraryAndIdentityApis)
{
    void *lib = System::loadLib("kernel32.dll");
    ASSERT_NE(lib, nullptr);

    void *symbol = System::getLibSymbol(lib, "GetCurrentThreadId");
    EXPECT_NE(symbol, nullptr);
    System::freeLib(lib);

    const TkString locale = System::getLocale();
    EXPECT_FALSE(locale.isEmpty());

    const TkString computerName = System::getComputerName();
    EXPECT_FALSE(computerName.isEmpty());

    const TkString userName = System::getUserName();
    EXPECT_FALSE(userName.isEmpty());

    TkString envUserName;
    if (System::getEnvVar("USERNAME", envUserName))
        EXPECT_EQ(userName, envUserName);
}

TEST_F(PlatformSystemApiTest, ProcessMutexAndDpiApis)
{
    const TkString mutexName = TkString("ToolkitPlatformSystemMutex_") + TkUuid::create().toString(UuidFormatType::kN);
    EXPECT_TRUE(System::createProcessMutex(mutexName.get()));
    System::releaseProcessMutex();

    System::setHighDpiAware(true);
    EXPECT_GE(System::getDpi(), 96);
    EXPECT_GE(System::getDpiScale(), 1.0f);
}

TEST_F(PlatformSystemApiTest, MouseAndDisplayApis)
{
    const Pos originalPos = System::getMousePos();
    System::setMousePos(originalPos);
    const Pos restoredPos = System::getMousePos();
    const f32 deltaX = restoredPos.x - originalPos.x;
    const f32 deltaY = restoredPos.y - originalPos.y;
    EXPECT_TRUE(deltaX >= -1.0f && deltaX <= 1.0f);
    EXPECT_TRUE(deltaY >= -1.0f && deltaY <= 1.0f);

    const Pos desktopSize = System::getDesktopSize();
    EXPECT_GT(desktopSize.x, 0.0f);
    EXPECT_GT(desktopSize.y, 0.0f);

    const Rect monitorBounds = System::getMonitorBounds(originalPos);
    EXPECT_GT(monitorBounds.w, 0.0f);
    EXPECT_GT(monitorBounds.h, 0.0f);

    const Rect virtualDesktopBounds = System::getVDesktopBounds();
    EXPECT_GT(virtualDesktopBounds.w, 0.0f);
    EXPECT_GT(virtualDesktopBounds.h, 0.0f);

    const Pos virtualDesktopSize = System::GetVDesktopSize();
    EXPECT_GT(virtualDesktopSize.x, 0.0f);
    EXPECT_GT(virtualDesktopSize.y, 0.0f);
}

TEST_F(PlatformSystemApiTest, BatteryApis)
{
    const BatteryInfo battery = System::getBatteryStats();
    EXPECT_GE(battery.percent, 0.0f);
    EXPECT_LE(battery.percent, 1.0f);
}

TEST_F(PlatformSystemApiTest, NetworkConnectionApis)
{
    const NetConType conType = System::getNetConType();
    EXPECT_LE(static_cast<u8>(conType), static_cast<u8>(NetConType::kEthernet));
}

TEST_F(PlatformSystemApiTest, ScreenOrientationApis)
{
    const ScreenOrienType orienType = System::getScreenOrienType();
    EXPECT_LE(static_cast<u8>(orienType), static_cast<u8>(ScreenOrienType::kLandscapeRight));
}
