/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "../UnixThread.hpp"

#if TK_ANDROID

#include <cstdio>
#include <pthread.h>
#include <sched.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef SYS_gettid
#define SYS_gettid __NR_gettid
#endif

#ifndef SYS_sched_setaffinity
#define SYS_sched_setaffinity __NR_sched_setaffinity
#endif

namespace toolkit
{
    u64 getCurrentThreadIdNative()
    {
        return static_cast<u64>(::syscall(SYS_gettid));
    }

    const u64 g_mainThreadId = getCurrentThreadIdNative();

    void ThreadImpl::killInternal(bool waitForJoin)
    {
        if (!m_thread)
            return;

        if (waitForJoin)
        {
            pthread_join(m_thread, nullptr);
        }
        else
        {
            pthread_detach(m_thread);
        }
    }

    u32 ThreadImpl::getStackSize(u32 stackSize)
    {
        return stackSize;
    }

    i32 ThreadImpl::start(pthread_attr_t& attr)
    {
        const i32 result = pthread_create(&m_thread, &attr, ThreadImpl::threadProc, this);
        if (result == 0 && m_name.hasChars())
        {
            char name[16]{};
            ::snprintf(name, sizeof(name), "%s", m_name.getText());
            pthread_setname_np(m_thread, name);
        }
        return result;
    }

    i32 ThreadImpl::convertType(ThreadPriority priority)
    {
        const i32 minPriority = sched_get_priority_min(SCHED_RR);
        const i32 maxPriority = sched_get_priority_max(SCHED_RR);
        if (minPriority < 0 || maxPriority < 0 || minPriority >= maxPriority)
            return 0;

        switch (priority)
        {
        case ThreadPriority::kHighest:
            return maxPriority;
        case ThreadPriority::kAboveNormal:
            return minPriority + (maxPriority - minPriority) * 3 / 4;
        case ThreadPriority::kNormal:
            return minPriority + (maxPriority - minPriority) / 2;
        case ThreadPriority::kBelowNormal:
            return minPriority + (maxPriority - minPriority) / 4;
        case ThreadPriority::kLowest:
            return minPriority;
        }
        return 0;
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
        u64 mask = affinityMask;
        ::syscall(SYS_sched_setaffinity, 0, sizeof(mask), &mask);
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
