/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/memory/Memory.hpp>
#include <toolkit/collection/StaticArray.hpp>
#include <toolkit/threading/Sync.hpp>

namespace toolkit
{

    // Clear callback used to initialize the given collection container type (clear array, etc.). Called when pool item is being reused or initialized.
    template <typename T>
    using ClearCallback = void (*)(T *);

    // Create callback spawns a new entry of the pooled collection
    template <typename T>
    using CreateCallback = T *(*)();

    template <typename T>
    static inline void defaultClearCallback(T *obj)
    {
        obj->clear();
    }

    template <typename T>
    static inline T *defaultCreateCallback()
    {
        return TkMemory::newObject<T>(TkMemory::getDefault());
    }


    // Cache container that holds a list of cached collections to allow reuse and reduced memory allocation amount. Helps with sharing data across code and usages. It's thread-safe
    template <typename T, ClearCallback<T> ClearCallback = defaultClearCallback<T>, CreateCallback<T> CreateCallback = defaultCreateCallback<T>>
    class CollectionPoolCache
    {
    public:
    public:
        // Helper object used to access the pooled collection and return it to the pool after usage (on code scope execution end).
        struct ScopeCache
        {
        public:
            T *value;

            ScopeCache() = delete;
            ScopeCache(const ScopeCache &other) = delete;
            ScopeCache &operator=(const ScopeCache &other) = delete;
            ScopeCache &operator=(ScopeCache &&other) noexcept = delete;

            ScopeCache(ScopeCache &&other) noexcept
            {
                value = other.value;
                other.value = nullptr;
            }

            ~ScopeCache()
            {
                m_pool->put(value);
            }

            T *operator->()
            {
                return value;
            }

            const T *operator->() const
            {
                return value;
            }

            T &operator*()
            {
                return *value;
            }

            const T &operator*() const
            {
                return *value;
            }

        private:
            CollectionPoolCache *m_pool;

            TK_FORCE_INLINE ScopeCache(CollectionPoolCache *pool, T *value)
            {
                m_pool = pool;
                value = value;
            }
        };

    private:
        TkMutex m_locker;
        TkStaticArray<T *, 64> m_pool;

    public:
        ~CollectionPoolCache()
        {
            m_pool.clear();
        }

    public:
        // Gets the collection instance from the pool. Can reuse the object from the pool or create a new one. Returns collection is always cleared and ready to use.
        TK_FORCE_INLINE ScopeCache get()
        {
            return ScopeCache(this, getUnscoped());
        }

        // Gets the collection instance from the pool. Can reuse the object from the pool or create a new one. Returns collection is always cleared and ready to use.
        T *getUnscoped()
        {
            T *result;
            m_locker.lock();
            if (m_pool.hasItems())
                result = m_pool.pop();
            else
                result = CreateCallback();
            m_locker.unlock();
            ClearCallback(result);
            return result;
        }

        // Puts the collection value back to the pool.
        void put(T *value)
        {
            m_locker.lock();
            m_pool.add(value);
            m_locker.unlock();
        }

        // Releases all the allocated resources (existing in the pool that are not during usage).
        void release()
        {
            m_locker.lock();
            m_pool.clear();
            m_pool.resize(0);
            m_locker.unlock();
        }
    };

}