// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/globals.hpp>

namespace toolkit {

    class System
    {
    public:
        static void* reserveMemory(PtrSize size);
        static void commitMemory(void* ptr, PtrSize size);
        static void releaseMemory(void* ptr, PtrSize size);
    };

}