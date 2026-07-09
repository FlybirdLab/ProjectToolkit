/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include "toolkit/types/StringView.hpp"

TEST(TkStringView, DefaultConstruct)
{
    TkStringView v;

    EXPECT_TRUE(v.isEmpty());
    EXPECT_EQ(v.length(), 0);
    EXPECT_STREQ(v.getText(), "");
}
TEST(TkStringView, ConstructFromCString)
{
    TkStringView v("Hello");

    EXPECT_FALSE(v.isEmpty());
    EXPECT_EQ(v.length(), 5);
    EXPECT_EQ(v, "Hello");
}
TEST(TkStringView, ConstructFromTkString)
{
    TkString s("Hello");
    TkStringView v(s);

    EXPECT_EQ(v.length(), s.length());
    EXPECT_EQ(v, s);
}
TEST(TkStringView, ConstructWithLength)
{
    const char raw[] = {'a','b','c','d'};
    TkStringView v(raw, 3);

    EXPECT_EQ(v.length(), 3);
    EXPECT_EQ(v, "abc");
}
TEST(TkStringView, CompareViewToView)
{
    TkStringView a("Hello");
    TkStringView b("Hello");
    TkStringView c("World");

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);
}
TEST(TkStringView, CompareWithCString)
{
    TkStringView v("Hello");

    EXPECT_TRUE(v == "Hello");
    EXPECT_FALSE(v == "hello");
}
TEST(TkStringView, CompareWithTkString)
{
    TkString s("Hello");
    TkStringView v("Hello");

    EXPECT_TRUE(v == s);
    EXPECT_FALSE(v != s);
}
TEST(TkStringView, IndexOperator)
{
    TkStringView v("ABC");

    EXPECT_EQ(v[0], 'A');
    EXPECT_EQ(v[1], 'B');
    EXPECT_EQ(v[2], 'C');
    EXPECT_EQ(v[3], '\0');
}
TEST(TkStringView, StartsEndsWithChar)
{
    TkStringView v("Hello");

    EXPECT_TRUE(v.startsWith('H'));
    EXPECT_FALSE(v.startsWith('h'));
    EXPECT_TRUE(v.startsWith('h', false));

    EXPECT_TRUE(v.endsWith('o'));
    EXPECT_FALSE(v.endsWith('O'));
    EXPECT_TRUE(v.endsWith('O', false));
}
TEST(TkStringView, StartsEndsWithString)
{
    TkStringView v("HelloWorld");

    EXPECT_TRUE(v.startsWith("Hello"));
    EXPECT_FALSE(v.startsWith("hello"));
    EXPECT_TRUE(v.startsWith("hello", false));

    EXPECT_TRUE(v.endsWith("World"));
    EXPECT_FALSE(v.endsWith("world"));
    EXPECT_TRUE(v.endsWith("world", false));
}
TEST(TkStringView, StartsEndsWithEmpty)
{
    TkStringView v("Hello");

    EXPECT_FALSE(v.startsWith(""));
    EXPECT_FALSE(v.endsWith(""));
}
TEST(TkStringView, FindChar)
{
    TkStringView v("abca");

    EXPECT_EQ(v.find('a'), 0);
    EXPECT_EQ(v.findLast('a'), 3);
    EXPECT_EQ(v.find('x'), -1);
}
TEST(TkStringView, FindOnEmpty)
{
    TkStringView v;

    EXPECT_EQ(v.find('a'), -1);
    EXPECT_EQ(v.findLast('a'), -1);
}
TEST(TkStringView, LeftRight)
{
    TkStringView v("Hello");

    EXPECT_EQ(v.left(2), "He");
    EXPECT_EQ(v.left(10), "Hello");
    EXPECT_EQ(v.left(0), "");

    EXPECT_EQ(v.right(2), "lo");
    EXPECT_EQ(v.right(10), "Hello");
}
TEST(TkStringView, Substring)
{
    TkStringView v("HelloWorld");

    EXPECT_EQ(v.substring(0, 5), "Hello");
    EXPECT_EQ(v.substring(5), "World");
    EXPECT_EQ(v.substring(3, 4), "loWo");
}
TEST(TkStringView, Compare)
{
    TkStringView a("abc");
    TkStringView b("abd");
    TkStringView c("ABC");

    EXPECT_LT(a.compare(b), 0);
    EXPECT_GT(b.compare(a), 0);
    EXPECT_EQ(a.compare("abc"), 0);

    EXPECT_EQ(a.compare(c, false), 0);
    EXPECT_NE(a.compare(c, true), 0);
}
TEST(TkStringView, ToString)
{
    TkStringView v("Hello");
    TkString s = v.toString();

    EXPECT_EQ(s, "Hello");
    EXPECT_NE(s.get(), v.get()); // 不共享内存
}
TEST(TkStringView, Hash)
{
    TkStringView a("Hello");
    TkStringView b("Hello");
    TkStringView c("World");

    EXPECT_EQ(HashFunc<TkStringView>::get(a), HashFunc<TkStringView>::get(b));
    EXPECT_NE(HashFunc<TkStringView>::get(a), HashFunc<TkStringView>::get(c));
}
