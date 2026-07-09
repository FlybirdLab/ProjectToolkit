/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "toolkit/collection/RingBuffer.hpp"
#include "common.hpp"

#include <cstdlib>

struct TestObj
{
    static i32 ctorCount;
    static i32 dtorCount;

    i32 value;

    TestObj(i32 v = 0) : value(v)
    {
        ++ctorCount;
    }

    TestObj(const TestObj& o) : value(o.value)
    {
        ++ctorCount;
    }

    ~TestObj()
    {
        ++dtorCount;
    }
};

i32 TestObj::ctorCount = 0;
i32 TestObj::dtorCount = 0;

TEST(RingBufferTest, Basic)
{
    TkRingBuffer<int> rb;

    rb.pushBack(1);
    rb.pushBack(2);
    rb.pushBack(3);

    ASSERT_EQ(rb.count(), 3);
    EXPECT_EQ(rb[0], 1);
    EXPECT_EQ(rb[1], 2);
    EXPECT_EQ(rb[2], 3);

    rb.popFront();

    ASSERT_EQ(rb.count(), 2);
    EXPECT_EQ(rb[0], 2);
    EXPECT_EQ(rb[1], 3);
}

TEST(RingBufferTest, WrapAround)
{
    TkRingBuffer<int> rb;

    for (int i = 0; i < 8; ++i)
        rb.pushBack(i);

    for (int i = 0; i < 4; ++i)
        rb.popFront();

    for (int i = 8; i < 12; ++i)
        rb.pushBack(i);

    ASSERT_EQ(rb.count(), 8);

    for (int i = 0; i < rb.count(); ++i)
    {
        EXPECT_EQ(rb[i], i + 4);
    }
}

TEST(RingBufferTest, ObjectLifetime)
{
    TestObj::ctorCount = 0;
    TestObj::dtorCount = 0;

    {
        TkRingBuffer<TestObj> rb;

        for (int i = 0; i < 1000; ++i)
            rb.pushBack(TestObj(i));

        for (int i = 0; i < 500; ++i)
            rb.popFront();

        rb.clear();
    }

    EXPECT_EQ(TestObj::ctorCount, TestObj::dtorCount)
        << "Ctor/Dtor count mismatch, possible leak or double free";
}

TEST(RingBufferTest, Stress)
{
    TkRingBuffer<int> rb;

    int ref[1024];
    int refCount = 0;

    constexpr int kIterations = 100000;

    for (int i = 0; i < kIterations; ++i)
    {
        if ((std::rand() & 1) && refCount > 0)
        {
            rb.popFront();
            --refCount;

            for (int j = 0; j < refCount; ++j)
                ref[j] = ref[j + 1];
        }
        else
        {
            int v = std::rand();
            rb.pushBack(v);
            ref[refCount++] = v;
        }

        ASSERT_EQ(rb.count(), refCount);

        for (int j = 0; j < refCount; ++j)
        {
            ASSERT_EQ(rb[j], ref[j])
                << "Mismatch at iteration " << i
                << ", index " << j;
        }
    }
}
