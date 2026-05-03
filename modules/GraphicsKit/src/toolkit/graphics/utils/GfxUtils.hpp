/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "PixelFormat.hpp"
#include <toolkit/graphics/common.hpp>

namespace toolkit
{

    class GfxUtils
    {
    public:
        // Calculate a subresource index for a texture
        TK_FORCE_INLINE static i32 calcSubresourceIndex(u32 mipSlice, i32 arraySlice, i32 mipLevels)
        {
            return mipSlice + arraySlice * mipLevels;
        }

        // Computes the image row pitch in bytes, and the slice pitch (size in bytes of the image) based on format, width, and height.
        static void computePitch(PixelFormat format, i32 width, i32 height, u32 &rowPitch, u32 &slicePitch);
        // Computes the feature level for the given shader profile.
        static GfxFeatureLevel getFeatureLevel(GfxShaderProfile profile);

        static u64 calculateTextureMemoryUsage(PixelFormat format, i32 width, i32 height, i32 mipLevels);
        static u64 calculateTextureMemoryUsage(PixelFormat format, i32 width, i32 height, i32 depth, i32 mipLevels);
    };
}