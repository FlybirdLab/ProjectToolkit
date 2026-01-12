/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */
// Created by cubevlmu on 2025/12/22.
// Copyright (c) 2025 Flybird Games. All rights reserved.
//
// Function calCapacityGrow algrithm from
//          http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2

#include "ContainerMemory.hpp"

namespace toolkit
{

	i32 calCapacityGrow(IAllocator &alloc, i32 capacity, const i32 minCapacity)
	{
		if (alloc.getTypeId() == AllocatorType::kStack)
		{
			return minCapacity;
		}
		else
		{
			if (capacity < minCapacity)
				capacity = minCapacity;
			if (capacity < 8)
			{
				capacity = 8;
			}
			else
			{
				capacity--;
				capacity |= capacity >> 1;
				capacity |= capacity >> 2;
				capacity |= capacity >> 4;
				capacity |= capacity >> 8;
				capacity |= capacity >> 16;
				u64 capacity64 = (u64)(capacity + 1) * 2;
				if (capacity64 > kMaxI32)
					capacity64 = kMaxI32;
				capacity = (i32)capacity64;
			}
			return capacity;
		}
	}

}