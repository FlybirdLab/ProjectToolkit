/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/threading/Sync.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/base/Assert.hpp>
#include <toolkit/base/Math.hpp>

namespace toolkit
{

    /// @brief Thread-safe append-only linear buffer for trivially managed value types.
    template <typename T>
    class TkConcurrentBuffer
    {
    public:
        /// @brief Creates an empty concurrent buffer.
        TkConcurrentBuffer(IAllocator &alloc = TkMemory::getDefault())
            : m_alloc{alloc}, m_data(nullptr)
        {
        }
        /// @brief Creates a buffer with preallocated storage for `capacity` items.
        TkConcurrentBuffer(i32 capacity, IAllocator &alloc = TkMemory::getDefault())
            : m_alloc{alloc}, m_capacity(capacity)
        {
            if (m_capacity > 0)
                m_data = (T *)TkMemory::allocate(alloc, m_capacity * sizeof(T));
            else
                m_data = nullptr;
        }

        ~TkConcurrentBuffer()
        {
            release();
        }

    public:
        // Gets the amount of the elements in the collection.
        /// @brief Returns the number of committed items.
        TK_FORCE_INLINE i64 count()
        {
            return TSync::atomicRead(&m_count);
        }

        // Get amount of the elements that can be holed by collection without resizing.
        /// @brief Returns the current storage capacity.
        TK_FORCE_INLINE i64 capacity() const
        {
            return m_capacity;
        }

        // Determines whether this collection isn't empty.
        TK_FORCE_INLINE bool hasItems() const
        {
            return m_count != 0;
        }

        // Determines whether this collection is empty.
        TK_FORCE_INLINE bool isEmpty() const
        {
            return m_count == 0;
        }

        // Gets the pointer to the first element in the collection.
        /// @brief Returns the raw storage pointer.
        TK_FORCE_INLINE T *get()
        {
            return m_data;
        }

        // Gets the pointer to the first element in the collection.
        TK_FORCE_INLINE const T *get() const
        {
            return m_data;
        }

        // Gets the last element.
        TK_FORCE_INLINE T &last()
        {
            TK_ASSERT(m_count > 0);
            return m_data[m_count - 1];
        }

        // Gets the last element.
        TK_FORCE_INLINE const T &last() const
        {
            TK_ASSERT(m_count > 0);
            return m_data[m_count - 1];
        }

        // Gets the first element.
        TK_FORCE_INLINE T &first()
        {
            TK_ASSERT(m_count > 0);
            return m_data[0];
        }

        // Gets the first element.
        TK_FORCE_INLINE const T &first() const
        {
            TK_ASSERT(m_count > 0);
            return m_data[0];
        }

        // Get or sets element by the index.
        TK_FORCE_INLINE T &operator[](i64 index)
        {
            TK_ASSERT(index >= 0 && index < m_count);
            return m_data[index];
        }

        // Get or sets element by the index.
        TK_FORCE_INLINE const T &operator[](i64 index) const
        {
            TK_ASSERT(index >= 0 && index < m_count);
            return m_data[index];
        }

    public:
        // Clear the collection but without changing its capacity.
        /// @brief Resets the logical item count without releasing storage.
        TK_FORCE_INLINE void clear()
        {
            TSync::interlockedExchange(&m_count, 0);
        }

        // Releases this buffer data.
        /// @brief Releases the backing storage and resets buffer state.
        void release()
        {
            m_resizeLocker.lock();
            if (m_data)
                TkMemory::free(m_alloc, m_data);
            m_data = nullptr;
            m_capacity = 0;
            m_count = 0;
            m_resizeLocker.unlock();
        }

        // Sets the custom size of the collection. Only for the custom usage with dedicated data.
        void setSize(i32 size)
        {
            TK_ASSERT(size >= 0 && size <= m_capacity);
            m_count = size;
        }

        /// @brief Appends one item and returns its insertion index.
        TK_FORCE_INLINE i64 add(const T &item)
        {
            return add(&item, 1);
        }

        /// @brief Appends `count` items and returns the starting index.
        i64 add(const T *items, i32 count)
        {
            const i64 index = TSync::interlockedAdd(&m_count, (i64)count);
            const i64 newCount = index + (i64)count;
            ensureCapacity(newCount);
            TkMemory::copyItems(m_data + index, items, count);
            return index;
        }

        // Add collection of items to the collection
        // @param collection Array with the items to add
        TK_FORCE_INLINE void add(TkConcurrentBuffer<T> &collection)
        {
            add(collection.get(), collection.count());
        }

        /// @brief Appends `count` default-constructed items and returns the starting index.
        i64 addDefault(i32 count = 1)
        {
            const i64 index = TSync::interlockedAdd(&m_count, (i64)count);
            const i64 newCount = index + (i64)count;
            ensureCapacity(newCount);
            TkMemory::constructItems(m_data + index, count);
            return index;
        }

        /// @brief Appends one default-constructed item and returns it.
        TK_FORCE_INLINE T &addOne()
        {
            const i64 index = TSync::interlockedAdd(&m_count, 1);
            const i64 newCount = index + 1;
            ensureCapacity(newCount);
            TkMemory::constructItems(m_data + index, 1);
            return m_data[index];
        }

        /// @brief Appends zeroed storage for `count` items and returns the starting index.
        i64 addZeroed(i32 count = 1)
        {
            const i64 index = TSync::interlockedAdd(&m_count, (i64)count);
            const i64 newCount = index + (i64)count;
            ensureCapacity(newCount);
            ZERO_MEMORY(get() + index, count * sizeof(T));
            return index;
        }

        /// @brief Ensures the backing storage can hold at least `minCapacity` items.
        void ensureCapacity(i64 minCapacity)
        {
            // Early out
            if (m_capacity >= minCapacity)
                return;

            m_resizeLocker.lock();

            // Skip if the other thread performed resizing
            if (m_capacity >= minCapacity)
            {
                m_resizeLocker.unlock();
                return;
            }

            // Compute the new capacity
            i64 newCapacity = m_capacity == 0 ? 8 : TkMath::roundUpToPowerOf2(m_capacity) * 2;
            if (newCapacity < minCapacity)
            {
                newCapacity = minCapacity;
            }
            TK_ASSERT(newCapacity > m_capacity);

            // Allocate new data
            T *newData = nullptr;
            if (newCapacity > 0)
            {
                newData = (T *)TkMemory::allocate(m_alloc, newCapacity * sizeof(T));
            }

            // Check if has data
            if (m_data)
            {
                // Check if preserve contents
                if (newData && m_count > 0)
                {
                    TkMemory::memoryCopy(newData, m_data, static_cast<psize>(m_count) * sizeof(T));
                }

                // Delete old data
                if (m_data)
                    TkMemory::free(m_alloc, m_data);
            }

            // Set state
            m_data = newData;
            m_capacity = newCapacity;

            m_resizeLocker.unlock();
        }

        // Swaps the contents of buffer with the other object without copy operation. Performs fast internal data exchange.
        void swap(TkConcurrentBuffer &other)
        {
            const auto count = m_count;
            const auto capacity = m_capacity;
            const auto data = m_data;

            m_count = other.m_count;
            m_capacity = other.m_capacity;
            m_data = other.m_data;

            other.m_count = count;
            other.m_capacity = capacity;
            other.m_data = data;
        }

    public:
        // Checks if the given element is in the collection.
        bool contains(const T &item) const
        {
            for (i32 i = 0; i < m_count; i++)
            {
                if (m_data[i] == item)
                {
                    return true;
                }
            }

            return false;
        }

        // Searches for the specified object and returns the zero-based index of the first occurrence within the entire collection.
        i32 indexOf(const T &item) const
        {
            for (i32 i = 0; i < m_count; i++)
            {
                if (m_data[i] == item)
                {
                    return i;
                }
            }

            return -1;
        }

    private:
        IAllocator &m_alloc;
        i64 m_count = 0, m_capacity = 0;
        T *m_data;
        TkMutex m_resizeLocker;
    };
}
