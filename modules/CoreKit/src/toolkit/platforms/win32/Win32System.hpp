/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/base/BaseSystem.hpp>

#if TK_WINDOWS

#include "toolkit/types/UUID.hpp"

#include <toolkit/types/String.hpp>
#include <toolkit/collection/Array.hpp>

#define UNICODE
#define _UNICODE

#define NOMINMAX
#ifdef MAX_PATH
#undef MAX_PATH
#endif
#include "Win32.hpp"
#include <WinSock2.h>

namespace toolkit
{

    struct SystemGlobals
    {
        TkUuid deviceId;
        CPUStats cpuStats;
        u64 programSizeMem;
        u64 clockFreq;
        f64 cycles2Seconds;
        WSAData wsaData;

        TkString userLocale, computerName, windowsName;
        HANDLE processMutex = nullptr;
        i32 dpi = 96;
        struct
        {
            i32 major = 0;
            i32 minor = 0;
            i32 build = 0;
        } version;
#if TK_WINDOW_KIT
        RECT virtualScreenBounds{0.0f, 0.0f, 0.0f, 0.0f};
#endif
#if TK_CRASH_LOG
#if TK_TRACY_ENABLE
        bool isSymInit = true;
#else
        bool isSymInit = false;
#endif
        TkArray<TkString> symbolsPath;
#endif
    };

    // Windows specific api
    class Win32System : public System
    {
    public:
        static bool readRegVal(void *root, const TkStringView key, const TkStringView name, TkString *result);
        static bool isWindows10();
    };

}

#endif