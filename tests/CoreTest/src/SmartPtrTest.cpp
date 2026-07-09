/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: khfahqp khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"
#include "TestAllocator.hpp"

using namespace toolkit;

namespace
{
    struct SmartPtrTracker
    {
        static i32 constructed;
        static i32 destructed;

        i32 value = 0;

        explicit SmartPtrTracker(i32 v)
            : value(v)
        {
            ++constructed;
        }

        ~SmartPtrTracker()
        {
            ++destructed;
        }

        static void reset()
        {
            constructed = 0;
            destructed = 0;
        }
    };

    i32 SmartPtrTracker::constructed = 0;
    i32 SmartPtrTracker::destructed = 0;
}

TEST(SmartPtr, MakeUniqueUsesAllocatorAndDestroysObject)
{
    SmartPtrTracker::reset();
    TestAllocator testAllocator;
    IAllocator alloc = testAllocator.asInterface();

    {
        auto ptr = makeUnique<SmartPtrTracker>(alloc, 42);
        ASSERT_NE(ptr.get(), nullptr);
        EXPECT_EQ(ptr->value, 42);
        EXPECT_EQ(SmartPtrTracker::constructed, 1);
        EXPECT_EQ(SmartPtrTracker::destructed, 0);
        EXPECT_EQ(testAllocator.allocCount, 1);
        EXPECT_EQ(testAllocator.freeCount, 0);
    }

    EXPECT_EQ(SmartPtrTracker::destructed, 1);
    EXPECT_EQ(testAllocator.freeCount, 1);
}

TEST(SmartPtr, MakeSharedAllocatesControlBlockFromAllocator)
{
    SmartPtrTracker::reset();
    TestAllocator testAllocator;
    IAllocator alloc = testAllocator.asInterface();

    TkWeakPtr<SmartPtrTracker> weak;
    {
        auto ptr = makeShared<SmartPtrTracker>(alloc, 7);
        weak = makeWeak(ptr);

        ASSERT_NE(ptr.get(), nullptr);
        EXPECT_EQ(ptr->value, 7);
        EXPECT_EQ(ptr.use_count(), 1);
        EXPECT_FALSE(expired(weak));
        EXPECT_EQ(lockWeak(weak)->value, 7);
        EXPECT_EQ(testAllocator.allocCount, 1);
        EXPECT_EQ(testAllocator.freeCount, 0);
    }

    EXPECT_EQ(SmartPtrTracker::destructed, 1);
    EXPECT_TRUE(expired(weak));
    EXPECT_EQ(testAllocator.freeCount, 0);

    weak.reset();
    EXPECT_EQ(testAllocator.freeCount, 1);
}

TEST(SmartPtr, PublicAliasesMatchToolkitTypes)
{
    static_assert(std::is_same_v<UniquePtr<SmartPtrTracker>, TkUniquePtr<SmartPtrTracker>>);
    static_assert(std::is_same_v<SharedPtr<SmartPtrTracker>, TkSharedPtr<SmartPtrTracker>>);
    static_assert(std::is_same_v<WeakPtr<SmartPtrTracker>, TkWeakPtr<SmartPtrTracker>>);
}
