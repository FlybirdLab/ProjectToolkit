/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/utils/PixelFormat.hpp>
#include <toolkit/math/Vector4.hpp>

namespace toolkit
{
    // Utility for writing and reading from different pixels formats within a single code path.
    struct TK_API PixelFormatSampler
    {
        typedef FVec4 (*ReadPixel)(const void *data);
        typedef void (*WritePixel)(void *data, const FVec4 &value);

    public:
        // Element format.
        PixelFormat Format;
        // Element size in bytes.
        i32 PixelSize;
        // Read data function.
        ReadPixel Read;
        // Write data function.
        WritePixel Write;

    public:
        // Stores the color into the specified texture data (uses no interpolation).
        void store(void *data, i32 x, i32 y, i32 rowPitch, const Color &color) const;

        // Samples the specified linear data (uses no interpolation).
        FVec4 sample(const void *data, i32 x) const;

        // Samples the specified texture data (uses no interpolation).
        Color samplePoint(const void *data, const FVec2 &uv, const IVec2 &size, i32 rowPitch) const;
        // Samples the specified texture data (uses no interpolation).
        Color samplePoint(const void *data, i32 x, i32 y, i32 rowPitch) const;

        // Samples the specified texture data (uses linear interpolation).
        Color sampleLinear(const void *data, const FVec2 &uv, const IVec2 &size, i32 rowPitch) const;

    public:
        // Tries to get a sampler tool for the specified format to read pixels.
        static const PixelFormatSampler *get(PixelFormat format);
    };

}