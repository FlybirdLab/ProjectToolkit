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

#include <cerrno>
#include <ctime>
#include <sched.h>
#include <unistd.h>

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
        pthread_attr_destroy(&attr);
        if (result != 0) {
            LogWarn("Failed to spawn a thread. Result code: %i", result);
            TkMemory::deletePtr(m_alloc, this);
            return nullptr;
        }

        const ThreadPriority priority = m_priority;
        m_priority = ThreadPriority::kNormal;
        setPriority(priority);

        return this;
    }


    void* ThreadImpl::threadProc(void* pThis)
    {
        auto thread = (ThreadImpl*)pThis;
#if TK_APPLE
        // Apple doesn't support creating named thread so assign name here
        {
            pthread_setname_np(thread->m_name.getText());
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

#if TK_TRACY_ENABLE
        tracy::setThreadName(m_name.getText());
#endif
        ThreadRegistry::add(this);
        TkThread::ThreadStarting(this);
        i32 exitCode = 1;
        m_isRunning = true;

        LogDebug("Thread '%s' ID=0x%llx started with priority %s", m_name.getText(), static_cast<unsigned long long>(m_id), toString(getPriority()));

        if (m_runnable->onInit())
        {
            exitCode = m_runnable->onRun();

            if (m_callAfterWork) // Prevent from calling this after calling AfterWork since object may be deleted
                m_runnable->onExit();
        }

        LogDebug("Thread '%s' ID=0x%llx exits with code %i", m_name.getText(), static_cast<unsigned long long>(m_id), exitCode);

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

        struct sched_param sched {};
        i32 policy = SCHED_RR;
        pthread_getschedparam(m_impl->m_thread, &policy, &sched);
        sched.sched_priority = policy == SCHED_OTHER ? 0 : m_impl->convertType(priority);
        pthread_setschedparam(m_impl->m_thread, policy, &sched);
    }


    void TkThread::sleep(i32 milliseconds)
    {
        if (milliseconds <= 0)
        {
            sched_yield();
            return;
        }

        timespec request {};
        request.tv_sec = milliseconds / 1000;
        request.tv_nsec = static_cast<long>(milliseconds % 1000) * 1000000L;
        while (nanosleep(&request, &request) != 0 && errno == EINTR)
        {
        }
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

        LogDebug("Killing thread '%s' ID=0x%llx", m_impl->m_name.getText(), static_cast<unsigned long long>(m_impl->m_id));

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
        auto* thread = TkMemory::newObject<ThreadImpl>(alloc, r, name, priority, alloc);
        return thread->setUpThread(stackSize);
    }

    void TkThread::destroy(ThreadHandle t) {
        TkMemory::deletePtr(t->getAllocator(), t);
    }
}

#endif
