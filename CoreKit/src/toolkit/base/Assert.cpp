// Created by cubevlmu on 2025/8/4.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Assert.hpp"
#include <toolkit/globals.hpp>

#include <cstdlib>
#include <cstdio>

#if TK_WINDOWS
#include <toolkit/platforms/win32/IncludeWindows.hpp>
#endif

#if TK_DEBUG

#include "toolkit/debug/Debug.hpp"

#if TK_WINDOWS
#define TK_DEBUG_BREAK() __debugbreak()
#else
#define TK_DEBUG_BREAK()  raise(SIGTRAP)
#endif

namespace toolkit {
    void assert(const char* msg, const char* func, const char* file, int line)
    {
#	if TK_ANDROID
        __android_log_print(ANDROID_LOG_ERROR, "ToolkitAssert", "Assertion failed: %s (%s:%d %s)", msg, file, line, func);
#	elif TK_LINUX
        if(runningFromATerminal()) {
            fprintf(stderr, "[ToolkitAssert] \033[1;31m Assertion failed: %s (%s:%d %s)\033[0m\n", msg, file, line, func);
	    }
#   elif TK_WINDOWS
        char txt[512];
        memset(&txt[0], 0, sizeof(char) * 512);
        snprintf((char*)&txt[0], 512, "%s\n\n%s:%d %s", msg, file, line, func);
        if(!IsDebuggerPresent())
        {
            MessageBoxA(nullptr, &txt[0], "Assert failed", MB_OK | MB_ICONERROR);
        } else {
            fprintf(stderr, "Assert failed %s", (const char*)&txt[0]);
        }
#	else
        {
            fprintf(stderr, "Assertion failed: %s (%s:%d %s)\n", msg, file, line, func);
        }
#   endif
        printf("| StackTrace:\n");
        getStackTrace([](const char* tr) {
            printf("|> %s\n", tr);
        });
        TK_DEBUG_BREAK();
    }
}

#endif