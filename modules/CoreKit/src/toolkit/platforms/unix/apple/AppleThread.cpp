/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "../UnixThread.hpp"

#if TK_APPLE

#include <mach/mach.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>

namespace toolkit
{
    u64 getCurrentThreadIdNative()
    {
        return static_cast<u64>(pthread_mach_thread_np(pthread_self()));
    }

    const u64 g_mainThreadId = getCurrentThreadIdNative();

    void ThreadImpl::killInternal(bool waitForJoin)
    {
        if (!m_thread)
            return;

        if (waitForJoin)
            pthread_join(m_thread, nullptr);
        else
            pthread_detach(m_thread);
    }

    u32 ThreadImpl::getStackSize(u32 stackSize)
    {
        return stackSize;
    }

    i32 ThreadImpl::start(pthread_attr_t& attr)
    {
        return pthread_create(&m_thread, &attr, ThreadImpl::threadProc, this);
    }

    i32 ThreadImpl::convertType(ThreadPriority priority)
    {
        switch (priority)
        {
        case ThreadPriority::kHighest:
            return 45;
        case ThreadPriority::kAboveNormal:
            return 37;
        case ThreadPriority::kNormal:
            return 31;
        case ThreadPriority::kBelowNormal:
            return 25;
        case ThreadPriority::kLowest:
            return 20;
        }
        return 31;
    }

    void TkThread::setThreadPriority(ThreadPriority priority)
    {
        sched_param sched{};
        i32 policy = SCHED_RR;
        pthread_getschedparam(pthread_self(), &policy, &sched);
        sched.sched_priority = policy == SCHED_OTHER ? 0 : ThreadImpl::convertType(priority);
        pthread_setschedparam(pthread_self(), policy, &sched);
    }

    void TkThread::setThreadAffinityMask(u64 affinityMask)
    {
#if TK_MACOS
        thread_affinity_policy policy;
        policy.affinity_tag = static_cast<integer_t>(affinityMask);
        thread_policy_set(pthread_mach_thread_np(pthread_self()), THREAD_AFFINITY_POLICY, reinterpret_cast<integer_t*>(&policy), THREAD_AFFINITY_POLICY_COUNT);
#else
        (void)affinityMask;
#endif
    }

    u64 TkThread::getCurrentThreadID()
    {
        return getCurrentThreadIdNative();
    }

    bool TkThread::isInMainThread()
    {
        return getCurrentThreadID() == getMainThreadId();
    }

    u64 TkThread::getMainThreadId()
    {
        return g_mainThreadId;
    }
}

#endif
