/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include "toolkit/collection/Array.hpp"

using namespace toolkit;

TEST(TkArray, DefaultConstructor)
{
    TkArray<int> a;
    EXPECT_EQ(a.count(), 0);
    EXPECT_EQ(a.capacity(), 0);
    EXPECT_TRUE(a.isEmpty());
    EXPECT_FALSE(a.hasItems());
    EXPECT_EQ(a.get(), nullptr);
}

TEST(TkArray, CapacityConstructor)
{
    TkArray<int> a(5);
    EXPECT_EQ(a.count(), 0);
    EXPECT_EQ(a.capacity(), 5);
    EXPECT_TRUE(a.isEmpty());
}

TEST(TkArray, InitializerListConstructor)
{
    TkArray<int> a{1, 2, 3};
    EXPECT_EQ(a.count(), 3);
    EXPECT_EQ(a[0], 1);
    EXPECT_EQ(a[1], 2);
    EXPECT_EQ(a[2], 3);
}

TEST(TkArray, PointerConstructor)
{
    int arr[] = {5, 6, 7};
    TkArray<int> a(arr, 3);
    EXPECT_EQ(a.count(), 3);
    EXPECT_EQ(a[0], 5);
    EXPECT_EQ(a[2], 7);
}

TEST(TkArray, CopyConstructor)
{
    TkArray<int> a{1, 2, 3};
    TkArray<int> b(a);
    EXPECT_EQ(b.count(), 3);
    EXPECT_EQ(b[1], 2);

    b[1] = 42;
    EXPECT_EQ(a[1], 2); // ensure deep copy
}

TEST(TkArray, MoveConstructor)
{
    TkArray<int> a{1, 2, 3};
    TkArray<int> b(std::move(a));
    EXPECT_EQ(b.count(), 3);
    EXPECT_EQ(a.count(), 0);
}

TEST(TkArray, AssignmentOperator)
{
    TkArray<int> a{1, 2, 3};
    TkArray<int> b;
    b = a;
    EXPECT_EQ(b.count(), 3);
    EXPECT_EQ(b[2], 3);
}

TEST(TkArray, MoveAssignment)
{
    TkArray<int> a{1, 2, 3};
    TkArray<int> b;
    b = std::move(a);
    EXPECT_EQ(b.count(), 3);
    EXPECT_EQ(a.count(), 0);
}

TEST(TkArray, AddAndAccess)
{
    TkArray<int> a;
    a.add(10);
    a.add(20);
    EXPECT_EQ(a.count(), 2);
    EXPECT_EQ(a[0], 10);
    EXPECT_EQ(a.last(), 20);
    EXPECT_EQ(a.first(), 10);
}

TEST(TkArray, Insert)
{
    TkArray<int> a{1, 3};
    a.insert(1, 2);
    EXPECT_EQ(a.count(), 3);
    EXPECT_EQ(a[0], 1);
    EXPECT_EQ(a[1], 2);
    EXPECT_EQ(a[2], 3);

    a.insert(0, 0);
    EXPECT_EQ(a[0], 0);
}

TEST(TkArray, Remove)
{
    TkArray<int> a{1, 2, 3, 2};
    EXPECT_TRUE(a.removeKeepOrder(2));
    EXPECT_EQ(a.count(), 3);
    EXPECT_EQ(a[1], 3);

    a.removeAtKeepOrder(0);
    EXPECT_EQ(a.count(), 2);
    EXPECT_EQ(a[0], 3);

    a.removeAllKeepOrder(2);
    EXPECT_EQ(a.count(), 1);
}

TEST(TkArray, StackOperations)
{
    TkArray<int> a;
    a.push(1);
    a.push(2);
    EXPECT_EQ(a.pop(), 2);
    EXPECT_EQ(a.pop(), 1);
}

TEST(TkArray, QueueOperations)
{
    TkArray<int> a;
    a.enqueue(1);
    a.enqueue(2);
    EXPECT_EQ(a.dequeue(), 1);
    EXPECT_EQ(a.dequeue(), 2);
}

TEST(TkArray, SetAll)
{
    TkArray<int> a{1, 2, 3};
    a.setAll(7);
    EXPECT_EQ(a[0], 7);
    EXPECT_EQ(a[2], 7);
}

TEST(TkArray, ResizeAndEnsureCapacity)
{
    TkArray<int> a;
    a.resize(5);
    EXPECT_EQ(a.count(), 5);

    a.ensureCapacity(10);
    EXPECT_GE(a.capacity(), 10);
}

TEST(TkArray, AddZeroedAndAddDefault)
{
    TkArray<int> a;
    a.addZeroed(3);
    EXPECT_EQ(a.count(), 3);
    for (int i = 0; i < 3; ++i)
        EXPECT_EQ(a[i], 0);

    TkArray<int> b;
    b.addDefault(2);
    EXPECT_EQ(b.count(), 2);
}

TEST(TkArray, ContainsAndFind)
{
    TkArray<int> a{1, 2, 3};
    EXPECT_TRUE(a.contains(2));
    EXPECT_FALSE(a.contains(4));

    int index;
    EXPECT_TRUE(a.find(3, index));
    EXPECT_EQ(index, 2);
    EXPECT_FALSE(a.find(5, index));
}

TEST(TkArray, Iterators)
{
    TkArray<int> a{1, 2, 3};
    int sum = 0;
    for (auto it = a.Begin(); it != a.End(); ++it)
        sum += *it;
    EXPECT_EQ(sum, 6);
}

TEST(TkArray, ReverseAndSwap)
{
    TkArray<int> a{1, 2, 3};
    TkArray<int> b{4, 5, 6};
    a.swap(b);
    EXPECT_EQ(a[0], 4);
    EXPECT_EQ(b[0], 1);

    a.reverse();
    EXPECT_EQ(a[0], 6);
    EXPECT_EQ(a[2], 4);
}
