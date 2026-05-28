/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "BaseSystem.hpp"

#include <toolkit/base/Assert.hpp>
#include <toolkit/threading/Sync.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/utils/Convertor.hpp>

#include <toolkit/threading/Thread.hpp>

namespace toolkit
{
    bool BaseSystem::init()
    {
#if TK_DEBUG
        // Validate atomic and interlocked operations
        i64 data = 0;
        TSync::atomicStore(&data, 11);
        TK_ASSERT(TSync::atomicRead(&data) == 11);
        TK_ASSERT(TSync::interlockedAdd(&data, 2) == 11);
        TK_ASSERT(TSync::atomicRead(&data) == 13);
        TK_ASSERT(TSync::interlockedIncrement(&data) == 14);
        TK_ASSERT(TSync::atomicRead(&data) == 14);
        TK_ASSERT(TSync::interlockedDecrement(&data) == 13);
        TK_ASSERT(TSync::atomicRead(&data) == 13);
        TK_ASSERT(TSync::interlockedExchange(&data, 10) == 13);
        TK_ASSERT(TSync::atomicRead(&data) == 10);
        TK_ASSERT(TSync::interlockedCompareExchange(&data, 11, 0) == 10);
        TK_ASSERT(TSync::atomicRead(&data) == 10);
        TK_ASSERT(TSync::interlockedCompareExchange(&data, 11, 10) == 10);
        TK_ASSERT(TSync::atomicRead(&data) == 11);
#endif

        //         srand((unsigned int)TSync::getTimeCycles());

        //         // Preallocate safety dynamic buffer to be released before Out Of Memory reporting to ensure code can properly execute
        // #ifndef PLATFORM_OUT_OF_MEMORY_BUFFER_SIZE
        // #define PLATFORM_OUT_OF_MEMORY_BUFFER_SIZE (1ull * 1024 * 1024) // 1 MB
        // #endif
        //         OutOfMemoryBuffer = Allocator::Allocate(PLATFORM_OUT_OF_MEMORY_BUFFER_SIZE);

        return true;
    }

    void BaseSystem::logInfo()
    {
        const auto cpuInfo = System::getCPUStats();
        LogInfo("CPU package count: %i, Core count: %i, Logical processors: %i", cpuInfo.packageCount, cpuInfo.coreCount, cpuInfo.logicalCoreCount);
        LogInfo("CPU Page size: %i, cache line size: %i bytes", cpuInfo.pageSize, cpuInfo.cacheLineSize);
        LogInfo("L1 cache: %i, L2 cache: %i, L3 cache: %i", cpuInfo.l1Cache, cpuInfo.l2Cache, cpuInfo.l3Cache);
        LogInfo("Clock speed: %s", TkConvertor::hertzToText(cpuInfo.speed).getText());

        const auto memStats = System::getMemoryStats();
        const f32 physicalUsedPercent = memStats.physicalMemSize > 0 ? TkConvertor::roundTo2DecimalPlaces((float)memStats.usedPhysicalSize * 100.0f / (float)memStats.physicalMemSize) : 0.0f;
        const f32 virtualUsedPercent = memStats.virtualMemSize > 0 ? TkConvertor::roundTo2DecimalPlaces((float)memStats.usedVirtualSize * 100.0f / (float)memStats.virtualMemSize) : 0.0f;
        LogInfo("Physical Memory: %llu total, %llu used (%f%%)", static_cast<unsigned long long>(memStats.physicalMemSize), static_cast<unsigned long long>(memStats.usedPhysicalSize), physicalUsedPercent);
        LogInfo("Virtual Memory: %llu total, %llu used (%f%%)", static_cast<unsigned long long>(memStats.virtualMemSize), static_cast<unsigned long long>(memStats.usedVirtualSize), virtualUsedPercent);
        LogInfo("Program Size: %llu", static_cast<unsigned long long>(memStats.programSizeMem));
#if TK_DEBUG
        LogInfo("Extra Development Memory: %llu", static_cast<unsigned long long>(memStats.extraDevelopmentMem));
#endif

        LogInfo("Main thread id: 0x%llx, Process id: %llu", static_cast<unsigned long long>(TkThread::getMainThreadId()), static_cast<unsigned long long>(System::getCurrentProcessId()));
    }
}
