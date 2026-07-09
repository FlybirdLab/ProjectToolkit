/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include <toolkit/reflect/invoking/CallFrame.hpp>

namespace
{
    struct DtorProbe
    {
        i32 id = 0;
        TkArray<i32>* order = nullptr;

        DtorProbe(i32 inId, TkArray<i32>& inOrder)
            : id(inId), order(&inOrder)
        {
        }

        ~DtorProbe()
        {
            order->add(id);
        }
    };

    struct LargeObject
    {
        byte data[2048] = {};
    };
}

TEST(ReflectCallFrameTest, AllocatesSmallAndLargeObjects)
{
    CallFrame frame;
    auto* a = frame.emplace<i32>(12);
    auto* b = frame.emplace<i32>(34);
    auto* large = frame.emplace<LargeObject>();

    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    ASSERT_NE(large, nullptr);
    EXPECT_EQ(*a, 12);
    EXPECT_EQ(*b, 34);
}

TEST(ReflectCallFrameTest, ResetCallsDestructorsInReverseOrder)
{
    TkArray<i32> order;
    CallFrame frame;
    frame.emplace<DtorProbe>(1, order);
    frame.emplace<DtorProbe>(2, order);
    frame.reset();

    ASSERT_EQ(order.count(), 2);
    EXPECT_EQ(order[0], 2);
    EXPECT_EQ(order[1], 1);

    auto* value = frame.emplace<i32>(9);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 9);
}

TEST(ReflectCallFrameTest, DestructorResetsAutomatically)
{
    TkArray<i32> order;
    {
        CallFrame frame;
        frame.emplace<DtorProbe>(7, order);
    }
    ASSERT_EQ(order.count(), 1);
    EXPECT_EQ(order[0], 7);
}
