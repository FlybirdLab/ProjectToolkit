/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Debug.hpp"
#include "Logger.hpp"

#include "toolkit/memory/StackAllocator.hpp"
#include "toolkit/utils/Convertor.hpp"
#include "toolkit/platforms/System.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/types/StringView.hpp"
#include "toolkit/types/StringBuilder.hpp"

namespace toolkit
{
    TkString Debug::getStackTrace(i32 skipCount, i32 maxDepth, void *context)
    {
        TkStringBuilder result{};
        TkArray<StackFrame> stackFrames = getStackFrames(skipCount, maxDepth, context);
        for (const auto &frame : stackFrames)
        {
            const TkStringView functionNameStr(frame.func);
            if (TkStringUtils::length(frame.file) != 0)
            {
                result.append("   at ").append(functionNameStr);
                if (!functionNameStr.endsWith(')'))
                    result.append("()");
                result.appendFormat("in %s:line %i\n", frame.file, frame.line);
            }
            else if (TkStringUtils::length(frame.func) != 0)
            {
                result.append("   at ").append(functionNameStr);
                if (!functionNameStr.endsWith(')'))
                    result.append("()");
                result.append('\n');
            }
            else
            {
                result.appendFormat("   at 0x%x\n", (u64)frame.programCounter);
            }
        }
        return result.toString();
    }

    void Debug::fatal(const TkStringView msg, void *context)
    {
        // Collect crash info (platform-dependant implementation that might collect stack trace and/or create memory dump)
        {
            // Log separation for crash info
            LogError("[Debug] Critical error! Reason: %s", msg);

            // Log stack trace
            const auto trace = Debug::getStackTrace(context ? 0 : 1, 60, context);
            LogError("[Debug] Stack trace:\n %s", trace);

            // Log memory stats
            {
                const MemoryStats memoryStats = System::getMemoryStats();
                const ProcessMemoryStats processMemoryStats = System::getProcessMemoryStats();
#define GET_MEM(totalUsed, processUsed) totalUsed > processUsed ? totalUsed - processUsed : 0
                const u64 externalUsedPhysical = GET_MEM(memoryStats.usedPhysicalSize, processMemoryStats.usedPhysicalSize);
                const u64 externalUsedVirtual = GET_MEM(memoryStats.usedVirtualSize, processMemoryStats.usedVirtualSize);
#undef GET_MEM

                LogError("[Debug] Memory status:");
                LogError("[Debug] System physical: %lld (%i%)", memoryStats.usedPhysicalSize, (i32)(100 * memoryStats.usedPhysicalSize / memoryStats.physicalMemSize));
                LogError("[Debug] System virtual: %lld (%i%)", memoryStats.usedVirtualSize, (i32)(100 * memoryStats.usedVirtualSize / memoryStats.virtualMemSize));

                LogError("[Debug] Process physical: %lld (%i%)", processMemoryStats.usedPhysicalSize, (i32)(100 * processMemoryStats.usedPhysicalSize / memoryStats.physicalMemSize));
                LogError("[Debug] Process virtual: %lld (%i%)", processMemoryStats.usedVirtualSize, (i32)(100 * processMemoryStats.usedVirtualSize / memoryStats.virtualMemSize));

                LogError("[Debug] External physical: %lld (%i%)", externalUsedPhysical, (i32)(100 * externalUsedPhysical / memoryStats.physicalMemSize));
                LogError("[Debug] External virtual: %lld (%i%)", externalUsedVirtual, (i32)(100 * externalUsedVirtual / memoryStats.virtualMemSize));
            }
        }
    }

    extern void system_msgbox_show(const char* msg, const char* title, int icon);

    void Debug::error(const TkStringView msg)
    {
#if TK_HEADLESS
        LogError("[Debug] Error : %s", msg);
#else
        system_msgbox_show(msg.get(), "Error", 1);
#endif
    }

    void Debug::warn(const TkStringView msg)
    {
#if TK_HEADLESS
        LogWarn("[Debug] Error: %s", msg);
#else
        system_msgbox_show(msg.get(), "Error", 1);
#endif
    }

    void Debug::info(const TkStringView msg)
    {
#if TK_HEADLESS
        LogInfo("[Debug] Error: %s", msg);
#else
        system_msgbox_show(msg.get(), "Error", 2);
#endif
    }

    void Debug::fatal(const TkStringView msg)
    {
        fatal(msg, nullptr);
    }

    void Debug::crash(i32 line, const char *file)
    {
        const TkString msg = TkString::format("Fatal crash!\nFile: %s\nLine: %s", file, line);
        LogError("[Debug] %s", msg);
        fatal(msg, nullptr);
    }

    void Debug::crashOOM(i32 line, const char *file)
    {
        IAllocator nilAlloc{nullptr, nullptr, nullptr, nullptr, AllocatorType::kUnkown};
        StackAllocator<300> stackAlloc{nilAlloc};
        auto it = stackAlloc.asInterface();
        FormatBuffer buf {it};
        if (file)
        {
            Formatter::formatRaw(buf, "Out of memory error!\nFile: %s\nLine: %s", file, line);
        }
        else
        {
            Formatter::formatRaw(buf, "Out of memory error!");
        }
        const TkStringView msg(buf.getPointer(), (i32)buf.getUsed());
        fatal(msg, nullptr);
    }

    void Debug::crashNotImpl(i32 line, const char *file, const char *info)
    {
        const TkString msg = TkString::format("TODO: %s\nFile: %s\nLine: %i", info, file, line);
        LogError("[Debug] %s", msg);
        fatal(msg, nullptr);
    }

    void Debug::assert(const char *message, const char *file, int line)
    {
        const TkString msg = TkString::format("Assertion failed!\nFile: %s\nLine: %i\n\nExpression: %s", file, line, message);
        LogError("[Debug] %s", msg);
        fatal(msg, nullptr);
    }

    void Debug::checkFailed(const char *message, const char *file, int line)
    {
        const TkString msg = TkString::format("Check failed!\nFile: %s\nLine: %i\n\nExpression: %s", file, line, message);
        LogError("[Debug] %s", msg);
    }

}