// Created by cubevlmu on 2025/12/22.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/memory/Memory.hpp>

namespace toolkit
{
	i32 calCapacityGrow(IAllocator &alloc, i32 capacity, const i32 minCapacity);
}