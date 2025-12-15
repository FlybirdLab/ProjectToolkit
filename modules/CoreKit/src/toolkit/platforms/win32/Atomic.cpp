// Created by cubevlmu on 2025/8/4.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include <toolkit/common.hpp>
#include "toolkit/types/Atomic.hpp"

#if TK_WINDOWS

#include <toolkit/platforms/win32/IncludeWindows.hpp>

namespace toolkit::atomic {

    bool compareExchangePtr(void*volatile* value, void* exchange, void* comperand) {
        return _InterlockedCompareExchangePointer(value, exchange, comperand) == comperand;
    }

    void* exchangePtr(void* volatile* value, void* exchange) {
        return _InterlockedExchangePointer(value, exchange);
    }

    void memoryBarrier() {
#ifdef _M_AMD64
        __faststorefence();
#elif defined _IA64_
        __mf();
#else
	    int Barrier;
	    __asm {
            xchg Barrier, eax
	    }
#endif
    }

}

#endif