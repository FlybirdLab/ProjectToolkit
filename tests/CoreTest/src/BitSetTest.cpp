/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include "toolkit/collection/BitSet.hpp"

using BS = TkBitSet<10, u8>;

TEST(BitSet, ConstructFalse)
{
    BS b(false);
    EXPECT_FALSE(b);
    EXPECT_EQ(b.getSetBitCount(), 0);
}

TEST(BitSet, ConstructTrue)
{
    BS b(true);
    EXPECT_TRUE(b);
    EXPECT_EQ(b.getSetBitCount(), 10);
}

TEST(BitSet, SetGetUnsetSingleBit)
{
    BS b(false);

    b.set(0);
    EXPECT_TRUE(b.get(0));
    EXPECT_EQ(b.getSetBitCount(), 1);

    b.unset(0);
    EXPECT_FALSE(b.get(0));
    EXPECT_EQ(b.getSetBitCount(), 0);
}

TEST(BitSet, BoundaryBits)
{
    BS b(false);

    b.set(0);
    b.set(9);

    EXPECT_TRUE(b.get(0));
    EXPECT_TRUE(b.get(9));
    EXPECT_EQ(b.getSetBitCount(), 2);
}

using BS1 = TkBitSet<17, u8>;

TEST(BitSet, ChunkBoundary)
{
    BS1 b(false);

    b.set(7);
    b.set(8);

    EXPECT_TRUE(b.get(7));
    EXPECT_TRUE(b.get(8));
}

TEST(BitSet, BitwiseOps)
{
    BS1 a(false), b(false);

    a.set(1).set(3);
    b.set(3).set(4);

    auto c = a | b;
    EXPECT_TRUE(c.get(1));
    EXPECT_TRUE(c.get(3));
    EXPECT_TRUE(c.get(4));

    auto d = a & b;
    EXPECT_TRUE(d.get(3));
    EXPECT_EQ(d.getSetBitCount(), 1);

    auto e = a ^ b;
    EXPECT_TRUE(e.get(1));
    EXPECT_TRUE(e.get(4));
    EXPECT_FALSE(e.get(3));
}

TEST(BitSet, BitwiseNot)
{
    BS1 b(false);
    b.set(0);

    auto c = ~b;

    EXPECT_FALSE(c.get(0));
    EXPECT_EQ(c.getSetBitCount(), 16);
}

TEST(BitSet, MSB_LSB)
{
    BS1 b(false);

    EXPECT_EQ(b.getMostSignificantBit(), kMaxU32);
    EXPECT_EQ(b.getLeastSignificantBit(), kMaxU32);

    b.set(2);
    b.set(7);

    EXPECT_EQ(b.getLeastSignificantBit(), 2);
    EXPECT_EQ(b.getMostSignificantBit(), 7);
}

TEST(BitSet, UnsetNLeastSignificantBits)
{
    BS1 b(true); // all bits set

    b.unsetNLeastSignificantBits(3);

    EXPECT_FALSE(b.get(0));
    EXPECT_FALSE(b.get(1));
    EXPECT_FALSE(b.get(2));
    EXPECT_TRUE(b.get(3));
}

TEST(BitSet, CopyAndCompare)
{
    BS1 a(false);
    a.set(1).set(5);

    BS1 b = a;
    EXPECT_EQ(a, b);

    BS1 c(false);
    c = a;
    EXPECT_EQ(a, c);
}
