// Created by cubevlmu on 2025/8/4.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/memory/Memory.hpp>
#include <toolkit/threading/Sync.hpp>

namespace toolkit {

    struct PoolAllocatorPage;

    class PageAllocator final : public AllocatorBase
    {
    public:
        PageAllocator();
        ~PageAllocator();

        void* alloc(psize size, psize align);
        void* realloc(void* ptr, psize newSize, psize oldSize, psize align);
        void free(void* ptr);

		IAllocator asInterface() const;

    private:
        void freeSmall(void* mem);
        void* allocSmall(size_t n);
        bool isSmallAlloc(void* p);
        void* reallocSmallAligned(void* mem, psize n, psize align);

    private:
        u8* m_small_allocations = nullptr;
        PoolAllocatorPage* m_free_lists[4];
        u32 m_page_count = 0;
        TkMutex m_mutex;
    };
}
