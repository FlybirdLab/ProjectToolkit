// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "PageAllocator.hpp"
#include "toolkit/base/Assert.hpp"
#include "toolkit/platforms/System.hpp"

#if TK_WINDOWS
#include <toolkit/platforms/win32/IncludeWindows.hpp>
#endif

namespace toolkit {

	static void* PageAllocator_Alloc(AllocatorBase* ctx, psize size, psize align) {
		return static_cast<PageAllocator*>(ctx)->alloc(size, align);
	}

	static void* PageAllocator_Realloc(AllocatorBase* ctx, void* ptr, psize newSize, psize oldSize, psize align) {
		return static_cast<PageAllocator*>(ctx)->realloc(ptr, newSize, oldSize, align);
	}

	static void PageAllocator_Free(AllocatorBase* ctx, void* ptr) {
		static_cast<PageAllocator*>(ctx)->free(ptr);
	}

#define minimum(a,b) ((a) > (b)) ? (a) : (b)

    static constexpr u32 PAGE_SIZE = 4096;
    static constexpr psize MAX_PAGE_COUNT = 16384;
    static constexpr u32 SMALL_ALLOC_MAX_SIZE = 64;

    struct PoolAllocatorPage {
        struct Header {
            PoolAllocatorPage* prev;
            PoolAllocatorPage* next;
            u32 first_free;
            u32 item_size;
        };
        u8 data[PAGE_SIZE - sizeof(Header)];
        Header header;
    };
    static_assert(sizeof(PoolAllocatorPage) == PAGE_SIZE);

    static u32 sizeToBin(psize n) {
        TK_ASSERT(n > 0);
        TK_ASSERT(n <= SMALL_ALLOC_MAX_SIZE);
#if TK_WINDOWS
        unsigned long res;
        return _BitScanReverse(&res, ((unsigned long)n - 1) >> 2) ? res : 0;
#else
        psize tmp = (n - 1) >> 2;
		auto res = tmp == 0 ? 0 : 31 - __builtin_clz(tmp);
		ASSERT(res <= lengthOf(((DefaultAllocator*)nullptr)->m_free_lists));
		return res;
#endif
    }

    void initPage(u32 item_size, PoolAllocatorPage* page) {
        System::commitMemory(page, PAGE_SIZE);
        page = new (page) PoolAllocatorPage;  //TODO test it!
        page->header.first_free = 0;
        page->header.prev = nullptr;
        page->header.next = nullptr;
        page->header.item_size = item_size;

        for (u32 i = 0; i < sizeof(page->data) / item_size; ++i) {
            *(u32*)&page->data[i * item_size] = u32(i * item_size + item_size);
        }
    }

    TK_FORCE_INLINE static PoolAllocatorPage* getPage(void* ptr) {
        return (PoolAllocatorPage*)((uintptr_t)ptr & ~u64(PAGE_SIZE - 1));
    }

    void PageAllocator::freeSmall(void* mem) {
        u8* ptr = (u8*)mem;
        PoolAllocatorPage* page = getPage(ptr);

        TkMutexGuard guard(m_mutex);
        if (page->header.first_free + page->header.item_size > sizeof(page->data)) {
            TK_ASSERT(!page->header.next);
            TK_ASSERT(!page->header.prev);
            const u32 bin = sizeToBin(page->header.item_size);
            page->header.next = m_free_lists[bin];
            m_free_lists[bin] = page;
        }

        *(u32*)ptr = page->header.first_free;
        page->header.first_free = u32(ptr - page->data);
    }

    void* PageAllocator::reallocSmallAligned(void* mem, psize n, psize align) {
        PoolAllocatorPage* p = getPage(mem);
        if (n <= SMALL_ALLOC_MAX_SIZE) {
            const u32 bin = sizeToBin(n);
            if (sizeToBin(p->header.item_size) == bin) return mem;
        }

        void* new_mem = alloc(n, align);
        memcpy(new_mem, mem, minimum((psize)p->header.item_size, n)); //TODO replace with toolkit-api
        free(mem);
        return new_mem;
    }

    void* PageAllocator::allocSmall(psize n) {
        const u32 bin = sizeToBin(n);

        TkMutexGuard guard(m_mutex);

        if (!m_small_allocations) {
            m_small_allocations = (u8*)System::reserveMemory(PAGE_SIZE * MAX_PAGE_COUNT);
        }
        PoolAllocatorPage* p = m_free_lists[bin];
        if (!p) {
            if (m_page_count == MAX_PAGE_COUNT) return nullptr;

            p = (PoolAllocatorPage*)(m_small_allocations + PAGE_SIZE * m_page_count);
            initPage(8 << bin, p);
            m_free_lists[bin] = p;
            ++m_page_count;
        }

        TK_ASSERT(p->header.item_size > 0);
        TK_ASSERT(p->header.first_free + n < sizeof(p->data));
        void* res = &p->data[p->header.first_free];
        p->header.first_free = *(u32*)res;

        const bool is_page_full = p->header.first_free + p->header.item_size > sizeof(p->data);
        if (is_page_full) {
            if (m_free_lists[bin] == p) {
                m_free_lists[bin] = p->header.next;
            }
            if (p->header.next) {
                p->header.next->header.prev = p->header.prev;
            }
            if (p->header.prev) {
                p->header.prev->header.next = p->header.next;
            }
            p->header.next = p->header.prev = nullptr;
        }

        return res;
    }

    bool PageAllocator::isSmallAlloc(void* p) {
        return m_small_allocations && p >= m_small_allocations && p < m_small_allocations + (PAGE_SIZE * MAX_PAGE_COUNT);
    }

    PageAllocator::PageAllocator() {
        m_page_count = 0;
        memset(m_free_lists, 0, sizeof(m_free_lists));
    }

    PageAllocator::~PageAllocator() {
        System::releaseMemory(m_small_allocations, PAGE_SIZE * MAX_PAGE_COUNT);
    }

#if TK_WINDOWS
    void* PageAllocator::alloc(psize size, psize align)
    {
        if (size <= SMALL_ALLOC_MAX_SIZE && align <= size) {
            return allocSmall(size);
        }
        return _aligned_malloc(size, align);
    }


    void PageAllocator::free(void* ptr)
    {
        if (isSmallAlloc(ptr)) {
            freeSmall(ptr);
            return;
        }
        _aligned_free(ptr);
    }


    void* PageAllocator::realloc(void* ptr, psize new_size, psize, psize align)
    {
        if (isSmallAlloc(ptr)) {
            return reallocSmallAligned(ptr, new_size, align);
        }
        return _aligned_realloc(ptr, new_size, align);
    }

#else
    void* DefaultAllocator::alloc(psize size, psize align)
	{
		if (size <= SMALL_ALLOC_MAX_SIZE && align <= size) {
			return allocSmall(size);
		}
		return aligned_alloc(align, size);
	}


	void DefaultAllocator::free(void* ptr)
	{
		if (isSmallAlloc(ptr)) {
			freeSmall(ptr);
			return;
		}
		free(ptr);
	}


	void* DefaultAllocator::realloc(void* ptr, psize new_size, psize old_size, psize align)
	{
		if (isSmallAlloc(ptr)) {
			return reallocSmallAligned(ptr, new_size, align);
		}
		// POSIX and glibc do not provide a way to realloc with alignment preservation
		if (new_size == 0) {
			free(ptr);
			return nullptr;
		}
		void* newptr = aligned_alloc(align, new_size);
		if (newptr == nullptr) {
			return nullptr;
		}
		if (ptr) memcpy(newptr, ptr, minimum(new_size, old_size));
		free(ptr);
		return newptr;
	}
#endif

	IAllocator PageAllocator::asInterface() const
	{
		return IAllocator { PageAllocator_Alloc, PageAllocator_Realloc, PageAllocator_Free, (AllocatorBase*)this, AllocatorType::kPage};
	}

}