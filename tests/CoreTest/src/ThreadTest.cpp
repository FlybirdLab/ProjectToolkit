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
    std::atomic<i32>  exitCode{-1};

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

static void onThreadStarting(TkThread* t)
{
    LogInfo("[Event] ThreadStarting name=%s, id=%i",
            t->getName(), t->getID());
    g_threadStarted.store(true);
}

static void onThreadExiting(TkThread* t, i32 code)
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