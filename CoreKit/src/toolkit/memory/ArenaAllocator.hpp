// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/memory/IAllocator.hpp>
#include <toolkit/types/Atomic.hpp>
#include <toolkit/threading/Sync.hpp>

namespace toolkit {

    class ArenaAllocator final : public IAllocator
    {
    public:
        ArenaAllocator(u32 reserved, IAllocator& parent);
        ~ArenaAllocator();

        void reset();
        void* alloc(PtrSize size, PtrSize align) override;
        void free(void* ptr) override;
        void* realloc(void* ptr, PtrSize new_size, PtrSize old_size, PtrSize align) override;

    private:
        IAllocator& m_parent;
        u32 m_commited_bytes = 0;
        u32 m_reserved;
        TkAtomic<i32> m_end {0};
        u8* m_mem;
        TkMutex m_mutex;
    };
}