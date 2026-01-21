/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "TestAllocator.hpp"

namespace toolkit
{

    static void *TestAllocator_Alloc(AllocatorBase *ctx, psize size, psize align)
    {
        return static_cast<TestAllocator *>(ctx)->alloc(size, align);
    }

    static void *TestAllocator_Realloc(AllocatorBase *ctx, void *ptr, psize newSize, psize oldSize, psize align)
    {
        return static_cast<TestAllocator *>(ctx)->realloc(ptr, newSize, oldSize, align);
    }

    static void TestAllocator_Free(AllocatorBase *ctx, void *ptr)
    {
        static_cast<TestAllocator *>(ctx)->free(ptr);
    }

    TestAllocator::TestAllocator(IAllocator &parent)
        : m_parent {parent}
    {
    }

    IAllocator TestAllocator::asInterface() const
    {
        return IAllocator(TestAllocator_Alloc, TestAllocator_Realloc, TestAllocator_Free, (AllocatorBase*)this, m_parent.getTypeId());
    }
}