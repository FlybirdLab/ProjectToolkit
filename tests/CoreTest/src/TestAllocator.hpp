/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/memory/StdAllocator.hpp"
#include "toolkit/base/Assert.hpp"

namespace toolkit
{
    class TestAllocator : public AllocatorBase
    {
    public:
        size_t allocCount = 0;
        size_t freeCount = 0;

    public:
        TestAllocator(IAllocator &parent = TkMemory::getDefault());
        ~TestAllocator() = default;

        void* alloc(psize size, psize align)
        {
            allocCount++;
            return TkMemory::allocate(m_parent, size, align);
        }
        void *realloc(void *ptr, psize newSize, psize oldSize, psize align)
        {
            TK_ASSERT(false && "Not imple");
            return nullptr;
        }
        void free(void *ptr)
        {
            freeCount++;
            TkMemory::free(m_parent, ptr);
        }

        IAllocator asInterface() const;

    private:
        IAllocator &m_parent;
    };

}