// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/common.hpp>

namespace toolkit {

    class System
    {
    public:
        static void* reserveMemory(psize size);
        static void commitMemory(void* ptr, psize size);
        static void releaseMemory(void* ptr, psize size);

        static void* alloc(psize size);
        static void* allocAligned(psize size, psize align);
        static void free(void*);
    };

}