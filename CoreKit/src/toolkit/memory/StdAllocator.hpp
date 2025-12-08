// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/memory/Memory.hpp>

namespace toolkit {

    /// Using rpmalloc as a allocator fallback with standard malloc
    class StdAllocator final : public AllocatorBase
    {
    public:
        StdAllocator();
        ~StdAllocator() = default;

        void* alloc(psize size, psize align);
        void* realloc(void* ptr, psize newSize, psize oldSize, psize align);
        void free(void* ptr);

	    IAllocator asInterface() const;

	private:
#if TK_DEBUG
#endif
    };

}