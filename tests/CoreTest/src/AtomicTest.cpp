/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

#include <toolkit/types/Atomic.hpp>

TEST(TkAtomic, DefaultConstruct)
{
    TkAtomic<int> a;
    EXPECT_EQ(a.load(), 0);
}

TEST(TkAtomic, StoreLoad)
{
    TkAtomic<int> a;
    a.store(42);
    EXPECT_EQ(a.load(), 42);
}

TEST(TkAtomic, Exchange)
{
    TkAtomic<int> a(10);
    int old = a.exchange(20);
    EXPECT_EQ(old, 10);
    EXPECT_EQ(a.load(), 20);
}

TEST(TkAtomic, FetchAdd)
{
    TkAtomic<int> a(1);
    EXPECT_EQ(a.fetchAdd(2), 1);
    EXPECT_EQ(a.load(), 3);
}

TEST(TkAtomic, FetchSub)
{
    TkAtomic<int> a(5);
    EXPECT_EQ(a.fetchSub(3), 5);
    EXPECT_EQ(a.load(), 2);
}

TEST(TkAtomic, FetchOr)
{
    TkAtomic<int> a(0b0011);
    EXPECT_EQ(a.fetchOr(0b0100), 0b0011);
    EXPECT_EQ(a.load(), 0b0111);
}

TEST(TkAtomic, FetchAnd)
{
    TkAtomic<int> a(0b0111);
    EXPECT_EQ(a.fetchAnd(0b0011), 0b0111);
    EXPECT_EQ(a.load(), 0b0011);
}

TEST(TkAtomic, CompareExchangeSuccess)
{
    TkAtomic<int> a(10);
    int expected = 10;
    bool ok = a.compareExchange(expected, 20);
    EXPECT_TRUE(ok);
    EXPECT_EQ(a.load(), 20);
}

TEST(TkAtomic, CompareExchangeFail)
{
    TkAtomic<int> a(10);
    int expected = 5;
    bool ok = a.compareExchange(expected, 20);
    EXPECT_FALSE(ok);
    EXPECT_EQ(expected, 10);
    EXPECT_EQ(a.load(), 10);
}

TEST(TkAtomic, MinSingleThread)
{
    TkAtomic<int> a(10);
    a.min(5);
    EXPECT_EQ(a.load(), 5);

    a.min(8);
    EXPECT_EQ(a.load(), 5);
}

TEST(TkAtomic, MaxSingleThread)
{
    TkAtomic<int> a(10);
    a.max(20);
    EXPECT_EQ(a.load(), 20);

    a.max(15);
    EXPECT_EQ(a.load(), 20);
}

#include <thread>
#include <vector>

TEST(TkAtomic, FetchAddMultiThread)
{
    TkAtomic<int> counter(0);

    constexpr int kThreads = 8;
    constexpr int kIters = 100000;

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i)
    {
        threads.emplace_back([&]()
                             {
            for (int j = 0; j < kIters; ++j)
                counter.fetchAdd(1); });
    }

    for (auto &t : threads)
        t.join();

    EXPECT_EQ(counter.load(), kThreads * kIters);
}

TEST(TkAtomic, MinMultiThread)
{
    TkAtomic<int> value(1000);

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back([&, i]()
                             { value.min(i * 10); });
    }

    for (auto &t : threads)
        t.join();

    EXPECT_EQ(value.load(), 0);
}

TEST(TkAtomic, MaxMultiThread)
{
    TkAtomic<int> value(0);

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back([&, i]()
                             { value.max(i * 10); });
    }

    for (auto &t : threads)
        t.join();

    EXPECT_EQ(value.load(), 90);
}
