/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "TextureData.hpp"

#include "toolkit/collection/Array.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/utils/Color.hpp"
#include "PixelFormatSampler.hpp"

namespace toolkit
{

    TextureMipData::TextureMipData()
        : rowPitch(0), depthPitch(0), lines(0)
    {
    }

    TextureMipData::TextureMipData(const TextureMipData &other)
        : rowPitch(other.rowPitch), depthPitch(other.depthPitch), lines(other.lines), data(other.data)
    {
    }

    TextureMipData::TextureMipData(TextureMipData &&other) noexcept
        : rowPitch(other.rowPitch), depthPitch(other.depthPitch), lines(other.lines), data(moveTemp(other.data))
    {
    }

    TextureMipData &TextureMipData::operator=(const TextureMipData &other)
    {
        if (this == &other)
            return *this;
        rowPitch = other.rowPitch;
        depthPitch = other.depthPitch;
        lines = other.lines;
        data = other.data;
        return *this;
    }

    TextureMipData &TextureMipData::operator=(TextureMipData &&other) noexcept
    {
        if (this == &other)
            return *this;
        rowPitch = other.rowPitch;
        depthPitch = other.depthPitch;
        lines = other.lines;
        data = moveTemp(other.data);
        return *this;
    }

    bool TextureMipData::getPixels(TkArray<Color32> &pixels, i32 width, i32 height, PixelFormat format) const
    {
        const i32 size = width * height;
        if (data.isInvalid() || size < 1)
            return false;
        pixels.resize(size);
        byte *dst = (byte *)pixels.get();
        const i32 dstRowSize = width * sizeof(Color32);
        const byte *src = data.get();
        const i32 srcRowSize = rowPitch;
        switch (format)
        {
        case PixelFormat::kR8G8B8A8_SInt:
        case PixelFormat::kR8G8B8A8_Typeless:
        case PixelFormat::kR8G8B8A8_SNorm:
        case PixelFormat::kR8G8B8A8_UInt:
        case PixelFormat::kR8G8B8A8_UNorm:
        case PixelFormat::kR8G8B8A8_UNorm_sRGB:
        case PixelFormat::kR8G8_B8G8_UNorm:
        case PixelFormat::kB8G8R8A8_Typeless:
        case PixelFormat::kB8G8R8A8_UNorm:
        case PixelFormat::kB8G8R8A8_UNorm_sRGB:
        case PixelFormat::kB8G8R8X8_Typeless:
        case PixelFormat::kB8G8R8X8_UNorm:
        case PixelFormat::kB8G8R8X8_UNorm_sRGB:
            if (srcRowSize == dstRowSize)
                TkMemory::memoryCopy(dst, src, rowPitch * lines);
            else
            {
                for (u32 row = 0; row < lines; row++)
                {
                    TkMemory::memoryCopy(dst, src, dstRowSize);
                    dst += dstRowSize;
                    src += srcRowSize;
                }
            }
            break;
        default:
        {
            // Try to use texture sampler utility
            auto sampler = PixelFormatSampler::get(format);
            if (sampler)
            {
                for (i32 y = 0; y < height; y++)
                {
                    for (i32 x = 0; x < width; x++)
                    {
                        Color c = sampler->samplePoint(src, x, y, rowPitch);
                        *(Color32 *)(dst + dstRowSize * y + x * sizeof(Color32)) = Color32(c);
                    }
                }
                return true;
            }
            LogError("Unsupported texture data format %i.", format);
            return false;
        }
        }
        return true;
    }

    bool TextureMipData::getPixels(TkArray<Color> &pixels, i32 width, i32 height, PixelFormat format) const
    {
        const i32 size = width * height;
        if (data.isInvalid() || size < 1)
            return false;
        pixels.resize(size);
        byte *dst = (byte *)pixels.get();
        const i32 dstRowSize = width * sizeof(Color);
        const byte *src = data.get();
        const i32 srcRowSize = rowPitch;
        switch (format)
        {
        case PixelFormat::kR32G32B32A32_Typeless:
        case PixelFormat::kR32G32B32A32_Float:
            if (srcRowSize == dstRowSize)
                TkMemory::memoryCopy(dst, src, rowPitch * lines);
            else
            {
                for (u32 row = 0; row < lines; row++)
                {
                    TkMemory::memoryCopy(dst, src, dstRowSize);
                    dst += dstRowSize;
                    src += srcRowSize;
                }
            }
            break;
        default:
        {
            // Try to use texture sampler utility
            auto sampler = PixelFormatSampler::get(format);
            if (sampler)
            {
                for (i32 y = 0; y < height; y++)
                {
                    for (i32 x = 0; x < width; x++)
                    {
                        Color c = sampler->samplePoint(src, x, y, rowPitch);
                        *(Color *)(dst + dstRowSize * y + x * sizeof(Color)) = c;
                    }
                }
                return true;
            }
            LogError("Unsupported texture data format %i.", format);
            return false;
        }
        }
        return true;
    }

    void TextureMipData::copy(void *data, u32 datarowPitch, u32 datadepthPitch, u32 dataDepthSlices, u32 targetrowPitch)
    {
        // Check if target row pitch is the same
        if (targetrowPitch == datarowPitch || targetrowPitch == 0)
        {
            lines = datadepthPitch / datarowPitch;
            depthPitch = datadepthPitch;
            rowPitch = datarowPitch;

            // Single memory copy
            this->data.copy((byte *)data, datadepthPitch * dataDepthSlices);
        }
        else
        {
            lines = datadepthPitch / datarowPitch;
            depthPitch = targetrowPitch * lines;
            rowPitch = targetrowPitch;

            // Convert row by row
            this->data.allocate(depthPitch * dataDepthSlices);
            for (u32 depth = 0; depth < dataDepthSlices; depth++)
            {
                byte *src = (byte *)data + depth * datadepthPitch;
                byte *dst = this->data.get() + depth * depthPitch;
                for (u32 row = 0; row < lines; row++)
                {
                    TkMemory::memoryCopy(dst + row * rowPitch, src + row * datarowPitch, rowPitch);
                }
            }
        }
    }

    i32 TextureData::getArraySize() const
    {
        return items.count();
    }

    i32 TextureData::getMipLevels() const
    {
        return items.hasItems() ? items[0].mips.count() : 0;
    }

    void TextureData::clear()
    {
        items.resize(0, false);
    }

    bool TextureData::getPixels(TkArray<Color32> &pixels, i32 mipIndex, i32 arrayIndex)
    {
        if (items.isValidIndex(arrayIndex) && items.get()[arrayIndex].mips.isValidIndex(mipIndex))
        {
            const i32 mipWidth = TkMath::max(1, width >> mipIndex);
            const i32 mipHeight = TkMath::max(1, height >> mipIndex);
            return items.get()[arrayIndex].mips.get()[mipIndex].getPixels(pixels, mipWidth, mipHeight, format);
        }
        return true;
    }

    bool TextureData::getPixels(TkArray<Color> &pixels, i32 mipIndex, i32 arrayIndex)
    {
        if (items.isValidIndex(arrayIndex) && items.get()[arrayIndex].mips.isValidIndex(mipIndex))
        {
            const i32 mipWidth = TkMath::max(1, width >> mipIndex);
            const i32 mipHeight = TkMath::max(1, height >> mipIndex);
            return items.get()[arrayIndex].mips.get()[mipIndex].getPixels(pixels, mipWidth, mipHeight, format);
        }
        return true;
    }

}