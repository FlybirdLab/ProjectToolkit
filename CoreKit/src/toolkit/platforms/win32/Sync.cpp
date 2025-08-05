// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include <thread>

#include "toolkit/threading/Sync.hpp"
#include "toolkit/base/Assert.hpp"

#include <toolkit/platforms/win32/IncludeWindows.hpp>

namespace toolkit {

    TkMutex::TkMutex()
        : m_handle {}
    {
        InitializeCriticalSection(&m_handle);
    }

    TkMutex::~TkMutex() {
        DeleteCriticalSection(&m_handle);
    }

    void TkMutex::lock() {
        EnterCriticalSection(&m_handle);
    }

    bool TkMutex::tryLock() {
        const BOOL enter = TryEnterCriticalSection(&m_handle);
        return enter != 0;
    }

    void TkMutex::unlock() {
        LeaveCriticalSection(&m_handle);
    }


    TkRwMutex::TkRwMutex()
        : m_handle{}
    {
        InitializeSRWLock(&m_handle);
    }

    TkRwMutex::~TkRwMutex() = default;

    void TkRwMutex::lockRead() {
        AcquireSRWLockShared(&m_handle);
    }

    void TkRwMutex::lockWrite() {
        AcquireSRWLockExclusive(&m_handle);
    }

    void TkRwMutex::unlockRead() {
        ReleaseSRWLockShared(&m_handle);
    }

    void TkRwMutex::unlockWrite() {
        ReleaseSRWLockExclusive(&m_handle);
    }


    TkCondVar::TkCondVar()
        : m_handle{}
    {
        InitializeConditionVariable(&m_handle);
    }

    TkCondVar::~TkCondVar() = default;

    void TkCondVar::notifyOne() {
        WakeConditionVariable(&m_handle);
    }

    void TkCondVar::notifyAll() {
        WakeAllConditionVariable(&m_handle);
    }

    void TkCondVar::wait(TkMutex& mtx) {
        SleepConditionVariableCS(&m_handle, &mtx.m_handle, kMaxU32);
    }


    void TkSpinLock::lock() {
        for (u32 spinCount = 0; !tryLock(); ++spinCount) {
            if (spinCount < 16) {
#if TK_SIMD_SSE
                _mm_pause();
#endif
            } else {
                std::this_thread::yield(); //TODO use toolkit api instead
                spinCount = 0;
            }
        }
    }

    void TkSpinLock::unlock() {
        m_lock.store(false, AtomicMemOrder::kRelease);
    }

    bool TkSpinLock::tryLock() {
        return !m_lock.load(AtomicMemOrder::kRelaxed) && !m_lock.exchange(true, AtomicMemOrder::kAcquire);
    }


    TkBarrier::TkBarrier(u32 size)
        : m_cvar{}
        , m_mtx{}
    {
        InitializeCriticalSection(&m_mtx);
        InitializeConditionVariable(&m_cvar);

        m_threshold = size;
        m_count = size;
        m_generation = 0;
    }

    TkBarrier::~TkBarrier() {
        DeleteCriticalSection(&m_mtx);
    }

    void TkBarrier::wait() {
        EnterCriticalSection(&m_mtx);
        const u32 gen = m_generation;

        if(--m_count == 0)
        {
            ++m_generation;
            m_count = m_threshold;
            WakeAllConditionVariable(&m_cvar);
        }
        else
        {
            while(gen == m_generation)
            {
                SleepConditionVariableCS(&m_cvar, &m_mtx, kMaxU32);
            }
        }

        LeaveCriticalSection(&m_mtx);
    }


    TkSemaphore::TkSemaphore(i32 size, i32 maxSize) {
        m_id = ::CreateSemaphore(nullptr, size, maxSize, nullptr);
        TK_ASSERT(m_id);
    }

    TkSemaphore::~TkSemaphore() {
        ::CloseHandle(m_id);
    }

    void TkSemaphore::signal(u32 count) {
        BOOL res = ::ReleaseSemaphore(m_id, count, nullptr);
        TK_ASSERT(res);
    }

    void TkSemaphore::wait() {
        ::WaitForSingleObject(m_id, INFINITE);
    }

    bool TkSemaphore::wait(u32 timeout) {
        auto res = ::WaitForSingleObject(m_id, timeout);
        return WAIT_OBJECT_0 == res;
    }


}