// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "TagAllocator.hpp"

namespace toolkit {

    TagAllocator::TagAllocator(IAllocator allocator, const char* tag_name)
        : m_tag {tag_name}
		, m_direct_parent {allocator}
    {
    }

    thread_local TagAllocator* active_allocator = nullptr;

    void* TagAllocator::alloc(psize size, psize align) {
        active_allocator = this;
        return TMemory::allocate(m_direct_parent, size, align);
    }

    void TagAllocator::free(void* ptr) {
        TMemory::free(m_direct_parent, ptr);
    }

    void* TagAllocator::realloc(void* ptr, psize new_size, psize old_size, psize align) {
        active_allocator = this;
        return TMemory::reallocate(m_direct_parent, ptr, new_size, old_size, align);
    }

    TagAllocator* TagAllocator::getActiveAllocator() {
        return active_allocator;
    }

	IAllocator TagAllocator::asInterface()
	{
		// No interface for tag allocator
		return m_direct_parent;
	}

}