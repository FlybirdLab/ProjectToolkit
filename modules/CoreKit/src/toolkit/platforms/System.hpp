/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */
// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/Math.hpp>

namespace toolkit
{
    enum class NetConType : u8
    {
        kNone,
        kUnknown,
        kAirplaneMode,
        kCell,
        kWiFi,
        kBluetooth,
        kEthernet,
    };
    extern const char *toString(NetConType v);

    enum class ScreenOrienType : u8
    {
        kUnknown,
        kPortrait,
        kPortraitUpsideDown,
        kLandscapeLeft,
        kLandscapeRight,
    };
    extern const char *toString(ScreenOrienType v);

    struct BatteryInfo final
    {
        enum
        {
            kUnknown,
            kCharging,
            kDischarging,
            kConnected,
        } state = BatteryInfo::kUnknown;
        f32 percent = 1.0f;
    };

    struct CPUStats final
    {
        u32 l1Cache;
        u32 l2Cache;
        u32 l3Cache;
        u32 packageCount;
        u32 coreCount;
        u32 logicalCoreCount;
        u32 pageSize;
        u64 speed;
        u32 cacheLineSize;
    };

    struct MemoryStats final
    {
        u64 physicalMemSize = 0;
        u64 usedPhysicalSize = 0;
        u64 virtualMemSize = 0;
        u64 usedVirtualSize = 0;
        u64 programSizeMem = 0;
        u64 extraDevelopmentMem = 0;
    };

    struct ProcessMemoryStats
    {
        u64 usedPhysicalSize = 0;
        u64 usedVirtualSize = 0;
    };

    struct TimeStats
    {
        i32 year;
        i32 month;
        i32 dayOfWeek;
        i32 day;
        i32 hour;
        i32 minute;
        i32 second;
        i32 millisecond;
    };

    typedef void *LibHandle;
    constexpr LibHandle kInvalidHandle = nullptr;

    class System
    {
    public:
        static void *reserveMemory(psize size);
        static void* allocatePages(u64 numPages, u64 pageSize);
        static void commitMemory(void *ptr, psize size);
        static void releaseMemory(void *ptr, psize size);

        static void *alloc(psize size);
        static void *allocAligned(psize size, psize align);
        static void *realloc(void *ptr, psize size);
        static void *reallocAligned(void *ptr, psize size, psize newSize, psize align);
        static void free(void *);

    public:
        // Invoke in main function to setup toolkit platform data
        static bool init();
        // Log the platform's information to console
        static void logInfo();
        // Setup platform service before run application
        static void beforeRun();
        // Update platform service in main loop for handling events or others.
        static void update();
        // Invoke before application quit to clean up
        static void beforeQuit();
        // For application quit platform service
        static void quit();

    public:
        // Get the name of the operating system.
        static TkString getOpSysName();
        // Get the version of the operating system.
        static TkVersion getOpSysVersion();
        // Get the CPU's information.
        static CPUStats getCPUStats();
        // Get the memory stats at current.
        static MemoryStats getMemoryStats();
        // Get the process current memory stats.
        static ProcessMemoryStats getProcessMemoryStats();
        // Get the current process unique identifier.
        static u64 getCurrentProcessId();

    public:
        // Load local shared library as handle
        static LibHandle loadLib(const char *filename);
        // Release loaded shared library's handle
        static void freeLib(LibHandle handle);
        // Get symbol address from shared library
        static void *getLibSymbol(LibHandle handle, const char *symbol);

        // Create new sub-process of current app
        static i32 newProcess(struct ProcessInitArgs &args);

    public:
        // Get the current time in seconds
        static double getTimeSeconds();
        // Get the current time as CPU cycles counter
        static u64 getTimeCycles();
        // Get the system clock frequency
        static u64 getClockFrequency();
        // Get current system time
        static void getSystemTime(TimeStats &time);
        // Get current UTC time
        static void getUTCTime(TimeStats &time);

    public:
        // Get full path of the executable file.
        static TkString getExecPath();
        // Get the (almost) unique ID of the current user device.
        static TkUuid getUniqueDeviceId();
        // Get the current working directory of the process.
        static TkString getWorkDir();
        // Get the temporary dir isolated from system app data
        static TkString getTempDir();
        // Set the current working directory of the process.
        static bool setWorkDir(const TkString &path);

    public:
        // Open a given url with broswer.
        static bool openUrl(const TkStringView url);

        // Set the High DPI awareness.
        static void setHighDpiAware(bool enable);
        // Get the primary monitor's DPI setting
        static i32 getDpi();
        // Get the primary monitor's DPI setting scale factor (1 is default). Includes custom DPI scale.
        static float getDpiScale();

        // Get the mouse cursor position in screen-space coordinates.
        static Pos getMousePos();
        // Set the mouse cursor position in screen-space coordinates.
        static void setMousePos(const Pos pos);

        // Get the origin position and size of the monitor at the given screen-space location.
        static Rect getMonitorBounds(const Pos pos);
        // Get size of the primary desktop.
        static Pos getDesktopSize();

        // Get virtual bounds of the desktop made of all the monitors outputs attached.
        static Rect getVDesktopBounds();
        // Get virtual size of the desktop made of all the monitors outputs attached.
        static Pos GetVDesktopSize();

        // Get the battery information.
        static BatteryInfo getBatteryStats();
        // Get the current network connection type.
        static NetConType getNetConType();
        // Get the current screen orientation type.
        static ScreenOrienType getScreenOrienType();

        // Get the current locale culture (eg. "pl-PL" or "en-US").
        static TkString getLocale();
        // Get the computer machine name.
        static TkString getComputerName();
        // Get the user name.
        static TkString getUserName();

        // TODO Move to WindowKit
        //  Return true if app has user focus.
        //  static bool getHasFocus();
        //
        //  Return true if app is paused, only basic service is updated until app end or resume.
        //  static bool getIsPaused();

        // Create the unique identifier.
        static void createUuid(TkUuid &result);

    public:
        // Get the process environment variables (pairs of key and value).
        static void getEnvVars(TkHashMap<TkString, TkString> &result);
        // Get the environment variable value.
        static bool getEnvVar(const TkStringView name, TkString &value);
        // Set the environment variable value.
        static bool setEnvVar(const TkStringView name, const TkString &value);

    public:
        // Create the system mutex.
        static bool createProcessMutex(const char *name);
        // Release the system mutex.
        static void releaseProcessMutex();

    public:
        // Clear the clipboard contents.
        static void clearClipBoard();
        // Set text to the clipboard.
        static void setClipBoard(const TkStringView text);
        // Set the raw bytes data to the clipboard.
        static void setClipBoard(const TkSpan<byte> data);
        // Set the files to the clipboard.
        static void setClipBoard(const TkArray<TkString> &files);
        // Get the text from the clipboard.
        static TkString getClipBoardAsText();
        // Get the raw bytes data from the clipboard.
        static TkArray<byte> getClipBoardAsData();
        // Get the file paths from the clipboard.
        static TkArray<TkString> getClipBoardAsFiles();
    };

}