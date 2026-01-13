/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include <toolkit/platforms/unix/UnixThread.hpp>

#if TK_APPLE

#include <sched.h>
#include <pthread.h>
#include <mach/thread_policy.h>
#include <mach/thread_act.h>
#include <unistd.h>

namespace toolkit {
    
    void TkThread::setThreadPriority(ThreadPriority priority) {
        struct sched_param sched;
        ZERO_MEMORY(&sched, sizeof(struct sched_param));
        i32 policy = SCHED_RR;
        pthread_getschedparam(pthread_self(), &policy, &sched);
        sched.sched_priority = ThreadImpl::convertType(priority);
        pthread_setschedparam(pthread_self(), policy, &sched);
    }

    void TkThread::setThreadAffinityMask(u64 affinityMask) {
        thread_affinity_policy policy;
        policy.affinity_tag = affinityMask;
        thread_policy_set(pthread_mach_thread_np(pthread_self()), THREAD_AFFINITY_POLICY, (integer_t *) &policy,
                          THREAD_AFFINITY_POLICY_COUNT);
    }

    void TkThread::sleep(i32 milliseconds) {
        usleep(milliseconds * 1000);
    }

    u64 TkThread::getCurrentThreadID() {
        return (u64) pthread_mach_thread_np(pthread_self());
    }

    void ThreadImpl::killInternal(bool waitForJoin)
    {
        if (waitForJoin)
            pthread_join(m_thread, nullptr);
        pthread_kill(m_thread, SIGKILL);
    }

    u32 ThreadImpl::getStackSize(u32 s)
    {
        return s;
    }

    i32 ThreadImpl::start(pthread_attr_t &attr)
    {
        return pthread_create(&m_thread, &attr, threadProc, this);
    }

    i32 ThreadImpl::convertType(ThreadPriority t)
    {
        switch (t)
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
}

#endif