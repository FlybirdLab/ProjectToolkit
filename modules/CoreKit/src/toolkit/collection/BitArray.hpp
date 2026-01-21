/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * Test target: BitArrayTest.cpp
 */

#pragma once

#include "toolkit/memory/Memory.hpp"
#include <toolkit/memory/ContainerMemory.hpp>
#include <toolkit/base/Math.hpp>
#include <toolkit/base/Assert.hpp>

namespace toolkit
{

    // Template for dynamic array with variable capacity that stores the bit values
    class TkBitArray
    {
    public:
        static constexpr i32 kBitsPerItem = 64;

    public:
        TK_FORCE_INLINE TkBitArray(IAllocator &alloc = TkMemory::getDefault())
            : m_alloc{alloc}
        {
        }
        explicit TkBitArray(const i32 capacity, IAllocator &alloc = TkMemory::getDefault());
        TkBitArray(const TkBitArray &other, IAllocator &alloc = TkMemory::getDefault()) noexcept;
        TkBitArray(TkBitArray &&other) noexcept;
        // The assignment operator that deletes the current collection of items and the copies items from the other array.
        TkBitArray& operator=(const TkBitArray &other) noexcept;
        // The move assignment operator that deletes the current collection of items and the moves items from the other array.
        TkBitArray& operator=(TkBitArray &&other) noexcept;
        ~TkBitArray();

    public:
        // Gets the pointer to the bits storage data (linear allocation).
        TK_FORCE_INLINE u64 *get()
        {
            return m_data;
        }

        // Gets the pointer to the bits storage data (linear allocation).
        TK_FORCE_INLINE const u64 *get() const
        {
            return m_data;
        }

        // Gets the amount of the items in the collection.
        TK_FORCE_INLINE i32 count() const
        {
            return m_count;
        }

        // Gets the amount of the items that can be contained by collection without resizing.
        TK_FORCE_INLINE i32 capacity() const
        {
            return m_capacity;
        }

        // Returns true if collection isn't empty.
        TK_FORCE_INLINE bool hasItems() const
        {
            return m_count != 0;
        }

        // Returns true if collection is empty.
        TK_FORCE_INLINE bool isEmpty() const
        {
            return m_count == 0;
        }

        // Gets the item at the given index.
        TK_FORCE_INLINE bool operator[](const i32 index) const
        {
            return get(index);
        }

        // Gets the item at the given index.
        bool get(const i32 index) const;
        // Sets the item at the given index.
        void set(const i32 index, const bool value);

    public:
        // Clear the collection without changing its capacity.
        TK_FORCE_INLINE void clear()
        {
            m_count = 0;
        }

        // Changes the capacity of the collection.
        void setCapacity(const i32 capacity, const bool preserveContents = true);

        // Resizes the collection to the specified size. If the size is equal or less to the current capacity no additional memory reallocation in performed.
        void resize(const i32 size, const bool preserveContents = true);
        
        // Ensures the collection has given capacity (or more).
        void ensureCapacity(const i32 minCapacity, const bool preserveContents = true);

        // Sets all items to the given value
        void setAll(const bool value);

        // Adds the specified item to the collection.
        void add(const bool item);

        // Adds the specified item to the collection.
        void add(const bool *items, const i32 count);
        // Adds the other collection to the collection.
        void add(const TkBitArray &other);

        // Swaps the contents of collection with the other object without copy operation. Performs fast internal data exchange.
        void swap(TkBitArray &other);

    public:
        bool operator==(const TkBitArray &other) const;
        TK_FORCE_INLINE bool operator!=(const TkBitArray &other) const
        {
            return !operator==(other);
        }

    private:
        TK_FORCE_INLINE static i32 toItemCount(const i32 size)
        {
            return TkMath::divideAndRoundUp<i32>(size, kBitsPerItem);
        }

        TK_FORCE_INLINE static i32 toItemCapacity(const i32 size)
        {
            if (size <= 0)
                return 0;
            return TkMath::divideAndRoundUp(size, kBitsPerItem);
        }

    private:
        i32 m_count = 0;
        i32 m_capacity = 0;
        IAllocator &m_alloc;
        u64 *m_data = nullptr;
    };
}