/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/reflect/invoking/CallFrame.hpp>
#include <toolkit/reflect/RkManager.hpp>
#include <toolkit/memory/Memory.hpp>

namespace toolkit
{
    inline psize alignUp(const psize value, const psize alignment)
    {
        const psize mask = alignment - 1;
        return (value + mask) & ~mask;
    }

    CallFrame::CallFrame() = default;

    CallFrame::~CallFrame()
    {
        reset();
    }

    void *CallFrame::allocate(const psize size, const psize align)
    {
        TK_ASSERT(size > 0);
        TK_ASSERT(align > 0);

        const psize alignedOffset = alignUp(m_inlineOffset, align);
        if (alignedOffset + size <= kInlineCapacity)
        {
            void *ptr = m_inlineBuffer + alignedOffset;
            m_inlineOffset = alignedOffset + size;
            return ptr;
        }

        void *ptr = TkMemory::allocate(RkManager::getAllocator(), size, align);
        m_heapBlocks.add({ptr});
        return ptr;
    }

    void CallFrame::addDestructor(void *ptr, void (*destroy)(void *))
    {
        TK_ASSERT(ptr != nullptr);
        TK_ASSERT(destroy != nullptr);
        m_destructors.add({ptr, destroy});
    }

    void CallFrame::reset()
    {
        for (i32 i = m_destructors.count() - 1; i >= 0; --i)
        {
            DestructorRecord &record = m_destructors[i];
            if (record.destroy != nullptr)
                record.destroy(record.ptr);
        }
        m_destructors.clear();

        for (HeapBlock &block : m_heapBlocks)
        {
            if (block.ptr != nullptr)
                TkMemory::free(RkManager::getAllocator(), block.ptr);
        }
        m_heapBlocks.clear();
        m_inlineOffset = 0;
    }
}
