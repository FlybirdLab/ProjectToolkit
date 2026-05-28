/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/base/BaseSystem.hpp>

#if TK_POSIX

#include <toolkit/types/String.hpp>
#include <toolkit/types/UUID.hpp>

namespace toolkit
{
    struct UnixSystemGlobals
    {
        TkUuid deviceId;
        CPUStats cpuStats;
        u64 programSizeMem = 0;
        u64 clockFreq = 1000000000ull;
        f64 cycles2Seconds = 1.0 / 1000000000.0;
        TkString tempDir;
        i32 processMutex = -1;
    };

    class UnixSystem : public System
    {
    public:
        static TkString getLastErrMsg();
    };
}

#endif
