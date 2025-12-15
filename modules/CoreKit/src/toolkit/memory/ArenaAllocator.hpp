// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/memory/Memory.hpp>
#include <toolkit/types/Atomic.hpp>
#include <toolkit/threading/Sync.hpp>

namespace toolkit {

    class ArenaAllocator final : public AllocatorBase
    {
    public:
        ArenaAllocator(u32 reserved);
        ~ArenaAllocator();

        void reset();
        void* alloc(psize size, psize align);
        void free(void* ptr);
        void* realloc(void* ptr, psize newSize, psize oldSize, psize align);

		IAllocator asInterface() const;

    private:
        u32 m_commited_bytes = 0;
        u32 m_reserved;
        TkAtomic<i32> m_end {0};
        u8* m_mem;
        TkMutex m_mutex;
    };
}