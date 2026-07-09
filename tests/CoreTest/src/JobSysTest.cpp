/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include <toolkit/threading/JobSys.hpp>
#include <toolkit/base/Function.hpp>
#include <toolkit/types/Span.hpp>
#include <toolkit/platforms/System.hpp>

#include <atomic>
#include <chrono>
#include <thread>

class TkJobSysFixture : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        System::init();
        System::beforeRun();
        ASSERT_TRUE(TkJobSys::init());
    }

    static void TearDownTestSuite()
    {
        TkJobSys::beforeExit();
        TkJobSys::dispose();
        System::beforeQuit();
        System::quit();
    }

    void SetUp() override
    {
        TkJobSys::setJobStartingOnDispatch(true);
    }
};

TEST_F(TkJobSysFixture, ThreadCount)
{
    const i32 thds = TkJobSys::getThreadsCount();
    ASSERT_GT(thds, 0);
}

TEST_F(TkJobSysFixture, ExecuteSingleJobSync)
{
    std::atomic<i32> counter{0};

    TkJobSys::execute([&](i32 idx) {
        EXPECT_EQ(idx, 0);
        counter.fetch_add(1, std::memory_order_relaxed);
    }, 1);

    ASSERT_EQ(counter.load(std::memory_order_relaxed), 1);
}

TEST_F(TkJobSysFixture, DispatchAndWaitLabel)
{
    constexpr i32 kJobs = 32;
    std::atomic<i32> counter{0};

    const i64 label = TkJobSys::dispatch([&](i32) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }, kJobs);

    TkJobSys::wait(label);
    ASSERT_EQ(counter.load(std::memory_order_relaxed), kJobs);
}

TEST_F(TkJobSysFixture, WaitAll)
{
    std::atomic<i32> counter{0};

    TkJobSys::dispatch([&](i32) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }, 10);

    TkJobSys::dispatch([&](i32) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }, 20);

    TkJobSys::wait();
    ASSERT_EQ(counter.load(std::memory_order_relaxed), 30);
}

TEST_F(TkJobSysFixture, ParallelIncrementCorrectness)
{
    constexpr i32 kJobs = 1000;
    std::atomic<i32> counter{0};

    const i64 label = TkJobSys::dispatch([&](i32) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }, kJobs);

    TkJobSys::wait(label);
    ASSERT_EQ(counter.load(std::memory_order_relaxed), kJobs);
}

TEST_F(TkJobSysFixture, JobIndexRange)
{
    constexpr i32 kJobs = 64;
    std::atomic<i32> seen{0};

    const i64 label = TkJobSys::dispatch([&](i32 idx) {
        EXPECT_GE(idx, 0);
        EXPECT_LT(idx, kJobs);
        seen.fetch_add(1, std::memory_order_relaxed);
    }, kJobs);

    TkJobSys::wait(label);
    ASSERT_EQ(seen.load(std::memory_order_relaxed), kJobs);
}

TEST_F(TkJobSysFixture, JobDependencies)
{
    std::atomic<i32> order{0};

    const i64 jobA = TkJobSys::dispatch([&](i32) {
        EXPECT_EQ(order.load(std::memory_order_relaxed), 0);
        order.store(1, std::memory_order_relaxed);
    }, 1);

    i64 dependencies[] = {jobA};
    const i64 jobB = TkJobSys::dispatch(
        [&](i32) {
            EXPECT_EQ(order.load(std::memory_order_relaxed), 1);
            order.store(2, std::memory_order_relaxed);
        },
        TkSpan<i64>(dependencies, 1),
        1);

    TkJobSys::wait(jobB);
    ASSERT_EQ(order.load(std::memory_order_relaxed), 2);
}

TEST_F(TkJobSysFixture, MultipleDependencies)
{
    std::atomic<i32> completed{0};

    const i64 a = TkJobSys::dispatch([&](i32) {
        completed.fetch_add(1, std::memory_order_relaxed);
    }, 1);
    const i64 b = TkJobSys::dispatch([&](i32) {
        completed.fetch_add(1, std::memory_order_relaxed);
    }, 1);

    i64 dependencies[] = {a, b};
    const i64 c = TkJobSys::dispatch(
        [&](i32) {
            EXPECT_EQ(completed.load(std::memory_order_relaxed), 2);
            completed.fetch_add(1, std::memory_order_relaxed);
        },
        TkSpan<i64>(dependencies, 2),
        1);

    TkJobSys::wait(c);
    ASSERT_EQ(completed.load(std::memory_order_relaxed), 3);
}

TEST_F(TkJobSysFixture, DisableAutoStart)
{
    TkJobSys::setJobStartingOnDispatch(false);

    std::atomic<i32> counter{0};

    TkJobSys::dispatch([&](i32) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }, 10);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_EQ(counter.load(std::memory_order_relaxed), 0);

    TkJobSys::setJobStartingOnDispatch(true);
    TkJobSys::wait();

    ASSERT_EQ(counter.load(std::memory_order_relaxed), 10);
}

TEST_F(TkJobSysFixture, StressDispatchWait)
{
    constexpr i32 kRounds = 50;
    constexpr i32 kJobs = 64;

    for (i32 r = 0; r < kRounds; r++)
    {
        std::atomic<i32> counter{0};

        const i64 label = TkJobSys::dispatch([&](i32) {
            counter.fetch_add(1, std::memory_order_relaxed);
        }, kJobs);

        TkJobSys::wait(label);
        ASSERT_EQ(counter.load(std::memory_order_relaxed), kJobs);
    }
}
