/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "UnixThread.hpp"

#if TK_POSIX

#if TK_USE_RPMALLOC
#include <rpmalloc/rpmalloc.h>
#endif

#include <toolkit/base/Function.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/threading/ThreadRegistry.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/threading/Runnable.hpp>

namespace toolkit {

    TkAction<TkThread*> TkThread::ThreadStarting{};
    TkAction<TkThread*, i32> TkThread::ThreadExiting{};


    IRunnable* TkThread::getRunnable() {
        return m_impl->m_runnable;
    }


    TkThread* ThreadImpl::setUpThread(u32 stackSize)
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        stackSize = getStackSize(stackSize);
        if (stackSize != 0)
            pthread_attr_setstacksize(&attr, stackSize);
        const i32 result = start(attr);
        if (result != 0) {
            LogWarn("Failed to spawn a thread. Result code: {0}", result);
            TkMemory::deletePtr(m_alloc, this);
            return nullptr;
        }

        setPriority(getPriority());

        return this;
    }


    void* ThreadImpl::threadProc(void* pThis)
    {
        auto thread = (ThreadImpl*)pThis;
#if PLATFORM_APPLE_FAMILY
        // Apple doesn't support creating named thread so assign name here
        {
            pthread_setname_np(TkString{ thread->m_name }.get());
        }
#endif
        const i32 exitCode = thread->run();
        return (void*)(uintptr)exitCode;
    }


    i32 ThreadImpl::run()
    {
        // Setup
        TK_ASSERT(m_runnable);
        m_id = getCurrentThreadID();
        
#if TK_USE_RPMALLOC
        // FIX: Rpmalloc need initlize thread local storage on each thread.
        rpmalloc_thread_initialize();
#endif

#if TRACY_ENABLE
        char threadName[100];
        const i32 threadNameLength = math::min<i32>(ARRAY_COUNT(threadName) - 1, m_name.length());
        string::convertUTF162ANSI(*m_name, threadName, threadNameLength);
        threadName[threadNameLength] = 0;
        tracy::setThreadName(threadName);
#endif
        ThreadRegistry::add(this);
        TkThread::ThreadStarting(this);
        i32 exitCode = 1;
        m_isRunning = true;

        LogInfo("Thread \'{0}\' ID=0x{1:x} started with priority {2}", m_name, m_id, toString(getPriority())); //TODO ::toolkit::

        if (m_runnable->onInit())
        {
            exitCode = m_runnable->onRun();

            if (m_callAfterWork) // Prevent from calling this after calling AfterWork since object may be deleted
                m_runnable->onExit();
        }

        LogInfo("Thread \'{0}\' ID=0x{1:x} exits with code {2}", m_name, m_id, exitCode);

        // End
        if (m_callAfterWork)
        {
            m_callAfterWork = false;
            m_runnable->afterWork(false);
        }
        m_isRunning = false;
        TkThread::ThreadExiting(this, exitCode);
        ThreadRegistry::remove(this);

        return exitCode;
    }


    ThreadImpl::ThreadImpl(IRunnable* runnable, const TkString& name, ThreadPriority priority, IAllocator& alloc)
        : TkThread(this)
        , m_runnable{ runnable }
        , m_name{ name }
        , m_priority{ priority }
        , m_thread{ 0 }
        , m_alloc {alloc}
    {
        TK_ASSERT(runnable);
    }

    IAllocator& TkThread::getAllocator() {
        return m_impl->m_alloc;
    }

    ThreadPriority TkThread::getPriority() {
        return m_impl->m_priority;
    }


    void TkThread::setPriority(ThreadPriority priority) {
        if (m_impl->m_priority == priority)
            return;
        m_impl->m_priority = priority;

        struct sched_param sched;
        ZERO_MEMORY(&sched, sizeof(struct sched_param));
        i32 policy = SCHED_RR;
        pthread_getschedparam(m_impl->m_thread, &policy, &sched);
        sched.sched_priority = m_impl->convertType(priority);
        pthread_setschedparam(m_impl->m_thread, policy, &sched);
    }


    u64 TkThread::getID() {
        return m_impl->m_id;
    }


    bool TkThread::isRunning() {
        return m_impl->m_isRunning;
    }


    const TkString& TkThread::getName() {
        return m_impl->m_name;
    }


    void TkThread::kill(bool waitForJoin) {
        if (!m_impl->m_isRunning)
        {
            m_impl->m_thread = 0;
            return;
        }
        TK_ASSERT(getID());

        // Stop runnable object
        if (m_impl->m_callAfterWork && m_impl->m_runnable)
        {
            m_impl->m_runnable->onStop();
        }

        LogInfo("Killing thread \'{0}\' ID=0x{1:x}", m_impl->m_name, m_impl->m_id);

        // Kill platform thread
        m_impl->killInternal(waitForJoin);
        m_impl->m_thread = 0;

        // End
        if (m_impl->m_callAfterWork)
        {
            m_impl->m_callAfterWork = false;
            m_impl->m_runnable->afterWork(true);
        }
        m_impl->m_isRunning = false;
        ThreadRegistry::remove(this);
    }


    void TkThread::join() {
        pthread_join(m_impl->m_thread, nullptr);
        m_impl->m_thread = 0;
    }


    ThreadHandle TkThread::newThread(struct IRunnable* r, const TkString& name, ThreadPriority priority, IAllocator& alloc) {
        return TkThread::newThread(r, name, alloc, priority, 0);
    }


    ThreadHandle TkThread::newThread(IRunnable* r, const TkString& name, IAllocator& alloc, ThreadPriority priority, u32 stackSize) {
        auto* thread = TkMemory::newObject<ThreadImpl>(alloc, r, name, priority);
        thread->setUpThread(stackSize);

        return thread;
    }

    void TkThread::destroy(ThreadHandle t) {
        TkMemory::deletePtr(t->getAllocator(), t);
    }
}

#endif