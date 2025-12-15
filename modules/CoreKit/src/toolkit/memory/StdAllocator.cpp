// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "StdAllocator.hpp"

#include "toolkit/platforms/System.hpp"
#include "toolkit/base/Assert.hpp"

namespace toolkit {

	static void* StdAllocator_Alloc(AllocatorBase* ctx, psize size, psize align) {
		return static_cast<StdAllocator*>(ctx)->alloc(size, align);
	}

	static void* StdAllocator_Realloc(AllocatorBase* ctx, void* ptr, psize newSize, psize oldSize, psize align) {
		return static_cast<StdAllocator*>(ctx)->realloc(ptr, newSize, oldSize, align);
	}

	static void StdAllocator_Free(AllocatorBase* ctx, void* ptr) {
		static_cast<StdAllocator*>(ctx)->free(ptr);
	}

    StdAllocator::StdAllocator() = default;

    void* StdAllocator::alloc(psize size, psize align) {
        return System::allocAligned(size, align);
    }

    void *StdAllocator::realloc(void *ptr, psize newSize, psize oldSize, psize align) {
        return System::reallocAligned(ptr, newSize, oldSize, align);
    }

    void StdAllocator::free(void *ptr) {
        System::free(ptr);
    }

	IAllocator StdAllocator::asInterface() const
	{
		return IAllocator { StdAllocator_Alloc, StdAllocator_Realloc, StdAllocator_Free, (AllocatorBase*)this, AllocatorType::kStd};
	}

}