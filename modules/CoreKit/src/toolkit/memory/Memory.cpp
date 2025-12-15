// Created by cubevlmu on 2025/11/30.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Memory.hpp"
#include "StdAllocator.hpp"

extern "C" {

void* toolkit_system_malloc(size_t size) {
	return toolkit::TkMemory::allocate(toolkit::TkMemory::getDefault(), size);
}

void* toolkit_system_realloc(void* ptr, size_t size) {
	return toolkit::TkMemory::reallocate(toolkit::TkMemory::getDefault(), ptr, size, -1); //TODO fix bug
}

void toolkit_system_free(void* ptr) {
	return toolkit::TkMemory::free(toolkit::TkMemory::getDefault(), ptr);
}

}

namespace toolkit {

	IAllocator::IAllocator(AllocFn af, ReallocFn rf, FreeFn ff, AllocatorBase* ctx, AllocatorType typeId)
	    : alloc {af}
		, realloc {rf}
		, free {ff}
		, ctx{ctx}
		, typeId {typeId}
	{
	}

	IAllocator::~IAllocator()
	{
		ctx = nullptr;
	}

	bool TkMemory::isValid(IAllocator& a)
	{
		return a.ctx != nullptr && a.alloc != nullptr && a.free != nullptr;
	}

	static StdAllocator s_alloc {};

	IAllocator& TkMemory::getDefault()
	{
		static IAllocator alloc = s_alloc.asInterface();
		return alloc;
	}
}