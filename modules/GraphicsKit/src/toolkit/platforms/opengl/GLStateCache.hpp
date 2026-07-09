/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/platforms/opengl/common.hpp>

#if GFX_API_OPENGL || GFX_API_OPENGLES

namespace toolkit
{

    // Phase-1 placeholder. The real state cache (bound program / VAO / blend /
    // depth-stencil / rasterizer state) is implemented in Phase 2 once draw
    // submission is wired up.
    class GLStateCache
    {
    public:
        GLStateCache() = default;
        ~GLStateCache() = default;

        void reset() {}
    };

}

#endif
