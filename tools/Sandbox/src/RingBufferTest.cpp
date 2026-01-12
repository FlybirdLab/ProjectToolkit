/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "RingBufferTest.hpp"

namespace toolkit
{
    struct TestObj
    {
        static i32 ctorCount;
        static i32 dtorCount;

        i32 value;

        TestObj(i32 v = 0) : value(v)
        {
            ++ctorCount;
        }

        TestObj(const TestObj &o) : value(o.value)
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

    void ringBufferBasic()
    {
        TkRingBuffer<int> rb;

        rb.pushBack(1);
        rb.pushBack(2);
        rb.pushBack(3);

        TK_ASSERT(rb.count() == 3);
        TK_ASSERT(rb[0] == 1);
        TK_ASSERT(rb[1] == 2);
        TK_ASSERT(rb[2] == 3);

        rb.popFront();
        TK_ASSERT(rb.count() == 2);
        TK_ASSERT(rb[0] == 2);
        TK_ASSERT(rb[1] == 3);
    }

    void ringBufferWrap()
    {
        TkRingBuffer<int> rb;

        for (int i = 0; i < 8; ++i)
            rb.pushBack(i);

        for (int i = 0; i < 4; ++i)
            rb.popFront();

        for (int i = 8; i < 12; ++i)
            rb.pushBack(i);

        TK_ASSERT(rb.count() == 8);

        for (int i = 0; i < rb.count(); ++i)
        {
            TK_ASSERT(rb[i] == i + 4);
        }
    }

    void ringBufferObjectLifetime()
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

        TK_ASSERT(TestObj::ctorCount == TestObj::dtorCount);
    }

    void ringBufferStress()
    {
        TkRingBuffer<int> rb;

        int ref[1024];
        int refCount = 0;

        for (int i = 0; i < 100000; ++i)
        {
            if ((rand() & 1) && refCount > 0)
            {
                rb.popFront();
                --refCount;
                for (int j = 0; j < refCount; ++j)
                    ref[j] = ref[j + 1];
            }
            else
            {
                int v = rand();
                rb.pushBack(v);
                ref[refCount++] = v;
            }

            TK_ASSERT(rb.count() == refCount);
            for (int j = 0; j < refCount; ++j)
                TK_ASSERT(rb[j] == ref[j]);
        }
    }

    void testRingBuffer()
    {
        ringBufferBasic();
        ringBufferWrap();
        ringBufferObjectLifetime();
        ringBufferStress();
    }
}