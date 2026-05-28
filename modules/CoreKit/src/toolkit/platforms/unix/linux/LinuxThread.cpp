/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include <toolkit/platforms/unix/UnixThread.hpp>

#if TK_LINUX

#include <sched.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

namespace toolkit {

    static const u64 g_mainThreadId = TkThread::getCurrentThreadID();

    bool TkThread::isInMainThread()
    {
        return getCurrentThreadID() == getMainThreadId();
    }

    u64 TkThread::getMainThreadId()
    {
        return g_mainThreadId;
    }

    void TkThread::setThreadPriority(ThreadPriority priority)
    {
        struct sched_param sched{};
        i32 policy = SCHED_RR;
        pthread_getschedparam(pthread_self(), &policy, &sched);
        sched.sched_priority = ThreadImpl::convertType(priority);
        pthread_setschedparam(pthread_self(), policy, &sched);
    }

    void TkThread::setThreadAffinityMask(u64 affinityMask)
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        for (i32 i = 0; i < 64; ++i)
        {
            if ((affinityMask & (1ull << i)) != 0)
                CPU_SET(i, &cpuset);
        }
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    }

    void TkThread::sleep(i32 milliseconds)
    {
        usleep(static_cast<useconds_t>(milliseconds) * 1000);
    }

    u64 TkThread::getCurrentThreadID()
    {
        const pthread_t thread = pthread_self();
        u64 result = 0;
        memcpy(&result, &thread, sizeof(thread) < sizeof(result) ? sizeof(thread) : sizeof(result));
        return result;
    }
    
    void ThreadImpl::killInternal(bool waitForJoin) {
        if (waitForJoin)
            pthread_join(m_thread, nullptr);
        pthread_kill(m_thread, SIGKILL);
    }


    u32 ThreadImpl::getStackSize(u32 s) {
        return s;
    }


    i32 ThreadImpl::start(pthread_attr_t& attr) {
        const int result = pthread_create(&m_thread, &attr, threadProc, this);
        if (result == 0)
            pthread_setname_np(m_thread, m_name.getText());
        return result;
    }


    // Linux & Android version
    i32 ThreadImpl::convertType(ThreadPriority t)
    {
        switch (t)
        {
        case ThreadPriority::kHighest:
            return 30;
        case ThreadPriority::kAboveNormal:
            return 25;
        case ThreadPriority::kNormal:
            return 15;
        case ThreadPriority::kBelowNormal:
            return 5;
        case ThreadPriority::kLowest:
            return 1;
        }
        return 0;
    }
}

#endif
