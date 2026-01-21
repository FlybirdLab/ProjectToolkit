/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

namespace toolkit
{
    struct StackFrame
    {
        char module[256];
        char func[256];
        char file[256];
        i32 line;
        void *programCounter;
    };
    typedef void (*symbol_fn)(StackFrame);

    class Debug
    {
    public:
        // Write log to system's default debug console.
        static void log(const TkStringView msg);
        // Show the fatal error message to the user.
        static void fatal(const TkStringView msg, void *context);
        // Show the fatal error message to the user.
        static void fatal(const TkStringView msg);
        // Show the error message to the user.
        static void error(const TkStringView msg);
        // Show the warning message to the user.
        static void warn(const TkStringView msg);
        // Show the information message to the user.
        static void info(const TkStringView msg);

    public:
        static bool isDebuggerPresent();

        // Get current native stack trace information.
        static TkArray<StackFrame> getStackFrames(i32 skipCount = 0, i32 maxDepth = 60, void *context = nullptr);
        // Get current native stack trace information without heap-alloc.
        static void getStackFrames(symbol_fn fn, i32 skipCount = 0, i32 maxDepth = 60, void *context = nullptr);
        // Get current native stack trace information as string.
        static TkString getStackTrace(i32 skipCount = 0, i32 maxDepth = 60, void *context = nullptr);
        // Crash dump data handling
        static void collectCrashData(const TkString &crashDataFolder, void *context = nullptr);

    public:
        // Make a fatal crash.
        static void crash(i32 line, const char *file);
        // Make a fatal crash occurred on memory allocation fail.
        static void crashOOM(i32 line = -1, const char *file = nullptr);
        // Make a fatal crash due to code not being implemented.
        static void crashNotImpl(i32 line, const char *file, const char *info);
        // Make a fatal crash due to assertion fail.
        static void assert(const char *message, const char *file, int line);
        // Make a error message log due to runtime value check fail.
        static void checkFailed(const char *message, const char *file, int line);
    };

}