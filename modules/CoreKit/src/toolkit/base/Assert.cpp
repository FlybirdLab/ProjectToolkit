/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: khfahqp khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Assert.hpp"
#include "toolkit/types/StringView.hpp"
#include <toolkit/common.hpp>

#include <cstdio>
#include <signal.h>

#if TK_WINDOWS
#include <toolkit/platforms/win32/IncludeWindows.hpp>
#endif

#if TK_DEBUG

#include "toolkit/debug/Debug.hpp"

#if TK_LINUX
namespace toolkit
{
        bool runningFromATerminal();
}
#endif

#if TK_WINDOWS
#define TK_DEBUG_BREAK() __debugbreak()
#else
#define TK_DEBUG_BREAK() raise(SIGTRAP)
#endif

namespace toolkit
{
        void tk_assert(const char *msg, const char *func, const char *file, int line)
        {
#if TK_ANDROID
                __android_log_print(ANDROID_LOG_ERROR, "ToolkitAssert", "Assertion failed: %s (%s:%d %s)", msg, file, line, func);
#elif TK_LINUX
                if (runningFromATerminal())
                {
                        fprintf(stderr, "[ToolkitAssert] \033[1;31m Assertion failed: %s (%s:%d %s)\033[0m\n", msg, file, line, func);
                }
#elif TK_WINDOWS
                char txt[512];
                memset(&txt[0], 0, sizeof(char) * 512);
                fprintf(stderr, "Assertion failed: %s (%s:%d %s)\n", msg, file, line, func);
                snprintf((char *)&txt[0], 512, "%s\n\n%s:%d %s", msg, file, line, func);
                if (!IsDebuggerPresent())
                {
                        MessageBoxA(nullptr, &txt[0], "Assert failed", MB_OK | MB_ICONERROR);
                }
                else
                {
                        fprintf(stderr, "Assert failed %s", (const char *)&txt[0]);
                }
#else
                {
                        fprintf(stderr, "Assertion failed: %s (%s:%d %s)\n", msg, file, line, func);
                }
#endif
                printf("| StackTrace:\n");

                printStackTrace();
                TK_DEBUG_BREAK();
        }

        static bool s_isTouchMain = false;
        static void printStackFrame(StackFrame frame)
        {
                const TkStringView fnNameStr(frame.func);
                if (s_isTouchMain)
                        return;
                if (fnNameStr == "main" || fnNameStr == "main()")
                        s_isTouchMain = true;
                if (TkStringUtils::length(frame.file) != 0)
                {
                        fprintf(stderr, "   at %s", fnNameStr.get());
                        if (!fnNameStr.endsWith(')'))
                                fprintf(stderr, "() in %s:line %i\n", frame.file, frame.line);
                }
                else if (TkStringUtils::length(frame.func) != 0)
                {
                        fprintf(stderr, "   at %s", fnNameStr.get());
                        if (!fnNameStr.endsWith(')'))
                                fprintf(stderr, "()");
                        fprintf(stderr, "\n");
                }
                else
                {
                        fprintf(stderr, "   at 0x%lx\n", (u64)frame.programCounter);
                }
        }

        void printStackTrace()
        {
                Debug::getStackFrames(printStackFrame, 0, 60, nullptr);
                s_isTouchMain = false;
        }
}

#endif
