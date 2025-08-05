// Created by cubevlmu on 2025/8/4.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/memory/IAllocator.hpp>
#include <toolkit/threading/Sync.hpp>

namespace toolkit {

    struct PoolAllocatorPage;

    class PoolAllocator final : public IAllocator
    {
    public:
        PoolAllocator();
        ~PoolAllocator() override;

        void* alloc(PtrSize size, PtrSize align) override;
        void* realloc(void* ptr, PtrSize newSize, PtrSize oldSize, PtrSize align) override;
        void free(void* ptr) override;

    private:
        void freeSmall(void* mem);
        void* allocSmall(size_t n);
        bool isSmallAlloc(void* p);
        void* reallocSmallAligned(void* mem, PtrSize n, PtrSize align);

    private:
        u8* m_small_allocations = nullptr;
        PoolAllocatorPage* m_free_lists[4];
        u32 m_page_count = 0;
        TkMutex m_mutex;
    };
}