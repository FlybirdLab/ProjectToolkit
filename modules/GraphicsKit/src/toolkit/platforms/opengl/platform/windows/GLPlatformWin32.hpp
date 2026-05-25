/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/opengl/platform/GLPlatform.hpp>

#if GFX_API_OPENGL && TK_WINDOWS

#include <toolkit/platforms/opengl/GLNative.hpp>

namespace toolkit
{

    // Win32 WGL helpers. Not exposed to other backends -- callers go through
    // the GLPlatform static API.
    namespace GLPlatformWin32
    {
        bool init();
        void shutdown();

        bool createDeviceContext(GLPlatformContext &outContext);
        bool createWindowContext(HWND hwnd, GLPlatformContext &outContext, GLPlatformContext *sharedContext);
        void destroyContext(GLPlatformContext &context);

        bool makeCurrent(GLPlatformContext *context);
        void clearCurrent();
        void swapBuffers(GLPlatformContext &context);
        void setSwapInterval(bool vsync);

        void *getProcAddress(const char *name);
        bool isCurrent(GLPlatformContext *context);
    }

}

#endif
