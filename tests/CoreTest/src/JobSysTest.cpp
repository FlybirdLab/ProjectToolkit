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

class TkJobSysFixture : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        System::init();
        System::beforeRun();
        System::logInfo();
        ASSERT_TRUE(TkJobSys::init());
    }

    static void TearDownTestSuite()
    {
        TkJobSys::beforeExit();
        TkJobSys::dispose();
        System::beforeQuit();
        System::quit();
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
        counter++;
    }, 1);

    ASSERT_EQ(counter.load(), 1);
}

TEST_F(TkJobSysFixture, DispatchAndWaitLabel)
{
    constexpr i32 kJobs = 32;
    std::atomic<i32> counter{0};

    i64 label = TkJobSys::dispatch([&](i32) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }, kJobs);

    TkJobSys::wait(label);
    ASSERT_EQ(counter.load(), kJobs);
}

TEST_F(TkJobSysFixture, WaitAll)
{
    std::atomic<i32> counter{0};

    TkJobSys::dispatch([&](i32) {
        counter++;
    }, 10);

    TkJobSys::dispatch([&](i32) {
        counter++;
    }, 20);

    TkJobSys::wait();
    ASSERT_EQ(counter.load(), 30);
}

TEST_F(TkJobSysFixture, ParallelIncrementCorrectness)
{
    constexpr i32 kJobs = 1000;
    std::atomic<i32> counter{0};

    i64 label = TkJobSys::dispatch([&](i32) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }, kJobs);

    TkJobSys::wait(label);
    ASSERT_EQ(counter.load(), kJobs);
}

TEST_F(TkJobSysFixture, JobIndexRange)
{
    constexpr i32 kJobs = 64;
    std::atomic<i32> seen{0};

    i64 label = TkJobSys::dispatch([&](i32 idx) {
        EXPECT_GE(idx, 0);
        EXPECT_LT(idx, kJobs);
        seen.fetch_add(1);
    }, kJobs);

    TkJobSys::wait(label);
    ASSERT_EQ(seen.load(), kJobs);
}

TEST_F(TkJobSysFixture, JobDependencies)
{
    std::atomic<i32> order{0};

    i64 jobA = TkJobSys::dispatch([&](i32) {
        EXPECT_EQ(order.load(), 0);
        order.store(1);
    }, 1);

    i64 jobB = TkJobSys::dispatch(
        [&](i32) {
            EXPECT_EQ(order.load(), 1);
            order.store(2);
        },
        TkSpan<i64>(&jobA, 1),
        1
    );

    TkJobSys::wait(jobB);
    ASSERT_EQ(order.load(), 2);
}

TEST_F(TkJobSysFixture, MultipleDependencies)
{
    std::atomic<i32> completed{0};

    i64 a = TkJobSys::dispatch([&](i32) { completed++; }, 1);
    i64 b = TkJobSys::dispatch([&](i32) { completed++; }, 1);

    i64 c = TkJobSys::dispatch(
        [&](i32) {
            EXPECT_EQ(completed.load(), 2);
            completed++;
        },
        TkSpan<i64>({a, b}),
        1
    );

    TkJobSys::wait(c);
    ASSERT_EQ(completed.load(), 3);
}

TEST_F(TkJobSysFixture, DisableAutoStart)
{
    TkJobSys::setJobStartingOnDispatch(false);

    std::atomic<i32> counter{0};

    TkJobSys::dispatch([&](i32) {
        counter++;
    }, 10);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_EQ(counter.load(), 0);

    TkJobSys::setJobStartingOnDispatch(true);
    TkJobSys::wait();

    ASSERT_EQ(counter.load(), 10);
}

TEST_F(TkJobSysFixture, StressDispatchWait)
{
    constexpr i32 kRounds = 100;
    constexpr i32 kJobs = 64;

    for (i32 r = 0; r < kRounds; r++)
    {
        std::atomic<i32> counter{0};

        i64 label = TkJobSys::dispatch([&](i32) {
            counter.fetch_add(1, std::memory_order_relaxed);
        }, kJobs);

        TkJobSys::wait(label);
        ASSERT_EQ(counter.load(), kJobs);
    }
}

