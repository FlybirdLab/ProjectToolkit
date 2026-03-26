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

    /// @brief Dynamic packed bit array backed by 64-bit storage words.
    class TkBitArray : protected TLinearContainerBase<u64>
    {
        using Base = TLinearContainerBase<u64>;
        using Base::m_alloc;
        using Base::m_capacity;
        using Base::m_data;
        using Base::allocateStorage;
        using Base::freeStorage;
        using Base::stealStorageFrom;
        using Base::growCapacity;

    public:
        static constexpr i32 kBitsPerItem = 64;

    public:
        /// @brief Creates an empty bit array.
        TK_FORCE_INLINE TkBitArray(IAllocator &alloc = TkMemory::getDefault())
            : Base{alloc}
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
        /// @brief Returns the raw word storage pointer.
        TK_FORCE_INLINE u64 *get()
        {
            return m_data;
        }

        // Gets the pointer to the bits storage data (linear allocation).
        TK_FORCE_INLINE const u64 *get() const
        {
            return m_data;
        }

        /// @brief Returns the number of bits stored in the array.
        TK_FORCE_INLINE i32 count() const
        {
            return m_count;
        }

        /// @brief Returns the number of bits available without growing storage.
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

        /// @brief Returns the bit value at `index`.
        bool get(const i32 index) const;
        /// @brief Sets the bit value at `index`.
        void set(const i32 index, const bool value);

    public:
        /// @brief Clears all bits while preserving allocated storage.
        TK_FORCE_INLINE void clear()
        {
            m_count = 0;
        }

        /// @brief Changes the bit capacity and optionally preserves contents.
        void setCapacity(const i32 capacity, const bool preserveContents = true);

        /// @brief Resizes the bit count and initializes new bits when growing.
        void resize(const i32 size, const bool preserveContents = true);
        
        /// @brief Ensures storage for at least `minCapacity` bits.
        void ensureCapacity(const i32 minCapacity, const bool preserveContents = true);

        /// @brief Sets every stored bit to the same value.
        void setAll(const bool value);

        /// @brief Appends one bit to the end of the array.
        void add(const bool item);

        // Adds the specified item to the collection.
        void add(const bool *items, const i32 count);
        // Adds the other collection to the collection.
        void add(const TkBitArray &other);

        /// @brief Exchanges storage with another bit array that uses the same allocator.
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
    };
}
