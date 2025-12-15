// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "ArenaAllocator.hpp"

#include "toolkit/platforms/System.hpp"
#include "toolkit/base/Assert.hpp"

namespace toolkit {

	static void* ArenaAllocator_Alloc(AllocatorBase* ctx, psize size, psize align) {
		return static_cast<ArenaAllocator*>(ctx)->alloc(size, align);
	}

	static void* ArenaAllocator_Realloc(AllocatorBase* ctx, void* ptr, psize newSize, psize oldSize, psize align) {
		return static_cast<ArenaAllocator*>(ctx)->realloc(ptr, newSize, oldSize, align);
	}

	static void ArenaAllocator_Free(AllocatorBase* ctx, void* ptr) {
		(void)ctx; (void)ptr;
	}

    ArenaAllocator::ArenaAllocator(u32 reserved)
    {
        m_reserved = reserved;
        m_mem = (u8*)System::reserveMemory(reserved);
    }

    ArenaAllocator::~ArenaAllocator() {
        TK_ASSERT(m_end.getRaw() == 0);
        System::releaseMemory(m_mem, m_reserved);
    }

    void ArenaAllocator::reset() {
        m_end.store(0);
    }

    bool isPowOfTwo(int num) {
        return (num > 0) && (num & (num - 1)) == 0;
    }

    static u32 roundUp(u32 val, u32 align) {
        TK_ASSERT(isPowOfTwo(align));
        return (val + align - 1) & ~(align - 1);
    }

    void* ArenaAllocator::alloc(psize size, psize align)
	{
        TK_ASSERT(size < 0xffFFffFF);
        u32 start;
        for (;;) {
            const u32 end = m_end.load();
            start = roundUp(end, (u32)align);
            if (m_end.compareExchange((int&)end, i32(start + size))) break;
        }

        if (start + size <= m_commited_bytes) return m_mem + start;

        TkMutexGuard guard(m_mutex);
        if (start + size <= m_commited_bytes) return m_mem + start;

        const u32 commited = roundUp(start + (u32)size, 4096);
        TK_ASSERT(commited < m_reserved);
        System::commitMemory(m_mem + m_commited_bytes, commited - m_commited_bytes);
        m_commited_bytes = commited;

        return m_mem + start;
    }

    void ArenaAllocator::free(void* ptr) {
	    (void)ptr;
        /*everything should be "deallocated" with reset()*/
    }

    void* ArenaAllocator::realloc(void* ptr, psize newSize, psize, psize align) {
        if (!ptr) return alloc(newSize, align);
        // realloc not supported
        TK_ASSERT(false);
        return nullptr;
    }

	IAllocator ArenaAllocator::asInterface() const
	{
		return IAllocator(ArenaAllocator_Alloc, ArenaAllocator_Realloc, ArenaAllocator_Free, (AllocatorBase*)this, AllocatorType::kArena);
	}

}