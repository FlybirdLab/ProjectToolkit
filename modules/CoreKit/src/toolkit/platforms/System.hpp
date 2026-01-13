/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */
// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/common.hpp>

namespace toolkit
{

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


    struct MemoryStats
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
    

    class System
    {
    public:
        static void *reserveMemory(psize size);
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
        static TkVersion GetOpSysVersion();
        // Get the CPU's information.
        static CPUStats getCPUStats();
        // Get the memory stats at current.
        static MemoryStats getMemoryStats();
        // Get the process current memory stats.
        static ProcessMemoryStats getProcessMemoryStats();
        /// Get the current process unique identifier.
        static u64 getCurrentProcessId();
    };

}