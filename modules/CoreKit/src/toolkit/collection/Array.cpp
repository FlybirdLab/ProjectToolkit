// Created by cubevlmu on 2025/12/15.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Array.hpp"

#include <toolkit/memory/StackAllocator.hpp>

namespace toolkit {

	psize getStackAllocatorCapacity(IAllocator& alloc) {
		if (alloc.getTypeId() != AllocatorType::kStack)
			return -1;
		auto* ptr = (StackAllocatorBase*)alloc.operator AllocatorBase *();
		return ptr->getCapacity();
	}

}