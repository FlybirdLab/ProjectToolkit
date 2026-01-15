// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.
// Minimal Windows.h declaration.

#pragma once

#include <cstdarg>

extern "C" {

typedef void VOID;
typedef int BOOL;
typedef char CHAR;
typedef short SHORT;
typedef int INT;
typedef long LONG;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned long ULONG;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned __int64 ULONG_PTR;
typedef __int64 LONG_PTR;
typedef __int64 LONGLONG;
typedef ULONG_PTR SIZE_T;
typedef LONG HRESULT;
typedef BYTE BOOLEAN;
typedef ULONG_PTR DWORD_PTR;
typedef DWORD *LPDWORD;
typedef void *HANDLE;
typedef void *PVOID;
typedef void *LPVOID;
typedef const CHAR *LPCSTR, *PCSTR;
typedef const CHAR *PCZZSTR;
typedef CHAR *LPSTR;
typedef struct HINSTANCE__ *HINSTANCE;
typedef HINSTANCE HMODULE;
typedef wchar_t WCHAR;
typedef const WCHAR *PCWSTR;
typedef WCHAR *NWPSTR, *LPWSTR, *PWSTR;
typedef const void *LPCVOID;
typedef HANDLE HLOCAL;
struct HWND__;
typedef struct HWND__ *HWND;

typedef struct _SECURITY_ATTRIBUTES SECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;
typedef DWORD(__stdcall *PTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;
typedef struct _RTL_CRITICAL_SECTION RTL_CRITICAL_SECTION, CRITICAL_SECTION, *LPCRITICAL_SECTION, *PCRITICAL_SECTION;
typedef struct _RTL_SRWLOCK RTL_SRWLOCK, *PSRWLOCK;
typedef struct _RTL_CONDITION_VARIABLE RTL_CONDITION_VARIABLE, *PCONDITION_VARIABLE;

typedef struct _SHFILEOPSTRUCTA *LPSHFILEOPSTRUCTA;
typedef struct _WIN32_FIND_DATAA WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;

typedef union _LARGE_INTEGER LARGE_INTEGER;

typedef struct _CONSOLE_SCREEN_BUFFER_INFO CONSOLE_SCREEN_BUFFER_INFO, *PCONSOLE_SCREEN_BUFFER_INFO;

typedef struct _SYSTEM_INFO SYSTEM_INFO, *LPSYSTEM_INFO;

__declspec(dllimport) HANDLE __stdcall
CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress,
             LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
__declspec(dllimport) DWORD_PTR __stdcall SetThreadAffinityMask(HANDLE hThread, DWORD_PTR dwThreadAffinityMask);
__declspec(dllimport) DWORD __stdcall WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
__declspec(dllimport) BOOL __stdcall CloseHandle(HANDLE hObject);
__declspec(dllimport) DWORD __stdcall GetCurrentThreadId(VOID);
__declspec(dllimport) HRESULT __stdcall SetThreadDescription(HANDLE hThread, PCWSTR lpThreadDescription);
__declspec(dllimport) HRESULT __stdcall GetThreadDescription(HANDLE hThread, PWSTR *ppszThreadDescription);
__declspec(dllimport) HANDLE __stdcall GetCurrentThread();

__declspec(dllimport) VOID __stdcall InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
__declspec(dllimport) VOID __stdcall EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
__declspec(dllimport) BOOL __stdcall TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
__declspec(dllimport) VOID __stdcall LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
__declspec(dllimport) VOID __stdcall DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

__declspec(dllimport) VOID __stdcall InitializeSRWLock(PSRWLOCK SRWLock);
__declspec(dllimport) VOID __stdcall AcquireSRWLockExclusive(PSRWLOCK SRWLock);
__declspec(dllimport) VOID __stdcall AcquireSRWLockShared(PSRWLOCK SRWLock);
__declspec(dllimport) VOID __stdcall ReleaseSRWLockExclusive(PSRWLOCK SRWLock);
__declspec(dllimport) VOID __stdcall ReleaseSRWLockShared(PSRWLOCK SRWLock);
__declspec(dllimport) BOOLEAN __stdcall TryAcquireSRWLockExclusive(PSRWLOCK SRWLock);
__declspec(dllimport) BOOLEAN __stdcall TryAcquireSRWLockShared(PSRWLOCK SRWLock);

__declspec(dllimport) VOID __stdcall InitializeConditionVariable(PCONDITION_VARIABLE ConditionVariable);
__declspec(dllimport) BOOL __stdcall
SleepConditionVariableCS(PCONDITION_VARIABLE ConditionVariable, PCRITICAL_SECTION CriticalSection,
                         DWORD dwMilliseconds);
__declspec(dllimport) VOID __stdcall WakeAllConditionVariable(PCONDITION_VARIABLE ConditionVariable);
__declspec(dllimport) VOID __stdcall WakeConditionVariable(PCONDITION_VARIABLE ConditionVariable);

__declspec(dllimport) DWORD __stdcall GetFileAttributesA(LPCSTR lpFileName);
__declspec(dllimport) int __stdcall SHFileOperationA(LPSHFILEOPSTRUCTA lpFileOp);
__declspec(dllimport) BOOL __stdcall CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
__declspec(dllimport) HRESULT __stdcall
SHGetFolderPathA(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath);
__declspec(dllimport) HANDLE __stdcall FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);
__declspec(dllimport) BOOL __stdcall FindClose(HANDLE hFindFile);
__declspec(dllimport) BOOL __stdcall FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData);
__declspec(dllimport) DWORD __stdcall GetTempPathA(DWORD nBufferLength, LPSTR lpBuffer);

// Other
__declspec(dllimport) DWORD __stdcall GetLastError(VOID);
__declspec(dllimport) BOOL __stdcall QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency);
__declspec(dllimport) BOOL __stdcall QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount);
__declspec(dllimport) VOID __stdcall Sleep(DWORD dwMilliseconds);
__declspec(dllimport) HANDLE __stdcall GetStdHandle(DWORD nStdHandle);
__declspec(dllimport) BOOL __stdcall
GetConsoleScreenBufferInfo(HANDLE hConsoleOutput, PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo);
__declspec(dllimport) BOOL __stdcall SetConsoleTextAttribute(HANDLE hConsoleOutput, WORD wAttributes);
__declspec(dllimport) VOID __stdcall GetSystemInfo(LPSYSTEM_INFO lpSystemInfo);
__declspec(dllimport) DWORD __stdcall GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
__declspec(dllimport) int __stdcall MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
__declspec(dllimport) DWORD __stdcall
FormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize,
               va_list *Arguments);
__declspec(dllimport) HLOCAL __stdcall LocalFree(HLOCAL hMem);
__declspec(dllimport) BOOL __stdcall IsDebuggerPresent();
__declspec(dllimport) BOOL __stdcall SetProcessDPIAware();

}

namespace toolkit {

    constexpr DWORD INVALID_FILE_ATTRIBUTES = (DWORD)-1;
    constexpr DWORD FILE_ATTRIBUTE_DIRECTORY = 0x00000010;
    constexpr DWORD MAX_PATH = 260;
    static const HANDLE INVALID_HANDLE_VALUE = (HANDLE)(LONG_PTR)-1;
    constexpr DWORD ERROR_NO_MORE_FILES = 18L;
    constexpr WORD FO_DELETE = 0x0003;
    constexpr WORD FOF_NOCONFIRMATION = 0x0010;
    constexpr WORD FOF_NOERRORUI = 0x0400;
    constexpr WORD FOF_SILENT = 0x0004;
    constexpr WORD CSIDL_PROFILE = 0x0028;
    constexpr DWORD STD_OUTPUT_HANDLE = (DWORD)-11;
    constexpr HRESULT S_OK = 0;
    constexpr DWORD INFINITE = 0xFFFFFFFF;
    constexpr DWORD ERROR_INSUFFICIENT_BUFFER = 122l;
    constexpr DWORD ERROR_SUCCESS = 0;
    constexpr WORD FOREGROUND_BLUE = 0x0001;
    constexpr WORD FOREGROUND_GREEN = 0x0002;
    constexpr WORD FOREGROUND_RED = 0x0004;
    constexpr WORD FOREGROUND_INTENSITY = 0x0008;
    constexpr WORD BACKGROUND_BLUE = 0x0010;
    constexpr WORD BACKGROUND_GREEN = 0x0020;
    constexpr WORD BACKGROUND_RED = 0x0040;

    constexpr WORD MB_OK = 0x00000000L;
    constexpr WORD MB_ICONWARNING = 0x00000030L;
    constexpr WORD MB_ICONERROR = 0x00000010L;

    constexpr DWORD FORMAT_MESSAGE_ALLOCATE_BUFFER = 0x00000100;
    constexpr DWORD FORMAT_MESSAGE_IGNORE_INSERTS = 0x00000200;
    constexpr DWORD FORMAT_MESSAGE_FROM_STRING = 0x00000400;
    constexpr DWORD FORMAT_MESSAGE_FROM_HMODULE = 0x00000800;
    constexpr DWORD FORMAT_MESSAGE_FROM_SYSTEM = 0x00001000;
    constexpr DWORD FORMAT_MESSAGE_ARGUMENT_ARRAY = 0x00002000;
    constexpr DWORD FORMAT_MESSAGE_MAX_WIDTH_MASK = 0x000000FF;
    constexpr DWORD LANG_NEUTRAL = 0x00;
    constexpr DWORD SUBLANG_DEFAULT = 0x01;

// Types
    typedef union _LARGE_INTEGER
    {
        struct
        {
            ULONG LowPart;
            LONG HighPart;
        } DUMMYSTRUCTNAME;
        struct
        {
            ULONG LowPart;
            LONG HighPart;
        } u;
        LONGLONG QuadPart;
    } LARGE_INTEGER;

    typedef struct _SECURITY_ATTRIBUTES
    {
        DWORD nLength;
        LPVOID lpSecurityDescriptor;
        BOOL bInheritHandle;
    } SECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

    typedef struct _LIST_ENTRY
    {
        struct _LIST_ENTRY* Flink;
        struct _LIST_ENTRY* Blink;
    } LIST_ENTRY, *PLIST_ENTRY;

    typedef struct _RTL_CRITICAL_SECTION_DEBUG
    {
        WORD Type;
        WORD CreatorBackTraceIndex;
        struct _RTL_CRITICAL_SECTION* CriticalSection;
        LIST_ENTRY ProcessLocksList;
        DWORD EntryCount;
        DWORD ContentionCount;
        DWORD Flags;
        WORD CreatorBackTraceIndexHigh;
        WORD SpareWORD;
    } RTL_CRITICAL_SECTION_DEBUG, *PRTL_CRITICAL_SECTION_DEBUG;

#pragma pack(push, 8)
    typedef struct _RTL_CRITICAL_SECTION
    {
        PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
        LONG LockCount;
        LONG RecursionCount;
        HANDLE OwningThread;
        HANDLE LockSemaphore;
        ULONG_PTR SpinCount;
    } CRITICAL_SECTION, *LPCRITICAL_SECTION, *PCRITICAL_SECTION;
#pragma pack(pop)

    typedef struct _RTL_SRWLOCK
    {
        PVOID Ptr;
    } SRWLOCK, *PSRWLOCK;

    typedef struct _RTL_CONDITION_VARIABLE
    {
        PVOID Ptr;
    } CONDITION_VARIABLE, *PRTL_CONDITION_VARIABLE, *PCONDITION_VARIABLE;

    typedef WORD FILEOP_FLAGS;

    typedef struct _SHFILEOPSTRUCTA
    {
        HWND hwnd;
        UINT wFunc;
        PCZZSTR pFrom;
        PCZZSTR pTo;
        FILEOP_FLAGS fFlags;
        BOOL fAnyOperationsAborted;
        LPVOID hNameMappings;
        PCSTR lpszProgressTitle;
    } SHFILEOPSTRUCTA, *LPSHFILEOPSTRUCTA;

    typedef struct _FILETIME
    {
        DWORD dwLowDateTime;
        DWORD dwHighDateTime;
    } FILETIME, *PFILETIME, *LPFILETIME;

    typedef struct _WIN32_FIND_DATAA
    {
        DWORD dwFileAttributes;
        FILETIME ftCreationTime;
        FILETIME ftLastAccessTime;
        FILETIME ftLastWriteTime;
        DWORD nFileSizeHigh;
        DWORD nFileSizeLow;
        DWORD dwReserved0;
        DWORD dwReserved1;
        CHAR cFileName[260];
        CHAR cAlternateFileName[14];
    } WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;

    typedef struct _COORD
    {
        SHORT X;
        SHORT Y;
    } COORD, *PCOORD;

    typedef struct _SMALL_RECT
    {
        SHORT Left;
        SHORT Top;
        SHORT Right;
        SHORT Bottom;
    } SMALL_RECT, *PSMALL_RECT;

    typedef struct _CONSOLE_SCREEN_BUFFER_INFO
    {
        COORD dwSize;
        COORD dwCursorPosition;
        WORD wAttributes;
        SMALL_RECT srWindow;
        COORD dwMaximumWindowSize;
    } CONSOLE_SCREEN_BUFFER_INFO, *PCONSOLE_SCREEN_BUFFER_INFO;

    typedef struct _SYSTEM_INFO
    {
        union
        {
            DWORD dwOemId; // Obsolete field...do not use
            struct
            {
                WORD wProcessorArchitecture;
                WORD wReserved;
            } DUMMYSTRUCTNAME;
        } DUMMYUNIONNAME;
        DWORD dwPageSize;
        LPVOID lpMinimumApplicationAddress;
        LPVOID lpMaximumApplicationAddress;
        DWORD_PTR dwActiveProcessorMask;
        DWORD dwNumberOfProcessors;
        DWORD dwProcessorType;
        DWORD dwAllocationGranularity;
        WORD wProcessorLevel;
        WORD wProcessorRevision;
    } SYSTEM_INFO, *LPSYSTEM_INFO;

// Critical section
    inline void InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
    {
        ::InitializeCriticalSection(reinterpret_cast<::LPCRITICAL_SECTION>(lpCriticalSection));
    }

    inline void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
    {
        ::EnterCriticalSection(reinterpret_cast<::LPCRITICAL_SECTION>(lpCriticalSection));
    }

    inline BOOL TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
    {
        return ::TryEnterCriticalSection(reinterpret_cast<::LPCRITICAL_SECTION>(lpCriticalSection));
    }

    inline void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
    {
        ::LeaveCriticalSection(reinterpret_cast<::LPCRITICAL_SECTION>(lpCriticalSection));
    }

    inline void DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
    {
        ::DeleteCriticalSection(reinterpret_cast<::LPCRITICAL_SECTION>(lpCriticalSection));
    }

// Shared lock
    inline void InitializeSRWLock(PSRWLOCK SRWLock)
    {
        ::InitializeSRWLock(reinterpret_cast<::PSRWLOCK>(SRWLock));
    }

    inline void AcquireSRWLockExclusive(PSRWLOCK SRWLock)
    {
        ::AcquireSRWLockExclusive(reinterpret_cast<::PSRWLOCK>(SRWLock));
    }

    inline void AcquireSRWLockShared(PSRWLOCK SRWLock)
    {
        ::AcquireSRWLockShared(reinterpret_cast<::PSRWLOCK>(SRWLock));
    }

    inline void ReleaseSRWLockExclusive(PSRWLOCK SRWLock)
    {
        ::ReleaseSRWLockExclusive(reinterpret_cast<::PSRWLOCK>(SRWLock));
    }

    inline void ReleaseSRWLockShared(PSRWLOCK SRWLock)
    {
        ::ReleaseSRWLockShared(reinterpret_cast<::PSRWLOCK>(SRWLock));
    }

    inline BOOL TryAcquireSRWLockExclusive(PSRWLOCK SRWLock)
    {
        return ::TryAcquireSRWLockExclusive(reinterpret_cast<::PSRWLOCK>(SRWLock));
    }

    inline BOOL TryAcquireSRWLockShared(PSRWLOCK SRWLock)
    {
        return ::TryAcquireSRWLockShared(reinterpret_cast<::PSRWLOCK>(SRWLock));
    }

// Condition var
    inline void InitializeConditionVariable(PCONDITION_VARIABLE ConditionVariable)
    {
        ::InitializeConditionVariable(reinterpret_cast<::PCONDITION_VARIABLE>(ConditionVariable));
    }

    inline BOOL SleepConditionVariableCS(PCONDITION_VARIABLE ConditionVariable, PCRITICAL_SECTION CriticalSection, DWORD dwMilliseconds)
    {
        return ::SleepConditionVariableCS(reinterpret_cast<::PCONDITION_VARIABLE>(ConditionVariable),
                                          reinterpret_cast<::PCRITICAL_SECTION>(CriticalSection), dwMilliseconds);
    }

    inline void WakeAllConditionVariable(PCONDITION_VARIABLE ConditionVariable)
    {
        ::WakeAllConditionVariable(reinterpret_cast<::PCONDITION_VARIABLE>(ConditionVariable));
    }

    inline void WakeConditionVariable(PCONDITION_VARIABLE ConditionVariable)
    {
        ::WakeConditionVariable(reinterpret_cast<::PCONDITION_VARIABLE>(ConditionVariable));
    }

// Filesystem
    inline DWORD GetFileAttributesA(LPCSTR lpFileName)
    {
        return ::GetFileAttributesA(lpFileName);
    }

    inline int SHFileOperationA(LPSHFILEOPSTRUCTA lpFileOp)
    {
        return ::SHFileOperationA(reinterpret_cast<::LPSHFILEOPSTRUCTA>(lpFileOp));
    }

    inline BOOL CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
    {
        return ::CreateDirectoryA(lpPathName, reinterpret_cast<::LPSECURITY_ATTRIBUTES>(lpSecurityAttributes));
    }

    inline HRESULT SHGetFolderPathA(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath)
    {
        return ::SHGetFolderPathA(hwnd, csidl, hToken, dwFlags, pszPath);
    }

    inline HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
    {
        return ::FindFirstFileA(lpFileName, reinterpret_cast<::LPWIN32_FIND_DATAA>(lpFindFileData));
    }

    inline BOOL FindClose(HANDLE hFindFile)
    {
        return ::FindClose(hFindFile);
    }

    inline BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
    {
        return ::FindNextFileA(hFindFile, reinterpret_cast<::LPWIN32_FIND_DATAA>(lpFindFileData));
    }

    inline DWORD GetTempPathA(DWORD nBufferLength, LPSTR lpBuffer)
    {
        return ::GetTempPathA(nBufferLength, lpBuffer);
    }

// Other
    inline BOOL QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency)
    {
        return ::QueryPerformanceFrequency(reinterpret_cast<::LARGE_INTEGER*>(lpFrequency));
    }

    inline BOOL QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
    {
        return ::QueryPerformanceCounter(reinterpret_cast<::LARGE_INTEGER*>(lpPerformanceCount));
    }

    inline BOOL GetConsoleScreenBufferInfo(HANDLE hConsoleOutput, PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo)
    {
        return ::GetConsoleScreenBufferInfo(hConsoleOutput, reinterpret_cast<::PCONSOLE_SCREEN_BUFFER_INFO>(lpConsoleScreenBufferInfo));
    }

    inline VOID GetSystemInfo(LPSYSTEM_INFO lpSystemInfo)
    {
        ::GetSystemInfo(reinterpret_cast<::LPSYSTEM_INFO>(lpSystemInfo));
    }

    inline DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
    {
        return ::GetModuleFileNameA(hModule, lpFilename, nSize);
    }

    inline int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
    {
        return ::MessageBoxA(hWnd, lpText, lpCaption, uType);
    }

    inline DWORD FormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list* Arguments)
    {
        return ::FormatMessageA(dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, Arguments);
    }

    inline HLOCAL LocalFree(HLOCAL hMem)
    {
        return ::LocalFree(hMem);
    }

    inline BOOL IsDebuggerPresent()
    {
        return ::IsDebuggerPresent();
    }

    inline BOOL SetProcessDPIAware()
    {
        return ::SetProcessDPIAware();
    }
}