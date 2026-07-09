/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/threading/Sync.hpp>
#include <toolkit/threading/Thread.hpp>

namespace toolkit
{

#define THREAD_LOCAL_USE_DYNAMIC_BUCKETS (TK_MACOS || TK_WINDOWS || TK_LINUX)

    // Per-thread local variable storage for basic types (POD). Implemented using atomic with per-thread storage indexed via thread id hashing. Consider using 'THREADLOCAL' define before the variable instead.
    template <typename T, i32 MaxThreads = TK_PLATFORM_THREADS_LIMIT>
    class ThreadLocal
    {
    public:
        ThreadLocal()
        {
            memset(m_staticBuckets, 0, sizeof(m_staticBuckets));
        }

#if THREAD_LOCAL_USE_DYNAMIC_BUCKETS
        ~ThreadLocal()
        {
            TkMemory::free(TkMemory::getDefault(), m_dynamicBuckets);
        }
#endif

    public:
        TK_FORCE_INLINE T &get()
        {
            return getBucket().val;
        }
        TK_FORCE_INLINE void set(const T &value)
        {
            getBucket().val = value;
        }

        i32 count() const
        {
            i32 result = 0;
            for (i32 i = 0; i < MaxThreads; i++)
            {
                if (TSync::atomicRead((i64 volatile *)&m_staticBuckets[i].thdId) != 0)
                    result++;
            }
#if THREAD_LOCAL_USE_DYNAMIC_BUCKETS
            if (auto dynamicBuckets = (Bucket *)TSync::atomicRead((intptr volatile *)&m_dynamicBuckets))
            {
                for (i32 i = 0; i < MaxThreads; i++)
                {
                    if (TSync::atomicRead((i64 volatile *)&dynamicBuckets[i].thdId) != 0)
                        result++;
                }
            }
#endif
            return result;
        }

        void getvals(TkArray<T> &result) const
        {
            for (i32 i = 0; i < MaxThreads; i++)
            {
                if (TSync::atomicRead((i64 volatile *)&m_staticBuckets[i].thdId) != 0)
                    result.add(m_staticBuckets[i].val);
            }
#if THREAD_LOCAL_USE_DYNAMIC_BUCKETS
            if (auto dynamicBuckets = (Bucket *)TSync::atomicRead((intptr volatile *)&m_dynamicBuckets))
            {
                for (i32 i = 0; i < MaxThreads; i++)
                {
                    if (TSync::atomicRead((i64 volatile *)&dynamicBuckets[i].thdId) != 0)
                        result.add(dynamicBuckets[i].val);
                }
            }
#endif
        }

        void clear()
        {
            memset(m_staticBuckets, 0, sizeof(m_staticBuckets));
#if THREAD_LOCAL_USE_DYNAMIC_BUCKETS
            TkMemory::free(TkMemory::getDefault(), m_dynamicBuckets);
            m_dynamicBuckets = nullptr;
#endif
        }

    protected:
        static_assert(TIsPODType<T>::Value, "Only POD types are supported");

        struct Bucket
        {
            volatile i64 thdId;
            T val;
        };

        Bucket &getBucket()
        {
            const i64 key = (i64)TkThread::getCurrentThreadID();

            // Search statically allocated buckets
            i32 index = (i32)(key & (MaxThreads - 1));
            i32 spaceLeft = MaxThreads;
            while (spaceLeft)
            {
                const i64 value = TSync::atomicRead(&m_staticBuckets[index].thdId);
                if (value == key)
                    return m_staticBuckets[index];
                if (value == 0 && TSync::interlockedCompareExchange(&m_staticBuckets[index].thdId, key, 0) == 0)
                    return m_staticBuckets[index];
                index = (index + 1) & (MaxThreads - 1);
                spaceLeft--;
            }

#if THREAD_LOCAL_USE_DYNAMIC_BUCKETS
        // Allocate dynamic buckets if missing
        DYNAMIC:
            auto dynamicBuckets = (Bucket *)TSync::atomicRead((intptr volatile *)&m_dynamicBuckets);
            if (!dynamicBuckets)
            {
                dynamicBuckets = (Bucket *)TkMemory::allocate(TkMemory::getDefault(), DynamicMaxThreads * sizeof(Bucket), 16);
                memset(dynamicBuckets, 0, DynamicMaxThreads * sizeof(Bucket));
                if (TSync::interlockedCompareExchange((intptr volatile *)&m_dynamicBuckets, (intptr)dynamicBuckets, 0) != 0)
                {
                    TkMemory::free(TkMemory::getDefault(), dynamicBuckets);
                    goto DYNAMIC;
                }
            }

            // Search dynamically allocated buckets
            index = (i32)(key & (DynamicMaxThreads - 1));
            spaceLeft = DynamicMaxThreads;
            while (spaceLeft)
            {
                const i64 value = TSync::atomicRead(&dynamicBuckets[index].thdId);
                if (value == key)
                    return dynamicBuckets[index];
                if (value == 0 && TSync::interlockedCompareExchange(&dynamicBuckets[index].thdId, key, 0) == 0)
                    return dynamicBuckets[index];
                index = (index + 1) & (DynamicMaxThreads - 1);
                spaceLeft--;
            }
#endif

            return *(Bucket *)nullptr;
        }

    protected:
        Bucket m_staticBuckets[MaxThreads];
#if THREAD_LOCAL_USE_DYNAMIC_BUCKETS
        Bucket *m_dynamicBuckets = nullptr;
        constexpr static i32 DynamicMaxThreads = 1024;
#endif
    };
}