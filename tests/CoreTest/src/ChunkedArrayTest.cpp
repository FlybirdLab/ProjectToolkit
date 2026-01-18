/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include "toolkit/collection/ChunkedArray.hpp"

/* ================= Test Object ================= */

struct TestObjA
{
    static inline i32 ctorCount = 0;
    static inline i32 dtorCount = 0;
    static inline i32 copyCount = 0;

    i32 value = 0;

    TestObjA()
    {
        ++ctorCount;
    }

    explicit TestObjA(i32 v)
        : value(v)
    {
        ++ctorCount;
    }

    TestObjA(const TestObjA &o)
        : value(o.value)
    {
        ++copyCount;
        ++ctorCount;
    }

    ~TestObjA()
    {
        ++dtorCount;
    }

    bool operator==(const TestObjA &o) const
    {
        return value == o.value;
    }

    static void reset()
    {
        ctorCount = dtorCount = copyCount = 0;
    }
};

/* ================= Basic ================= */

TEST(ChunkedArray, Basic)
{
    TestObjA::reset();

    TkChunkedArray<TestObjA, 4> arr;

    EXPECT_TRUE(arr.isEmpty());
    EXPECT_EQ(arr.count(), 0);
    EXPECT_EQ(arr.capacity(), 0);

    for (i32 i = 0; i < 10; ++i)
        arr.add(TestObjA(i));

    EXPECT_EQ(arr.count(), 10);
    EXPECT_GE(arr.capacity(), 10);

    for (i32 i = 0; i < arr.count(); ++i)
        EXPECT_EQ(arr[i].value, i);

    // iterator sequence
    i32 expect = 0;
    for (auto it = arr.begin(); it != arr.end(); ++it)
    {
        EXPECT_EQ(it->value, expect);
        ++expect;
    }

    EXPECT_EQ(expect, 10);
}

/* ================= Remove ================= */

TEST(ChunkedArray, Remove)
{
    TestObjA::reset();

    TkChunkedArray<TestObjA, 4> arr;
    for (i32 i = 0; i < 9; ++i)
        arr.add(TestObjA(i));

    auto it = arr.iteratorAt(3);
    arr.remove(it);

    EXPECT_EQ(arr.count(), 8);

    // index 3 should be last element (swap-remove semantics)
    EXPECT_EQ(arr[3].value, 8);

    bool found[9] = {};
    for (i32 i = 0; i < arr.count(); ++i)
        found[arr[i].value] = true;

    EXPECT_FALSE(found[3]);
    for (i32 i = 0; i < 9; ++i)
    {
        if (i == 3)
            continue;
        EXPECT_TRUE(found[i]);
    }
}

/* ================= Resize ================= */

TEST(ChunkedArray, Resize)
{
    TestObjA::reset();

    TkChunkedArray<TestObjA, 4> arr;

    arr.resize(10);
    EXPECT_EQ(arr.count(), 10);

    arr.resize(3);
    EXPECT_EQ(arr.count(), 3);

    arr.resize(16);
    EXPECT_EQ(arr.count(), 16);
    EXPECT_GE(arr.capacity(), 16);
}

/* ================= Lifetime ================= */

TEST(ChunkedArray, Lifetime)
{
    TestObjA::reset();

    {
        TkChunkedArray<TestObjA, 4> arr;
        for (i32 i = 0; i < 10; ++i)
            arr.add(TestObjA(i));

        arr.clear();
        EXPECT_EQ(arr.count(), 0);
    }

    // All constructed objects should be destroyed
    EXPECT_GE(TestObjA::dtorCount, TestObjA::ctorCount);
}

/* ================= Iterator Edge ================= */

TEST(ChunkedArray, IteratorEdge)
{
    TkChunkedArray<i32, 3> arr;

    EXPECT_EQ(arr.begin(), arr.end());

    arr.add(1);
    auto it = arr.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(it, arr.end());
}

TEST(ChunkedArray, CopyMove)
{
    TkChunkedArray<i32, 4> a;
    a.add(1);
    a.add(2);

    TkChunkedArray<i32, 4> b = a;
    EXPECT_EQ(b.count(), 2);

    TkChunkedArray<i32, 4> c = std::move(a);
    EXPECT_EQ(c.count(), 2);
    EXPECT_EQ(a.count(), 0);
}
