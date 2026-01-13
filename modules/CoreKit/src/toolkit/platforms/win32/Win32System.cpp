/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */
// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "toolkit/platforms/System.hpp"

#include "toolkit/base/Assert.hpp"

#include <toolkit/platforms/win32/IncludeWindows.hpp>
#include <rpmalloc/rpmalloc.h>

namespace toolkit {

    void *System::reserveMemory(psize size) {
        return VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_READWRITE);
    }

    void System::commitMemory(void *ptr, psize size) {
        VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    }

    void System::releaseMemory(void *ptr, psize) {
        VirtualFree(ptr, 0, MEM_RELEASE);
    }

#if TK_USE_RPMALLOC
    typedef u16 offset_t;
#define align_mem_up(num, align) (((num) + ((align) - 1)) & ~((align) - 1))

    void *System::alloc(psize size) {
        return rpmalloc(size);
    }

    void *System::allocAligned(psize size, psize alignment) {
	    return rpaligned_alloc(alignment, size);
//
//        void* ptr = nullptr;
//
//        // Alignment always has to be power of two
//        TK_ASSERT((alignment & (alignment - 1)) == 0);
//
//        if (alignment && size)
//        {
//            u32 pad = sizeof(offset_t) + (alignment - 1);
//            void* p = rpmalloc(size + pad);
//            if (p)
//            {
//                // Add the offset size to malloc's pointer
//                ptr = (void*)align_mem_up(((uintptr_t)p + sizeof(offset_t)), alignment);
//
//                // Calculate the offset and store it behind aligned pointer
//                *((offset_t*)ptr - 1) = (offset_t)((uintptr_t)ptr - (uintptr_t)p);
//            }
//        }
//        return ptr;
    }

    void System::free(void* ptr) {
        if (!ptr) return;

//        // Walk backwards from the passed-in pointer to get the pointer offset
//        offset_t offset = *((offset_t*)ptr - 1);
//
//        // Get original pointer
//        void* p = (void*)((u8*)ptr - offset);

        // Free memory
        rpfree(ptr);
    }

	void* System::realloc(void* ptr, psize size)
	{
		return rprealloc(ptr, size);
	}

	void* System::reallocAligned(void* ptr, psize size, psize newSize, psize align)
	{
		return rpaligned_realloc(ptr, align, size, newSize, 0);
	}

    bool System::init() {
        // allow cmd.exe switch encoder to utf8
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        // record main thread id
        return true;
    }

#endif
}