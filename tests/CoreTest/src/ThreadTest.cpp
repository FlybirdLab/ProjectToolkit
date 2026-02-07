/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "toolkit/threading/Thread.hpp"
#include "common.hpp"
#include "toolkit/threading/Runnable.hpp"

#include <atomic>

struct TestRunnable : public IRunnable
{
    std::atomic<bool> inited{false};
    std::atomic<bool> exited{false};
    std::atomic<bool> afterWorked{false};
    std::atomic<bool> stopped{false};
    std::atomic<i32> exitCode{-1};

    bool onInit() override
    {
        LogInfo("[Runnable] onInit");
        inited.store(true, std::memory_order_relaxed);
        return true;
    }

    i32 onRun() override
    {
        LogInfo("[Runnable] onRun begin, tid=%i", TkThread::getCurrentThreadID());

        TkThread::setThreadPriority(ThreadPriority::kAboveNormal);
        TkThread::setThreadAffinityMask(1ull << 0);

        for (int i = 0; i < 5; ++i)
        {
            TkThread::sleep(50);
        }

        LogInfo("[Runnable] onRun end");
        return 42;
    }

    void onExit() override
    {
        LogInfo("[Runnable] onExit");
        exited.store(true, std::memory_order_relaxed);
    }

    void afterWork(bool killed) override
    {
        LogInfo("[Runnable] afterWork killed=%b", killed);
        afterWorked.store(true, std::memory_order_relaxed);
    }

    void onStop() override
    {
        LogInfo("[Runnable] onStop");
        stopped.store(true, std::memory_order_relaxed);
    }
};

static std::atomic<bool> g_threadStarted{false};
static std::atomic<bool> g_threadExited{false};

static void onThreadStarting(TkThread *t)
{
    LogDebug("[Event] Test!!!");
    LogInfo("[Event] ThreadStarting name=%s, id=%i",
            t->getName(), t->getID());
    g_threadStarted.store(true);
}

static void onThreadExiting(TkThread *t, i32 code)
{
    LogInfo("[Event] ThreadExiting name=%s, code=%i",
            t->getName(), code);
    g_threadExited.store(true);
}

TEST(ThreadTest, RunnableLifecycle)
{
    TkThread::ThreadStarting.bind(onThreadStarting);
    TkThread::ThreadExiting.bind(onThreadExiting);

    TestRunnable runnable;

    ThreadHandle th = TkThread::newThread(
        &runnable,
        "TestWorker",
        ThreadPriority::kNormal);

    ASSERT_TRUE(th != nullptr);

    th->setPriority(ThreadPriority::kHighest);

    th->join();

    // ---------- Runnable ----------
    EXPECT_TRUE(runnable.inited.load());
    EXPECT_TRUE(runnable.exited.load());
    EXPECT_TRUE(runnable.afterWorked.load());

    EXPECT_FALSE(runnable.stopped.load());

    // ---------- Thread event test ----------
    EXPECT_TRUE(g_threadStarted.load());
    EXPECT_TRUE(g_threadExited.load());

    TkThread::destroy(th);
}

TEST(TSync, BasicLockUnlock)
{
    TkMutex mtx;
    mtx.lock();
    mtx.unlock();
    SUCCEED();
}

TEST(TSync, MultiThreadIncrement)
{
    TkMutex mtx;
    int value = 0;

    constexpr int kThreads = 8;
    constexpr int kIters = 100000;

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i)
    {
        threads.emplace_back([&]
                             {
            for (int j = 0; j < kIters; ++j)
            {
                TkMutexGuard<TkMutex> g(mtx);
                ++value;
            } });
    }

    for (auto &t : threads)
        t.join();

    ASSERT_EQ(value, kThreads * kIters);
}

TEST(TSync, ReadWriteBasic)
{
    TkRwMutex rw;
    int value = 0;

    std::atomic<bool> stop{false};

    std::thread writer([&] {
        for (int i = 0; i < 1000; ++i)
        {
            rw.lockWrite();
            ++value;
            rw.unlockWrite();
        }
        stop.store(true);
    });

    std::thread reader([&] {
        while (!stop.load())
        {
            rw.lockRead();
            int v = value;
            (void)v;
            rw.unlockRead();
        }
    });

    writer.join();
    reader.join();

    ASSERT_EQ(value, 1000);
}

TEST(TSync, NotifyOne)
{
    TkMutex mtx;
    TkCondVar cv;

    bool ready = false;

    std::thread t([&] {
        mtx.lock();
        while (!ready)
            cv.wait(mtx);
        mtx.unlock();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    mtx.lock();
    ready = true;
    cv.notifyOne();
    mtx.unlock();

    t.join();
    SUCCEED();
}

TEST(TSync, NotifyAll)
{
    TkMutex mtx;
    TkCondVar cv;

    int wakeCount = 0;
    constexpr int kThreads = 5;

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i)
    {
        threads.emplace_back([&] {
            mtx.lock();
            cv.wait(mtx);
            ++wakeCount;
            mtx.unlock();
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    mtx.lock();
    cv.notifyAll();
    mtx.unlock();

    for (auto& t : threads)
        t.join();

    ASSERT_EQ(wakeCount, kThreads);
}

TEST(TSync, WaitTimeout)
{
    TkMutex mtx;
    TkCondVar cv;

    mtx.lock();
    bool res = cv.wait(mtx, 10);
    mtx.unlock();

    ASSERT_FALSE(res);
}

TEST(TSync, Basic)
{
    TkSpinLock lock;
    lock.lock();
    lock.unlock();
    SUCCEED();
}

TEST(TSync, SpinLockMultiThreadIncrement)
{
    TkSpinLock lock;
    int value = 0;

    constexpr int kThreads = 8;
    constexpr int kIters = 50000;

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i)
    {
        threads.emplace_back([&] {
            for (int j = 0; j < kIters; ++j)
            {
                lock.lock();
                ++value;
                lock.unlock();
            }
        });
    }

    for (auto& t : threads)
        t.join();

    ASSERT_EQ(value, kThreads * kIters);
}

TEST(TSync, TkBarrierBasic)
{
    constexpr int kThreads = 4;
    TkBarrier barrier(kThreads);

    std::atomic<int> counter{0};

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i)
    {
        threads.emplace_back([&] {
            ++counter;
            barrier.wait();
            ++counter;
        });
    }

    for (auto& t : threads)
        t.join();

    ASSERT_EQ(counter.load(), kThreads * 2);
}

TEST(TSync, TkSemaphoreBasic)
{
    TkSemaphore sem(0, 1);

    bool entered = false;

    std::thread t([&] {
        sem.wait();
        entered = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_FALSE(entered);

    sem.signal();
    t.join();

    ASSERT_TRUE(entered);
}

TEST(TSync, Timeout)
{
    TkSemaphore sem(0, 1);
    bool res = sem.wait(10);
    ASSERT_FALSE(res);
}

TEST(TSync, InterlockedOps)
{
    i64 v = 0;

    ASSERT_EQ(TSync::interlockedIncrement(&v), 1);
    ASSERT_EQ(TSync::interlockedIncrement(&v), 2);
    ASSERT_EQ(TSync::interlockedDecrement(&v), 1);
    ASSERT_EQ(TSync::interlockedAdd(&v, 10), 1);
    ASSERT_EQ(v, 11);
}

TEST(TSync, AtomicReadStore)
{
    i32 v = 0;
    TSync::atomicStore(&v, 42);
    ASSERT_EQ(TSync::atomicRead(&v), 42);
}