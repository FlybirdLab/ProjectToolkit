/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Win32Thread.hpp"

#if TK_WINDOWS

#include "toolkit/debug/Logger.hpp"
#include "toolkit/base/Function.hpp"
#include "toolkit/threading/ThreadRegistry.hpp"
#include "toolkit/threading/Runnable.hpp"
#include "toolkit/utils/Convertor.hpp"

#include <Windows.h>
#include <process.h>

namespace toolkit
{
    bool TkThread::isInMainThread()
    {
        // TODO Impl
        TK_ASSERT(false && "Not impl");
        return false;
    }

    u64 TkThread::getMainThreadId()
    {
        // TODO Impl
        TK_ASSERT(false && "Not impl");
        return 0;
    }

    TkAction<TkThread *> TkThread::ThreadStarting{};
    TkAction<TkThread *, i32> TkThread::ThreadExiting{};

    IRunnable *TkThread::getRunnable()
    {
        return m_impl->m_runnable;
    }

    TkThread *ThreadImpl::setUpThread(u32 stackSize)
    {
        stackSize = getStackSize(stackSize);

        const i32 result = start(stackSize);
        if (result != 0)
        {
            LogError("Failed to spawn a thread. Result code: %i", result);
            TkMemory::deletePtr(m_alloc, this);
            return nullptr;
        }

        setPriority(getPriority());
        return this;
    }

    void ThreadImpl::killInternal(bool waitForJoin)
    {
        if (!m_thread)
            return;

        if (waitForJoin)
        {
            WaitForSingleObject(m_thread, INFINITE);
        }
        else
        {
            TerminateThread(m_thread, 1);
        }

        CloseHandle(m_thread);
    }

    u32 ThreadImpl::getStackSize(u32 s)
    {
        return s;
    }

    i32 ThreadImpl::start(u32 stackSize)
    {
        unsigned threadId = 0;

        m_thread = (HANDLE)_beginthreadex(
            nullptr,   // security
            stackSize, // stack size（0 = default）
            &ThreadImpl::threadProc,
            this,
            0, // start immediately
            &threadId);

        return m_thread ? 0 : -1;
    }

    i32 ThreadImpl::convertType(ThreadPriority t)
    {
        switch (t)
        {
        case ThreadPriority::kHighest:
            return THREAD_PRIORITY_HIGHEST;
        case ThreadPriority::kAboveNormal:
            return THREAD_PRIORITY_ABOVE_NORMAL;
        case ThreadPriority::kNormal:
            return THREAD_PRIORITY_NORMAL;
        case ThreadPriority::kBelowNormal:
            return THREAD_PRIORITY_BELOW_NORMAL;
        case ThreadPriority::kLowest:
            return THREAD_PRIORITY_LOWEST;
        }
        return THREAD_PRIORITY_NORMAL;
    }

    void TkThread::setThreadPriority(ThreadPriority priority)
    {
        ::SetThreadPriority(::GetCurrentThread(), ThreadImpl::convertType(priority));
    }

    void TkThread::setThreadAffinityMask(u64 affinityMask)
    {
        ::SetThreadAffinityMask(::GetCurrentThread(), (DWORD_PTR)affinityMask);
    }

    void TkThread::sleep(i32 milliseconds)
    {
        static thread_local HANDLE timer = NULL;
        if (timer == NULL)
        {
            // Attempt to create high-resolution timer for each thread (Windows 10 build 17134 or later)
            timer = CreateWaitableTimerEx(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
            if (timer == NULL) // fallback for older versions of Windows
                timer = CreateWaitableTimer(NULL, TRUE, NULL);
        }

        // Negative value is relative to current time, minimum waitable time is 10 microseconds
        ::LARGE_INTEGER dueTime;
        dueTime.QuadPart = -int64_t(milliseconds) * 10000;

        ::SetWaitableTimerEx(timer, &dueTime, 0, NULL, NULL, NULL, 0);
        WaitForSingleObject(timer, INFINITE);
    }

    u64 TkThread::getCurrentThreadID()
    {
        return (u64)GetCurrentThreadId();
    }

    unsigned __stdcall ThreadImpl::threadProc(void *pThis)
    {
        auto *thread = static_cast<ThreadImpl *>(pThis);

        StringAsUTF16<128> to{thread->m_name.get()};
        if (!thread->m_name.isEmpty())
        {
            SetThreadDescription(
                GetCurrentThread(),
                (PCWSTR)to.get());
        }

        const i32 exitCode = thread->run();
        return (unsigned)exitCode;
    }

    i32 ThreadImpl::run()
    {
        // Setup
        TK_ASSERT(m_runnable);
        m_id = getCurrentThreadID();
#if TK_TRACY_ENABLE
        tracy::setThreadName(m_name.get());
#endif
        ThreadRegistry::add(this);
        TkThread::ThreadStarting(this);
        i32 exitCode = 1;
        m_isRunning = true;

        LogDebug("Thread '%s' ID=0x%llx started with priority %s", m_name.get(), m_id, toString(getPriority()));

        if (m_runnable->onInit())
        {
            exitCode = m_runnable->onRun();

            if (m_callAfterWork) // Prevent from calling this after calling AfterWork since object may be deleted
                m_runnable->onExit();
        }

        LogDebug("Thread \'%s\' ID=0x%llx exits with code %i", m_name.get(), m_id, exitCode);

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

    ThreadImpl::ThreadImpl(IRunnable *runnable, const TkString &name, ThreadPriority priority, IAllocator &alloc)
        : TkThread(this), m_runnable{runnable}, m_name{name}, m_priority{priority}, m_thread{0}, m_alloc{alloc}
    {
        TK_ASSERT(runnable);
    }

    ThreadPriority TkThread::getPriority()
    {
        return m_impl->m_priority;
    }

    void TkThread::setPriority(ThreadPriority priority)
    {
        if (m_impl->m_priority == priority)
            return;

        m_impl->m_priority = priority;
        SetThreadPriority(m_impl->m_thread, ThreadImpl::convertType(priority));
    }

    IAllocator &TkThread::getAllocator()
    {
        return m_impl->m_alloc;
    }

    u64 TkThread::getID()
    {
        return m_impl->m_id;
    }

    bool TkThread::isRunning()
    {
        return m_impl->m_isRunning;
    }

    const TkString &TkThread::getName()
    {
        return m_impl->m_name;
    }

    void TkThread::kill(bool waitForJoin)
    {
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

        LogDebug("Killing thread \'%s\' ID=0x%llx", m_impl->m_name, m_impl->m_id);

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

    void TkThread::join()
    {
        if (!m_impl->m_thread)
            return;

        WaitForSingleObject(m_impl->m_thread, INFINITE);
        CloseHandle(m_impl->m_thread);
        m_impl->m_thread = nullptr;
    }

    ThreadHandle TkThread::newThread(struct IRunnable *r, const TkString &name, ThreadPriority priority, IAllocator &alloc)
    {
        return TkThread::newThread(r, name, alloc, priority, 0);
    }

    ThreadHandle TkThread::newThread(IRunnable *r, const TkString &name, IAllocator &alloc, ThreadPriority priority, u32 stackSize)
    {
        auto *thread = TkMemory::newObject<ThreadImpl>(alloc, r, name, priority, alloc);
        thread->setUpThread(stackSize);

        return thread;
    }

    void TkThread::destroy(ThreadHandle t)
    {
        TkMemory::deletePtr(t->getAllocator(), t);
    }
}

#endif