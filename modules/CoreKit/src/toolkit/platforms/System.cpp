/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "System.hpp"

#include <toolkit/base/Assert.hpp>
#include <toolkit/threading/Sync.hpp>

#if TK_WINDOWS
#include <toolkit/platforms/win32/Win32System.hpp>
#endif

namespace toolkit
{

    bool System::init()
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

        return false;
    }
}