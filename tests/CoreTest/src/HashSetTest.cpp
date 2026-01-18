/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include "toolkit/collection/HashSet.hpp"

// ------------------------------------------------------------
// Basic add / contains
// ------------------------------------------------------------
TEST(HashSetTest, Basic)
{
    TkHashSet<int> s;
    EXPECT_TRUE(s.isEmpty());

    s.add(1);
    s.add(2);
    s.add(3);

    EXPECT_EQ(s.count(), 3);
    EXPECT_TRUE(s.contains(1));
    EXPECT_TRUE(s.contains(2));
    EXPECT_TRUE(s.contains(3));
    EXPECT_FALSE(s.contains(4));
}

// ------------------------------------------------------------
// Duplicate insert
// ------------------------------------------------------------
TEST(HashSetTest, Duplicate)
{
    TkHashSet<int> s;

    EXPECT_TRUE(s.add(42));
    EXPECT_FALSE(s.add(42));
    EXPECT_EQ(s.count(), 1);
}

// ------------------------------------------------------------
// Remove
// ------------------------------------------------------------
TEST(HashSetTest, Remove)
{
    TkHashSet<int> s;

    for (int i = 0; i < 100; ++i)
        s.add(i);

    for (int i = 0; i < 50; ++i)
        EXPECT_TRUE(s.remove(i));

    for (int i = 0; i < 50; ++i)
        EXPECT_FALSE(s.contains(i));

    for (int i = 50; i < 100; ++i)
        EXPECT_TRUE(s.contains(i));

    EXPECT_EQ(s.count(), 50);
}

// ------------------------------------------------------------
// Rehash / large volume
// ------------------------------------------------------------
TEST(HashSetTest, Rehash)
{
    TkHashSet<int> s;

    for (int i = 0; i < 10000; ++i)
        s.add(i);

    for (int i = 0; i < 10000; ++i)
        ASSERT_TRUE(s.contains(i));

    for (int i = 0; i < 5000; ++i)
        s.remove(i);

    for (int i = 0; i < 5000; ++i)
        EXPECT_FALSE(s.contains(i));

    for (int i = 5000; i < 10000; ++i)
        EXPECT_TRUE(s.contains(i));
}

// ------------------------------------------------------------
// Object lifetime
// ------------------------------------------------------------
struct TestObjB
{
    static int Alive;
    int v;

    explicit TestObjB(int v) : v(v) { ++Alive; }
    TestObjB(const TestObjB& o) : v(o.v) { ++Alive; }
    ~TestObjB() { --Alive; }

    bool operator==(const TestObjB& o) const { return v == o.v; }
};

namespace toolkit
{
    template <>
    struct HashFunc<TestObjB>
    {
        static u32 get(const TestObjB& k)
        {
            return HashFunc<i32>::get(k.v);
        }
    };
}

int TestObjB::Alive = 0;

TEST(HashSetTest, Lifetime)
{
    {
        TkHashSet<TestObjB> s;

        for (int i = 0; i < 100; ++i)
            s.add(TestObjB(i));

        EXPECT_EQ(TestObjB::Alive, 100);

        for (int i = 0; i < 50; ++i)
            s.remove(TestObjB(i));

        EXPECT_EQ(TestObjB::Alive, 50);
    }

    EXPECT_EQ(TestObjB::Alive, 0);
}