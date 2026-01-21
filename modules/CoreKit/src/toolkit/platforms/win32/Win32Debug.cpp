/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "toolkit/debug/Debug.hpp"

#if TK_WINDOWS

#include <toolkit/common.hpp>
#include "Win32System.hpp"

#include "toolkit/utils/Convertor.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/base/Math.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/types/StringView.hpp"

#include <toolkit/collection/Array.hpp>
#include <DbgHelp.h>

#pragma comment(lib, "Dbghelp.lib")

#if TK_CRASH_LOG || TK_TRACY_ENABLE

// Lock for symbols list, shared with Tracy
extern "C"
{
    static HANDLE tk_dbgHelpLock;

    void tk_dbg_setup()
    {
        tk_dbgHelpLock = ::CreateMutexW(nullptr, FALSE, nullptr);
    }

    void tk_dbg_lock()
    {
        ::WaitForSingleObject(tk_dbgHelpLock, INFINITE);
    }

    void tk_dbg_unlock()
    {
        ::ReleaseMutex(tk_dbgHelpLock);
    }
}
#endif

namespace toolkit
{
    extern SystemGlobals s_global;

    void Debug::log(const TkStringView msg)
    {
        char buffer[512];
        char *str;
        if (msg.length() + 3 < lengthOf(buffer))
            str = buffer;
        else
            str = (char*)TkMemory::allocate(TkMemory::getDefault(), (msg.length() + 3) * sizeof(char), 16);
        TkMemory::memoryCopy(str, msg.get(), msg.length() * sizeof(char));
        str[msg.length() + 0] = '\r';
        str[msg.length() + 1] = '\n';
        str[msg.length() + 2] = 0;
        StringAsUTF16 u16{buffer};
        ::OutputDebugStringW(u16.get());
        if (str != buffer)
            TkMemory::free(TkMemory::getDefault(), str);
    }

    bool Debug::isDebuggerPresent()
    {
        return ::IsDebuggerPresent();
    }

    TkArray<StackFrame> Debug::getStackFrames(i32 skipCount, i32 maxDepth, void *context)
    {
        TkArray<StackFrame> result{};
        ::tk_dbg_lock();

        // Initialize
        HANDLE process = GetCurrentProcess();
        HANDLE thread = GetCurrentThread();
        if (!s_global.isSymInit)
        {
            s_global.isSymInit = true;
            TkString symbolSearchPath;
            for (auto &path : s_global.symbolsPath)
            {
                symbolSearchPath += path;
                symbolSearchPath += ";";
            }
            symbolSearchPath += System::getWorkDir();
            StringAsUTF16 pth{symbolSearchPath.get()};
            ::SymInitializeW(process, pth.get(), TRUE);
        }

        // Capture the backtrace
        i32 count;
        void *backtrace[100];
        maxDepth = TkMath::min<i32>(maxDepth, lengthOf(backtrace));
        if (context)
        {
            auto &ctx = *(((EXCEPTION_POINTERS *)context)->ContextRecord);
            STACKFRAME64 stack;
            memset(&stack, 0, sizeof(stack));
            DWORD imageType;
#ifdef _M_IX86
            imageType = IMAGE_FILE_MACHINE_I386;
            stack.AddrPC.Offset = ctx.Eip;
            stack.AddrPC.Mode = AddrModeFlat;
            stack.AddrFrame.Offset = ctx.Ebp;
            stack.AddrFrame.Mode = AddrModeFlat;
            stack.AddrStack.Offset = ctx.Esp;
            stack.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
            imageType = IMAGE_FILE_MACHINE_AMD64;
            stack.AddrPC.Offset = ctx.Rip;
            stack.AddrPC.Mode = AddrModeFlat;
            stack.AddrFrame.Offset = ctx.Rsp;
            stack.AddrFrame.Mode = AddrModeFlat;
            stack.AddrStack.Offset = ctx.Rsp;
            stack.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
            imageType = IMAGE_FILE_MACHINE_IA64;
            stack.AddrPC.Offset = ctx.StIIP;
            stack.AddrPC.Mode = AddrModeFlat;
            stack.AddrFrame.Offset = ctx.IntSp;
            stack.AddrFrame.Mode = AddrModeFlat;
            stack.AddrBStore.Offset = ctx.RsBSP;
            stack.AddrBStore.Mode = AddrModeFlat;
            stack.AddrStack.Offset = ctx.IntSp;
            stack.AddrStack.Mode = AddrModeFlat;
#elif _M_ARM64
            imageType = IMAGE_FILE_MACHINE_ARM64;
            stack.AddrPC.Offset = ctx.Pc;
            stack.AddrPC.Mode = AddrModeFlat;
            stack.AddrFrame.Offset = ctx.Fp;
            stack.AddrFrame.Mode = AddrModeFlat;
            stack.AddrStack.Offset = ctx.Sp;
            stack.AddrStack.Mode = AddrModeFlat;
#else
#error "Platform not supported!"
#endif
            count = 0;
            for (i32 i = 0; i < skipCount; i++)
                StackWalk64(imageType, process, thread, &stack, &ctx, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr);
            while (count < maxDepth && StackWalk64(imageType, process, thread, &stack, &ctx, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
                backtrace[count++] = (void *)stack.AddrPC.Offset;
        }
        else
        {
            skipCount++;
            count = RtlCaptureStackBackTrace(skipCount, maxDepth, backtrace, nullptr);
        }

        // Walk the stack to collect the symbols
        result.resize(count);
        TkMemory::memoryClear(result.get(), count * sizeof(StackFrame));
        for (i32 i = 0; i < count; i++)
        {
            auto &frame = result[i];
            frame.programCounter = backtrace[i];

            // Get function name
            alignas(IMAGEHLP_SYMBOL64) byte symbolData[sizeof(IMAGEHLP_SYMBOL64) + lengthOf(frame.func)];
            IMAGEHLP_SYMBOL64 *symbol = (IMAGEHLP_SYMBOL64 *)symbolData;
            symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
            symbol->MaxNameLength = lengthOf(frame.func) - 1;
            DWORD64 displacement = 0;
            if (SymGetSymFromAddr64(process, (DWORD64)frame.programCounter, &displacement, symbol))
            {
                UnDecorateSymbolName(symbol->Name, frame.func, lengthOf(frame.func), UNDNAME_COMPLETE);
            }

            // Get filename and line number
            IMAGEHLP_LINE64 line;
            line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
            DWORD offset;
            if (SymGetLineFromAddr64(process, (DWORD64)frame.programCounter, &offset, &line))
            {
                frame.line = line.LineNumber;
                const i32 fileNameLength = TkMath::min<i32>(lengthOf(frame.file) - 1, TkStringUtils::length(line.FileName));
                memcpy(frame.file, line.FileName, fileNameLength);
                frame.file[fileNameLength] = '\0';
            }

            // Get module name
            IMAGEHLP_MODULE64 module;
            module.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
            if (SymGetModuleInfo64(process, (DWORD64)frame.programCounter, &module))
            {
                const i32 moduleNameLength = TkMath::min<i32>(lengthOf(frame.module) - 1, TkStringUtils::length(module.ImageName));
                memcpy(frame.module, module.ImageName, moduleNameLength);
                frame.module[moduleNameLength] = '\0';
            }
        }

        tk_dbg_unlock();
        return result;
    }

    void Debug::getStackFrames(symbol_fn fn, i32 skipCount, i32 maxDepth, void *context)
    {
        ::tk_dbg_lock();

        // Initialize
        HANDLE process = GetCurrentProcess();
        HANDLE thread = GetCurrentThread();
        if (!s_global.isSymInit)
        {
            s_global.isSymInit = true;
            TkString symbolSearchPath;
            for (auto &path : s_global.symbolsPath)
            {
                symbolSearchPath += path;
                symbolSearchPath += ";";
            }
            symbolSearchPath += System::getWorkDir();
            StringAsUTF16 pth{symbolSearchPath.get()};
            ::SymInitializeW(process, pth.get(), TRUE);
        }

        // Capture the backtrace
        i32 count;
        void *backtrace[100];
        maxDepth = TkMath::min<i32>(maxDepth, lengthOf(backtrace));
        if (context)
        {
            auto &ctx = *(((EXCEPTION_POINTERS *)context)->ContextRecord);
            STACKFRAME64 stack;
            memset(&stack, 0, sizeof(stack));
            DWORD imageType;
#ifdef _M_IX86
            imageType = IMAGE_FILE_MACHINE_I386;
            stack.AddrPC.Offset = ctx.Eip;
            stack.AddrPC.Mode = AddrModeFlat;
            stack.AddrFrame.Offset = ctx.Ebp;
            stack.AddrFrame.Mode = AddrModeFlat;
            stack.AddrStack.Offset = ctx.Esp;
            stack.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
            imageType = IMAGE_FILE_MACHINE_AMD64;
            stack.AddrPC.Offset = ctx.Rip;
            stack.AddrPC.Mode = AddrModeFlat;
            stack.AddrFrame.Offset = ctx.Rsp;
            stack.AddrFrame.Mode = AddrModeFlat;
            stack.AddrStack.Offset = ctx.Rsp;
            stack.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
            imageType = IMAGE_FILE_MACHINE_IA64;
            stack.AddrPC.Offset = ctx.StIIP;
            stack.AddrPC.Mode = AddrModeFlat;
            stack.AddrFrame.Offset = ctx.IntSp;
            stack.AddrFrame.Mode = AddrModeFlat;
            stack.AddrBStore.Offset = ctx.RsBSP;
            stack.AddrBStore.Mode = AddrModeFlat;
            stack.AddrStack.Offset = ctx.IntSp;
            stack.AddrStack.Mode = AddrModeFlat;
#elif _M_ARM64
            imageType = IMAGE_FILE_MACHINE_ARM64;
            stack.AddrPC.Offset = ctx.Pc;
            stack.AddrPC.Mode = AddrModeFlat;
            stack.AddrFrame.Offset = ctx.Fp;
            stack.AddrFrame.Mode = AddrModeFlat;
            stack.AddrStack.Offset = ctx.Sp;
            stack.AddrStack.Mode = AddrModeFlat;
#else
#error "Platform not supported!"
#endif
            count = 0;
            for (i32 i = 0; i < skipCount; i++)
                StackWalk64(imageType, process, thread, &stack, &ctx, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr);
            while (count < maxDepth && StackWalk64(imageType, process, thread, &stack, &ctx, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
                backtrace[count++] = (void *)stack.AddrPC.Offset;
        }
        else
        {
            skipCount++;
            count = RtlCaptureStackBackTrace(skipCount, maxDepth, backtrace, nullptr);
        }

        // Walk the stack to collect the symbols
        for (i32 i = 0; i < count; i++)
        {
            StackFrame frame {};
            frame.programCounter = backtrace[i];

            // Get function name
            alignas(IMAGEHLP_SYMBOL64) byte symbolData[sizeof(IMAGEHLP_SYMBOL64) + lengthOf(frame.func)];
            IMAGEHLP_SYMBOL64 *symbol = (IMAGEHLP_SYMBOL64 *)symbolData;
            symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
            symbol->MaxNameLength = lengthOf(frame.func) - 1;
            DWORD64 displacement = 0;
            if (SymGetSymFromAddr64(process, (DWORD64)frame.programCounter, &displacement, symbol))
            {
                UnDecorateSymbolName(symbol->Name, frame.func, lengthOf(frame.func), UNDNAME_COMPLETE);
            }

            // Get filename and line number
            IMAGEHLP_LINE64 line;
            line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
            DWORD offset;
            if (SymGetLineFromAddr64(process, (DWORD64)frame.programCounter, &offset, &line))
            {
                frame.line = line.LineNumber;
                const i32 fileNameLength = TkMath::min<i32>(lengthOf(frame.file) - 1, TkStringUtils::length(line.FileName));
                memcpy(frame.file, line.FileName, fileNameLength);
                frame.file[fileNameLength] = '\0';
            }

            // Get module name
            IMAGEHLP_MODULE64 module;
            module.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
            if (SymGetModuleInfo64(process, (DWORD64)frame.programCounter, &module))
            {
                const i32 moduleNameLength = TkMath::min<i32>(lengthOf(frame.module) - 1, TkStringUtils::length(module.ImageName));
                memcpy(frame.module, module.ImageName, moduleNameLength);
                frame.module[moduleNameLength] = '\0';
            }

            fn(frame);
        }

        tk_dbg_unlock();
    }

    void Debug::collectCrashData(const TkString &crashDataFolder, void *context)
    {
        // Create mini dump file for crash debugging
        struct CrashInfo
        {
            LPEXCEPTION_POINTERS exceptionPointers;
            DWORD callerThreadId;
            TkString miniDumpPath;
        };
        CrashInfo crashInfo;
        crashInfo.exceptionPointers = (PEXCEPTION_POINTERS)context;
        crashInfo.callerThreadId = GetCurrentThreadId();
        crashInfo.miniDumpPath = crashDataFolder / "Minidump.dmp";
        LogError("Creating Mini Dump to %s", crashInfo.miniDumpPath);
        auto threadFunc = [](void *data) -> DWORD
        {
            auto info = (CrashInfo *)data;
            const HANDLE process = GetCurrentProcess();
            StringAsUTF16 path{info->miniDumpPath.get()};
            const HANDLE file = ::CreateFileW(path.get(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            const MINIDUMP_TYPE minidumpType = (MINIDUMP_TYPE)(MiniDumpWithFullMemoryInfo | MiniDumpFilterMemory | MiniDumpWithHandleData | MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules);
            MINIDUMP_EXCEPTION_INFORMATION minidumpExceptionInfo;
            minidumpExceptionInfo.ThreadId = info->callerThreadId;
            minidumpExceptionInfo.ExceptionPointers = info->exceptionPointers;
            minidumpExceptionInfo.ClientPointers = FALSE;
            MiniDumpWriteDump(process, GetProcessId(process), file, minidumpType, info->exceptionPointers ? &minidumpExceptionInfo : nullptr, nullptr, nullptr);
            CloseHandle(file);
            return 0;
        };
        DWORD threadID;
        const auto handle = CreateThread(0, 0x8000, threadFunc, &crashInfo, 0, &threadID);
        WaitForSingleObject(handle, INFINITE);
    }

    void system_msgbox_show(const char* msg, const char* title, int icon) {
#if !TK_HEADLESS
        StringAsUTF16<MAX_PATH> oMsg {msg};
        switch (icon)
        {
        case 1:
            MessageBoxW(nullptr, oMsg.get(), L"Error", MB_OK | MB_ICONERROR);
            break;
        case 2:
            MessageBoxW(nullptr, oMsg.get(), L"Error", MB_OK | MB_ICONINFORMATION);
            break;
        case 3:
            MessageBoxW(nullptr, oMsg.get(), L"Error", MB_OK | MB_ICONMASK);
            break;
        }
#endif
    }
}

#endif