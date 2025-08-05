// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "toolkit/platforms/System.hpp"

#include <toolkit/platforms/win32/IncludeWindows.hpp>

namespace toolkit {

    void* System::reserveMemory(PtrSize size) {
        return VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_READWRITE);
    }

    void System::commitMemory(void* ptr, PtrSize size) {
        VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    }

    void System::releaseMemory(void* ptr, PtrSize) {
        VirtualFree(ptr, 0, MEM_RELEASE);
    }

}