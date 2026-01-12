/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "HashMapTest.hpp"

namespace toolkit
{
    struct TestValue
    {
        i32 x;

        TestValue() : x(0) {}
        explicit TestValue(i32 v) : x(v) {}

        bool operator==(const TestValue &rhs) const
        {
            return x == rhs.x;
        }
    };

    void basicInsertFind()
    {
        TkHashMap<i32, TestValue> map;

        map.add(1, TestValue{10});
        map.add(2, TestValue{20});
        map.add(3, TestValue{30});

        TK_ASSERT(map.count() == 3);
        TK_ASSERT(map.containsKey(1));
        TK_ASSERT(map.containsKey(2));
        TK_ASSERT(map.containsKey(3));

        TK_ASSERT(map.at(1).x == 10);
        TK_ASSERT(map.at(2).x == 20);
        TK_ASSERT(map.at(3).x == 30);
    }

    void atAndOperator()
    {
        TkHashMap<i32, i32> map;

        map[10] = 100;
        map[20] = 200;

        TK_ASSERT(map.count() == 2);
        TK_ASSERT(map.at(10) == 100);
        TK_ASSERT(map.at(20) == 200);
    }

    void removeAndCompact()
    {
        TkHashMap<i32, i32> map;

        for (i32 i = 0; i < 64; ++i)
            map.add(i, i * 10);

        for (i32 i = 0; i < 64; i += 2)
            map.remove(i);

        TK_ASSERT(map.count() == 32);

        // force compact
        map.compact();

        for (i32 i = 1; i < 64; i += 2)
            TK_ASSERT(map.containsKey(i));

        for (i32 i = 0; i < 64; i += 2)
            TK_ASSERT(!map.containsKey(i));
    }

    void rehash()
    {
        TkHashMap<i32, i32> map;

        map.setCapacity(8);
        for (i32 i = 0; i < 100; ++i)
            map.add(i, i);

        TK_ASSERT(map.count() == 100);

        for (i32 i = 0; i < 100; ++i)
            TK_ASSERT(map.at(i) == i);
    }

    void iterator()
    {
        TkHashMap<i32, i32> map;

        for (i32 i = 0; i < 10; ++i)
            map.add(i, i * 2);

        i32 sum = 0;
        for (auto it = map.Begin(); it.isNotEnd(); ++it)
            sum += it->Value;

        TK_ASSERT(sum == 2 * (0 + 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9));
    }

    void copyAndMove()
    {
        TkHashMap<i32, i32> a;
        for (i32 i = 0; i < 16; ++i)
            a.add(i, i);

        // copy ctor
        TkHashMap<i32, i32> b = a;
        TK_ASSERT(b.count() == a.count());
        for (i32 i = 0; i < 16; ++i)
            TK_ASSERT(b.at(i) == i);

        // move ctor
        TkHashMap<i32, i32> c = std::move(a);
        TK_ASSERT(c.count() == 16);

        // move assign
        TkHashMap<i32, i32> d;
        d = std::move(c);
        TK_ASSERT(d.count() == 16);
    }

    void cloneDifferentAllocator(IAllocator &allocA, IAllocator &allocB)
    {
        TkHashMap<i32, i32> a(allocA);
        for (i32 i = 0; i < 32; ++i)
            a.add(i, i * 3);

        TkHashMap<i32, i32> b(allocB);
        b = a;

        TK_ASSERT(b.count() == a.count());
        for (i32 i = 0; i < 32; ++i)
            TK_ASSERT(b.at(i) == i * 3);
    }

    void testHashMap()
    {
        basicInsertFind();
        atAndOperator();
        removeAndCompact();
        rehash();
        iterator();
        copyAndMove();

        // ArenaAllocator arenaA(...);
        // ArenaAllocator arenaB(...);
        // Test_CloneDifferentAllocator(arenaA, arenaB);
    }
}