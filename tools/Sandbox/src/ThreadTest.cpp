/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "ThreadTest.hpp"

#include <toolkit/threading/Thread.hpp>
#include <toolkit/threading/Runnable.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/base/Function.hpp>

namespace toolkit
{

    using namespace toolkit;

    struct TestRunnable : public IRunnable
    {
        bool onInit() override
        {
            LogInfo("[Runnable] onInit");
            return true;
        }

        i32 onRun() override
        {
            LogInfo("[Runnable] onRun begin, tid=%i", TkThread::getCurrentThreadID());

            TkThread::setThreadPriority(ThreadPriority::kAboveNormal);
            TkThread::setThreadAffinityMask(1ull << 0);

            for (int i = 0; i < 5; ++i)
            {
                LogInfo("[Runnable] running %i", i);
                TkThread::sleep(200);
            }

            LogInfo("[Runnable] onRun end");
            return 42;
        }

        void onExit() override
        {
            LogInfo("[Runnable] onExit");
        }

        void afterWork(bool killed) override
        {
            LogInfo("[Runnable] afterWork killed=%b", killed);
        }

        void onStop() override
        {
            LogInfo("[Runnable] onStop");
        }
    };
    static void onThreadStarting(TkThread *t)
    {
        LogInfo("[Event] ThreadStarting name=%s, id=%i",
                t->getName(), t->getID());
    }

    static void onThreadExiting(TkThread *t, i32 code)
    {
        LogInfo("[Event] ThreadExiting name=%s, code=%i",
                t->getName(), code);
    }
    
    void testThread()
    {
        using namespace toolkit;

        // TkThread::getMainThreadId();
        // or : TkThread::s_mainThreadId = TkThread::getCurrentThreadID();

        // LogInfo("[Main] tid={0}", TkThread::getCurrentThreadID());
        // LogInfo("[Main] isInMainThread={0}", TkThread::isInMainThread());

        TkThread::ThreadStarting.bind(onThreadStarting);
        TkThread::ThreadExiting.bind(onThreadExiting);

        TestRunnable runnable;

        ThreadHandle th = TkThread::newThread(
            &runnable,
            "TestWorker",
            ThreadPriority::kNormal);

        TK_ASSERT(th);

        th->setPriority(ThreadPriority::kHighest);

        TkThread::sleep(500);

        th->join();

        LogInfo("[Main] worker joined");

        TkThread::destroy(th);

        LogInfo("[Main] exit");
    }

}