// Created by cubevlmu on 2025/8/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/memory/Memory.hpp>
#include <toolkit/base/Assert.hpp>

namespace toolkit {

	class StackAllocatorBase : public AllocatorBase
	{
	public:
		virtual void* alloc(psize size, psize align) = 0;
		virtual void free(void* ptr) = 0;
		virtual void* realloc(void* ptr, psize new_size, psize old_size, psize align) = 0;

		IAllocator asInterface() const;
	};

    /// An allocator based on stack data.
    template<psize CAPACITY, psize ALIGN = 16>
    class StackAllocator final : public StackAllocatorBase
	{
    public:
        explicit StackAllocator(IAllocator &fallback) : m_fallback(fallback) {}
        ~StackAllocator() { TK_ASSERT(!m_allocated); }

        void* alloc(psize size, psize align)
		{
            TK_ASSERT(align <= ALIGN);
            if (!m_allocated && size <= CAPACITY) {
                m_allocated = true;
                return m_mem;
            }
			return TMemory::allocate(m_fallback, size, align);
        }

        void free(void* ptr)
		{
            if (!ptr) return;
            if (ptr == m_mem) {
                m_allocated = false;
                return;
            }
            TK_ASSERT(ptr < m_mem || ptr > m_mem + CAPACITY);
			TMemory::free(m_fallback, ptr);
        }

        void* realloc(void* ptr, psize new_size, psize old_size, psize align)
		{
            TK_ASSERT(align <= ALIGN);
            if (!ptr) return alloc(new_size, align);
            if (ptr == m_mem) {
                TK_ASSERT(m_allocated);
                if (new_size <= CAPACITY) return m_mem;

                m_allocated = false;
				void* n = TMemory::allocate(m_fallback, new_size, align);
	            TMemory::memoryCopy(n, m_mem, CAPACITY);
                return n;
            }

            TK_ASSERT(ptr < m_mem || ptr > m_mem + CAPACITY);
            if (new_size > CAPACITY)
				return TMemory::reallocate(m_fallback, ptr, new_size, old_size, align);
            TMemory::memoryCopy(m_mem, ptr, new_size);
            m_allocated = true;
	        TMemory::free(m_fallback, ptr);
            return m_mem;
        }


    private:
        bool m_allocated = false;
        alignas(ALIGN) u8 m_mem[CAPACITY];
        IAllocator& m_fallback;
    };
}