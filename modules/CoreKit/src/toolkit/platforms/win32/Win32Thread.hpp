/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>

#if TK_WINDOWS

#include <toolkit/threading/Thread.hpp>

namespace toolkit {

    struct ThreadImpl : public TkThread
    {
    public:
        IAllocator& m_alloc;
        IRunnable* m_runnable;
#if TK_DEBUG
        TkString m_runnableName;
#endif
        ThreadPriority m_priority;
        TkString m_name;
        u64 m_id = 0;
        bool m_isRunning = false;
        bool m_callAfterWork = true;

        void* m_thread = nullptr;

        i32 run();

        static i32 convertType(ThreadPriority);
        void killInternal(bool waitForJoin);
        u32 getStackSize(u32);
        i32 start(u32 stackSize);

        TkThread* setUpThread(u32 stackSize);

        ThreadImpl(IRunnable* runnable, const TkString& name, ThreadPriority priority, IAllocator& alloc);
        ~ThreadImpl() {
            TK_ASSERT(m_thread == 0);
        }

    public:
        static unsigned __stdcall threadProc(void* pThis);
    };
}

#endif