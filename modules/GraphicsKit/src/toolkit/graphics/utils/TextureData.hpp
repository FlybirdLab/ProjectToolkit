/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "PixelFormat.hpp"
#include "toolkit/collection/DataCollection.hpp"
#include "toolkit/collection/StaticArray.hpp"
#include "toolkit/utils/Color32.hpp"
#include <toolkit/graphics/common.hpp>

namespace toolkit
{

    // Single texture mip map entry data.
    class TK_API TextureMipData
    {
    public:
        TextureMipData();
        TextureMipData(const TextureMipData &other);
        TextureMipData(TextureMipData &&other) noexcept;
        TextureMipData &operator=(const TextureMipData &other);
        TextureMipData &operator=(TextureMipData &&other) noexcept;

        bool GetPixels(TkArray<Color32> &pixels, i32 width, i32 height, PixelFormat format) const;
        bool GetPixels(TkArray<Color> &pixels, i32 width, i32 height, PixelFormat format) const;
        void Copy(void *data, u32 dataRowPitch, u32 dataDepthPitch, u32 dataDepthSlices, u32 targetRowPitch);

        template <typename T>
        T &Get(i32 x, i32 y)
        {
            return *(T *)(Data.get() + y * RowPitch + x * sizeof(T));
        }

        template <typename T>
        const T &Get(i32 x, i32 y) const
        {
            return *(const T *)(Data.get() + y * RowPitch + x * sizeof(T));
        }

    public:
        u32 RowPitch;
        u32 DepthPitch;
        u32 Lines;
        ByteCollection Data;
    };

    // Texture data container (used to keep data downloaded from the GPU)
    class TK_API TextureData
    {
    public:
        // Single entry of the texture array. Contains collection of mip maps.
        struct TK_API ArrayEntry
        {
            // The mip maps collection.
            TkStaticArray<TextureMipData, GFX_MAX_TEXTURE_MIP_LEVELS> mips;
        };

    public:
        // Gather texture data
        TextureMipData *getData(i32 arrayIndex, i32 mipLevel)
        {
            return &items[arrayIndex].mips[mipLevel];
        }

        // Gather texture data
        const TextureMipData *getData(i32 arrayIndex, i32 mipLevel) const
        {
            return &items[arrayIndex].mips[mipLevel];
        }

        // Get amount of texture slices in the array.
        i32 getArraySize() const;
        // Get amount of mip maps in the texture.
        i32 getMipLevels() const;

        // Clear allocated memory.
        void clear();

    public:
        // Get the texture pixels as Color32 array.
        bool getPixels(TkArray<Color32> &pixels, i32 mipIndex = 0, i32 arrayIndex = 0);
        // Get the texture pixels as Color array.
        bool getPixels(TkArray<Color> &pixels, i32 mipIndex = 0, i32 arrayIndex = 0);

    public:
        // Top level texture surface width (in pixels).
        i32 width = 0;
        // Top level texture surface height (in pixels).
        i32 height = 0;
        // Top level texture surface depth (in pixels).
        i32 depth = 0;

        // The texture data format.
        PixelFormat format = PixelFormat::kUnknown;
        // The items collection (depth slices or array slices).
        TkStaticArray<ArrayEntry, 6> items;
    };

}