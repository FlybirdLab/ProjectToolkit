// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "TagAllocator.hpp"

namespace toolkit {

    TagAllocator::TagAllocator(IAllocator& allocator, const char* tag_name)
        : m_tag(tag_name)
    {
        m_effective_allocator = m_direct_parent = &allocator;
        while (m_effective_allocator->getParent() && dynamic_cast<TagAllocator*>(m_effective_allocator)) {
            m_effective_allocator = m_effective_allocator->getParent();
        }
    }

    thread_local TagAllocator* active_allocator = nullptr;

    void* TagAllocator::alloc(size_t size, size_t align) {
        active_allocator = this;
        return m_effective_allocator->alloc(size, align);
    }

    void TagAllocator::free(void* ptr) {
        m_effective_allocator->free(ptr);
    }

    void* TagAllocator::realloc(void* ptr, size_t new_size, size_t old_size, size_t align) {
        active_allocator = this;
        return m_effective_allocator->realloc(ptr, new_size, old_size, align);
    }

    TagAllocator* TagAllocator::getActiveAllocator() {
        return active_allocator;
    }

}