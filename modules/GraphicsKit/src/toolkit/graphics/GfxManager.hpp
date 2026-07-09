/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/threading/Sync.hpp"
#include <toolkit/graphics/common.hpp>

namespace toolkit
{
    // GfxManager device manager that creates, manages and releases graphics device and related objects.
    class TK_API GfxManager
    {
    public:
        static bool useVSync;

    public:
        static TkMutex sharedMutex;

    public:
        static GfxDevice *getDevice();
        static bool createDevice(GfxBackendType backend);
        static void disposeDevice();

    public:
        static bool init();
        static void cleanup();
        static void dispose();
    };

}