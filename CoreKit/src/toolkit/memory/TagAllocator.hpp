// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/memory/IAllocator.hpp>

namespace toolkit {

    /// Effiective allocator for group allocating and relocate allocator to control allocate logic
    class TagAllocator final : public IAllocator
    {
    public:
        TagAllocator(IAllocator& allocator, const char* tag_name);
        ~TagAllocator() override;

        void* alloc(PtrSize size, PtrSize align) override;
        void* realloc(void* ptr, PtrSize newSize, PtrSize oldSize, PtrSize align) override;
        void free(void* ptr) override;

        IAllocator* getParent() const override { return m_direct_parent; }
        static TagAllocator* getActiveAllocator();

    private:
        IAllocator* m_direct_parent;
        IAllocator* m_effective_allocator;
        const char* m_tag;
    };

}