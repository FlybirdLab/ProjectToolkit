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


    /// @brief Thread-safe cache that reuses collection objects to reduce heap churn.
    template <typename T, ClearCallback<T> ClearCallback = defaultClearCallback<T>, CreateCallback<T> CreateCallback = defaultCreateCallback<T>>
    class CollectionPoolCache
    {
    public:
    public:
        /// @brief RAII handle that returns a pooled object back to the cache on destruction.
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
                if (m_pool && value)
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
                this->value = value;
            }
        };

    private:
        TkMutex m_locker;
        TkStaticArray<T *, 64> m_pool;

    public:
        ~CollectionPoolCache()
        {
            release();
        }

    public:
        /// @brief Returns a scoped pooled object that is cleared before use.
        TK_FORCE_INLINE ScopeCache get()
        {
            return ScopeCache(this, getUnscoped());
        }

        /// @brief Returns a cleared pooled object without wrapping it in RAII.
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

        /// @brief Returns an object back to the cache for later reuse.
        void put(T *value)
        {
            m_locker.lock();
            m_pool.add(value);
            m_locker.unlock();
        }

        /// @brief Destroys all currently cached objects held by the pool.
        void release()
        {
            m_locker.lock();
            for (i32 i = 0; i < m_pool.count(); ++i)
            {
                if (m_pool[i])
                    TkMemory::deletePtr(m_pool[i]);
            }
            m_pool.clear();
            m_locker.unlock();
        }
    };

}
