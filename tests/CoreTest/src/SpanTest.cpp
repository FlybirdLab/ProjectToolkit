/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include "toolkit/types/Span.hpp"
#include "toolkit/collection/Array.hpp"

TEST(TkSpan, DefaultConstructor)
{
    TkSpan<int> s;
    EXPECT_FALSE(s.isValid());
    EXPECT_TRUE(s.isInvalid());
    EXPECT_EQ(s.length(), 0);
    EXPECT_EQ(s.get(), nullptr);
}

TEST(TkSpan, PointerLengthConstructor)
{
    int arr[] = {1, 2, 3, 4};
    TkSpan<int> s(arr, 4);
    EXPECT_TRUE(s.isValid());
    EXPECT_EQ(s.length(), 4);
    EXPECT_EQ(s.get(), arr);

    for (int i = 0; i < 4; ++i)
        EXPECT_EQ(s[i], arr[i]);
}

TEST(TkSpan, BeginEndConstructor)
{
    int arr[] = {5, 6, 7, 8};
    TkSpan<int> s(arr, arr + 4);
    EXPECT_EQ(s.length(), 4);
    for (int i = 0; i < 4; ++i)
        EXPECT_EQ(s[i], arr[i]);
}

TEST(TkSpan, ArrayTemplateConstructor)
{
    int arr[3] = {10, 20, 30};
    TkSpan<int> s(arr);
    EXPECT_EQ(s.length(), 3);
    for (int i = 0; i < 3; ++i)
        EXPECT_EQ(s[i], arr[i]);
}

TEST(TkSpan, OperatorIndex)
{
    int arr[] = {1, 2, 3};
    TkSpan<int> s(arr);
    EXPECT_EQ(s[0], 1);
    EXPECT_EQ(s[1], 2);
    EXPECT_EQ(s[2], 3);

    // const span
    const TkSpan<int> cs(arr, 3);
    EXPECT_EQ(cs[0], 1);
    EXPECT_EQ(cs[1], 2);
    EXPECT_EQ(cs[2], 3);
}

TEST(TkSpan, Iterators)
{
    int arr[] = {1, 2, 3, 4, 5};
    TkSpan<int> s(arr);
    int sum = 0;
    for (auto it = s.begin(); it != s.end(); ++it)
        sum += *it;
    EXPECT_EQ(sum, 15);

    sum = 0;
    for (auto it = s.begin(); it != s.end(); ++it)
        sum += *it;
    EXPECT_EQ(sum, 15);

    // const iterators
    const TkSpan<int> cs(arr);
    sum = 0;
    for (auto it = cs.begin(); it != cs.end(); ++it)
        sum += *it;
    EXPECT_EQ(sum, 15);
}

TEST(TkSpan, GetTemplate)
{
    int arr[] = {1, 2, 3};
    TkSpan<int> s(arr, 3);

    auto p = s.get<double>(); // cast to U*
    EXPECT_EQ((int*)p, arr);
}

TEST(TkSpan, ToSpanFunctions)
{
    int arr[] = {1, 2, 3, 4};
    auto s1 = toSpan(arr, 4);
    EXPECT_EQ(s1.length(), 4);
    EXPECT_EQ(s1[0], 1);

    TkArray<int> a;
    a.add(5);
    a.add(6);
    a.add(7);
    auto s2 = toSpan(a);
    EXPECT_EQ(s2.length(), 3);
    EXPECT_EQ(s2[1], 6);
}

TEST(TkSpan, SpanContains)
{
    int arr[] = {10, 20, 30};
    TkSpan<int> s(arr);

    EXPECT_TRUE(spanContains(s, 10));
    EXPECT_TRUE(spanContains(s, 30));
    EXPECT_FALSE(spanContains(s, 40));

    TkSpan<int> empty;
    EXPECT_FALSE(spanContains(empty, 0));
}

TEST(TkSpan, EdgeCases)
{
    int single[] = {42};
    TkSpan<int> s(single);
    EXPECT_EQ(s.length(), 1);
    EXPECT_EQ(s[0], 42);

    TkSpan<int> empty(nullptr, 0);
    EXPECT_TRUE(empty.isInvalid());
    EXPECT_EQ(empty.length(), 0);
}