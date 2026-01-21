/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include "toolkit/utils/StringUtils.hpp"
#include "toolkit/types/Span.hpp"

TEST(TkStringUtils, EqualStrings)
{
    EXPECT_TRUE(TkStringUtils::equalStrings("abc", "abc"));
    EXPECT_FALSE(TkStringUtils::equalStrings("abc", "abcd"));
    EXPECT_FALSE(TkStringUtils::equalStrings("abc", "ab"));

    EXPECT_TRUE(TkStringUtils::equalStrings("abc", "abcd", 3));
    EXPECT_TRUE(TkStringUtils::equalStrings("", "", 0));
}

TEST(TkStringUtils, EqualStrings_StringView)
{
    TkStringView a("hello");
    TkStringView b("hello");
    TkStringView c("hell");

    EXPECT_TRUE(TkStringUtils::equalStrings(a, b));
    EXPECT_FALSE(TkStringUtils::equalStrings(a, c));
}

TEST(TkStringUtils, EqualIStrings)
{
    EXPECT_TRUE(TkStringUtils::equalIStrings("AbC", "aBc"));
    EXPECT_FALSE(TkStringUtils::equalIStrings("abc", "abd"));

    EXPECT_TRUE(TkStringUtils::equalIStrings("ABC", "abcd", 3));
}

TEST(TkStringUtils, CompareStrings)
{
    EXPECT_EQ(TkStringUtils::compareStrings("abc", "abc"), 0);
    EXPECT_LT(TkStringUtils::compareStrings("abc", "abd"), 0);
    EXPECT_GT(TkStringUtils::compareStrings("abd", "abc"), 0);
}

TEST(TkStringUtils, CompareStrings_MaxCount)
{
    EXPECT_EQ(TkStringUtils::compareStrings("abc", "abd", 2), 0);
    EXPECT_LT(TkStringUtils::compareStrings("abc", "abd", 3), 0);
}

TEST(TkStringUtils, CompareIStrings)
{
    EXPECT_EQ(TkStringUtils::compareIStrings("AbC", "aBc"), 0);
    EXPECT_LT(TkStringUtils::compareIStrings("abc", "ABD"), 0);
}

TEST(TkStringUtils, FindChar)
{
    TkStringView v("hello");

    EXPECT_EQ(TkStringUtils::find(v, 'e'), v.begin() + 1);
    EXPECT_EQ(TkStringUtils::find(v, 'o'), v.begin() + 4);
    EXPECT_EQ(TkStringUtils::find(v, 'x'), nullptr);
}

TEST(TkStringUtils, ReverseFind)
{
    TkStringView v("abca");

    EXPECT_EQ(TkStringUtils::reverseFind(v, 'a'), v.begin() + 3);
    EXPECT_EQ(TkStringUtils::reverseFind(v, 'b'), v.begin() + 1);
    EXPECT_EQ(TkStringUtils::reverseFind(v, 'x'), nullptr);
}

TEST(TkStringUtils, Contains)
{
    EXPECT_TRUE(TkStringUtils::contains("hello", 'h'));
    EXPECT_FALSE(TkStringUtils::contains("hello", 'x'));
}

TEST(TkStringUtils, StartsEndsWith)
{
    TkStringView v("hello.cpp");

    EXPECT_TRUE(TkStringUtils::startsWith(v, "hello"));
    EXPECT_FALSE(TkStringUtils::startsWith(v, "ello"));

    EXPECT_TRUE(TkStringUtils::endsWith(v, ".cpp"));
    EXPECT_FALSE(TkStringUtils::endsWith(v, ".h"));
}

TEST(TkStringUtils, StartsEndsWithInsensitive)
{
    TkStringView v("Hello.CPP");

    EXPECT_TRUE(TkStringUtils::startsWithInsensitive(v, "hello"));
    EXPECT_TRUE(TkStringUtils::endsWithInsensitive(v, ".cpp"));
}

TEST(TkStringUtils, FromString_Integer)
{
    i32 v = 0;
    EXPECT_TRUE(TkStringUtils::fromString("123", v));
    EXPECT_EQ(v, 123);

    EXPECT_TRUE(TkStringUtils::fromString("-42", v));
    EXPECT_EQ(v, -42);

    EXPECT_FALSE(TkStringUtils::fromString("12a", v));
}

TEST(TkStringUtils, FromString_Float)
{
    f32 v = 0;
    EXPECT_TRUE(TkStringUtils::fromString("3.14", v));
    EXPECT_NEAR(v, 3.14f, 1e-4f);

    EXPECT_FALSE(TkStringUtils::fromString("abc", v));
}

TEST(TkStringUtils, FromString_Bool)
{
    bool v = false;

    EXPECT_TRUE(TkStringUtils::fromString("true", v));
    EXPECT_TRUE(v);

    EXPECT_TRUE(TkStringUtils::fromString("false", v));
    EXPECT_FALSE(v);
}

TEST(TkStringUtils, ToString_Integer)
{
    EXPECT_EQ(TkStringUtils::toString(0), "0");
    EXPECT_EQ(TkStringUtils::toString(123), "123");
    EXPECT_EQ(TkStringUtils::toString(-45), "-45");
}

TEST(TkStringUtils, ToStringPretty)
{
    char buf[32];
    TkStringUtils::toStringPretty(1234567, TkSpan<char>{buf});
    EXPECT_STREQ(buf, "1,234,567");
}

TEST(TkStringUtils, CopyAndCat)
{
    char buf[16] = {};
    TkStringUtils::copyString(buf, "abc");
    EXPECT_STREQ(buf, "abc");

    TkStringUtils::catString(buf, "def");
    EXPECT_STREQ(buf, "abcdef");
}

TEST(TkStringUtils, MakeLowercase)
{
    char buf[16];
    EXPECT_TRUE(TkStringUtils::makeLowercase(buf, "AbC"));
    EXPECT_STREQ(buf, "abc");
}