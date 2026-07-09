/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

#include <toolkit/threading/Task.hpp>
#include <toolkit/threading/ThreadPool.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/base/Function.hpp>
#include <toolkit/threading/Thread.hpp>

#include <thread>

class ThreadPoolTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        System::init();
        System::beforeRun();
        ASSERT_TRUE(TkThreadPool::init());
    }

    static void TearDownTestSuite()
    {
        TkThreadPool::beforeExit();
        TkThreadPool::dispose();
        System::beforeQuit();
        System::quit();
    }
};

TEST_F(ThreadPoolTest, SingleTaskExecute)
{
    std::atomic<int> value{0};

    auto task = TkTask::startNew([&]()
                                 { value.store(42); });

    EXPECT_FALSE(task->wait(2000));
    EXPECT_EQ(value.load(), 42);
}

TEST_F(ThreadPoolTest, MultipleTasksExecute)
{
    constexpr int kTasks = 64;
    std::atomic<int> counter{0};
    TkArray<TkTask *> tasks;

    for (int i = 0; i < kTasks; i++)
    {
        tasks.add(TkTask::startNew([&]()
                                   { counter.fetch_add(1); }));
    }

    for (int i = 0; i < tasks.count(); i++)
        EXPECT_FALSE(tasks[i]->wait(2000));

    EXPECT_EQ(counter.load(), kTasks);
}

TEST_F(ThreadPoolTest, TaskWaitTimeout)
{
    auto task = TkTask::startNew([]()
                                 { TkThread::sleep(100); });

    bool timeout = task->wait(10);
    EXPECT_TRUE(timeout); // timeout == true

    EXPECT_FALSE(task->wait(500));
}

TEST_F(ThreadPoolTest, BoolTaskFail)
{
    auto task = TkTask::startNew([]() -> bool
                                 {
                                     return true; // true = failed
                                 });

    EXPECT_TRUE(task->wait(2000));
    EXPECT_TRUE(task->isFailed());
}

TEST_F(ThreadPoolTest, ContinueWithChain)
{
    std::atomic<int> value{0};

    auto t1 = TkMemory::newObject<ThreadPoolActionTask>(
        TkMemory::getDefault(),
        [&]()
        {
            EXPECT_EQ(value.load(), 0);
            value.store(1);
        });

    auto t2 = t1->continueWith([&]()
                               {
        EXPECT_EQ(value.load(), 1);
        value.store(2); });

    auto t3 = t2->continueWith([&]()
                               {
        EXPECT_EQ(value.load(), 2);
        value.store(3); });

    t1->start();

    const double startTime = System::getTimeSeconds() * 1000.0;
    while (value.load() != 3 && System::getTimeSeconds() * 1000.0 - startTime < 2000.0)
        TkThread::sleep(1);

    EXPECT_EQ(value.load(), 3);
}

TEST_F(ThreadPoolTest, CancelBeforeRun)
{
    std::atomic<bool> ran{false};

    auto task = TkTask::startNew([&]()
                                 { ran.store(true); });

    task->cancel();

    EXPECT_TRUE(task->wait(2000));
    EXPECT_FALSE(ran.load());
    EXPECT_TRUE(task->isCanceled());
}

TEST_F(ThreadPoolTest, CancelWhileRunning)
{
    std::atomic<bool> entered{false};

    auto task = TkTask::startNew([&]()
                                 {
        entered.store(true);
        TkThread::sleep(50); });

    while (!entered.load())
        std::this_thread::yield();

    task->cancel();

    EXPECT_TRUE(task->wait(3000));
}

TEST_F(ThreadPoolTest, MainThreadTaskExecute)
{
    std::atomic<int> value{0};

    auto task = TkMemory::newObject<MainThreadActionTask>(
        TkMemory::getDefault(),
        [&]() -> void
        { value.store(1); });

    task->start();

    MainThreadTask::runAll(0.016f);

    EXPECT_EQ(value.load(), 1);
}

TEST_F(ThreadPoolTest, MainThreadTaskDelay)
{
    std::atomic<int> value{0};

    auto task = TkMemory::newObject<MainThreadActionTask>(
        TkMemory::getDefault(),
        [&]() -> void
        { value.store(1); });

    task->initialDelay = 0.05f;
    task->start();

    MainThreadTask::runAll(0.01f);
    EXPECT_EQ(value.load(), 0);

    MainThreadTask::runAll(0.05f);
    EXPECT_EQ(value.load(), 1);
}

TEST_F(ThreadPoolTest, StressTest)
{
    constexpr int kTasks = 500;
    std::atomic<int> counter{0};

    for (int i = 0; i < kTasks; i++)
    {
        TkTask::startNew([&]()
                         { counter.fetch_add(1); });
    }

    while (counter.load() != kTasks)
        TkThread::sleep(1);

    EXPECT_EQ(counter.load(), kTasks);
}
