/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

#include "toolkit/types/StringBuilder.hpp"

TEST(TkStringBuilder, ConstructAndBasicState)
{
    TkStringBuilder b;

    EXPECT_EQ(b.length(), 0);
    EXPECT_GE(b.capacity(), 0);
    EXPECT_STREQ(b.begin(), "");
}

TEST(TkStringBuilder, AppendChar)
{
    TkStringBuilder b;
    b.append('a').append('b').append('c');

    EXPECT_EQ(b.length(), 3);
    EXPECT_EQ(b.toString(), "abc");
}

TEST(TkStringBuilder, AppendCString)
{
    TkStringBuilder b;
    b.append("hello");

    EXPECT_EQ(b.length(), 5);
    EXPECT_EQ(b.toString(), "hello");
}

TEST(TkStringBuilder, AppendCStringWithLength)
{
    TkStringBuilder b;
    b.append("hello world", 5);

    EXPECT_EQ(b.toString(), "hello");
}

TEST(TkStringBuilder, AppendTkString)
{
    TkString s("abc");
    TkStringBuilder b;
    b.append(s);

    EXPECT_EQ(b.toString(), "abc");
}

TEST(TkStringBuilder, AppendTkStringView)
{
    TkStringView v("hello world", 5);
    TkStringBuilder b;
    b.append(v);

    EXPECT_EQ(b.toString(), "hello");
}

TEST(TkStringBuilder, AppendInteger)
{
    TkStringBuilder b;
    b.append(123).append(' ').append(-45);

    EXPECT_EQ(b.toString(), "123 -45");
}

TEST(TkStringBuilder, AppendUnsigned)
{
    TkStringBuilder b;
    b.append((u32)42);

    EXPECT_EQ(b.toString(), "42");
}

TEST(TkStringBuilder, AppendFormat)
{
    TkStringBuilder b;
    b.appendFormat("hello %i", 123);

    TkString str = b.toString();
    EXPECT_EQ(str, "hello 123");
}

TEST(TkStringBuilder, AppendFormatMultiple)
{
    TkStringBuilder b;
    b.appendFormat("%i + %i = %i", 1, 2, 3);

    EXPECT_EQ(b.toString(), "1 + 2 = 3");
}

TEST(TkStringBuilder, AppendLineBasic)
{
    TkStringBuilder b;
    b.appendLine("hello");

#if TK_WINDOWS
    EXPECT_EQ(b.toString(), "hello\r\n");
#else
    EXPECT_EQ(b.toString(), "hello\n");
#endif
}

TEST(TkStringBuilder, AppendLineMultiple)
{
    TkStringBuilder b;
    b.appendLine(1)
        .appendLine(2)
        .appendLine("end");

    TkString s = b.toString();
    EXPECT_TRUE(s.contains("1"));
    EXPECT_TRUE(s.contains("2"));
    EXPECT_TRUE(s.contains("end"));
}

TEST(TkStringBuilder, ClearAndReuse)
{
    TkStringBuilder b;
    b.append("hello");
    b.clear();

    EXPECT_EQ(b.length(), 0);
    EXPECT_STREQ(b.begin(), "");

    b.append("world");
    EXPECT_EQ(b.toString(), "world");
}

TEST(TkStringBuilder, ToStringView)
{
    TkStringBuilder b;
    b.append("hello");

    TkStringView v = b.toStringView();
    EXPECT_EQ(v.length(), 5);
    EXPECT_EQ(v, "hello");
}

TEST(TkStringBuilder, ToStringIndependence)
{
    TkStringBuilder b;
    b.append("abc");

    TkString s = b.toString();
    b.append("def");

    EXPECT_EQ(s, "abc");
    EXPECT_EQ(b.toString(), "abcdef");
}

TEST(TkStringBuilder, HashFunction)
{
    TkStringBuilder a;
    a.append("hello");

    TkStringBuilder b;
    b.append("hello");

    EXPECT_EQ(HashFunc<TkStringBuilder>::get(a),
              HashFunc<TkStringBuilder>::get(b));
}

TEST(TkStringBuilder, FormatCustom)
{
    TkStringBuilder b;
    b.append("value = ");

    TkString s = TkString::format("%s", b);
    EXPECT_EQ(s, "value = ");
}

TEST(TkStringBuilder, LargeAppend)
{
    TkStringBuilder b;
    for (int i = 0; i < 10000; ++i)
        b.append('a');

    EXPECT_EQ(b.length(), 10000);
}
