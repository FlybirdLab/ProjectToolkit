/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/common.hpp>

namespace toolkit
{
    // Utility for pooling render target resources with reusing and sharing resources during rendering.
    class TK_API GfxTargetPool
    {
    public:
        // Flushes the temporary render targets.
        static void flush(bool force = false, i32 framesOffset = -1);
        // Gets a temporary render target.
        static GfxTexture *get(const class GfxTextureDescription &desc);
        // Releases a temporary render target.
        static void release(GfxTexture *rt);
    };

// Utility to set name to the pooled render target (compiled-put in Release builds)
#if GFX_ENABLE_RESOURCE_NAMING
#define GFX_TARGET_POOL_SET_NAME(rt, name) rt->SetName(TEXT(name));
#else
#define GFX_TARGET_POOL_SET_NAME(rt, name)
#endif

}