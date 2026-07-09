/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/Assert.hpp>

namespace toolkit {

    /// @brief Small inline buffer that keeps the most recent samples for simple statistics.
    template<typename T, i32 Size>
    class TkSamplesBuffer
    {
    public:
        // Gets amount of elements in the collection.
        TK_FORCE_INLINE i32 count() const
        {
            return m_count;
        }

        // Gets amount of elements that can be added to the collection.
        TK_FORCE_INLINE i32 capacity() const
        {
            return Size;
        }

        // Returns true if collection has any elements added.
        TK_FORCE_INLINE bool hasItems() const
        {
            return m_count > 0;
        }

        // Returns true if collection is empty.
        TK_FORCE_INLINE bool isEmpty() const
        {
            return m_count < 1;
        }

        // Gets pointer to the first element in the collection.
        TK_FORCE_INLINE T* get() const
        {
            return m_data;
        }

        // Gets or sets element at the specified index.
        TK_FORCE_INLINE T& at(i32 index) const
        {
            TK_ASSERT(index >= 0 && index < m_count);
            return m_data[index];
        }

        // Gets or sets element at the specified index.
        TK_FORCE_INLINE T& operator[](i32 index)
        {
            TK_ASSERT(index >= 0 && index < m_count);
            return m_data[index];
        }

        // Gets or sets element at the specified index.
        TK_FORCE_INLINE const T& operator[](i32 index) const
        {
            TK_ASSERT(index >= 0 && index < m_count);
            return m_data[index];
        }

        // Gets the first element value.
        TK_FORCE_INLINE T first() const
        {
            TK_ASSERT(hasItems());
            return m_data[0];
        }

        // Gets last element value.
        TK_FORCE_INLINE T last() const
        {
            TK_ASSERT(hasItems());
            return m_data[m_count - 1];
        }

    public:
        /// @brief Inserts a sample at the front and shifts older samples back.
        void add(const T& value)
        {
            if (m_count != Size)
                m_count++;
            if (m_count > 1)
            {
                for (i32 i = m_count - 1; i > 0; i--)
                    m_data[i] = m_data[i - 1];
            }
            m_data[0] = value;
        }

        /// @brief Assigns the same value to all currently stored samples.
        void setAll(const T value)
        {
            for (i32 i = 0; i < m_count; i++)
                m_data[i] = value;
        }

        /// @brief Clears all samples without touching underlying inline storage.
        TK_FORCE_INLINE void clear()
        {
            m_count = 0;
        }

    public:
        /// @brief Returns the minimum value among stored samples.
        T minimum() const
        {
            TK_ASSERT(hasItems());
            T result = m_data[0];
            for (i32 i = 1; i < m_count; i++)
            {
                if (m_data[i] < result)
                    result = m_data[i];
            }
            return result;
        }

        /// @brief Returns the maximum value among stored samples.
        T maximum() const
        {
            TK_ASSERT(hasItems());
            T result = m_data[0];
            for (i32 i = 1; i < m_count; i++)
            {
                if (m_data[i] > result)
                    result = m_data[i];
            }
            return result;
        }

        /// @brief Returns the arithmetic mean of stored samples.
        T average() const
        {
            TK_ASSERT(hasItems());
            T result = m_data[0];
            for (i32 i = 1; i < m_count; i++)
            {
                result += m_data[i];
            }
            return result / m_count;
        }

    protected:
        i32 m_count = 0;
        T m_data[Size];
    };

}
