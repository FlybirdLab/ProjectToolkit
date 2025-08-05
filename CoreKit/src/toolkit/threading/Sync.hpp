// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/types/Atomic.hpp>

#if TK_POSIX
#	include <pthread.h>
#	include <semaphore.h>
#else
#	include <toolkit/platforms/win32/Win32.hpp>
#endif

namespace toolkit {

    /// Cross-platform mutex implement
    class TkMutex
    {
        friend class TkCondVar;
    public:
        TkMutex();
        ~TkMutex();
        TkMutex(const TkMutex&) = delete; // Non-copyable
        TkMutex& operator=(const TkMutex&) = delete; // Non-copyable

        void lock();
        bool tryLock();
        void unlock();
    private:
#if TK_POSIX
        pthread_mutex_t m_handle;
#else
        CRITICAL_SECTION m_handle;
#endif
    };


    /// Cross-platform mutex implement for read&write situation
    class TkRwMutex
    {
    public:
        TkRwMutex();
        ~TkRwMutex();
        TkRwMutex(const TkRwMutex&) = delete; // Non-copyable
        TkRwMutex& operator=(const TkRwMutex&) = delete; // Non-copyable

        void lockRead();
        void lockWrite();
        void unlockRead();
        void unlockWrite();

    private:
#if TK_POSIX
        pthread_rwlock_t m_handle;
#else
        SRWLOCK m_handle;
#endif
    };


    /// Cross-platform implement for ConditionVariable
    class TkCondVar
    {
    public:
        TkCondVar();
        ~TkCondVar();
        TkCondVar(const TkCondVar&) = delete; // Non-copyable
        TkCondVar& operator=(const TkCondVar&) = delete; // Non-copyable

        void notifyOne();
        void notifyAll();
        void wait(TkMutex&);

    private:
#if TK_POSIX
        pthread_cond_t m_handle;
#else
        CONDITION_VARIABLE m_handle;
#endif
    };


    /// Cross-platform SpinLock implement.
    /// High performance substitute for mutex if only using in short time
    class TkSpinLock
    {
    public:
        TkSpinLock() = default;
        ~TkSpinLock() = default;
        TkSpinLock(const TkSpinLock&) = delete; // Non-copyable
        TkSpinLock& operator=(const TkSpinLock&) = delete; // Non-copyable

        void lock();
        void unlock();
        bool tryLock();

    private:
        TkAtomic<bool> m_lock;
    };


    /// A barrier for thread synchronization.
    class TkBarrier
    {
    public:
        explicit TkBarrier(u32 size);
        ~TkBarrier();
        TkBarrier(const TkBarrier&) = delete; // Non-copyable
        TkBarrier& operator=(const TkBarrier&) = delete; // Non-copyable

        void wait();
    private:
#if TK_POSIX
        pthread_barrier_t m_handle;
#else
        CONDITION_VARIABLE m_cvar;
        CRITICAL_SECTION m_mtx;
        u32 m_threshold;
        u32 m_count;
        u32 m_generation;
#endif
    };


    /// Cross-platform Semaphore implement for thread synchronize
    class TkSemaphore
    {
    public:
        TkSemaphore(i32 size, i32 maxSize);
        ~TkSemaphore();
        TkSemaphore(const TkSemaphore&) = delete; // Non-copyable
        TkSemaphore& operator=(const TkSemaphore&) = delete; // Non-copyable

        void signal(u32 count = 1);
        void wait();
        bool wait(u32 timeout);
    private:
#if TK_POSIX
        struct {
			pthread_mutex_t mutex;
			pthread_cond_t cond;
			volatile i32 count;
		} m_id;
#else
        void* m_id;
#endif
    };


    /// Auto release guard for mutex when exit the scope
    template <typename T>
    class TkMutexGuard
    {
    public:
        explicit TkMutexGuard(T& mtx) : m_mtx(&mtx) { m_mtx->lock(); }
        TkMutexGuard(TkMutexGuard&& b) noexcept { m_mtx = b.m_mtx; b.m_mtx = nullptr; }
        TkMutexGuard(const TkMutexGuard& b) = delete;
        TkMutexGuard& operator=(TkMutexGuard&& b) = delete;
        TkMutexGuard& operator=(const TkMutexGuard& b) = delete;
        ~TkMutexGuard()
        {
            if (!m_mtx) return;
            m_mtx->unlock();
        }

    private:
        T* m_mtx;
    };
}