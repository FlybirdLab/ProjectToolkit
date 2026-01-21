/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "BitArray.hpp"

namespace toolkit
{

    TkBitArray::TkBitArray(const i32 capacity, IAllocator &alloc)
        : m_capacity(capacity), m_alloc{alloc}
    {
        if (capacity > 0)
            m_data = (u64 *)TkMemory::allocate(alloc, toItemCapacity(capacity));
    }

    TkBitArray::TkBitArray(const TkBitArray &other, IAllocator &alloc) noexcept
        : m_alloc{alloc}
    {
        m_count = m_capacity = other.count();
        if (m_capacity > 0)
        {
            const i32 itemsCapacity = toItemCapacity(m_capacity);
            m_data = (u64 *)TkMemory::allocate(alloc, itemsCapacity);
            TkMemory::memoryCopy(get(), other.get(), itemsCapacity * sizeof(u64));
        }
    }

    TkBitArray::TkBitArray(TkBitArray &&other) noexcept
        : m_alloc{other.m_alloc}
    {
        m_count = other.m_count;
        m_capacity = other.m_capacity;
        other.m_count = 0;
        other.m_capacity = 0;
        toolkit::swap(m_data, other.m_data);
    }

    TkBitArray &TkBitArray::operator=(const TkBitArray &other) noexcept
    {
        if (this != &other)
        {
            TK_ASSERT(&m_alloc == &other.m_alloc);

            if (m_capacity < other.m_count)
            {
                TkMemory::free(m_alloc, m_data);
                m_capacity = other.m_count;
                const i32 itemsCapacity = toItemCapacity(m_capacity);
                m_data = (u64 *)TkMemory::allocate(m_alloc, itemsCapacity * sizeof(u64));
                TkMemory::memoryCopy(get(), other.get(), itemsCapacity * sizeof(u64));
            }
            m_count = other.m_count;
        }
        return *this;
    }

    TkBitArray &TkBitArray::operator=(TkBitArray &&other) noexcept
    {
        if (this != &other)
        {
            TK_ASSERT(&m_alloc == &other.m_alloc);

            TkMemory::free(m_alloc, m_data);
            m_count = other.m_count;
            m_capacity = other.m_capacity;
            other.m_count = 0;
            other.m_capacity = 0;
            toolkit::swap(m_data, other.m_data);
        }
        return *this;
    }

    TkBitArray::~TkBitArray()
    {
        if (m_data)
            TkMemory::free(m_alloc, m_data);
    }

    bool TkBitArray::get(const i32 index) const
    {
        TK_ASSERT(index >= 0 && index < m_count);
        const u64 offset = index / kBitsPerItem;
        const u64 bitMask = u64(1) << (index & (kBitsPerItem - 1));
        const u64 item = m_data[offset];
        return (item & bitMask) != 0;
    }

    void TkBitArray::set(const i32 index, const bool value)
    {
        TK_ASSERT(index >= 0 && index < m_count);
        const u64 offset = index / kBitsPerItem;
        const u64 bitMask = u64(1) << (index & (kBitsPerItem - 1));
        u64 &item = m_data[offset];
        if (value)
            item |= bitMask;
        else
            item &= ~bitMask; // Clear the bit
    }

    void TkBitArray::setCapacity(const i32 capacity, const bool preserveContents)
    {
        // FIX:
        if (capacity == m_capacity)
            return;

        TK_ASSERT(capacity >= 0);

        const i32 newCount = preserveContents
                                 ? (m_count < capacity ? m_count : capacity)
                                 : 0;

        const i32 newWords = toItemCapacity(capacity);
        const i32 oldWords = toItemCount(m_count);
        const i32 copyWords = toItemCount(newCount);

        u64 *newData = newWords > 0
                           ? static_cast<u64 *>(TkMemory::allocate(m_alloc, newWords * sizeof(u64)))
                           : nullptr;

        if (!newData && newWords > 0)
            TK_ASSERT(false && "OutOfMemory Error");

        if (newData)
            TkMemory::memorySet(newData, 0, newWords * sizeof(u64));

        if (preserveContents && copyWords > 0)
            TkMemory::moveItems(newData, m_data, copyWords);

        TkMemory::free(m_alloc, m_data);

        m_data = newData;
        m_capacity = capacity;
        m_count = newCount;
    }

    void TkBitArray::resize(const i32 size, const bool preserveContents)
    {
        if (size <= m_count)
        {
            m_count = size;
            return;
        }

        const i32 oldCount = m_count;
        ensureCapacity(size, preserveContents);
        m_count = size;

        // FIX: init new bit
        const i32 oldWords = toItemCount(oldCount);
        const i32 newWords = toItemCount(size);

        if (newWords > oldWords)
        {
            std::memset(
                m_data + oldWords,
                0,
                (newWords - oldWords) * sizeof(u64));
        }
    }

    void TkBitArray::ensureCapacity(const i32 minCapacity, const bool preserveContents)
    {
        if (m_capacity < minCapacity)
        {
            const i32 capacity = calCapacityGrow(m_alloc, m_capacity, minCapacity);
            setCapacity(capacity, preserveContents);
        }
    }

    void TkBitArray::setAll(const bool value)
    {
        if (m_count != 0)
            TkMemory::memorySet(m_data, toItemCount(m_count) * sizeof(u64), value ? ~u64(0) : 0);
    }

    void TkBitArray::add(const bool item)
    {
        ensureCapacity(m_count + 1);
        ++m_count;
        set(m_count - 1, item);
    }

    void TkBitArray::add(const bool *items, const i32 count)
    {
        ensureCapacity(m_count + count);
        for (i32 i = 0; i < count; ++i)
            add(items[i]);
    }

    void TkBitArray::add(const TkBitArray &other)
    {
        ensureCapacity(m_count + other.count());
        for (i32 i = 0; i < other.count(); ++i)
            add(other[i]);
    }

    void TkBitArray::swap(TkBitArray &other)
    {
        toolkit::swap(m_count, other.m_count);
        toolkit::swap(m_capacity, other.m_capacity);
        toolkit::swap(m_data, other.m_data);
    }

    bool TkBitArray::operator==(const TkBitArray &other) const
    {
        if (m_count != other.m_count)
            return false;

        const i32 fullWords = m_count / kBitsPerItem;
        const i32 tailBits = m_count % kBitsPerItem;

        // FIX: compare full i64
        if (fullWords > 0)
        {
            if (std::memcmp(
                    m_data,
                    other.m_data,
                    fullWords * sizeof(u64)) != 0)
                return false;
        }

        // FIX: compare last bit at the tail
        if (tailBits > 0)
        {
            const u64 mask = (u64(1) << tailBits) - 1;
            return (m_data[fullWords] & mask) == (other.m_data[fullWords] & mask);
        }

        return true;
    }
}