/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: khfahqp khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"
#include "TestAllocator.hpp"

#include <type_traits>

using namespace toolkit;

TEST(AllocatorDebug, InterfaceInvalidAfterAllocatorSourceDies)
{
    IAllocator *interfaceCopy = nullptr;
    {
        TestAllocator allocator;
        IAllocator interface = allocator.asInterface();

        EXPECT_TRUE(TkMemory::isValid(interface));
        interfaceCopy = new IAllocator(interface);
    }

#if TK_DEBUG
    EXPECT_FALSE(TkMemory::isValid(*interfaceCopy));
#endif
    delete interfaceCopy;
}

TEST(AllocatorDebug, InterfaceGenerationRejectsReusedAllocatorAddress)
{
    using Storage = std::aligned_storage_t<sizeof(TestAllocator), alignof(TestAllocator)>;

    Storage storage;
    auto *first = new (&storage) TestAllocator();
    IAllocator staleInterface = first->asInterface();
    EXPECT_TRUE(TkMemory::isValid(staleInterface));
    first->~TestAllocator();

    auto *second = new (&storage) TestAllocator();
    IAllocator currentInterface = second->asInterface();

#if TK_DEBUG
    EXPECT_FALSE(TkMemory::isValid(staleInterface));
#endif
    EXPECT_TRUE(TkMemory::isValid(currentInterface));

    second->~TestAllocator();
}
