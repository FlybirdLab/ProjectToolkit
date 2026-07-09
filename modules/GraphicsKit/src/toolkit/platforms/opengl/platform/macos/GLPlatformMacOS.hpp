/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/opengl/platform/GLPlatform.hpp>

#if GFX_API_OPENGL && TK_APPLE

namespace toolkit
{

    namespace GLPlatformMacOS
    {
        bool init();
        void shutdown();

        bool createDeviceContext(GLPlatformContext &outContext);
        bool createWindowContext(void *nsWindow, GLPlatformContext &outContext, GLPlatformContext *sharedContext);
        void destroyContext(GLPlatformContext &context);

        bool makeCurrent(GLPlatformContext *context);
        void clearCurrent();
        void swapBuffers(GLPlatformContext &context);
        void setSwapInterval(bool vsync);
        void updateContext(GLPlatformContext &context);

        void *getProcAddress(const char *name);
        bool isCurrent(GLPlatformContext *context);
    }

}

#endif
