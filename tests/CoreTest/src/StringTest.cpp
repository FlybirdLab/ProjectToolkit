/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

#include "toolkit/types/String.hpp"
#include "toolkit/collection/Array.hpp"

TEST(TkString, ConstructBasic)
{
    TkString s("hello");
    EXPECT_EQ(s.length(), 5);
    EXPECT_STREQ(s.getText(), "hello");
}

TEST(TkString, EmptyString)
{
    TkString s;
    EXPECT_TRUE(s.isEmpty());
    EXPECT_STREQ(s.getText(), "");
}

TEST(TkString, CopyConstruct)
{
    TkString a("abc");
    TkString b(a);

    EXPECT_EQ(a, b);
    EXPECT_NE(a.get(), b.get()); // deepcopy
}

TEST(TkString, MoveConstruct)
{
    TkString a("abc");
    char *oldPtr = a.get();

    TkString b(std::move(a));
    EXPECT_EQ(b, "abc");
    EXPECT_TRUE(a.isEmpty());
    EXPECT_EQ(a.get(), nullptr);
}

TEST(TkString, Append)
{
    TkString s("abc");
    s.append("def", 3);

    EXPECT_EQ(s, "abcdef");
}

TEST(TkString, OperatorPlusEqual)
{
    TkString s("abc");
    s += 'd';
    s += "ef";

    EXPECT_EQ(s, "abcdef");
}

TEST(TkString, OperatorPlus)
{
    TkString a("abc");
    TkString b("def");

    TkString c = a + b;
    EXPECT_EQ(c, "abcdef");

    TkString d = "xx" + a;
    EXPECT_EQ(d, "xxabc");

    TkString e = a + "yy";
    EXPECT_EQ(e, "abcyy");
}

TEST(TkString, StartsEndsWith)
{
    TkString s("HelloWorld");

    EXPECT_TRUE(s.startsWith(TkStringView{"Hello"}));
    EXPECT_FALSE(s.startsWith(TkStringView{"hello"}));
    EXPECT_TRUE(s.startsWith(TkStringView{"hello"}, false));

    EXPECT_TRUE(s.endsWith(TkStringView{"World"}));
    EXPECT_TRUE(s.endsWith(TkStringView{"world"}, false));
}

TEST(TkString, Find)
{
    TkString s("abcabc");

    EXPECT_EQ(s.find('a'), 0);
    EXPECT_EQ(s.findLast('a'), 3);

    EXPECT_EQ(s.find("bc"), 1);
    EXPECT_EQ(s.findLast("bc"), 4);
}

TEST(TkString, ReplaceChar)
{
    TkString s("aAaA");
    i32 count = s.replace('a', 'x', false);

    EXPECT_EQ(count, 4);
    EXPECT_EQ(s, "xxxx");
}

TEST(TkString, ReplaceTextDifferentLength)
{
    TkString s("abc abc abc");
    i32 count = s.replace("abc", "x", true);

    EXPECT_EQ(count, 3);
    EXPECT_EQ(s, "x x x");
}

TEST(TkString, Insert)
{
    TkString s("HelloWorld");
    s.insert(5, TkString(" "));

    EXPECT_EQ(s, "Hello World");
}

TEST(TkString, Remove)
{
    TkString s("Hello World");
    s.remove(5, 1);

    EXPECT_EQ(s, "HelloWorld");
}

TEST(TkString, Split)
{
    TkString s("a,b,c");
    TkArray<TkString> parts;

    s.split(',', parts);

    EXPECT_EQ(parts.count(), 3);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[1], "b");
    EXPECT_EQ(parts[2], "c");
}

TEST(TkString, PathJoin)
{
    TkString s("root");
    s /= "folder";

    EXPECT_EQ(s, "root/folder");

    s /= "file.txt";
    EXPECT_EQ(s, "root/folder/file.txt");
}

TEST(TkString, PathJoinNoDuplicateSlash)
{
    TkString s("root/");
    s /= "/folder";

    EXPECT_EQ(s, "root//folder");
}

TEST(TkString, Compare)
{
    TkString a("abc");
    TkString b("abd");

    EXPECT_LT(a.compare(b), 0);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
}

TEST(TkString, Boundary)
{
    TkString s("abc");

    EXPECT_EQ(s.find(""), -1);
    EXPECT_EQ(s.find(nullptr), -1);

    s.clear();
    EXPECT_TRUE(s.isEmpty());

    s += "";
    EXPECT_TRUE(s.isEmpty());
}

TEST(TkString, Format_Basic)
{
    TkString s = TkString::format("Hello %s", "World");
    EXPECT_STREQ(*s, "Hello World");
    EXPECT_EQ(s.length(), 11);
}

TEST(TkString, Format_MultipleArgs)
{
    TkString s = TkString::format("%s %d %.2f", "Value:", 42, 3.14);
    EXPECT_STREQ(*s, "Value: 42 3.14");
}

TEST(TkString, Format_EmptyFormat)
{
    TkString s = TkString::format("");
    EXPECT_TRUE(s.isEmpty());
}

TEST(TkString, Format_NoArgs)
{
    TkString s = TkString::format("Just text");
    EXPECT_STREQ(*s, "Just text");
}

TEST(TkString, Format_BufferLifetime)
{
    TkString s;
    {
        s = TkString::format("Number: %d", 123);
    }
    EXPECT_STREQ(*s, "Number: 123");
}

TEST(TkString, Format_UTF8)
{
    TkString s = TkString::format("你好 %s", "世界");
    EXPECT_STREQ(*s, "你好 世界");

    s = TkString::format("测试 %s", s);
    EXPECT_STREQ(*s, "测试 你好 世界");
}

TEST(TkString, Format_Large)
{
    TkString s = TkString::format(
        "%s%s%s%s%s",
        "aaaaaaaaaa", "bbbbbbbbbb", "cccccccccc",
        "dddddddddd", "eeeeeeeeee"
    );
    EXPECT_EQ(s.length(), 50);
}
