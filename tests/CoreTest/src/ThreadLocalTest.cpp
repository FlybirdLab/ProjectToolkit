/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include <toolkit/collection/Array.hpp>
#include <toolkit/threading/ThreadLocal.hpp>
#include <toolkit/threading/ThreadRegistry.hpp>
#include <toolkit/threading/Thread.hpp>
#include <toolkit/threading/Runnable.hpp>
#include <toolkit/platforms/System.hpp>

#include <atomic>
#include <thread>
#include <vector>

namespace
{
    struct BlockingRunnable : public IRunnable
    {
        std::atomic<bool> started{false};
        std::atomic<bool> release{false};

        i32 onRun() override
        {
            started.store(true, std::memory_order_release);
            while (!release.load(std::memory_order_acquire))
            {
                TkThread::sleep(1);
            }
            return 0;
        }
    };
}

class ThreadLocalTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        System::init();
        System::beforeRun();
    }

    static void TearDownTestSuite()
    {
        System::beforeQuit();
        System::quit();
    }
};

TEST_F(ThreadLocalTest, StoresIndependentValuesPerThread)
{
    ThreadLocal<i32, 8> local;
    local.set(7);

    constexpr i32 kThreads = 4;
    std::vector<std::thread> threads;
    for (i32 i = 0; i < kThreads; i++)
    {
        threads.emplace_back([&, i] {
            local.set(100 + i);
            EXPECT_EQ(local.get(), 100 + i);
        });
    }

    for (auto &thread : threads)
        thread.join();

    EXPECT_EQ(local.get(), 7);

    TkArray<i32> values;
    local.getvals(values);
    EXPECT_EQ(values.count(), kThreads + 1);
    EXPECT_TRUE(values.contains(7));
    for (i32 i = 0; i < kThreads; i++)
        EXPECT_TRUE(values.contains(100 + i));
}

TEST_F(ThreadLocalTest, ClearResetsBuckets)
{
    ThreadLocal<i32, 8> local;
    local.set(11);
    ASSERT_EQ(local.count(), 1);

    local.clear();

    EXPECT_EQ(local.count(), 0);

    TkArray<i32> values;
    local.getvals(values);
    EXPECT_EQ(values.count(), 0);
}

TEST_F(ThreadLocalTest, ThreadRegistryTracksRunningThread)
{
    const i32 baseCount = ThreadRegistry::count();

    auto *runnable = new BlockingRunnable();
    ThreadHandle thread = TkThread::newThread(runnable, "RegistryWorker", ThreadPriority::kNormal);
    ASSERT_NE(thread, nullptr);

    while (!runnable->started.load(std::memory_order_acquire))
        TkThread::sleep(1);

    const u64 id = thread->getID();
    ASSERT_NE(id, 0ull);

    EXPECT_GE(ThreadRegistry::count(), baseCount + 1);
    EXPECT_EQ(ThreadRegistry::getThread(id), thread);

    runnable->release.store(true, std::memory_order_release);
    thread->join();

    EXPECT_EQ(ThreadRegistry::getThread(id), nullptr);
    EXPECT_EQ(ThreadRegistry::count(), baseCount);

    TkThread::destroy(thread);
    delete runnable;
}
