/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "toolkit/graphics/utils/TextureTool.hpp"

#if WITH_STB

#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/utils/PixelFormatSampler.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/platforms/FileSystem.hpp"
#include "toolkit/utils/Color.hpp"
#include "toolkit/utils/Color32.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

namespace toolkit
{
    static bool readFileBytes(const TkStringView &path, TkArray<byte> &data)
    {
        FileHandle file = TkFileSystem::openFile(path);
        if (file == kInvalidFileHandle)
            return false;
        const bool ok = TkFileSystem::readAsBytes(file, data);
        TkFileSystem::closeFile(file);
        return ok;
    }

    static bool isRgba8(PixelFormat format)
    {
        return format == PixelFormat::kR8G8B8A8_UNorm || format == PixelFormat::kR8G8B8A8_UNorm_sRGB;
    }

    static bool isRgba32Float(PixelFormat format)
    {
        return format == PixelFormat::kR32G32B32A32_Float;
    }

    static void setSingleMip(TextureData &textureData, i32 width, i32 height, PixelFormat format, const void *pixels, i32 rowPitch)
    {
        textureData.width = width;
        textureData.height = height;
        textureData.depth = 1;
        textureData.format = format;
        textureData.items.resize(1);
        textureData.items[0].mips.resize(1);

        TextureMipData &mip = textureData.items[0].mips[0];
        mip.rowPitch = rowPitch;
        mip.lines = height;
        mip.depthPitch = rowPitch * height;
        mip.data.copy(static_cast<const byte *>(pixels), mip.depthPitch);
    }

    static bool readRgba8(const TextureData &src, TkArray<Color32> &pixels)
    {
        if (src.items.isEmpty() || src.items[0].mips.isEmpty())
            return false;

        const TextureMipData &srcMip = src.items[0].mips[0];
        if (isRgba8(src.format) && srcMip.rowPitch == static_cast<u32>(src.width * sizeof(Color32)))
        {
            pixels.resize(src.width * src.height, false);
            TkMemory::memoryCopy(pixels.get(), srcMip.data.get(), srcMip.depthPitch);
            return true;
        }

        const PixelFormatSampler *sampler = PixelFormatSampler::get(src.format);
        if (!sampler)
            return false;

        pixels.resize(src.width * src.height, false);
        for (i32 y = 0; y < src.height; ++y)
        {
            for (i32 x = 0; x < src.width; ++x)
            {
                const Color color = sampler->samplePoint(srcMip.data.get(), x, y, srcMip.rowPitch);
                pixels[x + y * src.width] = Color32(color);
            }
        }
        return true;
    }

    static bool writeConverted(TextureData &dst, const TextureData &src, PixelFormat dstFormat)
    {
        const PixelFormatSampler *srcSampler = PixelFormatSampler::get(src.format);
        const PixelFormatSampler *dstSampler = PixelFormatSampler::get(dstFormat);
        if (!srcSampler || !dstSampler)
            return false;

        dst.width = src.width;
        dst.height = src.height;
        dst.depth = src.depth;
        dst.format = dstFormat;
        dst.items.resize(src.items.count());

        const i32 pixelSize = GfxPixelFormat::sizeInBytes(dstFormat);
        for (i32 arrayIndex = 0; arrayIndex < src.items.count(); ++arrayIndex)
        {
            const auto &srcSlice = src.items[arrayIndex];
            auto &dstSlice = dst.items[arrayIndex];
            dstSlice.mips.resize(srcSlice.mips.count(), false);

            for (i32 mipIndex = 0; mipIndex < srcSlice.mips.count(); ++mipIndex)
            {
                const i32 mipWidth = TkMath::max(src.width >> mipIndex, 1);
                const i32 mipHeight = TkMath::max(src.height >> mipIndex, 1);
                const TextureMipData &srcMip = srcSlice.mips[mipIndex];
                TextureMipData &dstMip = dstSlice.mips[mipIndex];
                dstMip.rowPitch = mipWidth * pixelSize;
                dstMip.lines = mipHeight;
                dstMip.depthPitch = dstMip.rowPitch * mipHeight;
                dstMip.data.allocate(dstMip.depthPitch);

                for (i32 y = 0; y < mipHeight; ++y)
                {
                    for (i32 x = 0; x < mipWidth; ++x)
                    {
                        Color color = srcSampler->samplePoint(srcMip.data.get(), x, y, srcMip.rowPitch);
                        dstSampler->store(dstMip.data.get(), x, y, dstMip.rowPitch, color);
                    }
                }
            }
        }
        return true;
    }

    bool TextureTool::exportTextureStb(ImageType type, const TkStringView &path, const TextureData &textureData)
    {
        if (textureData.getArraySize() != 1 || textureData.getMipLevels() < 1)
        {
            LogWarn("%s", "Exporting texture arrays, cubemaps, or empty textures is not supported by STB.");
            return false;
        }

        const TkString filePath(path);
        i32 result = 0;

        if (type == ImageType::kHDR)
        {
            const PixelFormatSampler *sampler = PixelFormatSampler::get(textureData.format);
            if (!sampler)
            {
                LogWarn("Texture data format %d is not supported.", static_cast<i32>(textureData.format));
                return false;
            }

            const TextureMipData &mip = textureData.items[0].mips[0];
            TkArray<f32> pixels;
            pixels.resize(textureData.width * textureData.height * 4, false);
            for (i32 y = 0; y < textureData.height; ++y)
            {
                for (i32 x = 0; x < textureData.width; ++x)
                {
                    Color color = sampler->samplePoint(mip.data.get(), x, y, mip.rowPitch);
                    if (GfxPixelFormat::isSRGB(textureData.format))
                        color = Color::srgbToLinear(color);
                    f32 *dst = pixels.get() + (x + y * textureData.width) * 4;
                    dst[0] = color.r;
                    dst[1] = color.g;
                    dst[2] = color.b;
                    dst[3] = color.a;
                }
            }
            result = stbi_write_hdr(filePath.getText(), textureData.width, textureData.height, 4, pixels.get());
        }
        else
        {
            TkArray<Color32> pixels;
            if (!readRgba8(textureData, pixels))
            {
                LogWarn("Texture data format %d is not supported.", static_cast<i32>(textureData.format));
                return false;
            }

            switch (type)
            {
            case ImageType::kTGA:
                result = stbi_write_tga(filePath.getText(), textureData.width, textureData.height, 4, pixels.get());
                break;
            case ImageType::kPNG:
                result = stbi_write_png(filePath.getText(), textureData.width, textureData.height, 4, pixels.get(), textureData.width * sizeof(Color32));
                break;
            case ImageType::kBMP:
                result = stbi_write_bmp(filePath.getText(), textureData.width, textureData.height, 4, pixels.get());
                break;
            case ImageType::kJPEG:
                result = stbi_write_jpg(filePath.getText(), textureData.width, textureData.height, 4, pixels.get(), 90);
                break;
            default:
                LogWarn("%s", "This image format is not supported by STB export.");
                return false;
            }
        }

        if (result == 0)
        {
            LogWarn("%s", "Saving texture failed.");
            return false;
        }
        return true;
    }

    bool TextureTool::importTextureStb(ImageType type, const TkStringView &path, TextureData &textureData, bool &hasAlpha)
    {
        TkArray<byte> fileData;
        if (!readFileBytes(path, fileData))
        {
            LogWarn("Failed to read texture data from '%s'.", path.getText());
            return false;
        }

        hasAlpha = false;

        if (type == ImageType::kHDR)
        {
            i32 width = 0;
            i32 height = 0;
            i32 comp = 0;
            float *stbData = stbi_loadf_from_memory(fileData.get(), fileData.count(), &width, &height, &comp, 4);
            if (!stbData)
            {
                LogWarn("Failed to load HDR image. %s", stbi_failure_reason());
                return false;
            }

            setSingleMip(textureData, width, height, PixelFormat::kR32G32B32A32_Float, stbData, width * 4 * sizeof(float));
            stbi_image_free(stbData);
            hasAlpha = comp == 4;
            return true;
        }

        if (type == ImageType::kRAW)
        {
            const i32 pixelCount = fileData.count() / static_cast<i32>(sizeof(u16));
            const i32 size = TkMath::sqrt(static_cast<f32>(pixelCount));
            if (size * size != pixelCount)
            {
                LogWarn("%s", "Invalid RAW file data size or format. Use 16-bit square RAW data.");
                return false;
            }

            setSingleMip(textureData, size, size, PixelFormat::kR16_UNorm, fileData.get(), size * static_cast<i32>(sizeof(u16)));
            return true;
        }

        if (type == ImageType::kDDS || type == ImageType::kEXR)
        {
            LogWarn("%s", "This image format is not supported by STB import.");
            return false;
        }

        i32 width = 0;
        i32 height = 0;
        i32 comp = 0;
        byte *stbData = stbi_load_from_memory(fileData.get(), fileData.count(), &width, &height, &comp, 4);
        if (!stbData)
        {
            LogWarn("Failed to load image. %s", stbi_failure_reason());
            return false;
        }

        setSingleMip(textureData, width, height, PixelFormat::kR8G8B8A8_UNorm, stbData, width * static_cast<i32>(sizeof(Color32)));
        if (comp == 4)
        {
            const Color32 *pixels = reinterpret_cast<const Color32 *>(stbData);
            for (i32 i = 0; i < width * height; ++i)
            {
                if (pixels[i].a != 255)
                {
                    hasAlpha = true;
                    break;
                }
            }
        }
        stbi_image_free(stbData);
        return true;
    }

    bool TextureTool::importTextureStb(ImageType type, const TkStringView &path, TextureData &textureData, const Options &options, TkString &errorMsg, bool &hasAlpha)
    {
        bool imported = false;
        if (type == ImageType::kInternal)
        {
            imported = options.internalLoad.isBinded() && options.internalLoad(textureData);
            if (!imported)
                errorMsg = "Internal texture loader failed.";
        }
        else
        {
            imported = importTextureStb(type, path, textureData, hasAlpha);
            if (!imported)
                errorMsg = "Cannot import texture with STB.";
        }

        if (!imported)
            return false;

        i32 dstWidth = textureData.width;
        i32 dstHeight = textureData.height;
        if (options.resize)
        {
            dstWidth = options.sizeX;
            dstHeight = options.keepAspectRatio ? TkMath::max(1, TkMath::roundToInt(static_cast<f32>(textureData.height) * dstWidth / textureData.width)) : options.sizeY;
        }
        else if (options.scale != 1.0f)
        {
            dstWidth = TkMath::max(1, TkMath::roundToInt(textureData.width * options.scale));
            dstHeight = TkMath::max(1, TkMath::roundToInt(textureData.height * options.scale));
        }

        dstWidth = TkMath::min(dstWidth, options.maxSize);
        dstHeight = TkMath::min(dstHeight, options.maxSize);

        if (dstWidth != textureData.width || dstHeight != textureData.height)
        {
            TextureData resized;
            if (!resizeStb(resized, textureData, dstWidth, dstHeight))
            {
                errorMsg = "Cannot resize texture.";
                return false;
            }
            textureData = moveTemp(resized);
        }

        const PixelFormat targetFormat = toPixelFormat(options.type, textureData.width, textureData.height, options.compress);
        if (targetFormat != PixelFormat::kUnknown && targetFormat != textureData.format)
        {
            TextureData converted;
            if (!convert(converted, textureData, targetFormat))
            {
                errorMsg = "Cannot convert texture.";
                return false;
            }
            textureData = moveTemp(converted);
        }

        return true;
    }

    bool TextureTool::convertStb(TextureData &dst, const TextureData &src, const PixelFormat dstFormat)
    {
#if WITH_ASTC
        if (GfxPixelFormat::isCompressedASTC(dstFormat))
            return convertAstc(dst, src, dstFormat);
#endif

        if (GfxPixelFormat::isCompressed(dstFormat))
        {
            LogWarn("STB conversion does not support compressed target format %d.", static_cast<i32>(dstFormat));
            return false;
        }

        return writeConverted(dst, src, dstFormat);
    }

    bool TextureTool::resizeStb(PixelFormat format, TextureMipData &dstMip, const TextureMipData &srcMip, i32 dstMipWidth, i32 dstMipHeight)
    {
        if (!isRgba8(format) && !isRgba32Float(format))
        {
            LogWarn("STB resize does not support format %d.", static_cast<i32>(format));
            return false;
        }

        const i32 srcMipHeight = srcMip.rowPitch != 0 ? static_cast<i32>(srcMip.depthPitch / srcMip.rowPitch) : 0;
        if (srcMipHeight <= 0)
            return false;

        if (isRgba32Float(format))
        {
            dstMip.rowPitch = dstMipWidth * 4 * sizeof(float);
            dstMip.lines = dstMipHeight;
            dstMip.depthPitch = dstMip.rowPitch * dstMipHeight;
            dstMip.data.allocate(dstMip.depthPitch);
            return stbir_resize_float_linear(
                       reinterpret_cast<const float *>(srcMip.data.get()),
                       static_cast<int>(srcMip.rowPitch / (4 * sizeof(float))),
                       srcMipHeight,
                       srcMip.rowPitch,
                       reinterpret_cast<float *>(dstMip.data.get()),
                       dstMipWidth,
                       dstMipHeight,
                       dstMip.rowPitch,
                       STBIR_RGBA) != nullptr;
        }

        dstMip.rowPitch = dstMipWidth * sizeof(Color32);
        dstMip.lines = dstMipHeight;
        dstMip.depthPitch = dstMip.rowPitch * dstMipHeight;
        dstMip.data.allocate(dstMip.depthPitch);
        return stbir_resize_uint8_linear(
                   srcMip.data.get(),
                   static_cast<int>(srcMip.rowPitch / sizeof(Color32)),
                   srcMipHeight,
                   srcMip.rowPitch,
                   dstMip.data.get(),
                   dstMipWidth,
                   dstMipHeight,
                   dstMip.rowPitch,
                   STBIR_RGBA) != nullptr;
    }

    bool TextureTool::resizeStb(TextureData &dst, const TextureData &src, i32 dstWidth, i32 dstHeight)
    {
        if (!isRgba8(src.format) && !isRgba32Float(src.format))
        {
            TextureData rgba;
            const PixelFormat tempFormat = GfxPixelFormat::isHDR(src.format) ? PixelFormat::kR32G32B32A32_Float : PixelFormat::kR8G8B8A8_UNorm;
            if (!convertStb(rgba, src, tempFormat))
                return false;
            return resizeStb(dst, rgba, dstWidth, dstHeight);
        }

        dst.width = dstWidth;
        dst.height = dstHeight;
        dst.depth = src.depth;
        dst.format = src.format;
        dst.items.resize(src.items.count());

        for (i32 arrayIndex = 0; arrayIndex < src.items.count(); ++arrayIndex)
        {
            const auto &srcSlice = src.items[arrayIndex];
            auto &dstSlice = dst.items[arrayIndex];
            dstSlice.mips.resize(1, false);
            if (!resizeStb(src.format, dstSlice.mips[0], srcSlice.mips[0], dstWidth, dstHeight))
                return false;
        }

        return true;
    }
}

#endif
