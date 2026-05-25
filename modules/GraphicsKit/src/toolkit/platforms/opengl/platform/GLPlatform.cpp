/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GLPlatform.hpp"

#if GFX_API_OPENGL

#include <toolkit/platforms/opengl/GLNative.hpp>
#include "toolkit/debug/Logger.hpp"

namespace toolkit
{

    namespace
    {
        bool g_gladLoaded = false;
    }

    // Per-platform implementations live in platform/<os>/ and provide the
    // concrete bodies of the static GLPlatform methods. The dispatching here
    // is purely the glad-loader plumbing that is identical on every platform.

    bool GLPlatform::loadGLFunctions()
    {
        if (g_gladLoaded)
            return true;
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(&GLPlatform::getProcAddress)))
        {
            LogError("GLPlatform::loadGLFunctions: gladLoadGLLoader failed");
            return false;
        }
        g_gladLoaded = true;
        return true;
    }

}

#endif
