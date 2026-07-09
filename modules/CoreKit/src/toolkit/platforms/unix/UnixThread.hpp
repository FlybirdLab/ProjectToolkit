/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: khfahqp khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

#if TK_POSIX

#include <toolkit/threading/Thread.hpp>
#include <pthread.h>

namespace toolkit
{
    struct ThreadImpl : public TkThread
    {
    public:
        IAllocator& m_alloc;
        IRunnable *m_runnable;
#if TK_DEBUG
        TkString m_runnableName;
#endif
        ThreadPriority m_priority;
        TkString m_name;
        u64 m_id = 0;
        bool m_isRunning = false;
        bool m_callAfterWork = true;

        pthread_t m_thread;

    public:
        i32 run();

        static i32 convertType(ThreadPriority);
        void killInternal(bool waitForJoin);
        u32 getStackSize(u32);
        i32 start(pthread_attr_t &attr);

        TkThread* setUpThread(u32 stackSize);

        ThreadImpl(IRunnable *runnable, const TkString &name, ThreadPriority priority, IAllocator& alloc);
        ~ThreadImpl()
        {
            TK_ASSERT(m_thread == 0);
        }

    public:
        static void *threadProc(void *pThis);
    };
}

#endif
