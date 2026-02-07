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
        return TryEnterCriticalSection(&m_handle);
        // return enter != 0;
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

    bool TkCondVar::wait(TkMutex& mtx, const i32 timeout) {
        return SleepConditionVariableCS(&m_handle, &mtx.m_handle, timeout);
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


	i64 TSync::interlockedExchange(volatile i64* dst, i64 exchange)
	{
		return _InterlockedExchange64(dst, exchange);
	}
	i32 TSync::interlockedCompareExchange(volatile i32* dst, i32 exchange, i32 comperand)
	{
		return _InterlockedCompareExchange((long volatile*)dst, exchange, comperand);
	}
	i64 TSync::interlockedCompareExchange(volatile i64* dst, i64 exchange, i64 comperand)
	{
		return _InterlockedCompareExchange64(dst, exchange, comperand);
	}
	i64 TSync::interlockedIncrement(volatile i64* dst)
	{
		return _InterlockedExchangeAdd64(dst, 1) + 1;
	}
	i64 TSync::interlockedDecrement(i64 volatile* dst)
	{
		return _InterlockedExchangeAdd64(dst, -1) - 1;
	}
	i64 TSync::interlockedAdd(i64 volatile* dst, i64 value)
	{
		return _InterlockedExchangeAdd64(dst, value);
	}
	i32 TSync::atomicRead(i32 const volatile* dst)
	{
		return (i32)_InterlockedCompareExchange((long volatile*)dst, 0, 0);
	}
	i64 TSync::atomicRead(i64 const volatile* dst)
	{
		return _InterlockedCompareExchange64((i64 volatile*)dst, 0, 0);
	}
	void TSync::atomicStore(i32 volatile* dst, i32 value)
	{
		_InterlockedExchange((long volatile*)dst, value);
	}
	void TSync::atomicStore(i64 volatile* dst, i64 value)
	{
		_InterlockedExchange64(dst, value);
	}

	void TSync::memoryBarrier()
	{
		_ReadWriteBarrier();
#if TK_PLATFORM_64BITS
		#if defined(_AMD64_)
    __faststorefence();
#elif defined(_IA64_)
	__mf();
#elif defined(_ARM64_)
    __dmb(_ARM64_BARRIER_ISH);
#else
#error "Invalid platform."
#endif
#else
		LONG barrier;
		__asm {
			xchg barrier, eax
		}
#endif
	}

	void TSync::prefetch(void const* ptr)
	{
#if _M_ARM64
		__prefetch((char const*)ptr);
#else
		_mm_prefetch((char const*)ptr, _MM_HINT_T0);
#endif
	}

}