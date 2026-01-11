// Created by cubevlmu on 2025/12/22.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/memory/ContainerMemory.hpp>
#include <toolkit/base/Math.hpp>
#include <toolkit/base/Assert.hpp>

namespace toolkit
{

	// Template for dynamic array with variable capacity that stores the bit values
	class TkBitArray final
	{
		friend TkBitArray;

	private:
		i32 m_count;
		i32 m_capacity;
		IAllocator &m_alloc;
		u64 *m_allocation;

		TK_FORCE_INLINE static i32 toItemCount(const i32 size)
		{
			return TkMath::divideAndRoundUp<i32>(size, sizeof(u64));
		}

		TK_FORCE_INLINE static i32 toItemCapacity(const i32 size)
		{
			return TkMath::max<i32>(TkMath::divideAndRoundUp<i32>(size, sizeof(u64)), 1);
		}

	public:
		TK_FORCE_INLINE TkBitArray(IAllocator &alloc = TkMemory::getDefault())
			: m_count{0}, m_capacity{0}, m_alloc{alloc}
		{
		}

		explicit TkBitArray(const i32 capacity, IAllocator &alloc = TkMemory::getDefault());
		TkBitArray(const TkBitArray &other) noexcept;

		TK_FORCE_INLINE TkBitArray(TkBitArray &&other) noexcept
			: m_alloc{other.m_alloc}
		{
			m_count = other.m_count;
			m_capacity = other.m_capacity;
			other.m_count = 0;
			other.m_capacity = 0;
			toolkit::swap(m_allocation, other.m_allocation);
		}

		// The assignment operator that deletes the current collection of items and the copies items from the other array.
		TkBitArray &operator=(const TkBitArray &other) noexcept;
		// The move assignment operator that deletes the current collection of items and the moves items from the other array.
		TkBitArray &operator=(TkBitArray &&other) noexcept;
		~TkBitArray() = default;

	public:
		TK_FORCE_INLINE u64 *get()
		{
			return m_allocation;
		}
		TK_FORCE_INLINE const u64 *get() const
		{
			return m_allocation;
		}
		TK_FORCE_INLINE i32 count() const
		{
			return m_count;
		}
		TK_FORCE_INLINE i32 capacity() const
		{
			return m_capacity;
		}
		TK_FORCE_INLINE bool hasItems() const
		{
			return m_count != 0;
		}
		TK_FORCE_INLINE bool isEmpty() const
		{
			return m_count == 0;
		}
		TK_FORCE_INLINE bool operator[](const i32 index) const
		{
			return get(index);
		}

		// Gets the item at the given index.
		bool get(const i32 index) const
		{
			TK_ASSERT(index >= 0 && index < m_count);
			const u64 offset = index / sizeof(u64);
			const u64 bitMask = (u64)(i32)(1 << (index & ((i32)sizeof(u64) - 1)));
			const u64 item = m_allocation[offset];
			return (item & bitMask) != 0;
		}

		// Sets the item at the given index.
		void set(const i32 index, const bool value)
		{
			TK_ASSERT(index >= 0 && index < m_count);
			const u64 offset = index / sizeof(u64);
			const u64 bitMask = (u64)(i32)(1 << (index & ((i32)sizeof(u64) - 1)));
			u64 &item = m_allocation[offset];
			if (value)
				item |= bitMask;
			else
				item &= ~bitMask; // Clear the bit
		}

	public:
		// Clear the collection without changing its capacity.
		TK_FORCE_INLINE void clear()
		{
			m_count = 0;
		}

		// Changes the capacity of the collection.
		void setCapacity(const i32 capacity, const bool preserveContents = true)
		{
			if (capacity == m_capacity)
				return;
			TK_ASSERT(capacity >= 0);
			const i32 count = preserveContents ? (m_count < capacity ? m_count : capacity) : 0;

			// m_allocation.relocate(toItemCapacity(capacity), toItemCount(m_count), toItemCount(count));

			m_capacity = capacity;
			m_count = count;
		}

		// Resizes the collection to the specified size. If the size is equal or less to the current capacity no additional memory reallocation in performed.
		void resize(const i32 size, const bool preserveContents = true)
		{
			if (m_count <= size)
				ensureCapacity(size, preserveContents);
			m_count = size;
		}

		// Ensures the collection has given capacity (or more).
		void ensureCapacity(const i32 minCapacity, const bool preserveContents = true)
		{
			if (m_capacity < minCapacity)
			{
				const i32 capacity = toolkit::calCapacityGrow(m_alloc, toItemCapacity(m_capacity), minCapacity);
				setCapacity(capacity, preserveContents);
			}
		}

		// Sets all items to the given value
		void setAll(const bool value)
		{
			if (m_count != 0)
				TkMemory::memorySet(m_allocation, toItemCount(m_count) * sizeof(u64), value ? kMaxU32 : 0);
		}

		// Adds the specified item to the collection.
		void add(const bool item)
		{
			ensureCapacity(m_count + 1);
			++m_count;
			set(m_count - 1, item);
		}

		// Adds the specified item to the collection.
		void add(const bool *items, const i32 count)
		{
			ensureCapacity(m_count + count);
			for (i32 i = 0; i < count; ++i)
				add(items[i]);
		}

		// Adds the other collection to the collection.
		void add(const TkBitArray &other)
		{
			ensureCapacity(m_count, other.count());
			for (i32 i = 0; i < other.count(); ++i)
				add(other[i]);
		}

		// Swaps the contents of collection with the other object without copy operation. Performs fast internal data exchange.
		void swap(TkBitArray &other)
		{
			toolkit::swap(m_count, other.m_count);
			toolkit::swap(m_capacity, other.m_capacity);
			toolkit::swap(m_allocation, other.m_allocation);
		}

	public:
		bool operator==(const TkBitArray &other) const
		{
			if (m_count == other.count())
			{
				for (i32 i = 0; i < m_count; i++)
				{
					if (!(get(i) == other.get(i)))
						return false;
				}
				return true;
			}
			return false;
		}

		bool operator!=(const TkBitArray &other) const
		{
			return !operator==(other);
		}
	};
}