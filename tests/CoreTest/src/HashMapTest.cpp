/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include "toolkit/collection/HashMap.hpp"

struct TestValue
{
    i32 x;

    TestValue() : x(0) {}
    explicit TestValue(i32 v) : x(v) {}

    bool operator==(const TestValue& rhs) const
    {
        return x == rhs.x;
    }
};

// ------------------------------------------------------------
// Basic insert / find / at
// ------------------------------------------------------------
TEST(HashMapTest, BasicInsertFind)
{
    TkHashMap<i32, TestValue> map;

    map.add(1, TestValue{10});
    map.add(2, TestValue{20});
    map.add(3, TestValue{30});

    ASSERT_EQ(map.count(), 3);

    EXPECT_TRUE(map.containsKey(1));
    EXPECT_TRUE(map.containsKey(2));
    EXPECT_TRUE(map.containsKey(3));

    EXPECT_EQ(map.at(1).x, 10);
    EXPECT_EQ(map.at(2).x, 20);
    EXPECT_EQ(map.at(3).x, 30);
}

// ------------------------------------------------------------
// operator[] + at
// ------------------------------------------------------------
TEST(HashMapTest, AtAndOperator)
{
    TkHashMap<i32, i32> map;

    map[10] = 100;
    map[20] = 200;

    ASSERT_EQ(map.count(), 2);
    EXPECT_EQ(map.at(10), 100);
    EXPECT_EQ(map.at(20), 200);
}

// ------------------------------------------------------------
// remove + compact
// ------------------------------------------------------------
TEST(HashMapTest, RemoveAndCompact)
{
    TkHashMap<i32, i32> map;

    for (i32 i = 0; i < 64; ++i)
        map.add(i, i * 10);

    for (i32 i = 0; i < 64; i += 2)
        map.remove(i);

    ASSERT_EQ(map.count(), 32);

    // force compact
    map.compact();

    for (i32 i = 1; i < 64; i += 2)
        EXPECT_TRUE(map.containsKey(i));

    for (i32 i = 0; i < 64; i += 2)
        EXPECT_FALSE(map.containsKey(i));
}

// ------------------------------------------------------------
// rehash / grow
// ------------------------------------------------------------
TEST(HashMapTest, Rehash)
{
    TkHashMap<i32, i32> map;

    map.setCapacity(8);

    for (i32 i = 0; i < 100; ++i)
        map.add(i, i);

    ASSERT_EQ(map.count(), 100);

    for (i32 i = 0; i < 100; ++i)
        ASSERT_EQ(map.at(i), i);
}

// ------------------------------------------------------------
// iterator
// ------------------------------------------------------------
TEST(HashMapTest, Iterator)
{
    TkHashMap<i32, i32> map;

    for (i32 i = 0; i < 10; ++i)
        map.add(i, i * 2);

    i32 sum = 0;
    for (auto it = map.Begin(); it.isNotEnd(); ++it)
        sum += it->Value;

    EXPECT_EQ(sum, 2 * (0 + 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9));
}

// ------------------------------------------------------------
// copy / move
// ------------------------------------------------------------
TEST(HashMapTest, CopyAndMove)
{
    TkHashMap<i32, i32> a;
    for (i32 i = 0; i < 16; ++i)
        a.add(i, i);

    // copy ctor
    TkHashMap<i32, i32> b = a;
    ASSERT_EQ(b.count(), a.count());
    for (i32 i = 0; i < 16; ++i)
        EXPECT_EQ(b.at(i), i);

    // move ctor
    TkHashMap<i32, i32> c = std::move(a);
    EXPECT_EQ(c.count(), 16);

    // move assign
    TkHashMap<i32, i32> d;
    d = std::move(c);
    EXPECT_EQ(d.count(), 16);
}

// ------------------------------------------------------------
// clone with different allocator
// ------------------------------------------------------------
#if 0
TEST(HashMapTest, CloneDifferentAllocator)
{
    ArenaAllocator arenaA(...);
    ArenaAllocator arenaB(...);

    TkHashMap<i32, i32> a(arenaA);
    for (i32 i = 0; i < 32; ++i)
        a.add(i, i * 3);

    TkHashMap<i32, i32> b(arenaB);
    b = a;

    ASSERT_EQ(b.count(), a.count());
    for (i32 i = 0; i < 32; ++i)
        EXPECT_EQ(b.at(i), i * 3);
}
#endif