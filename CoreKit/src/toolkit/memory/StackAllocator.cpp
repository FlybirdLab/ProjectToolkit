// Created by cubevlmu on 2025/12/1.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/memory/StackAllocator.hpp>

namespace toolkit {
	
	static void* StackAllocator_Alloc(AllocatorBase* ctx, psize size, psize align) {
		return static_cast<StackAllocatorBase*>(ctx)->alloc(size, align);
	}

	static void* StackAllocator_Realloc(AllocatorBase* ctx, void* ptr, psize newSize, psize oldSize, psize align) {
		return static_cast<StackAllocatorBase*>(ctx)->realloc(ptr, newSize, oldSize, align);
	}

	static void StackAllocator_Free(AllocatorBase* ctx, void* ptr) {
		static_cast<StackAllocatorBase*>(ctx)->free(ptr);
	}


	IAllocator StackAllocatorBase::asInterface() const {
		return IAllocator {StackAllocator_Alloc, StackAllocator_Realloc, StackAllocator_Free, (AllocatorBase*)this, AllocatorType::kStack};
	}
	
}