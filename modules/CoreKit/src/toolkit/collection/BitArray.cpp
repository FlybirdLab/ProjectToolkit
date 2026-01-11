// Created by cubevlmu on 2025/12/22.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "BitArray.hpp"

namespace toolkit {

	TkBitArray::TkBitArray(const i32 capacity, IAllocator& alloc)
		: m_count{0}
		, m_capacity{capacity}
		, m_alloc{alloc}
	{
		if (capacity > 0)
			m_allocation = (u64*)TkMemory::allocate(m_alloc, toItemCapacity(capacity) * sizeof(u64));
	}

	TkBitArray::TkBitArray(const TkBitArray& other) noexcept
	    : m_alloc {other.m_alloc}
	{
		m_count = m_capacity = other.count();
		if (m_capacity > 0)
		{
			const i32 itemsCapacity = toItemCapacity(m_capacity);
			m_allocation = (u64*)TkMemory::allocate(m_alloc, itemsCapacity * sizeof(u64));
			TkMemory::memoryCopy(get(), (void*)other.get(), itemsCapacity * sizeof(u64));
		}
	}

	TkBitArray& TkBitArray::operator=(const TkBitArray& other) noexcept
	{
		if (this != &other)
		{
			if (m_capacity < other.m_count)
			{
				TkMemory::free(m_alloc, m_allocation);
				m_capacity = other.m_count;
				const i32 itemsCapacity = toItemCapacity(m_capacity);
				m_allocation = (u64*)TkMemory::allocate(m_alloc, itemsCapacity * sizeof(u64));
				TkMemory::memoryCopy(get(), (void*)other.get(), itemsCapacity * sizeof(u64));
			}
			m_count = other.m_count;
		}
		return *this;
	}

	TkBitArray& TkBitArray::operator=(TkBitArray&& other) noexcept
	{
		if (this != &other)
		{
			TkMemory::free(m_alloc, m_allocation);
			m_count = other.m_count;
			m_capacity = other.m_capacity;
			m_alloc = other.m_alloc;

			other.m_count = 0;
			other.m_capacity = 0;

			// m_allocation.swap(other.m_allocation);
		}
		return *this;
	}
}