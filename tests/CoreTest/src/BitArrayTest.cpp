/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include "toolkit/collection/BitArray.hpp"

using namespace toolkit;

TEST(TkBitArrayTest, DefaultConstructor) {
    TkBitArray arr;
    EXPECT_EQ(arr.count(), 0);
    EXPECT_EQ(arr.capacity(), 0);
    EXPECT_TRUE(arr.isEmpty());
    EXPECT_FALSE(arr.hasItems());
}

TEST(TkBitArrayTest, ConstructorWithCapacity) {
    TkBitArray arr(100);
    EXPECT_EQ(arr.count(), 0);
    EXPECT_GE(arr.capacity(), 100);
}

TEST(TkBitArrayTest, CopyConstructor) {
    TkBitArray a;
    a.add(true);
    a.add(false);
    TkBitArray b(a);
    EXPECT_EQ(a.count(), b.count());
    EXPECT_EQ(a[0], b[0]);
    EXPECT_EQ(a[1], b[1]);
}

TEST(TkBitArrayTest, MoveConstructor) {
    TkBitArray a;
    a.add(true);
    TkBitArray b(std::move(a));
    EXPECT_EQ(b.count(), 1);
    EXPECT_TRUE(b[0]);
    EXPECT_EQ(a.count(), 0);
    EXPECT_EQ(a.capacity(), 0);
    EXPECT_EQ(a.get(), nullptr);
}

TEST(TkBitArrayTest, CopyAssignment) {
    TkBitArray a;
    a.add(true);
    a.add(false);
    TkBitArray b;
    b = a;
    EXPECT_EQ(b.count(), a.count());
    EXPECT_EQ(b[0], true);
    EXPECT_EQ(b[1], false);
}

TEST(TkBitArrayTest, MoveAssignment) {
    TkBitArray a;
    a.add(true);
    TkBitArray b;
    b = std::move(a);
    EXPECT_EQ(b.count(), 1);
    EXPECT_TRUE(b[0]);
    EXPECT_EQ(a.count(), 0);
}

TEST(TkBitArrayTest, AddAndAccessBits) {
    TkBitArray arr;
    arr.add(true);
    arr.add(false);
    arr.add(true);

    EXPECT_EQ(arr.count(), 3);
    EXPECT_TRUE(arr[0]);
    EXPECT_FALSE(arr[1]);
    EXPECT_TRUE(arr[2]);

    // set
    arr.set(1, true);
    EXPECT_TRUE(arr[1]);
}

TEST(TkBitArrayTest, SetAllBits) {
    TkBitArray arr;
    for (int i = 0; i < 100; ++i)
        arr.add(i % 2 == 0);

    arr.setAll(true);
    for (int i = 0; i < arr.count(); ++i)
        EXPECT_TRUE(arr[i]);

    arr.setAll(false);
    for (int i = 0; i < arr.count(); ++i)
        EXPECT_FALSE(arr[i]);
}

TEST(TkBitArrayTest, ResizeAndEnsureCapacity) {
    TkBitArray arr;
    arr.resize(10);
    EXPECT_EQ(arr.count(), 10);
    arr.resize(5);
    EXPECT_EQ(arr.count(), 5);

    int oldCapacity = arr.capacity();
    arr.ensureCapacity(100);
    EXPECT_GE(arr.capacity(), 100);
    EXPECT_GE(arr.capacity(), oldCapacity);
}

TEST(TkBitArrayTest, AddMultipleBits) {
    TkBitArray arr;
    bool bits[] = {true, false, true, true};
    arr.add(bits, 4);
    EXPECT_EQ(arr.count(), 4);
    EXPECT_TRUE(arr[0]);
    EXPECT_FALSE(arr[1]);
    EXPECT_TRUE(arr[2]);
    EXPECT_TRUE(arr[3]);
}

TEST(TkBitArrayTest, AddAnotherBitArray) {
    TkBitArray a;
    a.add(true);
    a.add(false);
    TkBitArray b;
    b.add(false);
    b.add(true);
    a.add(b);
    EXPECT_EQ(a.count(), 4);
    EXPECT_TRUE(a[0]);
    EXPECT_FALSE(a[1]);
    EXPECT_FALSE(a[2]);
    EXPECT_TRUE(a[3]);
}

TEST(TkBitArrayTest, SwapArrays) {
    TkBitArray a;
    a.add(true);
    TkBitArray b;
    b.add(false);
    b.add(true);

    a.swap(b);
    EXPECT_EQ(a.count(), 2);
    EXPECT_EQ(b.count(), 1);
    EXPECT_FALSE(a[0]);
    EXPECT_TRUE(a[1]);
    EXPECT_TRUE(b[0]);
}

TEST(TkBitArrayTest, EqualityAndInequality) {
    TkBitArray a;
    TkBitArray b;
    EXPECT_TRUE(a == b);

    a.add(true);
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);

    b.add(true);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(TkBitArrayTest, ClearArray) {
    TkBitArray arr;
    arr.add(true);
    arr.add(false);
    arr.clear();
    EXPECT_EQ(arr.count(), 0);
    EXPECT_TRUE(arr.isEmpty());
}

