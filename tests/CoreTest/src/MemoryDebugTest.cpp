/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: khfahqp khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"
#include "toolkit/memory/Memory.hpp"

using namespace toolkit;

TEST(MemoryDebug, ValidateAllocationAcceptsLiveAllocation)
{
    IAllocator &alloc = TkMemory::getDefault();
    void *ptr = TkMemory::allocate(alloc, 32, 16);

    ASSERT_NE(ptr, nullptr);
    EXPECT_TRUE(TkMemory::validateAllocation(ptr));
    EXPECT_TRUE(TkMemory::validateAllocations());

    TkMemory::free(alloc, ptr);
}

TEST(MemoryDebug, ValidateAllocationDetectsTailOverwrite)
{
#if TK_DEBUG
    IAllocator &alloc = TkMemory::getDefault();
    auto *ptr = static_cast<u8 *>(TkMemory::allocate(alloc, 16, 16));

    ASSERT_NE(ptr, nullptr);
    EXPECT_TRUE(TkMemory::validateAllocation(ptr));

    ptr[16] = 0;
    EXPECT_FALSE(TkMemory::validateAllocation(ptr));

    ptr[16] = 0xFD;
    EXPECT_TRUE(TkMemory::validateAllocation(ptr));
    TkMemory::free(alloc, ptr);
#else
    GTEST_SKIP() << "Memory guard checks are debug-only.";
#endif
}

TEST(MemoryDebug, ReallocatePreservesBytesAndTracking)
{
    IAllocator &alloc = TkMemory::getDefault();
    auto *ptr = static_cast<u8 *>(TkMemory::allocate(alloc, 8, 16));

    ASSERT_NE(ptr, nullptr);
    for (u8 i = 0; i < 8; ++i)
        ptr[i] = i;

    auto *reallocated = static_cast<u8 *>(TkMemory::reallocate(alloc, ptr, 32, 8, 16));
    ASSERT_NE(reallocated, nullptr);
    EXPECT_TRUE(TkMemory::validateAllocation(reallocated));

    for (u8 i = 0; i < 8; ++i)
        EXPECT_EQ(reallocated[i], i);

    TkMemory::free(alloc, reallocated);
}
