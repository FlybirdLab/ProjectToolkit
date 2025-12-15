// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "toolkit/debug/Debug.hpp"

#include "IncludeWindows.hpp"
#include <DbgHelp.h>
#include <iostream>

#pragma comment(lib, "Dbghelp.lib")

namespace toolkit {

    template<typename Lambda>
    inline void DumpStack(Lambda&& lambda)
    {
        const int maxFrames = 64;
        void* frames[maxFrames] = { 0 };

        USHORT captured = RtlCaptureStackBackTrace(0, maxFrames, frames, nullptr);
        if (captured == 0) return;

        HANDLE process = GetCurrentProcess();
        SymInitialize(process, NULL, TRUE);

        for (USHORT i = 0; i < captured; ++i)
        {
            DWORD64 address = (DWORD64)frames[i];

            char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = { 0 };
            PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
            pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            pSymbol->MaxNameLen = MAX_SYM_NAME;

            DWORD64 displacement = 0;
            if (SymFromAddr(process, address, &displacement, pSymbol))
            {
                lambda(pSymbol->Name);
            }
            else
            {
                lambda("Unknown");
            }
        }
    }

    void getStackTrace(const stack_trace_func f) {
        DumpStack(f);
    }
}