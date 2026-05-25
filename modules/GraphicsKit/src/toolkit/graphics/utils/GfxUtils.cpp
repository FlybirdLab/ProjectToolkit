/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxUtils.hpp"

#include "toolkit/base/Math.hpp"

namespace toolkit
{

    void GfxUtils::computePitch(PixelFormat format, i32 width, i32 height, u32 &rowPitch, u32 &slicePitch)
    {
        switch (format)
        {
        case PixelFormat::kBC1_Typeless:
        case PixelFormat::kBC1_UNorm:
        case PixelFormat::kBC1_UNorm_sRGB:
        case PixelFormat::kBC4_Typeless:
        case PixelFormat::kBC4_UNorm:
        case PixelFormat::kBC4_SNorm:
        {
            const u32 nbw = TkMath::max<u32>(1, (width + 3) / 4);
            const u32 nbh = TkMath::max<u32>(1, (height + 3) / 4);
            rowPitch = nbw * 8;
            slicePitch = rowPitch * nbh;
        }
        break;
        case PixelFormat::kBC2_Typeless:
        case PixelFormat::kBC2_UNorm:
        case PixelFormat::kBC2_UNorm_sRGB:
        case PixelFormat::kBC3_Typeless:
        case PixelFormat::kBC3_UNorm:
        case PixelFormat::kBC3_UNorm_sRGB:
        case PixelFormat::kBC5_Typeless:
        case PixelFormat::kBC5_UNorm:
        case PixelFormat::kBC5_SNorm:
        case PixelFormat::kBC6H_Typeless:
        case PixelFormat::kBC6H_Uf16:
        case PixelFormat::kBC6H_Sf16:
        case PixelFormat::kBC7_Typeless:
        case PixelFormat::kBC7_UNorm:
        case PixelFormat::kBC7_UNorm_sRGB:
        {
            const u32 nbw = TkMath::max<u32>(1, (width + 3) / 4);
            const u32 nbh = TkMath::max<u32>(1, (height + 3) / 4);
            rowPitch = nbw * 16;
            slicePitch = rowPitch * nbh;
        }
        break;
        case PixelFormat::kASTC_4x4_UNorm:
        case PixelFormat::kASTC_4x4_UNorm_sRGB:
        case PixelFormat::kASTC_6x6_UNorm:
        case PixelFormat::kASTC_6x6_UNorm_sRGB:
        case PixelFormat::kASTC_8x8_UNorm:
        case PixelFormat::kASTC_8x8_UNorm_sRGB:
        case PixelFormat::kASTC_10x10_UNorm:
        case PixelFormat::kASTC_10x10_UNorm_sRGB:
        {
            const i32 blockSize = GfxPixelFormat::computeBlockSize(format);
            const u32 nbw = TkMath::max<u32>(1, TkMath::divideAndRoundUp(width, blockSize));
            const u32 nbh = TkMath::max<u32>(1, TkMath::divideAndRoundUp(height, blockSize));
            rowPitch = nbw * 16; // All ASTC blocks use 128 bits
            slicePitch = rowPitch * nbh;
        }
        break;
        case PixelFormat::kR8G8_B8G8_UNorm:
        case PixelFormat::kG8R8_G8B8_UNorm:
        case PixelFormat::kYUY2:
            rowPitch = ((width + 1) >> 1) * 4;
            slicePitch = rowPitch * height;
            break;
        case PixelFormat::kNV12:
            rowPitch = width;
            slicePitch = width * height * 3 / 2;
            break;
        default:
            // Default byte alignment
            rowPitch = (width * GfxPixelFormat::sizeInBits(format) + 7) / 8;
            slicePitch = rowPitch * height;
            break;
        }
    }

    GfxFeatureLevel GfxUtils::getFeatureLevel(GfxShaderProfile profile)
    {
        switch (profile)
        {
        case GfxShaderProfile::kDirectX_SM6:
            return GfxFeatureLevel::kSM6;
        case GfxShaderProfile::kDirectX_SM5:
        case GfxShaderProfile::kVulkan_SM5:
            return GfxFeatureLevel::kSM5;
        case GfxShaderProfile::kDirectX_SM4:
            return GfxFeatureLevel::kSM4;
        case GfxShaderProfile::kGLSL_440:
        case GfxShaderProfile::kGLSL_410:
            // GLSL 4.1 Core lines up roughly with SM5 capability-wise (no
            // compute / tessellation in 4.1, but everything below). Mapping
            // these to ES2 was a stale placeholder from when the OpenGL
            // backend was a skeleton; it caused Parser to filter out every
            // function tagged FEATURE_LEVEL_SM4/SM5.
            return GfxFeatureLevel::kSM5;
        case GfxShaderProfile::kUnknown:
            return GfxFeatureLevel::kES2;
        default:
            return GfxFeatureLevel::kES2;
        }
    }

    u64 GfxUtils::calculateTextureMemoryUsage(PixelFormat format, i32 width, i32 height, i32 mipLevels)
    {
        u64 result = 0;

        if (mipLevels == 0)
            mipLevels = 69;

        u32 rowPitch, slicePitch;
        while (mipLevels > 0 && (width >= 1 || height >= 1))
        {
            computePitch(format, width, height, rowPitch, slicePitch);
            result += slicePitch;

            if (width > 1)
                width >>= 1;
            if (height > 1)
                height >>= 1;

            mipLevels--;
        }

        return result;
    }

    u64 GfxUtils::calculateTextureMemoryUsage(PixelFormat format, i32 width, i32 height, i32 depth, i32 mipLevels)
    {
        return calculateTextureMemoryUsage(format, width, height, mipLevels) * depth;
    }

}