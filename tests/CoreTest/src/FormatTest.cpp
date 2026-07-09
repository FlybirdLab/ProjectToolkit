/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

#include "TestAllocator.hpp"

#include "toolkit/base/Format.hpp"

TEST(Format, InlineBufferUsage)
{
    FormatBuffer buf;
    const char *text = "Hello";
    buf.appendData((void *)text, std::strlen(text));
    EXPECT_EQ(buf.getUsed(), std::strlen(text));
    EXPECT_EQ(std::memcmp(buf.getPointer(), text, buf.getUsed()), 0);

    buf.clear();
    EXPECT_EQ(buf.getUsed(), 0);
}

TEST(Format, HeapExpansion)
{
    FormatBuffer buf;
    std::string longStr(1000, 'A');

    buf.appendData((void *)longStr.data(), longStr.size());
    EXPECT_GE(buf.getUsed(), longStr.size());
    EXPECT_EQ(std::string(buf.getPointer(), buf.getUsed()), longStr);
}

TEST(Format, FormatBasicTypes)
{
    FormatBuffer buf;

    Formatter::format(buf, "%d", 123);
    EXPECT_STREQ(buf.getPointer(), "123");

    buf.clear();
    Formatter::format(buf, "%f", 3.14);
    EXPECT_STREQ(buf.getPointer(), "3.140000");

    buf.clear();
    Formatter::format(buf, "%s", "test");
    EXPECT_STREQ(buf.getPointer(), "test");
}

TEST(Format, FormatMultipleArgs)
{
    FormatBuffer buf;

    Formatter::format(buf, "%d-%s-%f", 42, "hello", 2.718);
    EXPECT_STREQ(buf.getPointer(), "42-hello-2.718000");
}

struct Point
{
    int x, y;
};
FORMAT_CUSTOM(Point, "(%d,%d)", 1, 2)

TEST(Format, FormatCustomType)
{

    FormatBuffer buf;
    Point p{1, 2};
    Formatter::format(buf, "%s", p);
    EXPECT_STREQ(buf.getPointer(), "(1,2)");
}

TEST(Format, FormatResizeInlineToHeap)
{
    FormatBuffer buf;

    std::string longText(600, 'X');
    Formatter::format(buf, "%s", longText.c_str());
    EXPECT_EQ(buf.getUsed(), longText.size());
    EXPECT_EQ(std::string(buf.getPointer(), buf.getUsed()), longText);
}

TEST(Format, FormatUnsupportedTypeFallback)
{
    struct Dummy
    {
        int a;
    };

    FormatBuffer buf;
    Dummy d{5};
    Formatter::format(buf, "%s", d);
    EXPECT_STREQ(buf.getPointer(), "[unsupported]");
}

TEST(Format, AppendAndCurrentPointer)
{
    FormatBuffer buf;
    buf.append(5);
    EXPECT_EQ(buf.getCurrentPointer() - buf.getPointer(), 5);

    char data[3] = {1, 2, 3};
    buf.appendData(data, 3);
    EXPECT_EQ(buf.getCurrentPointer() - buf.getPointer(), 8);
    EXPECT_EQ(buf.getPointer()[5], 1);
    EXPECT_EQ(buf.getPointer()[7], 3);
}

TEST(Format, InitialState)
{
    FormatBuffer buf(TkMemory::getDefault());

    EXPECT_EQ(buf.getUsed(), 0u);
    EXPECT_NE(buf.getPointer(), nullptr);
    EXPECT_EQ(buf.getPointer(), buf.getCurrentPointer());
}

TEST(Format, AppendWithinInlineBuffer)
{
    TestAllocator alloc;
    auto itf = alloc.asInterface();
    FormatBuffer buf(itf);

    buf.append(16);

    EXPECT_EQ(buf.getUsed(), 16u);
    EXPECT_EQ(alloc.allocCount, 0u);
}

TEST(Format, AppendExactInlineSize)
{
    TestAllocator alloc;
    auto itf = alloc.asInterface();
    FormatBuffer buf(itf);

    buf.append(FORMAT_BUFFER_SIZE);

    EXPECT_EQ(buf.getUsed(), FORMAT_BUFFER_SIZE);
    EXPECT_EQ(alloc.allocCount, 0u);
}

TEST(Format, AppendTriggerHeap)
{
    TestAllocator alloc;
    auto itf = alloc.asInterface();
    FormatBuffer buf(itf);

    buf.append(FORMAT_BUFFER_SIZE + 1);

    EXPECT_EQ(buf.getUsed(), FORMAT_BUFFER_SIZE + 1);
    EXPECT_EQ(alloc.allocCount, 1u);
}

TEST(Format, AppendDataCopiesCorrectly)
{
    TestAllocator alloc;
    auto itf = alloc.asInterface();
    FormatBuffer buf(itf);

    const char data[] = "hello";
    buf.appendData((void *)data, sizeof(data));

    EXPECT_EQ(buf.getUsed(), sizeof(data));
    EXPECT_STREQ(buf.getPointer(), data);
}

TEST(Format, AppendDataAndAppendMixed)
{
    TestAllocator alloc;
    auto itf = alloc.asInterface();
    FormatBuffer buf(itf);

    const char a[] = "abc";
    const char b[] = "def";

    buf.appendData((void *)a, 3);
    buf.appendData((void *)b, 3);

    EXPECT_EQ(buf.getUsed(), 6u);
    EXPECT_EQ(std::memcmp(buf.getPointer(), "abcdef", 6), 0);
}

TEST(Format, ReallocateKeepsData)
{
    TestAllocator alloc;
    auto itf = alloc.asInterface();
    FormatBuffer buf(itf);

    std::vector<char> data(FORMAT_BUFFER_SIZE + 32, 'x');
    buf.appendData(data.data(), data.size());

    EXPECT_EQ(buf.getUsed(), data.size());
    EXPECT_EQ(std::memcmp(buf.getPointer(), data.data(), data.size()), 0);
    EXPECT_EQ(alloc.allocCount, 1u);
}

TEST(Format, ClearResetsUsed)
{
    TestAllocator alloc;
    auto itf = alloc.asInterface();
    FormatBuffer buf(itf);

    buf.append(64);
    buf.clear();

    EXPECT_EQ(buf.getUsed(), 0u);

    buf.append(FORMAT_BUFFER_SIZE + 1);
    EXPECT_EQ(alloc.allocCount, 1u);
}

TEST(Format, MultipleGrowths)
{
    TestAllocator alloc;
    auto itf = alloc.asInterface();
    FormatBuffer buf(itf);

    buf.append(FORMAT_BUFFER_SIZE + 1);
    buf.append(FORMAT_BUFFER_SIZE * 2);

    EXPECT_GE(alloc.allocCount, 2u);
    EXPECT_EQ(buf.getUsed(), FORMAT_BUFFER_SIZE * 3 + 1);
}

TEST(Format, DestructorFreesHeap)
{
    TestAllocator alloc;
    {
        auto itf = alloc.asInterface();
        FormatBuffer buf(itf);
        buf.append(FORMAT_BUFFER_SIZE + 8);
        EXPECT_EQ(alloc.allocCount, 1u);
    }
    EXPECT_EQ(alloc.freeCount, 1u);
}
