/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include "toolkit/base/Hash.hpp"

// Helper enum for testing
enum class TestEnum : i32 { A = 1, B = 2, C = 100 };

TEST(Hashing, IntegerHashes) {
    EXPECT_EQ(HashFunc<u8>::get(42), 42u);
    EXPECT_EQ(HashFunc<i8>::get(-5), static_cast<u32>(-5));
    EXPECT_EQ(HashFunc<u16>::get(12345), 12345u);
    EXPECT_EQ(HashFunc<i16>::get(-12345), static_cast<u32>(-12345));
    EXPECT_EQ(HashFunc<u32>::get(0xDEADBEEF), 0xDEADBEEF);
    EXPECT_EQ(HashFunc<i32>::get(-42), static_cast<u32>(-42));
    EXPECT_EQ(HashFunc<u64>::get(0x123456789ABCDEF0ULL), (u32)0x9ABCDEF0 + ((u32)(0x12345678) * 23));
    EXPECT_EQ(HashFunc<i64>::get(-1LL), 4294967272u);
}

TEST(Hashing, FloatHashes) {
    f32 f = 3.14f;
    u32 fHash = *(u32 *)&f;
    EXPECT_EQ(HashFunc<f32>::get(f), fHash);

    f64 d = 2.718281828;
    u32 dHash = HashFunc<u64>::get(*(u64 *)&d);
    EXPECT_EQ(HashFunc<f64>::get(d), dHash);
}

TEST(Hashing, PointerHash) {
    int a;
    void* p = &a;
    u32 h = HashFunc<void*>::get(p);
    EXPECT_EQ(h, static_cast<u32>((i64)p >> 3));
}

TEST(Hashing, EnumHash) {
    EXPECT_EQ(HashFunc<TestEnum>::get(TestEnum::A), HashFunc<i32>::get(1));
    EXPECT_EQ(HashFunc<TestEnum>::get(TestEnum::B), HashFunc<i32>::get(2));
    EXPECT_EQ(HashFunc<TestEnum>::get(TestEnum::C), HashFunc<i32>::get(100));
}

TEST(Hashing, BasicUsage) {
    RuntimeHash h1 = RuntimeHash("test");
    RuntimeHash h2 = RuntimeHash("test");
    RuntimeHash h3 = RuntimeHash("other");

    EXPECT_EQ(h1, h2);
    EXPECT_NE(h1, h3);

    RuntimeHash fromU64 = RuntimeHash::fromU64(12345);
    EXPECT_EQ(fromU64.getHashValue(), 12345u);
}

TEST(Hashing, BasicUsage32) {
    RuntimeHash32 h1 = RuntimeHash32("abc");
    RuntimeHash32 h2 = RuntimeHash32("abc");
    RuntimeHash32 h3 = RuntimeHash32("def");

    EXPECT_EQ(h1, h2);
    EXPECT_NE(h1, h3);

    RuntimeHash32 fromU32 = RuntimeHash32::fromU32(9876);
    EXPECT_EQ(fromU32.getHashValue(), 9876u);
}

TEST(Hashing, BasicUsageStable) {
    StableHash h1 = StableHash("hello");
    StableHash h2 = StableHash("hello");
    StableHash h3 = StableHash("world");

    EXPECT_EQ(h1, h2);
    EXPECT_NE(h1, h3);
    EXPECT_TRUE(h1 < h3 || h3 < h1);
}

TEST(Hashing, BasicUsageStable32) {
    StableHash32 h1 = StableHash32("hello");
    StableHash32 h2 = StableHash32("hello");
    StableHash32 h3 = StableHash32("world");

    EXPECT_EQ(h1, h2);
    EXPECT_NE(h1, h3);
    EXPECT_TRUE(h1 < h3 || h3 < h1);
}

TEST(Hashing, CombineBasicFunctionality) {
    u32 h = 0;
    combineHash(h, 123u);
    u32 h2 = h;
    combineHash(h, 456u);

    EXPECT_NE(h, h2);

    int x = 42;
    combineHash(h, &x);
    EXPECT_NE(h, h2);
}

TEST(Hashing, SameValueSameHash) {
    int a = 123;
    int b = 123;
    EXPECT_EQ(HashFunc<int>::get(a), HashFunc<int>::get(b));

    f32 f1 = 3.14f;
    f32 f2 = 3.14f;
    EXPECT_EQ(HashFunc<f32>::get(f1), HashFunc<f32>::get(f2));

    RuntimeHash r1("abc");
    RuntimeHash r2("abc");
    EXPECT_EQ(HashFunc<RuntimeHash>::get(r1), HashFunc<RuntimeHash>::get(r2));
}

TEST(Hashing, DifferentValueDifferentHash) {
    int a = 1, b = 2;
    EXPECT_NE(HashFunc<int>::get(a), HashFunc<int>::get(b));

    f32 f1 = 3.14f, f2 = 2.71f;
    EXPECT_NE(HashFunc<f32>::get(f1), HashFunc<f32>::get(f2));

    RuntimeHash r1("abc");
    RuntimeHash r2("def");
    EXPECT_NE(HashFunc<RuntimeHash>::get(r1), HashFunc<RuntimeHash>::get(r2));
}
