/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "toolkit/graphics/utils/TextureTool.hpp"

#if WITH_ASTC

#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/utils/PixelFormat.hpp"
#include "toolkit/graphics/utils/GfxUtils.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/utils/Color.hpp"

#include <astcenc.h>

namespace toolkit
{

    bool TextureTool::convertAstc(TextureData &dst, const TextureData &src, const PixelFormat dstFormat)
    {
        PROFILE();
        TK_ASSERT(GfxPixelFormat::isCompressedASTC(dstFormat));
        const i32 blockSize = GfxPixelFormat::computeBlockSize(dstFormat);
        const i32 bytesPerBlock = 16; // All ASTC blocks use 128 bits

        // Configure the compressor run
        const bool isSRGB = GfxPixelFormat::isSRGB(dstFormat);
        const bool isHDR = GfxPixelFormat::isHDR(src.format);
        astcenc_profile astcProfile = isHDR ? ASTCENC_PRF_HDR_RGB_LDR_A : (isSRGB ? ASTCENC_PRF_LDR_SRGB : ASTCENC_PRF_LDR);
        float astcQuality = ASTCENC_PRE_MEDIUM;
        unsigned int astcFlags = 0; // TODO: add custom flags support for converter to handle ASTCENC_FLG_MAP_NORMAL
        astcenc_config astcConfig;
        astcenc_error astcError = astcenc_config_init(astcProfile, blockSize, blockSize, 1, astcQuality, astcFlags, &astcConfig);
        if (astcError != ASTCENC_SUCCESS)
        {
            LogWarn("Cannot compress image. ASTC failed with error: %s", astcenc_get_error_string(astcError));
            return false;
        }
        astcenc_swizzle astcSwizzle = {ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B, ASTCENC_SWZ_A};
        if (!GfxPixelFormat::hasAlpha(src.format))
        {
            // No alpha channel in use so fill with 1
            astcSwizzle.a = ASTCENC_SWZ_1;
        }

        // Allocate working state given config and thread_count
        astcenc_context *astcContext = nullptr;
        astcError = astcenc_context_alloc(&astcConfig, 1, &astcContext, nullptr);
        if (astcError != ASTCENC_SUCCESS)
        {
            LogWarn("Cannot compress image. ASTC failed with error: %s", astcenc_get_error_string(astcError));
            return false;
        }
        TextureData const *textureData = &src;
        TextureData converted;

        // Encoder uses full 4-component RGBA input image so convert it if needed
        if (GfxPixelFormat::computeComponentsCount(src.format) != 4 ||
            GfxPixelFormat::isCompressed(textureData->format) ||
            !GfxPixelFormat::isRgbAOrder(textureData->format))
        {
            const PixelFormat tempFormat = isHDR ? PixelFormat::kR16G16B16A16_Float : (GfxPixelFormat::isSRGB(src.format) ? PixelFormat::kR8G8B8A8_UNorm_sRGB : PixelFormat::kR8G8B8A8_UNorm);
            if (!TextureTool::convert(converted, *textureData, tempFormat))
            {
                astcenc_context_free(astcContext);
                return false;
            }
            else
            {
                textureData = &converted;
            }
        }

        // When converting from non-sRGB to sRGB we need to change the color-space manually (otherwise image is dark)
        if (GfxPixelFormat::isSRGB(src.format) != isSRGB)
        {
            if (textureData == &src)
                converted = src;
            TkFunc<void(Color &)> transform = [](Color &c)
            {
                c = Color::linearToSrgb(c);
            };
            if (!TextureTool::transform(converted, transform))
            {
                astcenc_context_free(astcContext);
                return false;
            }
            else
            {
                textureData = &converted;
            }
        }

        // Setup output
        dst.items.resize(textureData->items.count());
        dst.width = textureData->width;
        dst.height = textureData->height;
        dst.depth = 1;
        dst.format = dstFormat;

        // Compress all array slices
        for (i32 arrayIndex = 0; arrayIndex < textureData->items.count() && astcError == ASTCENC_SUCCESS; arrayIndex++)
        {
            const auto &srcSlice = textureData->items[arrayIndex];
            auto &dstSlice = dst.items[arrayIndex];
            auto mipLevels = srcSlice.mips.count();
            dstSlice.mips.resize(mipLevels, false);

            // Compress all mip levels
            for (i32 mipIndex = 0; mipIndex < mipLevels && astcError == ASTCENC_SUCCESS; mipIndex++)
            {
                const auto &srcMip = srcSlice.mips[mipIndex];
                auto &dstMip = dstSlice.mips[mipIndex];
                auto mipWidth = TkMath::max(textureData->width >> mipIndex, 1);
                auto mipHeight = TkMath::max(textureData->height >> mipIndex, 1);
                auto blocksWidth = TkMath::max(TkMath::divideAndRoundUp(mipWidth, blockSize), 1);
                auto blocksHeight = TkMath::max(TkMath::divideAndRoundUp(mipHeight, blockSize), 1);
                u32 mipRowPitch = 0;
                u32 mipSlicePitch = 0;
                GfxUtils::computePitch(textureData->format, mipWidth, mipHeight, mipRowPitch, mipSlicePitch);
                TK_ASSERT(srcMip.rowPitch == mipRowPitch);
                TK_ASSERT(srcMip.depthPitch == mipSlicePitch);
                TK_ASSERT(srcMip.lines == static_cast<u32>(mipHeight));

                // Allocate memory
                dstMip.rowPitch = blocksWidth * bytesPerBlock;
                dstMip.depthPitch = dstMip.rowPitch * blocksHeight;
                dstMip.lines = blocksHeight;
                dstMip.data.allocate(dstMip.depthPitch);

                // Compress image
                astcenc_image astcInput;
                astcInput.dim_x = mipWidth;
                astcInput.dim_y = mipHeight;
                astcInput.dim_z = 1;
                astcInput.data_type = isHDR ? ASTCENC_TYPE_F16 : ASTCENC_TYPE_U8;
                void *srcData = (void*)srcMip.data.get();
                astcInput.data = &srcData;
                astcError = astcenc_compress_image(astcContext, &astcInput, &astcSwizzle, dstMip.data.get(), dstMip.data.length(), 0);
                if (astcError == ASTCENC_SUCCESS)
                    astcError = astcenc_compress_reset(astcContext);
            }
        }

        // Clean up
        if (astcError != ASTCENC_SUCCESS)
        {
            LogWarn("Cannot compress image. ASTC failed with error: %s", astcenc_get_error_string(astcError));
            astcenc_context_free(astcContext);
            return false;
        }
        astcenc_context_free(astcContext);
        return true;
    }

}

#endif
