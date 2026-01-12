/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/memory/Memory.hpp>
#include <toolkit/base/Assert.hpp>

namespace toolkit
{

    template <typename T, psize Capacity>
    class TkStackArray
    {
    public:
        TK_FORCE_INLINE TkStackArray()
            : m_count(0)
        {
        }

        TkStackArray(std::initializer_list<T> initList)
        {
            m_count = static_cast<i32>(initList.size());
            if (m_count > 0)
            {
                TkMemory::constructItems(m_data, initList.begin(), m_count);
            }
        }

        TkStackArray(const T *data, const i32 length)
        {
            TK_ASSERT(length >= 0);
            m_count = length;
            if (length > 0)
            {
                TkMemory::constructItems(m_data, data, length);
            }
        }

        TkStackArray(const TkStackArray &other)
        {
            m_count = other.m_count;
            if (m_count > 0)
                TkMemory::constructItems(m_data, other.get(), other.m_count);
        }

        TkStackArray(const TkStackArray &other, i32 extraSize)
        {
            TK_ASSERT(extraSize >= 0);
            m_count = other.m_count + extraSize;

            TkMemory::constructItems(m_data, other.get(), other.m_count);
            TkMemory::constructItems(m_data + other.m_count, extraSize);
        }

        template <typename OtherT = T, psize OtherCapacity>
        explicit TkStackArray(const TkStackArray<OtherT, OtherCapacity> &other) noexcept
        {
            static_assert(OtherCapacity <= Capacity, "TkStackArray capacity overflow");
            m_count = other.count();
            if (m_count > 0)
                TkMemory::constructItems(m_data, other.get(), m_count);
        }

        TkStackArray(TkStackArray &&other) noexcept
        {
            m_count = other.m_count;
            for (i32 i = 0; i < m_count; ++i)
                m_data[i] = moveTemp(other.m_data[i]);

            TkMemory::destructItems(other.m_data, m_count);
            other.m_count = 0;
        }
        TkStackArray &operator=(std::initializer_list<T> initList) noexcept
        {
            clear();
            if (initList.size() > 0)
            {
                TK_ASSERT(static_cast<i32>(initList.size()) <= Capacity && "TkStackArray overflow");
                m_count = static_cast<i32>(initList.size());
                TkMemory::constructItems(m_data, initList.begin(), m_count);
            }
            return *this;
        }
        TkStackArray &operator=(const TkStackArray &other) noexcept
        {
            if (this != &other)
            {
                TkMemory::destructItems(m_data, m_count);
                m_count = other.count();
                TkMemory::constructItems(m_data, other.get(), m_count);
            }
            return *this;
        }
        TkStackArray &operator=(TkStackArray &&other) noexcept
        {
            if (this != &other)
            {
                TkMemory::destructItems(m_data, m_count);

                m_count = other.m_count;
                for (i32 i = 0; i < m_count; ++i)
                    m_data[i] = moveTemp(other.m_data[i]);

                TkMemory::destructItems(other.m_data, m_count);
                other.m_count = 0;
            }
            return *this;
        }

        ~TkStackArray()
        {
            TkMemory::destructItems(m_data, m_count);
        }

    public:
        TK_FORCE_INLINE i32 count() const
        {
            return m_count;
        }
        TK_FORCE_INLINE bool hasItems() const
        {
            return m_count != 0;
        }
        TK_FORCE_INLINE bool isEmpty() const
        {
            return m_count == 0;
        }

        bool isValidIndex(const i32 index) const
        {
            return index < m_count && index >= 0;
        }

        TK_FORCE_INLINE T *get()
        {
            return m_data;
        }

        TK_FORCE_INLINE const T *get() const
        {
            return m_data;
        }

        TK_FORCE_INLINE T &at(const i32 index)
        {
            TK_ASSERT(index >= 0 && index < m_count);
            return m_data[index];
        }
        TK_FORCE_INLINE const T &at(const i32 index) const
        {
            TK_ASSERT(index >= 0 && index < m_count);
            return m_data[index];
        }
        TK_FORCE_INLINE T &operator[](const i32 index)
        {
            TK_ASSERT(index >= 0 && index < m_count);
            return m_data[index];
        }
        TK_FORCE_INLINE const T &operator[](const i32 index) const
        {
            TK_ASSERT(index >= 0 && index < m_count);
            return m_data[index];
        }

        TK_FORCE_INLINE T &last()
        {
            TK_ASSERT(m_count > 0);
            return m_data[m_count - 1];
        }
        TK_FORCE_INLINE const T &last() const
        {
            TK_ASSERT(m_count > 0);
            return m_data[m_count - 1];
        }

        TK_FORCE_INLINE T &first()
        {
            TK_ASSERT(m_count > 0);
            return m_data[0];
        }
        TK_FORCE_INLINE const T &first() const
        {
            TK_ASSERT(m_count > 0);
            return m_data[0];
        }

        TK_FORCE_INLINE const T &back() const
        {
            return last();
        }
        TK_FORCE_INLINE T &back()
        {
            return last();
        }

    public:
        TK_FORCE_INLINE T *begin()
        {
            return &m_data[0];
        }
        TK_FORCE_INLINE T *end()
        {
            return &m_data[m_count];
        }
        TK_FORCE_INLINE const T *begin() const
        {
            return &m_data[0];
        }
        TK_FORCE_INLINE const T *end() const
        {
            return &m_data[m_count];
        }

    public:
        TK_FORCE_INLINE void clear()
        {
            TkMemory::destructItems(m_data, m_count);
            m_count = 0;
        }

        void resize(const i32 size, const bool preserveContents = true)
        {
            if (m_count > size)
                TkMemory::destructItems(m_data + size, m_count - size);
            else
            {
                TK_ASSERT(size <= Capacity && "TkStackArray overflow");
                TkMemory::constructItems(m_data + m_count, size - m_count);
            }
            m_count = size;
        }

        TK_FORCE_INLINE void reserve(u32 capacity)
        {
            TK_ASSERT(capacity <= Capacity && "TkStackArray overflow");
        }

        void setAll(const T &value)
        {
            T *data = m_data;
            for (i32 i = 0; i < m_count; i++)
                data[i] = value;
        }

        void set(const T *data, const i32 count)
        {
            TK_ASSERT(count <= Capacity && "TkStackArray overflow");
            TkMemory::destructItems(m_data, m_count);
            m_count = count;
            TkMemory::constructItems(m_data, data, m_count);
        }

        void add(const T &item)
        {
            TK_ASSERT(m_count + 1 <= Capacity && "TkStackArray overflow");
            TkMemory::constructItems(m_data + m_count, &item, 1);
            ++m_count;
        }
        void add(T &&item)
        {
            TK_ASSERT(m_count + 1 <= Capacity && "TkStackArray overflow");
            TkMemory::moveItems(m_data + m_count, &item, 1);
            ++m_count;
        }
        void add(const T *items, const i32 count)
        {
            TK_ASSERT(m_count + count <= Capacity && "TkStackArray overflow");
            TkMemory::constructItems(m_data + m_count, items, count);
            m_count += count;
        }
        template <typename OtherT, psize OtherCapacity>
        TK_FORCE_INLINE void add(const TkStackArray<OtherT, OtherCapacity> &other)
        {
            add(other.get(), other.count());
        }

        TK_FORCE_INLINE void addUnique(const T &item)
        {
            if (!contains(item))
                add(item);
        }

        TK_FORCE_INLINE void addDefault(const i32 count = 1)
        {
            TK_ASSERT(m_count + count <= Capacity && "TkStackArray overflow");
            TkMemory::constructItems(m_data + m_count, count);
            m_count += count;
        }

        TK_FORCE_INLINE void addUninitialized(const i32 count = 1)
        {
            TK_ASSERT(m_count + count <= Capacity && "TkStackArray overflow");
            m_count += count;
        }

        TK_FORCE_INLINE T &addOne()
        {
            TK_ASSERT(m_count + 1 <= Capacity && "TkStackArray overflow");
            TkMemory::constructItems(m_data + m_count, 1);
            ++m_count;
            return m_data[m_count - 1];
        }

        void insert(const i32 index, const T &item)
        {
            TK_ASSERT(index >= 0 && index <= m_count);
            TK_ASSERT(m_count + 1 <= Capacity && "TkStackArray overflow");
            T *data = m_data;
            TkMemory::constructItems(data + m_count, 1);
            for (i32 i = m_count - 1; i >= index; i--)
                data[i + 1] = data[i];
            m_count++;
            data[index] = item;
        }
        void insert(const i32 index, T &&item)
        {
            TK_ASSERT(index >= 0 && index <= m_count);
            TK_ASSERT(m_count + 1 <= Capacity && "TkStackArray overflow");
            T *data = m_data;
            TkMemory::constructItems(data + m_count, 1);
            for (i32 i = m_count - 1; i >= index; i--)
                data[i + 1] = moveTemp(data[i]);
            ++m_count;
            data[index] = moveTemp(item);
        }
        void insert(const i32 index)
        {
            TK_ASSERT(index >= 0 && index <= m_count);
            TK_ASSERT(m_count + 1 <= Capacity && "TkStackArray overflow");
            T *data = m_data;
            TkMemory::constructItems(data + m_count, 1);
            for (i32 i = m_count - 1; i >= index; i--)
                data[i + 1] = data[i];
            ++m_count;
        }

        template <typename TComparableType>
        bool contains(const TComparableType &item) const
        {
            const T *data = m_data;
            for (i32 i = 0; i < m_count; i++)
            {
                if (data[i] == item)
                    return true;
            }
            return false;
        }

        bool removeKeepOrder(const T &item)
        {
            const i32 index = find(item);
            if (index == -1)
                return true;
            removeAtKeepOrder(index);
            return false;
        }

        void removeAllKeepOrder(const T &item)
        {
            for (i32 i = count() - 1; i >= 0; --i)
            {
                if (m_data[i] == item)
                {
                    removeAtKeepOrder(i);
                    if (isEmpty())
                        break;
                }
            }
        }

        void removeAtKeepOrder(const i32 index)
        {
            TK_ASSERT(index < m_count && index >= 0);
            --m_count;
            T *data = m_data;
            if (index < m_count)
            {
                T *dst = data + index;
                T *src = data + (index + 1);
                const i32 count = m_count - index;
                for (i32 i = 0; i < count; ++i)
                    dst[i] = moveTemp(src[i]);
            }
            TkMemory::destructItems(data + m_count, 1);
        }

        bool remove(const T &item)
        {
            const i32 index = find(item);
            if (index == -1)
                return true;
            removeAt(index);
            return false;
        }

        void removeAll(const T &item)
        {
            for (i32 i = count() - 1; i >= 0; --i)
            {
                if (m_data[i] == item)
                {
                    removeAt(i);
                    if (isEmpty())
                        break;
                }
            }
        }

        void removeAt(const i32 index)
        {
            TK_ASSERT(index < m_count && index >= 0);
            --m_count;
            T *data = m_data;
            if (m_count)
                data[index] = data[m_count];
            TkMemory::destructItems(data + m_count, 1);
        }

        void removeLast()
        {
            TK_ASSERT(m_count > 0);
            --m_count;
            TkMemory::destructItems(m_data + m_count, 1);
        }

        void swap(TkStackArray &other)
        {
            const i32 minCount = m_count < other.m_count ? m_count : other.m_count;

            for (i32 i = 0; i < minCount; ++i)
                ::toolkit::swap(m_data[i], other.m_data[i]);

            if (m_count > other.m_count)
            {
                for (i32 i = minCount; i < m_count; ++i)
                    other.add(moveTemp(m_data[i]));
                TkMemory::destructItems(m_data + minCount, m_count - minCount);
            }
            else if (other.m_count > m_count)
            {
                for (i32 i = minCount; i < other.m_count; ++i)
                    add(moveTemp(other.m_data[i]));
                TkMemory::destructItems(other.m_data + minCount, other.m_count - minCount);
            }

            ::toolkit::swap(m_count, other.m_count);
        }

        void reverse()
        {
            T *data = m_data;
            const i32 count = m_count / 2;
            for (i32 i = 0; i < count; ++i)
                ::toolkit::swap(data[i], data[m_count - i - 1]);
        }

    public:
        TK_FORCE_INLINE void push(const T &item)
        {
            add(item);
        }

        TK_FORCE_INLINE T pop()
        {
            T item = moveTemp(last());
            removeLast();
            return item;
        }

        TK_FORCE_INLINE T &peek()
        {
            TK_ASSERT(m_count > 0);
            return m_data[m_count - 1];
        }

        TK_FORCE_INLINE const T &peek() const
        {
            TK_ASSERT(m_count > 0);
            return m_data[m_count - 1];
        }

    public:
        void enqueue(const T &item)
        {
            add(item);
        }

        void enqueue(T &&item)
        {
            add(moveTemp(item));
        }

        T dequeue()
        {
            TK_ASSERT(hasItems());
            T item = moveTemp(m_data[0]);
            removeAtKeepOrder(0);
            return item;
        }

    public:
        template <typename ComparableType>
        TK_FORCE_INLINE bool find(const ComparableType &item, i32 &index) const
        {
            index = find(item);
            return index != -1;
        }

        template <typename ComparableType>
        i32 find(const ComparableType &item) const
        {
            if (m_count > 0)
            {
                const T *TK_RESTRICT start = m_data;
                for (const T *TK_RESTRICT data = start, *TK_RESTRICT dataEnd = data + m_count; data != dataEnd; ++data)
                {
                    if (*data == item)
                        return static_cast<i32>(data - start);
                }
            }
            return -1;
        }

        template <typename ComparableType>
        TK_FORCE_INLINE bool findLast(const ComparableType &item, int &index) const
        {
            index = findLast(item);
            return index != -1;
        }

        template <typename ComparableType>
        i32 findLast(const ComparableType &item) const
        {
            if (m_count > 0)
            {
                const T *TK_RESTRICT end = m_data + m_count;
                for (const T *TK_RESTRICT data = end, *TK_RESTRICT dataStart = data - m_count; data != dataStart;)
                {
                    --data;
                    if (*data == item)
                        return static_cast<i32>(data - dataStart);
                }
            }
            return -1;
        }

    public:
        template <typename OtherT = T, psize OtherSize>
        bool operator==(const TkStackArray<OtherT, OtherSize> &other) const
        {
            if (m_count == other.count())
            {
                const T *data = m_data;
                const T *otherData = other.get();
                for (i32 i = 0; i < m_count; i++)
                {
                    if (!(data[i] == otherData[i]))
                        return false;
                }
                return true;
            }
            return false;
        }

        template <typename OtherT = T, psize OtherSize>
        bool operator!=(const TkStackArray<OtherT, OtherSize> &other) const
        {
            return !operator==(other);
        }

    public:
        // The collection iterator
        struct Iterator
        {
            friend TkStackArray;

        private:
            TkStackArray *m_array;
            i32 m_index;

            Iterator(TkStackArray *array, const i32 index)
                : m_array(array), m_index(index)
            {
            }

            Iterator(const TkStackArray *array, const i32 index)
                : m_array(const_cast<TkStackArray *>(array)), m_index(index)
            {
            }

        public:
            Iterator()
                : m_array(nullptr), m_index(-1)
            {
            }

            Iterator(const Iterator &i)
                : m_array(i.m_array), m_index(i.m_index)
            {
            }

            Iterator(Iterator &&i)
                : m_array(i.m_array), m_index(i.m_index)
            {
            }

        public:
            TK_FORCE_INLINE TkStackArray *getTkStackArray() const
            {
                return m_array;
            }

            TK_FORCE_INLINE i32 getIndex() const
            {
                return m_index;
            }

            TK_FORCE_INLINE bool isEnd() const
            {
                return m_index == m_array->m_count;
            }

            TK_FORCE_INLINE bool isNotEnd() const
            {
                return m_index != m_array->m_count;
            }

            TK_FORCE_INLINE T &operator*() const
            {
                return m_array->get()[m_index];
            }

            TK_FORCE_INLINE T *operator->() const
            {
                return &m_array->get()[m_index];
            }

            TK_FORCE_INLINE bool operator==(const Iterator &v) const
            {
                return m_array == v.m_array && m_index == v.m_index;
            }

            TK_FORCE_INLINE bool operator!=(const Iterator &v) const
            {
                return m_array != v.m_array || m_index != v.m_index;
            }

            Iterator &operator=(const Iterator &v)
            {
                m_array = v.m_array;
                m_index = v.m_index;
                return *this;
            }

            Iterator &operator=(Iterator &&v)
            {
                m_array = v.m_array;
                m_index = v.m_index;
                return *this;
            }

            Iterator &operator++()
            {
                if (m_index != m_array->m_count)
                    m_index++;
                return *this;
            }

            Iterator operator++(int)
            {
                Iterator temp = *this;
                if (m_index != m_array->m_count)
                    m_index++;
                return temp;
            }

            Iterator &operator--()
            {
                if (m_index > 0)
                    --m_index;
                return *this;
            }

            Iterator operator--(int)
            {
                Iterator temp = *this;
                if (m_index > 0)
                    --m_index;
                return temp;
            }
        };

    public:
        TK_FORCE_INLINE Iterator Begin() const
        {
            return Iterator(this, 0);
        }
        TK_FORCE_INLINE Iterator End() const
        {
            return Iterator(this, m_count);
        }

    private:
        i32 m_count;
        T m_data[Capacity];

    };

}

template <typename T, toolkit::psize OtherSize>
void *operator new(const toolkit::psize size, toolkit::TkStackArray<T, OtherSize> &array)
{
    using namespace toolkit;
    TK_ASSERT(size == sizeof(T));
    const i32 index = array.count();
    array.addUninitialized(1);
    return &array[index];
}

template <typename T, toolkit::psize OtherSize>
void *operator new(const toolkit::psize size, toolkit::TkStackArray<T, OtherSize> &array, const toolkit::i32 index)
{
    using namespace toolkit;
    TK_ASSERT(size == sizeof(T));
    array.insert(index);
    return &array[index];
}