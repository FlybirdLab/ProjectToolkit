// Created by cubevlmu on 2025/11/30.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Memory.hpp"
#include "StdAllocator.hpp"

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

	bool TMemory::isValid(IAllocator& a)
	{
		return a.ctx != nullptr && a.alloc != nullptr && a.free != nullptr;
	}

	static StdAllocator s_alloc {};

	IAllocator& TMemory::getDefault()
	{
		static IAllocator alloc = s_alloc.asInterface();
		return alloc;
	}
}