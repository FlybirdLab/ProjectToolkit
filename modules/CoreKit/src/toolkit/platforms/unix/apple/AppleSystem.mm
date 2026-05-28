/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/platforms/unix/UnixSystem.hpp>

#if TK_APPLE

#include <crt_externs.h>
#include <mach-o/dyld.h>

namespace toolkit::apple
{
    char **getEnvironment()
    {
        return *_NSGetEnviron();
    }

    bool getExecutablePath(char *buffer, u32 bufferSize)
    {
        u32 size = bufferSize;
        return _NSGetExecutablePath(buffer, &size) == 0;
    }
}

#endif
