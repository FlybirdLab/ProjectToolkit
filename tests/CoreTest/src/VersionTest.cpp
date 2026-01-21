/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include "toolkit/types/Version.hpp"

TEST(TkVersion, ConstructorAndToString)
{
    TkVersion v1(1, 2);
    EXPECT_EQ(v1.major(), 1);
    EXPECT_EQ(v1.minor(), 2);
    EXPECT_EQ(v1.build(), -1);
    EXPECT_EQ(v1.revision(), -1);
    EXPECT_EQ(v1.toString(), "1.2");

    TkVersion v2(1, 2, 3);
    EXPECT_EQ(v2.toString(), "1.2.3");

    TkVersion v3(1, 2, 3, 4);
    EXPECT_EQ(v3.toString(), "1.2.3.4");

    TkVersion vDefault;
    EXPECT_EQ(vDefault.major(), 0);
    EXPECT_EQ(vDefault.minor(), 0);
    EXPECT_EQ(vDefault.build(), -1);
    EXPECT_EQ(vDefault.revision(), -1);
    EXPECT_EQ(vDefault.toString(), "0.0");
}

TEST(TkVersion, CompareToAndOperators)
{
    TkVersion v1(1, 2, 3, 4);
    TkVersion v2(1, 2, 3, 5);
    TkVersion v3(1, 2, 4, 0);
    TkVersion v4(1, 3, 0, 0);
    TkVersion v5(2, 0, 0, 0);
    TkVersion v6(1, 2, 3, 4); // same as v1

    EXPECT_TRUE(v1 < v2);
    EXPECT_TRUE(v1 <= v2);
    EXPECT_TRUE(v2 > v1);
    EXPECT_TRUE(v2 >= v1);

    EXPECT_TRUE(v1 < v3);
    EXPECT_TRUE(v1 < v4);
    EXPECT_TRUE(v1 < v5);

    EXPECT_TRUE(v1 == v6);
    EXPECT_FALSE(v1 != v6);

    EXPECT_EQ(v1.compareTo(v6), 0);
    EXPECT_EQ(v1.compareTo(v2), -1);
    EXPECT_EQ(v2.compareTo(v1), 1);
}

TEST(TkVersion, ParseValid)
{
    TkVersion v;

    // 2-field
    EXPECT_TRUE(TkVersion::parse("1.2", v));
    EXPECT_EQ(v.major(), 1);
    EXPECT_EQ(v.minor(), 2);
    EXPECT_EQ(v.build(), -1);
    EXPECT_EQ(v.revision(), -1);

    // 3-field
    EXPECT_TRUE(TkVersion::parse("1.2.3", v));
    EXPECT_EQ(v.build(), 3);
    EXPECT_EQ(v.revision(), -1);

    // 4-field
    EXPECT_TRUE(TkVersion::parse("1.2.3.4", v));
    EXPECT_EQ(v.revision(), 4);
}

TEST(TkVersion, ParseInvalid)
{
    TkVersion v;

    // Empty string
    EXPECT_FALSE(TkVersion::parse("", v));

    // Not enough components
    EXPECT_FALSE(TkVersion::parse("1", v));

    // Too many components
    EXPECT_FALSE(TkVersion::parse("1.2.3.4.5", v));

    // Non-numeric components
    EXPECT_FALSE(TkVersion::parse("a.b", v));
    EXPECT_FALSE(TkVersion::parse("1.b", v));
    EXPECT_FALSE(TkVersion::parse("1.2.c", v));
    EXPECT_FALSE(TkVersion::parse("1.2.3.d", v));
}

TEST(TkVersion, ToStringFieldCount)
{
    TkVersion v(1, 2, 3, 4);

    EXPECT_EQ(v.toString(0), "");
    EXPECT_EQ(v.toString(1), "1");
    EXPECT_EQ(v.toString(2), "1.2");
    EXPECT_EQ(v.toString(3), "1.2.3");
    EXPECT_EQ(v.toString(4), "1.2.3.4");
    EXPECT_EQ(v.toString(5), "1.2.3.4"); // fieldCount > 4 treated same as 4
}

TEST(TkVersion, HashFunc)
{
    TkVersion v(1, 2, 3, 4);
    u32 hash = HashFunc<TkVersion>::get(v);

    u32 expected = ((1 & 15) << 28) | ((2 & 255) << 20) | ((3 & 255) << 12) | (4 & 4095);
    EXPECT_EQ(hash, expected);
}
