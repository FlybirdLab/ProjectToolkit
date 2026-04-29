/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxTexture.hpp>

#if GFX_API_NULL

namespace toolkit
{
    // Texture object for Null backend.
    class NullTexture : public GfxTexture
    {
    public:
        GfxTextureView *view(i32 arrayOrDepthIndex) const override
        {
            return nullptr;
        }
        GfxTextureView *view(i32 arrayOrDepthIndex, i32 mipMapIndex) const override
        {
            return nullptr;
        }
        GfxTextureView *viewArray() const override
        {
            return nullptr;
        }
        GfxTextureView *viewVolume() const override
        {
            return nullptr;
        }
        GfxTextureView *viewReadOnlyDepth() const override
        {
            return nullptr;
        }
        void *getNativePtr() const override
        {
            return nullptr;
        }
        bool getData(i32 arrayIndex, i32 mipMapIndex, TextureMipData &data, u32 mipRowPitch) override
        {
            return false;
        }

    protected:
        bool onInit() override
        {
            return false;
        }
        void onResidentMipsChange() override
        {
        }
    };

}

#endif