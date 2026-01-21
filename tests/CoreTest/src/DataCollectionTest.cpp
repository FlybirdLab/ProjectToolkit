/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include "toolkit/collection/DataCollection.hpp"

TEST(DataCollection, Base)
{
    int src[4] = {1, 2, 3, 4};

    TkDataCollection<int> a;
    a.link(src, 4);

    EXPECT_FALSE(a.isAllocated());
    EXPECT_EQ(a.length(), 4);
    EXPECT_EQ(a[2], 3);

    TkDataCollection<int> b;
    b.copy(a);

    EXPECT_TRUE(b.isAllocated());
    EXPECT_EQ(b.length(), 4);
    EXPECT_EQ(b[3], 4);
}

TEST(DataCollection, Copy)
{
    int src[2] = {10, 20};

    TkDataCollection<int> a;
    a.copy(src, 2);

    TkDataCollection<int> b = a;

    a[0] = 99;

    EXPECT_EQ(b[0], 10);
}

TEST(DataCollection, Move)
{
    int src[3] = {1, 2, 3};

    TkDataCollection<int> a;
    a.copy(src, 3);

    TkDataCollection<int> b = std::move(a);

    EXPECT_EQ(a.length(), 0);
    EXPECT_EQ(b.length(), 3);
    EXPECT_EQ(b[1], 2);
}

TEST(DataCollection, Append)
{
    int a[2] = {1, 2};
    int b[2] = {3, 4};

    TkDataCollection<int> c;
    c.link(a, 2);
    c.append(b, 2);

    EXPECT_TRUE(c.isAllocated());
    EXPECT_EQ(c.length(), 4);
    EXPECT_EQ(c[0], 1);
    EXPECT_EQ(c[3], 4);
}

TEST(DataCollection, AppendChain)
{
    TkDataCollection<int> c;

    int a = 1;
    int b = 2;
    int d = 3;

    c.append(&a, 1);
    c.append(&b, 1);
    c.append(&d, 1);

    EXPECT_EQ(c.length(), 3);
    EXPECT_EQ(c[2], 3);
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

TEST(DataCollection, Stream)
{
    MockStream s;

    int src[3] = {7, 8, 9};

    TkDataCollection<int> a;
    a.copy(src, 3);
    a.write(&s);

    s.offset = 0;

    TkDataCollection<int> b;
    b.read(&s, 3);

    EXPECT_EQ(b.length(), 3);
    EXPECT_EQ(b[2], 9);
}
