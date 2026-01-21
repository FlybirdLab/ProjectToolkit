/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

#include <toolkit/types/UUID.hpp>

TEST(TkUuid, BasicValidity)
{
    TkUuid e;
    EXPECT_FALSE(e.isValid());
    EXPECT_FALSE(static_cast<bool>(e));

    TkUuid a(1, 2, 3, 4);
    EXPECT_TRUE(a.isValid());
    EXPECT_TRUE(static_cast<bool>(a));

    TkUuid z(0, 0, 0, 0);
    EXPECT_FALSE(z.isValid());
}

TEST(TkUuid, Compare)
{
    TkUuid a(1, 2, 3, 4);
    TkUuid b(1, 2, 3, 4);
    TkUuid c(1, 2, 3, 5);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(TkUuid, IndexAccess)
{
    TkUuid a(10, 20, 30, 40);

    EXPECT_EQ(a[0], 10u);
    EXPECT_EQ(a[1], 20u);
    EXPECT_EQ(a[2], 30u);
    EXPECT_EQ(a[3], 40u);

    a[2] = 99;
    EXPECT_EQ(a.c, 99u);
}

TEST(TkUuid, ToStringFormats)
{
    TkUuid u(
        0x11223344,
        0x55667788,
        0x99aabbcc,
        0xddeeff00);

    EXPECT_EQ(
        u.toString(UuidFormatType::kN),
        "112233445566778899aabbccddeeff00");

    EXPECT_EQ(
        u.toString(UuidFormatType::kD),
        "11223344-5566-7788-99aa-bbccddeeff00");

    EXPECT_EQ(
        u.toString(UuidFormatType::kB),
        "{11223344-5566-7788-99aa-bbccddeeff00}");

    EXPECT_EQ(
        u.toString(UuidFormatType::kP),
        "(11223344-5566-7788-99aa-bbccddeeff00)");
}

TEST(TkUuid, ToStringBuffer)
{
    TkUuid u(1, 2, 3, 4);
    char buf[64] = {};

    u.toString(buf, UuidFormatType::kN);
    EXPECT_STREQ(
        buf,
        "00000001000000020000000300000004");
}

TEST(TkUuid, ParseValid)
{
    TkUuid u;

    EXPECT_TRUE(TkUuid::parse(
        "112233445566778899aabbccddeeff00", u));

    EXPECT_EQ(u.a, 0x11223344u);
    EXPECT_EQ(u.b, 0x55667788u);
    EXPECT_EQ(u.c, 0x99aabbccu);
    EXPECT_EQ(u.d, 0xddeeff00u);

    EXPECT_TRUE(TkUuid::parse(
        "11223344-5566-7788-99aa-bbccddeeff00", u));

    EXPECT_TRUE(TkUuid::parse(
        "{11223344-5566-7788-99aa-bbccddeeff00}", u));

    EXPECT_TRUE(TkUuid::parse(
        "(11223344-5566-7788-99aa-bbccddeeff00)", u));
}

TEST(TkUuid, ParseInvalid)
{
    TkUuid u;

    EXPECT_FALSE(TkUuid::parse("", u));
    EXPECT_FALSE(TkUuid::parse("xyz", u));
    EXPECT_FALSE(TkUuid::parse("11223344", u));
    EXPECT_FALSE(TkUuid::parse(
        "11223344-xxxx-xxxx-xxxx-xxxxxxxxxxxx", u));
}

TEST(TkUuid, Hash)
{
    TkUuid a(1, 2, 3, 4);
    TkUuid b(1, 2, 3, 4);
    TkUuid c(4, 3, 2, 1);

    EXPECT_EQ(
        HashFunc<TkUuid>::get(a),
        HashFunc<TkUuid>::get(b));

    EXPECT_NE(HashFunc<TkUuid>::get(a), 0u);
    EXPECT_NE(HashFunc<TkUuid>::get(c), 0u);
}

TEST(TkUuid, Create)
{
    TkUuid a = TkUuid::create();
    TkUuid b = TkUuid::create();

    EXPECT_TRUE(a.isValid());
    EXPECT_TRUE(b.isValid());

    EXPECT_NE(a, b);
}
