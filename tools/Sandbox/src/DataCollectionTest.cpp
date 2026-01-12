/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "DataCollectionTest.hpp"

#include <toolkit/collection/DataCollection.hpp>

namespace toolkit
{
    void testBase()
    {
        int src[4] = {1, 2, 3, 4};

        TkDataCollection<int> a;
        a.link(src, 4);

        TK_ASSERT(!a.isAllocated());
        TK_ASSERT(a.length() == 4);
        TK_ASSERT(a[2] == 3);

        TkDataCollection<int> b;
        b.copy(a);

        TK_ASSERT(b.isAllocated());
        TK_ASSERT(b.length() == 4);
        TK_ASSERT(b[3] == 4);
    }

    void testCopy()
    {
        int src[2] = {10, 20};

        TkDataCollection<int> a;
        a.copy(src, 2);

        TkDataCollection<int> b = a;

        a[0] = 99;

        TK_ASSERT(b[0] == 10);
    }

    void testMove()
    {
        int src[3] = {1, 2, 3};

        TkDataCollection<int> a;
        a.copy(src, 3);

        TkDataCollection<int> b = std::move(a);

        TK_ASSERT(a.length() == 0);
        TK_ASSERT(b.length() == 3);
        TK_ASSERT(b[1] == 2);
    }

    void testAppend()
    {
        int a[2] = {1, 2};
        int b[2] = {3, 4};

        TkDataCollection<int> c;
        c.link(a, 2);
        c.append(b, 2);

        TK_ASSERT(c.isAllocated());
        TK_ASSERT(c.length() == 4);
        TK_ASSERT(c[0] == 1);
        TK_ASSERT(c[3] == 4);
    }

    void testAppendChain()
    {
        TkDataCollection<int> c;

        int a = 1;
        int b = 2;
        int d = 3;

        c.append(&a, 1);
        c.append(&b, 1);
        c.append(&d, 1);

        TK_ASSERT(c.length() == 3);
        TK_ASSERT(c[2] == 3);
    }

    struct MockStream
    {
        byte buffer[64];
        i32 offset = 0;

        void writeBytes(void *data, i32 size)
        {
            memcpy(buffer + offset, data, size);
            offset += size;
        }

        void readBytes(void *data, i32 size)
        {
            memcpy(data, buffer + offset, size);
            offset += size;
        }
    };

    void testStream()
    {
        MockStream s;

        int src[3] = {7, 8, 9};

        TkDataCollection<int> a;
        a.copy(src, 3);
        a.write(&s);

        s.offset = 0;

        TkDataCollection<int> b;
        b.read(&s, 3);

        TK_ASSERT(b.length() == 3);
        TK_ASSERT(b[2] == 9);
    }

    void testDataCollection()
    {
        testBase();
        testMove();
        testCopy();
        testAppend();
        testAppendChain();
        testStream();
    }

}