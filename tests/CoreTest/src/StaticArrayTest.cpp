/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include "toolkit/collection/StaticArray.hpp"

struct CounterObject
{
    static int ctor;
    static int dtor;
    static int copy;
    static int move;

    int value = 0;

    CounterObject() { ++ctor; }
    explicit CounterObject(int v) : value(v) { ++ctor; }

    CounterObject(const CounterObject& o) : value(o.value)
    {
        ++ctor;
        ++copy;
    }

    CounterObject(CounterObject&& o) noexcept : value(o.value)
    {
        ++ctor;
        ++move;
        o.value = -1;
    }

    CounterObject& operator=(const CounterObject& o)
    {
        value = o.value;
        ++copy;
        return *this;
    }

    CounterObject& operator=(CounterObject&& o) noexcept
    {
        value = o.value;
        o.value = -1;
        ++move;
        return *this;
    }

    ~CounterObject()
    {
        ++dtor;
    }

    bool operator==(const CounterObject& o) const
    {
        return value == o.value;
    }
};

int CounterObject::ctor = 0;
int CounterObject::dtor = 0;
int CounterObject::copy = 0;
int CounterObject::move = 0;

static void ResetCounter()
{
    CounterObject::ctor = 0;
    CounterObject::dtor = 0;
    CounterObject::copy = 0;
    CounterObject::move = 0;
}

TEST(TkStaticArray, DefaultConstruct)
{
    toolkit::TkStaticArray<int, 8> arr;
    EXPECT_EQ(arr.count(), 0);
    EXPECT_TRUE(arr.isEmpty());
}

TEST(TkStaticArray, InitializerList)
{
    toolkit::TkStaticArray<int, 8> arr{1, 2, 3};
    EXPECT_EQ(arr.count(), 3);
    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[2], 3);
}

TEST(TkStaticArray, PointerConstruct)
{
    int data[] = {4, 5, 6};
    toolkit::TkStaticArray<int, 8> arr(data, 3);

    EXPECT_EQ(arr.count(), 3);
    EXPECT_EQ(arr.first(), 4);
    EXPECT_EQ(arr.last(), 6);
}

TEST(TkStaticArray, CopyConstruct)
{
    ResetCounter();
    toolkit::TkStaticArray<CounterObject, 8> a;
    a.add(CounterObject(1));
    a.add(CounterObject(2));

    toolkit::TkStaticArray<CounterObject, 8> b(a);

    EXPECT_EQ(b.count(), 2);
    EXPECT_EQ(b[0].value, 1);
    EXPECT_EQ(b[1].value, 2);
}

TEST(TkStaticArray, MoveConstruct)
{
    ResetCounter();
    toolkit::TkStaticArray<CounterObject, 8> a;
    a.add(CounterObject(10));
    a.add(CounterObject(20));

    toolkit::TkStaticArray<CounterObject, 8> b(std::move(a));

    EXPECT_EQ(b.count(), 2);
    EXPECT_EQ(a.count(), 0);
}

TEST(TkStaticArray, CopyAssign)
{
    toolkit::TkStaticArray<int, 8> a{1, 2};
    toolkit::TkStaticArray<int, 8> b;

    b = a;

    EXPECT_EQ(b.count(), 2);
    EXPECT_EQ(b[1], 2);
}

TEST(TkStaticArray, MoveAssign)
{
    toolkit::TkStaticArray<int, 8> a{3, 4};
    toolkit::TkStaticArray<int, 8> b;

    b = std::move(a);

    EXPECT_EQ(b.count(), 2);
    EXPECT_EQ(a.count(), 0);
}

TEST(TkStaticArray, AddAndResize)
{
    toolkit::TkStaticArray<int, 8> arr;

    arr.add(1);
    arr.add(2);
    arr.add(3);

    EXPECT_EQ(arr.count(), 3);

    arr.resize(5);
    EXPECT_EQ(arr.count(), 5);

    arr.resize(2);
    EXPECT_EQ(arr.count(), 2);
}

TEST(TkStaticArray, Insert)
{
    toolkit::TkStaticArray<int, 8> arr{1, 2, 4};

    arr.insert(2, 3);

    EXPECT_EQ(arr.count(), 4);
    EXPECT_EQ(arr[2], 3);
    EXPECT_EQ(arr[3], 4);
}

TEST(TkStaticArray, RemoveAtKeepOrder)
{
    toolkit::TkStaticArray<int, 8> arr{1, 2, 3, 4};

    arr.removeAtKeepOrder(1);

    EXPECT_EQ(arr.count(), 3);
    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[1], 3);
    EXPECT_EQ(arr[2], 4);
}

TEST(TkStaticArray, RemoveAt)
{
    toolkit::TkStaticArray<int, 8> arr{1, 2, 3, 4};

    arr.removeAt(1);

    EXPECT_EQ(arr.count(), 3);
    EXPECT_TRUE(arr.contains(4));
}

TEST(TkStaticArray, Find)
{
    toolkit::TkStaticArray<int, 8> arr{5, 6, 7};

    EXPECT_EQ(arr.find(6), 1);
    EXPECT_EQ(arr.find(100), -1);
}

TEST(TkStaticArray, FindLast)
{
    toolkit::TkStaticArray<int, 8> arr{1, 2, 3, 2};

    EXPECT_EQ(arr.findLast(2), 3);
}

TEST(TkStaticArray, Iterator)
{
    toolkit::TkStaticArray<int, 8> arr{1, 2, 3};

    int sum = 0;
    for (auto it = arr.Begin(); it != arr.End(); ++it)
        sum += *it;

    EXPECT_EQ(sum, 6);
}

TEST(TkStaticArray, Swap)
{
    toolkit::TkStaticArray<int, 8> a{1, 2};
    toolkit::TkStaticArray<int, 8> b{3};

    a.swap(b);

    EXPECT_EQ(a.count(), 1);
    EXPECT_EQ(a[0], 3);
    EXPECT_EQ(b.count(), 2);
}

TEST(TkStaticArray, Reverse)
{
    toolkit::TkStaticArray<int, 8> arr{1, 2, 3};

    arr.reverse();

    EXPECT_EQ(arr[0], 3);
    EXPECT_EQ(arr[2], 1);
}

TEST(TkStaticArray, StackOps)
{
    toolkit::TkStaticArray<int, 8> arr;

    arr.push(1);
    arr.push(2);

    EXPECT_EQ(arr.peek(), 2);
    EXPECT_EQ(arr.pop(), 2);
    EXPECT_EQ(arr.pop(), 1);
}

TEST(TkStaticArray, QueueOps)
{
    toolkit::TkStaticArray<int, 8> arr;

    arr.enqueue(1);
    arr.enqueue(2);

    EXPECT_EQ(arr.dequeue(), 1);
    EXPECT_EQ(arr.dequeue(), 2);
}

TEST(TkStaticArray, PlacementNew)
{
    toolkit::TkStaticArray<CounterObject, 8> arr;
    ResetCounter();

    CounterObject* obj = new (arr) CounterObject(42);

    EXPECT_EQ(arr.count(), 1);
    EXPECT_EQ(obj->value, 42);
}

