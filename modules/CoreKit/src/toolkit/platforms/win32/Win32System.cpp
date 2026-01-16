/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */
// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#include "Win32System.hpp"

#if TK_WINDOWS

#include "toolkit/threading/Thread.hpp"
#include "toolkit/base/Assert.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/types/StringView.hpp"
#include "toolkit/collection/Array.hpp"
#include "toolkit/collection/HashMap.hpp"
#include "toolkit/base/Hash.hpp"
#include "toolkit/types/UUID.hpp"
#include "toolkit/base/Math.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/types/Version.hpp"
#include "toolkit/debug/Debug.hpp"
#include <toolkit/utils/Convertor.hpp>
#include "toolkit/platforms/ProcessInitArgs.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/types/Path.hpp"

#include <rpmalloc/rpmalloc.h>

#include <Psapi.h>
#include <IPHlpApi.h>
#include <oleauto.h>
#include <WinBase.h>
#include <xmmintrin.h>
#include <intrin.h>

#include <VersionHelpers.h>
#include <ShellAPI.h>
#include <objbase.h>
#undef ShellExecute
#if TK_CRASH_LOG
#include <DbgHelp.h>
#endif
#pragma comment(lib, "Iphlpapi.lib")

#ifdef STARTUPINFO
#undef STARTUPINFO
#endif

using StartupInfo  = STARTUPINFOW;
using StartupInfoEx = STARTUPINFOEXW;

#if TK_CRASH_LOG || TK_TRACY_ENABLE

extern "C"
{

    extern void tk_dbg_setup();
    extern void tk_dbg_lock();
    extern void tk_dbg_unlock();
}

#endif

namespace toolkit
{
    void *System::reserveMemory(psize size)
    {
        return VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_READWRITE);
    }

    void System::commitMemory(void *ptr, psize size)
    {
        VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    }

    void System::releaseMemory(void *ptr, psize)
    {
        VirtualFree(ptr, 0, MEM_RELEASE);
    }

#if TK_USE_RPMALLOC
    void *System::alloc(psize size)
    {
        return rpmalloc(size);
    }

    void *System::allocAligned(psize size, psize alignment)
    {
        return rpaligned_alloc(alignment, size);
    }

    void System::free(void *ptr)
    {
        if (!ptr)
            return;
        rpfree(ptr);
    }

    void *System::realloc(void *ptr, psize size)
    {
        return rprealloc(ptr, size);
    }

    void *System::reallocAligned(void *ptr, psize size, psize newSize, psize align)
    {
        return rpaligned_realloc(ptr, align, size, newSize, 0);
    }

#endif

    bool Win32System::readRegVal(void *root, const TkStringView key, const TkStringView name, TkString *result)
    {
        HKEY hKey;
        StringAsUTF16<MAX_PATH> u16{key.get()};
        if (::RegOpenKeyExW((HKEY)root, u16.get(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        {
            // "Could not open registry key";
            return false;
        }

        DWORD type;
        DWORD cbData;
        StringAsUTF16<MAX_PATH> u162{name.get()};
        if (::RegQueryValueExW(hKey, u162.get(), nullptr, &type, nullptr, &cbData) != ERROR_SUCCESS)
        {
            ::RegCloseKey(hKey);
            LogWarn("[WIN32] Could not read registry value");
            return false;
        }

        if (type != REG_SZ)
        {
            ::RegCloseKey(hKey);
            LogWarn("[WIN32] Incorrect registry value type");
            return false;
        }

        TkArray<wchar> data{};
        data.resize((i32)cbData / sizeof(wchar));
        if (::RegQueryValueExW(hKey, u162.get(), nullptr, nullptr, reinterpret_cast<LPBYTE>(data.get()), &cbData) != ERROR_SUCCESS)
        {
            ::RegCloseKey(hKey);
            LogWarn("[WIN32] Could not read registry value");
            return false;
        }

        ::RegCloseKey(hKey);

        StringAsUTF8<MAX_PATH> v{data.get()};
        *result = v.get();
        return true;
    }

} // namespace toolkit

namespace toolkit
{
    SystemGlobals s_global{};
    static constexpr u64 kClrException = 0xE0434352;
    static constexpr u64 kVcppException = 0xE06D7363;

#if TK_CRASH_LOG
    void onSymbolsPathModified()
    {
        if (!s_global.isSymInit)
            return;
        HANDLE process = ::GetCurrentProcess();
        ::SymCleanup(process);
        TkString symbolSearchPath{};
        for (auto &path : s_global.symbolsPath)
        {
            symbolSearchPath += path;
            symbolSearchPath += ";";
        }
        symbolSearchPath += System::getWorkDir();
        StringAsUTF16 to{symbolSearchPath.get()};
        ::SymInitializeW(process, to.get(), TRUE);
        // SymSetSearchPathW(process, *symbolSearchPath);
        // SymRefreshModuleList(process);
    }
#endif

    static_assert(sizeof(i32) == sizeof(long), "Invalid long size for Interlocked and Atomic operations in Win32Platform.");

    long __stdcall sehExceptionHandler(EXCEPTION_POINTERS *ep)
    {
        if (ep->ExceptionRecord->ExceptionCode == kClrException)
        {
            // Pass CLR exceptions back to runtime
            return EXCEPTION_CONTINUE_SEARCH;
        }

        // Get exception info
        TkString errorMsg("Unhandled exception: ");
        switch (ep->ExceptionRecord->ExceptionCode)
        {
#define CASE(x)         \
    case x:             \
        errorMsg += #x; \
        break;
            CASE(EXCEPTION_ARRAY_BOUNDS_EXCEEDED)
            CASE(EXCEPTION_DATATYPE_MISALIGNMENT)
            CASE(EXCEPTION_FLT_DENORMAL_OPERAND)
            CASE(EXCEPTION_FLT_DIVIDE_BY_ZERO)
            CASE(EXCEPTION_FLT_INVALID_OPERATION)
            CASE(EXCEPTION_ILLEGAL_INSTRUCTION)
            CASE(EXCEPTION_INT_DIVIDE_BY_ZERO)
            CASE(EXCEPTION_PRIV_INSTRUCTION)
            CASE(EXCEPTION_STACK_OVERFLOW)
#undef CASE
        case EXCEPTION_ACCESS_VIOLATION:
            errorMsg += "EXCEPTION_ACCESS_VIOLATION ";
            if (ep->ExceptionRecord->ExceptionInformation[0] == 0)
            {
                errorMsg += "reading address ";
            }
            else if (ep->ExceptionRecord->ExceptionInformation[0] == 1)
            {
                errorMsg += "writing address ";
            }
            errorMsg += TkString::format("0x%x", (u32)ep->ExceptionRecord->ExceptionInformation[1]);
            break;
        default:
            errorMsg += TkString::format("0x%x", (u32)ep->ExceptionRecord->ExceptionCode);
        }

        // Log exception and return to the crash location when using debugger
        if (Debug::isDebuggerPresent())
        {
            LogError(errorMsg.get());
            const TkString stackTrace = Debug::getStackTrace(0, 60, ep);
            LogError(stackTrace.get());
            return EXCEPTION_CONTINUE_SEARCH;
        }

        // Crash engine
        TK_ASSERT(false && errorMsg.get());
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // Helper function to count set bits in the processor mask
    static DWORD countSetBits(ULONG_PTR bitMask)
    {
        DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
        DWORD bitSetCount = 0;
        ULONG_PTR bitTest = static_cast<ULONG_PTR>(1) << LSHIFT;
        DWORD i;

        for (i = 0; i <= LSHIFT; ++i)
        {
            bitSetCount += ((bitMask & bitTest) ? 1 : 0);
            bitTest /= 2;
        }

        return bitSetCount;
    }

    static HMONITOR getPrimaryMonitorHandle()
    {
        const POINT ptZero = {0, 0};
        return ::MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
    }

    static i32 calculateDpi(HMODULE shCoreDll)
    {
        i32 dpiX = 96, dpiY = 96;

        if (shCoreDll)
        {
            typedef HRESULT(STDAPICALLTYPE * GetDPIForMonitorProc)(HMONITOR hmonitor, UINT dpiType, UINT * dpiX, UINT * dpiY);
            const GetDPIForMonitorProc getDPIForMonitor = (GetDPIForMonitorProc)::GetProcAddress(shCoreDll, "GetDpiForMonitor");

            if (getDPIForMonitor)
            {
                HMONITOR monitor = getPrimaryMonitorHandle();

                UINT x = 0, y = 0;
                HRESULT hr = getDPIForMonitor(monitor, 0, &x, &y);
                if (SUCCEEDED(hr) && (x > 0) && (y > 0))
                {
                    dpiX = (i32)x;
                    dpiY = (i32)y;
                }
            }

            ::FreeLibrary(shCoreDll);
        }

        return (dpiX + dpiY) / 2;
    }

    static TkString getLastErrMsg()
    {
        wchar *s = nullptr;
        ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                         nullptr, ::WSAGetLastError(),
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                         reinterpret_cast<LPWSTR>(&s), 0, nullptr);
        StringAsUTF8 u8{s};
        TkString str(u8.get());
        ::LocalFree(s);
        return str;
    }

    static LONG getStringRegKey(HKEY hKey, const wchar *strValueName, TkString &strValue, const TkString &strDefaultValue)
    {
        strValue = strDefaultValue;
        WCHAR szBuffer[512];
        DWORD dwBufferSize = sizeof(szBuffer);
        ULONG nError;
        nError = ::RegQueryValueExW(hKey, strValueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(szBuffer), &dwBufferSize);
        if (ERROR_SUCCESS == nError)
        {
            StringAsUTF8<MAX_PATH> u8{szBuffer};
            strValue = u8.get();
        }
        return nError;
    }

    static LONG getDWORDRegKey(HKEY hKey, const wchar *strValueName, DWORD &nValue, DWORD nDefaultValue)
    {
        nValue = nDefaultValue;
        DWORD dwBufferSize(sizeof(DWORD));
        DWORD nResult(0);
        LONG nError = ::RegQueryValueExW(hKey, strValueName, nullptr, nullptr, reinterpret_cast<LPBYTE>(&nResult), &dwBufferSize);
        if (ERROR_SUCCESS == nError)
        {
            nValue = nResult;
        }
        return nError;
    }

    static void getWindowsVersion()
    {
        // Get OS version

        HKEY hKey;
        LONG lRes = ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey);
        if (lRes == ERROR_SUCCESS)
        {
            getStringRegKey(hKey, L"ProductName", s_global.windowsName, "Windows");

            DWORD currentMajorVersionNumber;
            DWORD currentMinorVersionNumber;
            TkString currentBuildNumber;
            getDWORDRegKey(hKey, L"CurrentMajorVersionNumber", currentMajorVersionNumber, 0);
            getDWORDRegKey(hKey, L"CurrentMinorVersionNumber", currentMinorVersionNumber, 0);
            getStringRegKey(hKey, L"CurrentBuildNumber", currentBuildNumber, "0");
            s_global.version.major = currentMajorVersionNumber;
            s_global.version.minor = currentMinorVersionNumber;
            TkStringUtils::fromString(currentBuildNumber.get(), s_global.version.build);

            if (TkStringUtils::equalStrings(s_global.windowsName.get(), "Windows 7", 9))
            {
                s_global.version.major = 6;
                s_global.version.minor = 2;
            }
            else if (s_global.version.major >= 10 && s_global.version.build >= 22000)
            {
                // Windows 11
                s_global.windowsName.replace("10", "11");
            }
            else if (s_global.version.major == 0 && s_global.version.minor == 0)
            {
                TkString windowsVersion;
                getStringRegKey(hKey, L"CurrentVersion", windowsVersion, "");

                if (windowsVersion.hasChars())
                {
                    const i32 dot = windowsVersion.find('.');
                    if (dot != -1)
                    {
                        TkStringUtils::fromString(windowsVersion.substring(0, dot).get(), s_global.version.major);
                        TkStringUtils::fromString(windowsVersion.substring(dot + 1).get(), s_global.version.minor);
                    }
                }
            }
        }
        else
        {
            if (IsWindowsServer())
            {
                s_global.windowsName = "Windows Server";
                s_global.version.major = 6;
                s_global.version.minor = 3;
            }
            else if (IsWindows8Point1OrGreater())
            {
                s_global.windowsName = "Windows 8.1";
                s_global.version.major = 6;
                s_global.version.minor = 3;
            }
            else if (IsWindows8OrGreater())
            {
                s_global.windowsName = "Windows 8";
                s_global.version.major = 6;
                s_global.version.minor = 2;
            }
            else if (IsWindows7SP1OrGreater())
            {
                s_global.windowsName = "Windows 7 SP1";
                s_global.version.major = 6;
                s_global.version.minor = 2;
            }
            else if (IsWindows7OrGreater())
            {
                s_global.windowsName = "Windows 7";
                s_global.version.major = 6;
                s_global.version.minor = 1;
            }
            else if (IsWindowsVistaSP2OrGreater())
            {
                s_global.windowsName = "Windows Vista SP2";
                s_global.version.major = 6;
                s_global.version.minor = 1;
            }
            else if (IsWindowsVistaSP1OrGreater())
            {
                s_global.windowsName = "Windows Vista SP1";
                s_global.version.major = 6;
                s_global.version.minor = 1;
            }
            else if (IsWindowsVistaOrGreater())
            {
                s_global.windowsName = "Windows Vista";
                s_global.version.major = 6;
                s_global.version.minor = 0;
            }
        }
        ::RegCloseKey(hKey);
    }

    bool System::init()
    {
        // [BaseSystem::init]
        if (!BaseSystem::init())
            return false;

        // allow cmd.exe switch encoder to utf8
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        // record main thread id

        // Init timing
        LARGE_INTEGER frequency;
        const auto freqResult = QueryPerformanceFrequency(&frequency);
        TK_ASSERT(freqResult && frequency.QuadPart > 0);
        s_global.clockFreq = frequency.QuadPart;
        s_global.cycles2Seconds = 1.0 / static_cast<double>(frequency.QuadPart);

        // Estimate program size by checking physical memory usage on start
        s_global.programSizeMem = System::getProcessMemoryStats().usedPhysicalSize;

        // Count CPUs
        BOOL done = FALSE;
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = nullptr;
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr;
        DWORD returnLength = 0;
        DWORD logicalProcessorCount = 0;
        DWORD processorCoreCount = 0;
        DWORD processorL1CacheSize = 0;
        DWORD processorL2CacheSize = 0;
        DWORD processorL3CacheSize = 0;
        DWORD processorPackageCount = 0;
        DWORD byteOffset = 0;
        PCACHE_DESCRIPTOR cache;
        while (!done)
        {
            DWORD rc = GetLogicalProcessorInformation(buffer, &returnLength);
            if (rc == FALSE)
            {
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                {
                    if (buffer)
                    {
                        free(buffer);
                    }
                    buffer = static_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(malloc(returnLength));
                    if (buffer == nullptr)
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            else
            {
                done = TRUE;
            }
        }
        ptr = buffer;
        while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
        {
            switch (ptr->Relationship)
            {
            case RelationProcessorCore:
                processorCoreCount++;
                logicalProcessorCount += countSetBits(ptr->ProcessorMask);
                break;
            case RelationCache:
                cache = &ptr->Cache;
                if (cache->Level == 1)
                {
                    processorL1CacheSize += cache->Size;
                }
                else if (cache->Level == 2)
                {
                    processorL2CacheSize += cache->Size;
                }
                else if (cache->Level == 3)
                {
                    processorL3CacheSize += cache->Size;
                }
                break;
            case RelationProcessorPackage:
                processorPackageCount++;
                break;
            default:
                TK_ASSERT("Not impl");
                break;
            }
            byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
            ptr++;
        }
        free(buffer);

        // Set info about the CPU
        s_global.cpuStats.packageCount = processorPackageCount;
        s_global.cpuStats.coreCount = processorCoreCount;
        s_global.cpuStats.logicalCoreCount = logicalProcessorCount;
        s_global.cpuStats.l1Cache = processorL1CacheSize;
        s_global.cpuStats.l2Cache = processorL2CacheSize;
        s_global.cpuStats.l3Cache = processorL3CacheSize;
        SYSTEM_INFO siSysInfo;
        GetSystemInfo(&siSysInfo);
        s_global.cpuStats.pageSize = siSysInfo.dwPageSize;
        s_global.cpuStats.speed = s_global.clockFreq;
        {
#ifdef _M_ARM64
            s_global.cpuStats.cacheLineSize = 128;
#else
            int args[4];
            __cpuid(args, 0x80000006);
            s_global.cpuStats.cacheLineSize = args[2] & 0xFF;
            TK_ASSERT(s_global.cpuStats.cacheLineSize && TkMath::isPowerOfTwo(s_global.cpuStats.cacheLineSize));
#endif
        }

        // Setup unique device ID
        {
            s_global.deviceId = TkUuid::Empty;

            // A - Computer Name and User Name
            u32 hash = HashFunc<TkString>::get(System::getComputerName());
            combineHash(hash, HashFunc<TkString>::get(System::getUserName()));
            s_global.deviceId.a = hash;

            // B - MAC address
            s_global.deviceId.b = 0;
            ::IP_ADAPTER_ADDRESSES pAddresses[16];
            DWORD dwBufLen = sizeof(pAddresses);

            const DWORD dwStatus = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &dwBufLen);
            PIP_ADAPTER_ADDRESSES pAdapterInfo = dwStatus == ERROR_SUCCESS ? pAddresses : nullptr;
            while (pAdapterInfo)
            {
                hash = pAdapterInfo->PhysicalAddress[0];
                for (UINT i = 1; i < pAdapterInfo->PhysicalAddressLength; i++)
                    combineHash(hash, pAdapterInfo->PhysicalAddress[i]);
                s_global.deviceId.b = hash;

                pAdapterInfo = pAdapterInfo->Next;
            }

            // C - memory
            s_global.deviceId.c = (u32)System::getMemoryStats().physicalMemSize;

            // D - cpuid
            auto cpuInfo = System::getCPUStats();
            s_global.deviceId.d = (u32)cpuInfo.speed * cpuInfo.logicalCoreCount * cpuInfo.coreCount * cpuInfo.cacheLineSize;
        }

        // Init networking
        if (WSAStartup(MAKEWORD(2, 0), &s_global.wsaData) != 0)
            LogError("Unable to initializes native network! Error : %s", getLastErrMsg());

        return true;
    }

    void System::quit()
    {
        WSACleanup();
    }

    void System::createUuid(TkUuid &result)
    {
        CoCreateGuid(reinterpret_cast<GUID *>(&result));
    }

    TkString System::getExecPath()
    {
        wchar buffer[MAX_PATH];
        GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        StringAsUTF8<MAX_PATH> u8{buffer};
        return TkString{u8.get()};
    }

    TkString System::getWorkDir()
    {
        wchar buffer[MAX_PATH];
        GetCurrentDirectoryW(MAX_PATH, buffer);
        StringAsUTF8<MAX_PATH> u8{buffer};
        return TkString{u8.get()};
    }

    bool System::setWorkDir(const TkString &path)
    {
        StringAsUTF16<MAX_PATH> u16{path.get()};
        return !SetCurrentDirectoryW(u16.get());
    }

    void System::freeLib(LibHandle handle)
    {
        ::FreeLibrary((HMODULE)handle);
    }

    void *System::getLibSymbol(LibHandle handle, const char *symbol)
    {
        return (void *)::GetProcAddress((HMODULE)handle, symbol);
    }

    void System::getSystemTime(TimeStats &time)
    {
        SYSTEMTIME st{};
        ::GetLocalTime(&st);

        time.year = st.wYear;
        time.month = st.wMonth;
        time.dayOfWeek = st.wDayOfWeek;
        time.day = st.wDay;
        time.hour = st.wHour;
        time.minute = st.wMinute;
        time.second = st.wSecond;
        time.millisecond = st.wMilliseconds;
    }

    void System::getUTCTime(TimeStats &time)
    {
        SYSTEMTIME st{};
        ::GetSystemTime(&st);

        time.year = st.wYear;
        time.month = st.wMonth;
        time.dayOfWeek = st.wDayOfWeek;
        time.day = st.wDay;
        time.hour = st.wHour;
        time.minute = st.wMinute;
        time.second = st.wSecond;
        time.millisecond = st.wMilliseconds;
    }

    u64 System::getCurrentProcessId()
    {
        return ::GetCurrentProcessId();
    }

    MemoryStats System::getMemoryStats()
    {
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);
        GlobalMemoryStatusEx(&statex);
        MemoryStats result;
        result.physicalMemSize = statex.ullTotalPhys;
        result.usedPhysicalSize = statex.ullTotalPhys - statex.ullAvailPhys;
        result.virtualMemSize = statex.ullTotalVirtual;
        result.usedVirtualSize = statex.ullTotalVirtual - statex.ullAvailVirtual;
        result.programSizeMem = s_global.programSizeMem;
        return result;
    }

    u64 System::getClockFrequency()
    {
        return s_global.clockFreq;
    }

    TkUuid System::getUniqueDeviceId()
    {
        return s_global.deviceId;
    }

    f64 System::getTimeSeconds()
    {
        ::LARGE_INTEGER counter;
        ::QueryPerformanceCounter(&counter);
        return double(counter.QuadPart) * s_global.cycles2Seconds;
    }

    u64 System::getTimeCycles()
    {
        ::LARGE_INTEGER counter;
        ::QueryPerformanceCounter(&counter);
        return counter.QuadPart;
    }

    ProcessMemoryStats System::getProcessMemoryStats()
    {
        ::PROCESS_MEMORY_COUNTERS_EX countersEx;
        countersEx.cb = sizeof(countersEx);
        ::GetProcessMemoryInfo(GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS)&countersEx, sizeof(countersEx));
        ProcessMemoryStats result;
        result.usedPhysicalSize = countersEx.WorkingSetSize;
        result.usedVirtualSize = countersEx.PrivateUsage;
        return result;
    }

    CPUStats System::getCPUStats()
    {
        return s_global.cpuStats;
    }

    void System::logInfo()
    {
#if TK_CPU_ARCH_X86
        // Log CPU brand
        {
            char brandBuffer[0x40] = {};
            i32 cpuInfo[4] = {-1};
            __cpuid(cpuInfo, 0x80000000);
            if (cpuInfo[0] >= 0x80000004)
            {
                for (u32 i = 0; i < 3; i++)
                {
                    __cpuid(cpuInfo, 0x80000002 + i);
                    memcpy(brandBuffer + i * sizeof(cpuInfo), cpuInfo, sizeof(cpuInfo));
                }
            }
            LogInfo("CPU: %s", brandBuffer);
        }
#endif
        LogInfo("Microsoft %s 64-bit (%i.%i.%i)", s_global.windowsName, s_global.version.major, s_global.version.minor, s_global.version.build);
    }

    bool System::createProcessMutex(const char *name)
    {
        StringAsUTF16<MAX_PATH> u16{name};
        s_global.processMutex = ::CreateMutexW(nullptr, true, u16.get());
        if (s_global.processMutex && ::GetLastError() != ERROR_ALREADY_EXISTS)
        {
            return false;
        }

        return true;
    }

    void System::releaseProcessMutex()
    {
        if (s_global.processMutex)
        {
            ::ReleaseMutex(s_global.processMutex);
            s_global.processMutex = nullptr;
        }
    }

    bool Win32System::isWindows10()
    {
        return s_global.version.major >= 10;
    }

    void System::setHighDpiAware(bool enable)
    {
        const HMODULE shCoreDll = ::LoadLibraryW(L"Shcore.dll");
        if (!shCoreDll)
            return;
        typedef enum _PROCESS_DPI_AWARENESS
        {
            PROCESS_DPI_UNAWARE = 0,
            PROCESS_SYSTEM_DPI_AWARE = 1,
            PROCESS_PER_MONITOR_DPI_AWARE = 2
        } PROCESS_DPI_AWARENESS;
        typedef HRESULT(STDAPICALLTYPE * SetProcessDpiAwarenessProc)(PROCESS_DPI_AWARENESS Value);
        const SetProcessDpiAwarenessProc setProcessDpiAwareness = (SetProcessDpiAwarenessProc)::GetProcAddress(shCoreDll, "SetProcessDpiAwareness");
        if (setProcessDpiAwareness)
        {
            setProcessDpiAwareness(enable ? PROCESS_PER_MONITOR_DPI_AWARE : PROCESS_DPI_UNAWARE);
        }
        s_global.dpi = calculateDpi(shCoreDll);
        ::FreeLibrary(shCoreDll);
    }

    TkString System::getOpSysName()
    {
        return s_global.windowsName;
    }

    TkVersion System::getOpSysVersion()
    {
        return TkVersion{s_global.version.major, s_global.version.minor, s_global.version.build};
    }

    BatteryInfo System::getBatteryStats()
    {
        BatteryInfo info{};
        ::SYSTEM_POWER_STATUS status;
        ::GetSystemPowerStatus(&status);
        info.percent = (float)status.BatteryLifePercent / 255.0f;
        if (status.BatteryFlag & 8)
            info.state = BatteryInfo::kCharging;
        else if (status.BatteryFlag & 1 || status.BatteryFlag & 2 || status.BatteryFlag & 4)
            info.state = BatteryInfo::kDischarging;
        else if (status.ACLineStatus == 1 || status.BatteryFlag & 128)
            info.state = BatteryInfo::kConnected;
        return info;
    }

    i32 System::getDpi()
    {
        return s_global.dpi;
    }

    TkString System::getUserName()
    {
        return s_global.userLocale;
    }

    TkString System::getComputerName()
    {
        return s_global.computerName;
    }

    bool System::openUrl(const TkStringView url)
    {
        StringAsUTF16 str{url.get()};
        ::ShellExecuteW(nullptr, L"open", str.get(), nullptr, nullptr, SW_SHOWNORMAL);
        return true;
    }

    void *System::loadLib(const char *filename)
    {
        TK_ASSERT(filename);
        PROFILE();
#if TK_TRACY_ENABLE
        ZoneText(filename, TkStringUtils::length(filename));
#endif

        // Add folder to search path to load dependency libraries
        TkStringView folder = TkPath::getDir(filename);
        if (folder.hasChars() && TkPath::isRelative(folder))
            folder = TkStringView::Empty;
        if (folder.hasChars())
        {
            const TkString folderNullTerminated(folder);
            StringAsUTF16<MAX_PATH> u16{folderNullTerminated.get()};
            ::AddDllDirectory(u16.get());
        }

        // Avoiding windows dialog boxes if missing
        const DWORD errorMode = SEM_NOOPENFILEERRORBOX;
        DWORD prevErrorMode = 0;
        const BOOL hasErrorMode = ::SetThreadErrorMode(errorMode, &prevErrorMode);

        // Ensure that dll is properly searched
        ::SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS);

        // Load the library
        StringAsUTF16<MAX_PATH> libPth{filename};
        void *handle = ::LoadLibraryW(libPth.get());
        if (!handle)
        {
            LogWarn("Failed to load '%s' (Msg:%s)", filename, ::GetLastError());
        }

        if (hasErrorMode)
        {
            ::SetThreadErrorMode(prevErrorMode, nullptr);
        }

#if TK_CRASH_LOG
        // Refresh modules info during next stack trace collecting to have valid debug symbols information
        ::tk_dbg_lock();
        if (folder.hasChars() && !s_global.symbolsPath.contains(folder))
        {
            s_global.symbolsPath.add(folder);
            s_global.symbolsPath.last().replace('/', '\\');
            onSymbolsPathModified();
        }
        ::tk_dbg_unlock();
#endif

        return handle;
    }

    void System::getEnvVars(TkHashMap<TkString, TkString> &result)
    {
        const LPWCH environmentStr = ::GetEnvironmentStringsW();
        if (environmentStr)
        {
            LPWCH env = environmentStr;
            StringAsUTF8 envOut{env};
            while (*env != '\0')
            {
                if (*env != '=')
                {
                    WCHAR *str = wcschr(env, '=');
                    StringAsUTF8 out{str};
                    TK_ASSERT(str);
                    result[TkString(out.get(), (i32)(out.get() - envOut.get()))] = out.get() + 1;
                }
                while (*env != '\0')
                    env++;
                env++;
            }
            ::FreeEnvironmentStringsW(environmentStr);
        }
    }

    bool System::getEnvVar(const TkStringView name, TkString &value)
    {
        const i32 bufferSize = 512;
        wchar buffer[bufferSize];
        StringAsUTF16 u16Name{name.get()};
        DWORD result = ::GetEnvironmentVariableW(u16Name.get(), buffer, bufferSize);
        if (result == 0)
        {
            // LOG_WIN32_LAST_ERROR;
            return false;
        }
        if (bufferSize < result)
        {
            value.reserve(result);
            wchar *data = TkMemory::newArray<wchar>(result * sizeof(wchar));
            result = ::GetEnvironmentVariableW(u16Name.get(), data, result);
            StringAsUTF8 vv{data};
            value = vv.get();
            TkMemory::deleteArray(data, result * sizeof(wchar));

            if (!result)
            {
                LogError("[Win32] Error : %s", GetLastError());
                return false;
            }
        }
        else
        {
            StringAsUTF8 vv{buffer};
            value.set(vv.get(), result);
        }
        return true;
    }

    bool System::setEnvVar(const TkStringView name, const TkString &value)
    {
        StringAsUTF16 key{name.get()};
        StringAsUTF16 vv{value.get()};
        if (!::SetEnvironmentVariableW(key.get(), vv.get()))
        {
            LogError("[Win32] Error : %s", GetLastError());
            return false;
        }
        return true;
    }

    static bool isProcRunning(HANDLE handle)
    {
        return WaitForSingleObject(handle, 0) == WAIT_TIMEOUT;
    }

    static void readPipe(HANDLE pipe, TkArray<char> &rawData, TkArray<wchar> &logData, LogLevel logType, ProcessInitArgs& args)
    {
        // Check if any data is ready to read
        DWORD bytesAvailable = 0;
        if (PeekNamedPipe(pipe, nullptr, 0, nullptr, &bytesAvailable, nullptr) && bytesAvailable > 0)
        {
            // Read data
            rawData.clear();
            rawData.resize(bytesAvailable);
            DWORD bytesRead = 0;
            if (ReadFile(pipe, rawData.get(), bytesAvailable, &bytesRead, nullptr) && bytesRead > 0)
            {
                // Skip Windows-style lines
                rawData.removeAllKeepOrder('\r');

                // Remove last new line character
                if (rawData.last() == '\n')
                    rawData.removeLast();

                // Log contents
                logData.clear();
                logData.resize(rawData.count() + 1);
                i32 tmp;
                TkConvertor::ansiToUtf16(rawData.get(), logData.get(), rawData.count(), tmp);
                logData.last() = '\0';
                StringAsUTF8 u8 {logData.get()};
                if (args.logOutput)
                    TkLogger::getShared()->log(logType, TkStringView(u8.get(), rawData.count()).get());
                if (args.saveOutput)
                    args.output.add(u8.get(), rawData.count());
            }
        }
    }

    i32 System::newProcess(ProcessInitArgs &args)
    {
        LogInfo("Command: {0} {1}", args.file, args.args);
        if (args.workDir.hasChars())
        {
            LogInfo("Working directory: {0}", args.workDir);
        }
        const bool captureStdOut = args.logOutput || args.saveOutput;

        i32 result = 0;
        if (args.shellExecute)
        {
            SHELLEXECUTEINFOW shExecInfo = {0};
            shExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
            shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
            StringAsUTF16 argsFile{args.file.getText()};
            shExecInfo.lpFile = argsFile.get();
            StringAsUTF16 argsArgs{args.args.get()};
            shExecInfo.lpParameters = args.args.hasChars() ? argsArgs.get() : nullptr;
            StringAsUTF16 argsWorkDir{args.workDir.get()};
            shExecInfo.lpDirectory = args.workDir.hasChars() ? argsWorkDir.get() : nullptr;
            shExecInfo.nShow = args.hiddenWindow ? SW_HIDE : SW_SHOW;
            if (ShellExecuteExW(&shExecInfo) == FALSE)
            {
                result = 1;
                LogWarn("Cannot start process. Error code: 0x{0:x}", (u64)::GetLastError());
            }
            else if (args.waitForQuit)
            {
                WaitForSingleObject(shExecInfo.hProcess, INFINITE);
                DWORD exitCode;
                if (GetExitCodeProcess(shExecInfo.hProcess, &exitCode) != 0)
                    result = exitCode;
                CloseHandle(shExecInfo.hProcess);
            }
        }
        else
        {
            result = -1;
            const TkString cmdLine = args.file + " " + args.args;
            
            StringAsUTF16 u16 {cmdLine.getText()};
            StringAsUTF16 dirPth {args.workDir.get()};

            ::STARTUPINFOEX startupInfoEx;
            ZeroMemory(&startupInfoEx, sizeof(startupInfoEx));
            startupInfoEx.StartupInfo.cb = sizeof(startupInfoEx);
            if (args.hiddenWindow)
            {
                startupInfoEx.StartupInfo.dwFlags |= STARTF_USESHOWWINDOW;
                startupInfoEx.StartupInfo.wShowWindow |= SW_HIDE | SW_SHOWNOACTIVATE;
            }

            DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS | DETACHED_PROCESS;
            if (args.hiddenWindow)
                dwCreationFlags |= CREATE_NO_WINDOW;

            wchar *environmentStr = nullptr;
            if (args.envArgs.hasItems())
            {
                dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;

                i32 totalLength = 1;
                for (auto &e : args.envArgs)
                    totalLength += e.Key.length() + e.Value.length() + 2;

                environmentStr = (wchar *)TkMemory::allocate(TkMemory::getDefault(), totalLength * sizeof(wchar));

                wchar *env = environmentStr;
                for (auto &e : args.envArgs)
                {
                    auto &key = e.Key;
                    auto &value = e.Value;
                    TkMemory::memoryCopy(env, key.get(), key.length() * sizeof(wchar));
                    env += key.length();
                    *env++ = '=';
                    TkMemory::memoryCopy(env, value.get(), value.length() * sizeof(wchar));
                    env += value.length();
                    *env++ = 0;
                }
                *env++ = 0;
                TK_ASSERT((u64)env - (u64)environmentStr == (u64)(totalLength * sizeof(wchar)));
            }

            HANDLE stdOutRead = nullptr;
            HANDLE stdErrRead = nullptr;

            if (captureStdOut)
            {
                dwCreationFlags |= EXTENDED_STARTUPINFO_PRESENT;
                startupInfoEx.StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

                ::SECURITY_ATTRIBUTES securityAttributes;
                securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
                securityAttributes.bInheritHandle = TRUE;
                securityAttributes.lpSecurityDescriptor = nullptr;

                if (!::CreatePipe(&stdOutRead, &startupInfoEx.StartupInfo.hStdOutput, &securityAttributes, 0) ||
                    !::CreatePipe(&stdErrRead, &startupInfoEx.StartupInfo.hStdError, &securityAttributes, 0))
                {
                    LogWarn("CreatePipe failed");
                    return 1;
                }

                SIZE_T bufferSize = 0;
                if (!InitializeProcThreadAttributeList(nullptr, 1, 0, &bufferSize) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                {
                    startupInfoEx.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)TkMemory::allocate(TkMemory::getDefault(), bufferSize);
                    if (!InitializeProcThreadAttributeList(startupInfoEx.lpAttributeList, 1, 0, &bufferSize))
                    {
                        LogWarn("InitializeProcThreadAttributeList failed");
                        goto ERROR_EXIT;
                    }
                }

                HANDLE inheritHandles[2] = {startupInfoEx.StartupInfo.hStdOutput, startupInfoEx.StartupInfo.hStdError};
                if (!UpdateProcThreadAttribute(startupInfoEx.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, inheritHandles, sizeof(inheritHandles), nullptr, nullptr))
                {
                    LogWarn("UpdateProcThreadAttribute failed");
                    goto ERROR_EXIT;
                }
            }

            // Create the process
            ::PROCESS_INFORMATION procInfo;
            if (!CreateProcessW(nullptr, const_cast<LPWSTR>(u16.get()), nullptr, nullptr, TRUE, dwCreationFlags, (LPVOID)environmentStr, args.workDir.hasChars() ? dirPth.get() : nullptr, &startupInfoEx.StartupInfo, &procInfo))
            {
                LogWarn("Cannot start process. Error code: 0x{0:x}", (u64)GetLastError());
                goto ERROR_EXIT;
            }

            if (stdOutRead != nullptr)
            {
                // Keep reading std output and std error streams until process is running
                TkArray<char> rawData;
                TkArray<wchar> logData;
                do
                {
                    readPipe(stdOutRead, rawData, logData, LogLevel::kInfo, args);
                    readPipe(stdErrRead, rawData, logData, LogLevel::kError, args);
                    TkThread::sleep(1);
                } while (isProcRunning(procInfo.hProcess));
                readPipe(stdOutRead, rawData, logData, LogLevel::kInfo, args);
                readPipe(stdErrRead, rawData, logData, LogLevel::kError, args);
            }
            else
            {
                WaitForSingleObject(procInfo.hProcess, INFINITE);
            }

            DWORD exitCode;
            if (GetExitCodeProcess(procInfo.hProcess, &exitCode) != 0)
                result = exitCode;

            CloseHandle(procInfo.hProcess);
            CloseHandle(procInfo.hThread);

            // Cleanup
        ERROR_EXIT:
            if (startupInfoEx.StartupInfo.hStdOutput != nullptr)
                CloseHandle(startupInfoEx.StartupInfo.hStdOutput);
            if (startupInfoEx.StartupInfo.hStdError != nullptr)
                CloseHandle(startupInfoEx.StartupInfo.hStdError);
            if (stdOutRead != nullptr)
                CloseHandle(stdOutRead);
            if (stdErrRead != nullptr)
                CloseHandle(stdErrRead);
            if (startupInfoEx.lpAttributeList != nullptr)
            {
                DeleteProcThreadAttributeList(startupInfoEx.lpAttributeList);
                TkMemory::free(TkMemory::getDefault(), startupInfoEx.lpAttributeList);
            }
            if (environmentStr)
                TkMemory::free(TkMemory::getDefault(), environmentStr);
        }

        return result;
    }

}

#endif