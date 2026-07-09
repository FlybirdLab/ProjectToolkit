/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/threading/Sync.hpp>

#if TK_POSIX

#include <toolkit/base/Assert.hpp>
#include <toolkit/threading/Thread.hpp>

#include <errno.h>
#include <time.h>

namespace toolkit
{
    TkMutex::TkMutex()
    {
        pthread_mutexattr_t attributes;
        pthread_mutexattr_init(&attributes);
        pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m_handle, &attributes);
        pthread_mutexattr_destroy(&attributes);
    }

    TkMutex::~TkMutex()
    {
        pthread_mutex_destroy(&m_handle);
    }

    void TkMutex::lock()
    {
        pthread_mutex_lock(&m_handle);
    }

    bool TkMutex::tryLock()
    {
        return pthread_mutex_trylock(&m_handle) == 0;
    }

    void TkMutex::unlock()
    {
        pthread_mutex_unlock(&m_handle);
    }

    TkRwMutex::TkRwMutex()
    {
        pthread_rwlock_init(&m_handle, nullptr);
    }

    TkRwMutex::~TkRwMutex()
    {
        pthread_rwlock_destroy(&m_handle);
    }

    void TkRwMutex::lockRead()
    {
        pthread_rwlock_rdlock(&m_handle);
    }

    void TkRwMutex::lockWrite()
    {
        pthread_rwlock_wrlock(&m_handle);
    }

    void TkRwMutex::unlockRead()
    {
        pthread_rwlock_unlock(&m_handle);
    }

    void TkRwMutex::unlockWrite()
    {
        pthread_rwlock_unlock(&m_handle);
    }

    TkCondVar::TkCondVar()
    {
        pthread_cond_init(&m_handle, nullptr);
    }

    TkCondVar::~TkCondVar()
    {
        pthread_cond_destroy(&m_handle);
    }

    void TkCondVar::notifyOne()
    {
        pthread_cond_signal(&m_handle);
    }

    void TkCondVar::notifyAll()
    {
        pthread_cond_broadcast(&m_handle);
    }

    void TkCondVar::wait(TkMutex &mtx)
    {
        pthread_cond_wait(&m_handle, &mtx.m_handle);
    }

    bool TkCondVar::wait(TkMutex &mtx, const i32 timeout)
    {
        timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout / 1000;
        ts.tv_nsec += static_cast<long>(timeout % 1000) * 1000000l;
        ts.tv_sec += ts.tv_nsec / 1000000000l;
        ts.tv_nsec %= 1000000000l;
        return pthread_cond_timedwait(&m_handle, &mtx.m_handle, &ts) == 0;
    }

    void TkSpinLock::lock()
    {
        for (u32 spinCount = 0; !tryLock(); ++spinCount)
        {
            if (spinCount >= 16)
            {
                TkThread::sleep(0);
                spinCount = 0;
            }
        }
    }

    void TkSpinLock::unlock()
    {
        m_lock.store(false, AtomicMemOrder::kRelease);
    }

    bool TkSpinLock::tryLock()
    {
        return !m_lock.load(AtomicMemOrder::kRelaxed) && !m_lock.exchange(true, AtomicMemOrder::kAcquire);
    }

    TkBarrier::TkBarrier(u32 size)
    {
#if TK_APPLE
        pthread_mutex_init(&m_mtx, nullptr);
        pthread_cond_init(&m_cvar, nullptr);
        m_threshold = size;
        m_count = size;
        m_generation = 0;
#else
        const i32 result = pthread_barrier_init(&m_handle, nullptr, size);
        TK_ASSERT(result == 0);
#endif
    }

    TkBarrier::~TkBarrier()
    {
#if TK_APPLE
        pthread_cond_destroy(&m_cvar);
        pthread_mutex_destroy(&m_mtx);
#else
        pthread_barrier_destroy(&m_handle);
#endif
    }

    void TkBarrier::wait()
    {
#if TK_APPLE
        pthread_mutex_lock(&m_mtx);
        const u32 generation = m_generation;
        if (--m_count == 0)
        {
            ++m_generation;
            m_count = m_threshold;
            pthread_cond_broadcast(&m_cvar);
        }
        else
        {
            while (generation == m_generation)
                pthread_cond_wait(&m_cvar, &m_mtx);
        }
        pthread_mutex_unlock(&m_mtx);
#else
        pthread_barrier_wait(&m_handle);
#endif
    }

    TkSemaphore::TkSemaphore(i32 size, i32)
    {
        pthread_mutex_init(&m_id.mutex, nullptr);
        pthread_cond_init(&m_id.cond, nullptr);
        m_id.count = size;
    }

    TkSemaphore::~TkSemaphore()
    {
        pthread_cond_destroy(&m_id.cond);
        pthread_mutex_destroy(&m_id.mutex);
    }

    void TkSemaphore::signal(u32 count)
    {
        pthread_mutex_lock(&m_id.mutex);
        m_id.count += static_cast<i32>(count);
        for (u32 i = 0; i < count; ++i)
            pthread_cond_signal(&m_id.cond);
        pthread_mutex_unlock(&m_id.mutex);
    }

    void TkSemaphore::wait()
    {
        pthread_mutex_lock(&m_id.mutex);
        while (m_id.count <= 0)
            pthread_cond_wait(&m_id.cond, &m_id.mutex);
        --m_id.count;
        pthread_mutex_unlock(&m_id.mutex);
    }

    bool TkSemaphore::wait(u32 timeout)
    {
        timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout / 1000;
        ts.tv_nsec += static_cast<long>(timeout % 1000) * 1000000l;
        ts.tv_sec += ts.tv_nsec / 1000000000l;
        ts.tv_nsec %= 1000000000l;

        pthread_mutex_lock(&m_id.mutex);
        while (m_id.count <= 0)
        {
            const i32 result = pthread_cond_timedwait(&m_id.cond, &m_id.mutex, &ts);
            if (result == ETIMEDOUT)
            {
                pthread_mutex_unlock(&m_id.mutex);
                return false;
            }
            if (result != 0)
            {
                pthread_mutex_unlock(&m_id.mutex);
                return false;
            }
        }
        --m_id.count;
        pthread_mutex_unlock(&m_id.mutex);
        return true;
    }

    i64 TSync::interlockedExchange(i64 volatile *dst, i64 exchange)
    {
        return __atomic_exchange_n(dst, exchange, __ATOMIC_SEQ_CST);
    }

    i32 TSync::interlockedCompareExchange(i32 volatile *dst, i32 exchange, i32 comperand)
    {
        __atomic_compare_exchange_n(dst, &comperand, exchange, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        return comperand;
    }

    i64 TSync::interlockedCompareExchange(i64 volatile *dst, i64 exchange, i64 comperand)
    {
        __atomic_compare_exchange_n(dst, &comperand, exchange, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        return comperand;
    }

    i64 TSync::interlockedIncrement(i64 volatile *dst)
    {
        return __atomic_add_fetch(dst, 1, __ATOMIC_SEQ_CST);
    }

    i64 TSync::interlockedDecrement(i64 volatile *dst)
    {
        return __atomic_sub_fetch(dst, 1, __ATOMIC_SEQ_CST);
    }

    i64 TSync::interlockedAdd(i64 volatile *dst, i64 value)
    {
        return __atomic_fetch_add(dst, value, __ATOMIC_SEQ_CST);
    }

    i32 TSync::atomicRead(i32 const volatile *dst)
    {
        return __atomic_load_n(dst, __ATOMIC_SEQ_CST);
    }

    i64 TSync::atomicRead(i64 const volatile *dst)
    {
        return __atomic_load_n(dst, __ATOMIC_SEQ_CST);
    }

    void TSync::atomicStore(i32 volatile *dst, i32 value)
    {
        __atomic_store_n(dst, value, __ATOMIC_SEQ_CST);
    }

    void TSync::atomicStore(i64 volatile *dst, i64 value)
    {
        __atomic_store_n(dst, value, __ATOMIC_SEQ_CST);
    }

    void TSync::memoryBarrier()
    {
        __sync_synchronize();
    }

    void TSync::prefetch(void const *ptr)
    {
        TK_PREFETCH_MEMORY(ptr);
    }
}

#endif
