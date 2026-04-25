/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "PixelFormat.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/math/Vector3.hpp"
#include "toolkit/math/Vector4.hpp"
#include "PixelFormatSampler.hpp"
#include "toolkit/utils/Color32.hpp"
#include "toolkit/utils/Half.hpp"

namespace toolkit
{

    static i32 sizeOfInBits[(i32)PixelFormat::MAX];

#define InitFormat(formats, bitCount)             \
    for (int i = 0; i < lengthOf(formats); i++)   \
    {                                             \
        sizeOfInBits[(i32)formats[i]] = bitCount; \
    }

    void GfxPixelFormat::init()
    {
        // Clear lookup table
        TkMemory::memoryClear(sizeOfInBits, sizeof sizeOfInBits);

        PixelFormat formats1[] = {
            PixelFormat::kR1_UNorm};
        InitFormat(formats1, 1);

        PixelFormat formats2[] = {
            PixelFormat::kA8_UNorm,
            PixelFormat::kR8_SInt,
            PixelFormat::kR8_SNorm,
            PixelFormat::kR8_Typeless,
            PixelFormat::kR8_UInt,
            PixelFormat::kR8_UNorm,
            PixelFormat::kBC2_Typeless,
            PixelFormat::kBC2_UNorm,
            PixelFormat::kBC2_UNorm_sRGB,
            PixelFormat::kBC3_Typeless,
            PixelFormat::kBC3_UNorm,
            PixelFormat::kBC3_UNorm_sRGB,
            PixelFormat::kBC5_SNorm,
            PixelFormat::kBC5_Typeless,
            PixelFormat::kBC5_UNorm,
            PixelFormat::kBC6H_Sf16,
            PixelFormat::kBC6H_Typeless,
            PixelFormat::kBC6H_Uf16,
            PixelFormat::kBC7_Typeless,
            PixelFormat::kBC7_UNorm,
            PixelFormat::kBC7_UNorm_sRGB,
            PixelFormat::kASTC_4x4_UNorm,
            PixelFormat::kASTC_4x4_UNorm_sRGB,
            PixelFormat::kYUY2,
        };
        InitFormat(formats2, 8);

        PixelFormat formats3[] = {
            PixelFormat::kB5G5R5A1_UNorm,
            PixelFormat::kB5G6R5_UNorm,
            PixelFormat::kD16_UNorm,
            PixelFormat::kR16_Float,
            PixelFormat::kR16_SInt,
            PixelFormat::kR16_SNorm,
            PixelFormat::kR16_Typeless,
            PixelFormat::kR16_UInt,
            PixelFormat::kR16_UNorm,
            PixelFormat::kR8G8_SInt,
            PixelFormat::kR8G8_SNorm,
            PixelFormat::kR8G8_Typeless,
            PixelFormat::kR8G8_UInt,
            PixelFormat::kR8G8_UNorm,
        };
        InitFormat(formats3, 16);

        PixelFormat formats4[] = {
            PixelFormat::kB8G8R8X8_Typeless,
            PixelFormat::kB8G8R8X8_UNorm,
            PixelFormat::kB8G8R8X8_UNorm_sRGB,
            PixelFormat::kD24_UNorm_S8_UInt,
            PixelFormat::kD32_Float,
            PixelFormat::kD32_Float_S8X24_UInt,
            PixelFormat::kG8R8_G8B8_UNorm,
            PixelFormat::kR10G10B10_Xr_Bias_A2_UNorm,
            PixelFormat::kR10G10B10A2_Typeless,
            PixelFormat::kR10G10B10A2_UInt,
            PixelFormat::kR10G10B10A2_UNorm,
            PixelFormat::kR11G11B10_Float,
            PixelFormat::kR16G16_Float,
            PixelFormat::kR16G16_SInt,
            PixelFormat::kR16G16_SNorm,
            PixelFormat::kR16G16_Typeless,
            PixelFormat::kR16G16_UInt,
            PixelFormat::kR16G16_UNorm,
            PixelFormat::kR24_UNorm_X8_Typeless,
            PixelFormat::kR24G8_Typeless,
            PixelFormat::kR32_Float,
            PixelFormat::kR32_Float_X8X24_Typeless,
            PixelFormat::kR32_SInt,
            PixelFormat::kR32_Typeless,
            PixelFormat::kR32_UInt,
            PixelFormat::kR8G8_B8G8_UNorm,
            PixelFormat::kR8G8B8A8_SInt,
            PixelFormat::kR8G8B8A8_SNorm,
            PixelFormat::kR8G8B8A8_Typeless,
            PixelFormat::kR8G8B8A8_UInt,
            PixelFormat::kR8G8B8A8_UNorm,
            PixelFormat::kR8G8B8A8_UNorm_sRGB,
            PixelFormat::kB8G8R8A8_Typeless,
            PixelFormat::kB8G8R8A8_UNorm,
            PixelFormat::kB8G8R8A8_UNorm_sRGB,
            PixelFormat::kR9G9B9E5_SharedExp,
            PixelFormat::kX24_Typeless_G8_UInt,
            PixelFormat::kX32_Typeless_G8X24_UInt,
        };
        InitFormat(formats4, 32);

        PixelFormat formats5[] = {
            PixelFormat::kR16G16B16A16_Float,
            PixelFormat::kR16G16B16A16_SInt,
            PixelFormat::kR16G16B16A16_SNorm,
            PixelFormat::kR16G16B16A16_Typeless,
            PixelFormat::kR16G16B16A16_UInt,
            PixelFormat::kR16G16B16A16_UNorm,
            PixelFormat::kR32G32_Float,
            PixelFormat::kR32G32_SInt,
            PixelFormat::kR32G32_Typeless,
            PixelFormat::kR32G32_UInt,
            PixelFormat::kR32G8X24_Typeless,
        };
        InitFormat(formats5, 64);

        PixelFormat formats6[] = {
            PixelFormat::kR32G32B32_Float,
            PixelFormat::kR32G32B32_SInt,
            PixelFormat::kR32G32B32_Typeless,
            PixelFormat::kR32G32B32_UInt,
        };
        InitFormat(formats6, 96);

        PixelFormat formats7[] = {
            PixelFormat::kR32G32B32A32_Float,
            PixelFormat::kR32G32B32A32_SInt,
            PixelFormat::kR32G32B32A32_Typeless,
            PixelFormat::kR32G32B32A32_UInt,
        };
        InitFormat(formats7, 128);

        PixelFormat formats8[] = {
            PixelFormat::kBC1_Typeless,
            PixelFormat::kBC1_UNorm,
            PixelFormat::kBC1_UNorm_sRGB,
            PixelFormat::kBC4_SNorm,
            PixelFormat::kBC4_Typeless,
            PixelFormat::kBC4_UNorm,
        };
        InitFormat(formats8, 4);

        sizeOfInBits[(i32)PixelFormat::kNV12] = 12;
    }

    i32 GfxPixelFormat::sizeInBits(PixelFormat format)
    {
        return sizeOfInBits[(i32)format];
    }

    i32 GfxPixelFormat::alphaSizeInBits(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kR32G32B32A32_Typeless:
        case PixelFormat::kR32G32B32A32_Float:
        case PixelFormat::kR32G32B32A32_UInt:
        case PixelFormat::kR32G32B32A32_SInt:
            return 32;
        case PixelFormat::kR16G16B16A16_Typeless:
        case PixelFormat::kR16G16B16A16_Float:
        case PixelFormat::kR16G16B16A16_UNorm:
        case PixelFormat::kR16G16B16A16_UInt:
        case PixelFormat::kR16G16B16A16_SNorm:
        case PixelFormat::kR16G16B16A16_SInt:
            return 16;
        case PixelFormat::kR10G10B10A2_Typeless:
        case PixelFormat::kR10G10B10A2_UNorm:
        case PixelFormat::kR10G10B10A2_UInt:
        case PixelFormat::kR10G10B10_Xr_Bias_A2_UNorm:
            return 2;
        case PixelFormat::kR8G8B8A8_Typeless:
        case PixelFormat::kR8G8B8A8_UNorm:
        case PixelFormat::kR8G8B8A8_UNorm_sRGB:
        case PixelFormat::kR8G8B8A8_UInt:
        case PixelFormat::kR8G8B8A8_SNorm:
        case PixelFormat::kR8G8B8A8_SInt:
        case PixelFormat::kB8G8R8A8_UNorm:
        case PixelFormat::kB8G8R8A8_Typeless:
        case PixelFormat::kB8G8R8A8_UNorm_sRGB:
        case PixelFormat::kA8_UNorm:
            return 8;
        case PixelFormat::kB5G5R5A1_UNorm:
            return 1;
        case PixelFormat::kBC1_Typeless:
        case PixelFormat::kBC1_UNorm:
        case PixelFormat::kBC1_UNorm_sRGB:
            return 1; // or 0
        case PixelFormat::kBC2_Typeless:
        case PixelFormat::kBC2_UNorm:
        case PixelFormat::kBC2_UNorm_sRGB:
            return 4;
        case PixelFormat::kBC3_Typeless:
        case PixelFormat::kBC3_UNorm:
        case PixelFormat::kBC3_UNorm_sRGB:
            return 8;
        case PixelFormat::kBC7_Typeless:
        case PixelFormat::kBC7_UNorm:
        case PixelFormat::kBC7_UNorm_sRGB:
            return 8; // or 0
        default:
            return 0;
        }
    }

    bool GfxPixelFormat::isDepthStencil(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kR32G8X24_Typeless:
        case PixelFormat::kD32_Float_S8X24_UInt:
        case PixelFormat::kR32_Float_X8X24_Typeless:
        case PixelFormat::kX32_Typeless_G8X24_UInt:
        case PixelFormat::kD32_Float:
        case PixelFormat::kR24G8_Typeless:
        case PixelFormat::kD24_UNorm_S8_UInt:
        case PixelFormat::kR24_UNorm_X8_Typeless:
        case PixelFormat::kX24_Typeless_G8_UInt:
        case PixelFormat::kD16_UNorm:
            return true;
        default:
            return false;
        }
    }

    bool GfxPixelFormat::hasStencil(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kD24_UNorm_S8_UInt:
        case PixelFormat::kD32_Float_S8X24_UInt:
            return true;
        default:
            return false;
        }
    }

    bool GfxPixelFormat::isTypeless(const PixelFormat format, const bool partialTypeless)
    {
        switch (format)
        {
        case PixelFormat::kR32G32B32A32_Typeless:
        case PixelFormat::kR32G32B32_Typeless:
        case PixelFormat::kR16G16B16A16_Typeless:
        case PixelFormat::kR32G32_Typeless:
        case PixelFormat::kR32G8X24_Typeless:
        case PixelFormat::kR10G10B10A2_Typeless:
        case PixelFormat::kR8G8B8A8_Typeless:
        case PixelFormat::kR16G16_Typeless:
        case PixelFormat::kR32_Typeless:
        case PixelFormat::kR24G8_Typeless:
        case PixelFormat::kR8G8_Typeless:
        case PixelFormat::kR16_Typeless:
        case PixelFormat::kR8_Typeless:
        case PixelFormat::kBC1_Typeless:
        case PixelFormat::kBC2_Typeless:
        case PixelFormat::kBC3_Typeless:
        case PixelFormat::kBC4_Typeless:
        case PixelFormat::kBC5_Typeless:
        case PixelFormat::kB8G8R8A8_Typeless:
        case PixelFormat::kB8G8R8X8_Typeless:
        case PixelFormat::kBC6H_Typeless:
        case PixelFormat::kBC7_Typeless:
            return true;
        case PixelFormat::kR32_Float_X8X24_Typeless:
        case PixelFormat::kX32_Typeless_G8X24_UInt:
        case PixelFormat::kR24_UNorm_X8_Typeless:
        case PixelFormat::kX24_Typeless_G8_UInt:
            return partialTypeless;
        default:
            return false;
        }
    }

    bool GfxPixelFormat::isValid(const PixelFormat format)
    {
        return format > PixelFormat::kUnknown && format < PixelFormat::MAX;
    }

    bool GfxPixelFormat::isCompressed(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kBC1_Typeless:
        case PixelFormat::kBC1_UNorm:
        case PixelFormat::kBC1_UNorm_sRGB:
        case PixelFormat::kBC2_Typeless:
        case PixelFormat::kBC2_UNorm:
        case PixelFormat::kBC2_UNorm_sRGB:
        case PixelFormat::kBC3_Typeless:
        case PixelFormat::kBC3_UNorm:
        case PixelFormat::kBC3_UNorm_sRGB:
        case PixelFormat::kBC4_Typeless:
        case PixelFormat::kBC4_UNorm:
        case PixelFormat::kBC4_SNorm:
        case PixelFormat::kBC5_Typeless:
        case PixelFormat::kBC5_UNorm:
        case PixelFormat::kBC5_SNorm:
        case PixelFormat::kBC6H_Typeless:
        case PixelFormat::kBC6H_Uf16:
        case PixelFormat::kBC6H_Sf16:
        case PixelFormat::kBC7_Typeless:
        case PixelFormat::kBC7_UNorm:
        case PixelFormat::kBC7_UNorm_sRGB:
        case PixelFormat::kASTC_4x4_UNorm:
        case PixelFormat::kASTC_4x4_UNorm_sRGB:
        case PixelFormat::kASTC_6x6_UNorm:
        case PixelFormat::kASTC_6x6_UNorm_sRGB:
        case PixelFormat::kASTC_8x8_UNorm:
        case PixelFormat::kASTC_8x8_UNorm_sRGB:
        case PixelFormat::kASTC_10x10_UNorm:
        case PixelFormat::kASTC_10x10_UNorm_sRGB:
            return true;
        default:
            return false;
        }
    }

    bool GfxPixelFormat::isCompressedBC(PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kBC1_Typeless:
        case PixelFormat::kBC1_UNorm:
        case PixelFormat::kBC1_UNorm_sRGB:
        case PixelFormat::kBC2_Typeless:
        case PixelFormat::kBC2_UNorm:
        case PixelFormat::kBC2_UNorm_sRGB:
        case PixelFormat::kBC3_Typeless:
        case PixelFormat::kBC3_UNorm:
        case PixelFormat::kBC3_UNorm_sRGB:
        case PixelFormat::kBC4_Typeless:
        case PixelFormat::kBC4_UNorm:
        case PixelFormat::kBC4_SNorm:
        case PixelFormat::kBC5_Typeless:
        case PixelFormat::kBC5_UNorm:
        case PixelFormat::kBC5_SNorm:
        case PixelFormat::kBC6H_Typeless:
        case PixelFormat::kBC6H_Uf16:
        case PixelFormat::kBC6H_Sf16:
        case PixelFormat::kBC7_Typeless:
        case PixelFormat::kBC7_UNorm:
        case PixelFormat::kBC7_UNorm_sRGB:
            return true;
        default:
            return false;
        }
    }

    bool GfxPixelFormat::isCompressedASTC(PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kASTC_4x4_UNorm:
        case PixelFormat::kASTC_4x4_UNorm_sRGB:
        case PixelFormat::kASTC_6x6_UNorm:
        case PixelFormat::kASTC_6x6_UNorm_sRGB:
        case PixelFormat::kASTC_8x8_UNorm:
        case PixelFormat::kASTC_8x8_UNorm_sRGB:
        case PixelFormat::kASTC_10x10_UNorm:
        case PixelFormat::kASTC_10x10_UNorm_sRGB:
            return true;
        default:
            return false;
        }
    }

    bool GfxPixelFormat::isVideo(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kYUY2:
        case PixelFormat::kNV12:
            return true;
        default:
            return false;
        }
    }

    bool GfxPixelFormat::isSRGB(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kR8G8B8A8_UNorm_sRGB:
        case PixelFormat::kBC1_UNorm_sRGB:
        case PixelFormat::kBC2_UNorm_sRGB:
        case PixelFormat::kBC3_UNorm_sRGB:
        case PixelFormat::kB8G8R8A8_UNorm_sRGB:
        case PixelFormat::kB8G8R8X8_UNorm_sRGB:
        case PixelFormat::kBC7_UNorm_sRGB:
        case PixelFormat::kASTC_4x4_UNorm_sRGB:
        case PixelFormat::kASTC_6x6_UNorm_sRGB:
        case PixelFormat::kASTC_8x8_UNorm_sRGB:
        case PixelFormat::kASTC_10x10_UNorm_sRGB:
            return true;
        default:
            return false;
        }
    }

    bool GfxPixelFormat::isHDR(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kR11G11B10_Float:
        case PixelFormat::kR10G10B10A2_UNorm:
        case PixelFormat::kR16G16B16A16_Float:
        case PixelFormat::kR32G32B32A32_Float:
        case PixelFormat::kR16G16_Float:
        case PixelFormat::kR16_Float:
        case PixelFormat::kBC6H_Sf16:
        case PixelFormat::kBC6H_Uf16:
            return true;
        default:
            return false;
        }
    }

    bool GfxPixelFormat::isRgbAOrder(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kR32G32B32A32_Typeless:
        case PixelFormat::kR32G32B32A32_Float:
        case PixelFormat::kR32G32B32A32_UInt:
        case PixelFormat::kR32G32B32A32_SInt:
        case PixelFormat::kR32G32B32_Typeless:
        case PixelFormat::kR32G32B32_Float:
        case PixelFormat::kR32G32B32_UInt:
        case PixelFormat::kR32G32B32_SInt:
        case PixelFormat::kR16G16B16A16_Typeless:
        case PixelFormat::kR16G16B16A16_Float:
        case PixelFormat::kR16G16B16A16_UNorm:
        case PixelFormat::kR16G16B16A16_UInt:
        case PixelFormat::kR16G16B16A16_SNorm:
        case PixelFormat::kR16G16B16A16_SInt:
        case PixelFormat::kR32G32_Typeless:
        case PixelFormat::kR32G32_Float:
        case PixelFormat::kR32G32_UInt:
        case PixelFormat::kR32G32_SInt:
        case PixelFormat::kR32G8X24_Typeless:
        case PixelFormat::kR10G10B10A2_Typeless:
        case PixelFormat::kR10G10B10A2_UNorm:
        case PixelFormat::kR10G10B10A2_UInt:
        case PixelFormat::kR11G11B10_Float:
        case PixelFormat::kR8G8B8A8_Typeless:
        case PixelFormat::kR8G8B8A8_UNorm:
        case PixelFormat::kR8G8B8A8_UNorm_sRGB:
        case PixelFormat::kR8G8B8A8_UInt:
        case PixelFormat::kR8G8B8A8_SNorm:
        case PixelFormat::kR8G8B8A8_SInt:
            return true;
        default:
            return false;
        }
    }

    bool GfxPixelFormat::isBGRAOrder(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kB8G8R8A8_UNorm:
        case PixelFormat::kB8G8R8X8_UNorm:
        case PixelFormat::kB8G8R8A8_Typeless:
        case PixelFormat::kB8G8R8A8_UNorm_sRGB:
        case PixelFormat::kB8G8R8X8_Typeless:
        case PixelFormat::kB8G8R8X8_UNorm_sRGB:
            return true;
        default:
            return false;
        }
    }

    bool GfxPixelFormat::isNormalized(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kR16G16B16A16_UNorm:
        case PixelFormat::kR16G16B16A16_SNorm:
        case PixelFormat::kR10G10B10A2_UNorm:
        case PixelFormat::kR8G8B8A8_UNorm:
        case PixelFormat::kR8G8B8A8_UNorm_sRGB:
        case PixelFormat::kR8G8B8A8_SNorm:
        case PixelFormat::kR8G8_B8G8_UNorm:
        case PixelFormat::kG8R8_G8B8_UNorm:
        case PixelFormat::kB5G5R5A1_UNorm:
        case PixelFormat::kB8G8R8A8_UNorm:
        case PixelFormat::kB8G8R8X8_UNorm:
        case PixelFormat::kR10G10B10_Xr_Bias_A2_UNorm:
        case PixelFormat::kB8G8R8A8_UNorm_sRGB:
        case PixelFormat::kB8G8R8X8_UNorm_sRGB:
        case PixelFormat::kB5G6R5_UNorm:
        case PixelFormat::kR16G16_UNorm:
        case PixelFormat::kR16G16_SNorm:
        case PixelFormat::kR8G8_UNorm:
        case PixelFormat::kR8G8_SNorm:
        case PixelFormat::kD16_UNorm:
        case PixelFormat::kR16_UNorm:
        case PixelFormat::kR16_SNorm:
        case PixelFormat::kR8_UNorm:
        case PixelFormat::kR8_SNorm:
        case PixelFormat::kA8_UNorm:
        case PixelFormat::kR1_UNorm:
            return true;
        default:
            return false;
        }
    }

    bool GfxPixelFormat::isInteger(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kR32G32B32A32_UInt:
        case PixelFormat::kR32G32B32A32_SInt:
        case PixelFormat::kR16G16B16A16_UInt:
        case PixelFormat::kR16G16B16A16_SInt:
        case PixelFormat::kR10G10B10A2_UInt:
        case PixelFormat::kR8G8B8A8_UInt:
        case PixelFormat::kR8G8B8A8_SInt:
        case PixelFormat::kR32G32B32_UInt:
        case PixelFormat::kR32G32B32_SInt:
        case PixelFormat::kR32G32_UInt:
        case PixelFormat::kR32G32_SInt:
        case PixelFormat::kR16G16_UInt:
        case PixelFormat::kR16G16_SInt:
        case PixelFormat::kR8G8_UInt:
        case PixelFormat::kR8G8_SInt:
        case PixelFormat::kR32_UInt:
        case PixelFormat::kR32_SInt:
        case PixelFormat::kR16_UInt:
        case PixelFormat::kR16_SInt:
        case PixelFormat::kR8_UInt:
        case PixelFormat::kR8_SInt:
            return true;
        default:
            return false;
        }
    }

    i32 GfxPixelFormat::computeComponentsCount(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kR32G32B32A32_Typeless:
        case PixelFormat::kR32G32B32A32_Float:
        case PixelFormat::kR32G32B32A32_UInt:
        case PixelFormat::kR32G32B32A32_SInt:
        case PixelFormat::kR16G16B16A16_Typeless:
        case PixelFormat::kR16G16B16A16_Float:
        case PixelFormat::kR16G16B16A16_UNorm:
        case PixelFormat::kR16G16B16A16_UInt:
        case PixelFormat::kR16G16B16A16_SNorm:
        case PixelFormat::kR16G16B16A16_SInt:
        case PixelFormat::kR10G10B10A2_UNorm:
        case PixelFormat::kR10G10B10A2_UInt:
        case PixelFormat::kR8G8B8A8_Typeless:
        case PixelFormat::kR8G8B8A8_UNorm:
        case PixelFormat::kR8G8B8A8_UNorm_sRGB:
        case PixelFormat::kR8G8B8A8_UInt:
        case PixelFormat::kR8G8B8A8_SNorm:
        case PixelFormat::kR8G8B8A8_SInt:
        case PixelFormat::kR8G8_B8G8_UNorm:
        case PixelFormat::kG8R8_G8B8_UNorm:
        case PixelFormat::kBC1_Typeless:
        case PixelFormat::kBC1_UNorm:
        case PixelFormat::kBC1_UNorm_sRGB:
        case PixelFormat::kBC2_Typeless:
        case PixelFormat::kBC2_UNorm:
        case PixelFormat::kBC2_UNorm_sRGB:
        case PixelFormat::kBC3_Typeless:
        case PixelFormat::kBC3_UNorm:
        case PixelFormat::kBC3_UNorm_sRGB:
        case PixelFormat::kB5G5R5A1_UNorm:
        case PixelFormat::kB8G8R8A8_UNorm:
        case PixelFormat::kB8G8R8X8_UNorm:
        case PixelFormat::kR10G10B10_Xr_Bias_A2_UNorm:
        case PixelFormat::kB8G8R8A8_Typeless:
        case PixelFormat::kB8G8R8A8_UNorm_sRGB:
        case PixelFormat::kB8G8R8X8_Typeless:
        case PixelFormat::kB8G8R8X8_UNorm_sRGB:
        case PixelFormat::kASTC_4x4_UNorm:
        case PixelFormat::kASTC_4x4_UNorm_sRGB:
        case PixelFormat::kASTC_6x6_UNorm:
        case PixelFormat::kASTC_6x6_UNorm_sRGB:
        case PixelFormat::kASTC_8x8_UNorm:
        case PixelFormat::kASTC_8x8_UNorm_sRGB:
        case PixelFormat::kASTC_10x10_UNorm:
        case PixelFormat::kASTC_10x10_UNorm_sRGB:
            return 4;
        case PixelFormat::kR32G32B32_Typeless:
        case PixelFormat::kR32G32B32_Float:
        case PixelFormat::kR32G32B32_UInt:
        case PixelFormat::kR32G32B32_SInt:
        case PixelFormat::kR11G11B10_Float:
        case PixelFormat::kR9G9B9E5_SharedExp:
        case PixelFormat::kB5G6R5_UNorm:
            return 3;
        case PixelFormat::kR32G32_Typeless:
        case PixelFormat::kR32G32_Float:
        case PixelFormat::kR32G32_UInt:
        case PixelFormat::kR32G32_SInt:
        case PixelFormat::kR32G8X24_Typeless:
        case PixelFormat::kR16G16_Typeless:
        case PixelFormat::kR16G16_Float:
        case PixelFormat::kR16G16_UNorm:
        case PixelFormat::kR16G16_UInt:
        case PixelFormat::kR16G16_SNorm:
        case PixelFormat::kR16G16_SInt:
        case PixelFormat::kR24G8_Typeless:
        case PixelFormat::kR8G8_Typeless:
        case PixelFormat::kR8G8_UNorm:
        case PixelFormat::kR8G8_UInt:
        case PixelFormat::kR8G8_SNorm:
        case PixelFormat::kR8G8_SInt:
        case PixelFormat::kBC5_Typeless:
        case PixelFormat::kBC5_UNorm:
        case PixelFormat::kBC5_SNorm:
            return 2;
        case PixelFormat::kR32_Typeless:
        case PixelFormat::kD32_Float:
        case PixelFormat::kR32_Float:
        case PixelFormat::kR32_UInt:
        case PixelFormat::kR32_SInt:
        case PixelFormat::kD24_UNorm_S8_UInt:
        case PixelFormat::kR24_UNorm_X8_Typeless:
        case PixelFormat::kX24_Typeless_G8_UInt:
        case PixelFormat::kR16_Typeless:
        case PixelFormat::kR16_Float:
        case PixelFormat::kD16_UNorm:
        case PixelFormat::kR16_UNorm:
        case PixelFormat::kR16_UInt:
        case PixelFormat::kR16_SNorm:
        case PixelFormat::kR16_SInt:
        case PixelFormat::kR8_Typeless:
        case PixelFormat::kR8_UNorm:
        case PixelFormat::kR8_UInt:
        case PixelFormat::kR8_SNorm:
        case PixelFormat::kR8_SInt:
        case PixelFormat::kA8_UNorm:
        case PixelFormat::kR1_UNorm:
        case PixelFormat::kBC4_Typeless:
        case PixelFormat::kBC4_UNorm:
        case PixelFormat::kBC4_SNorm:
            return 1;
        default:
            return 0;
        }
    }

    i32 GfxPixelFormat::computeBlockSize(PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kBC1_Typeless:
        case PixelFormat::kBC1_UNorm:
        case PixelFormat::kBC1_UNorm_sRGB:
        case PixelFormat::kBC2_Typeless:
        case PixelFormat::kBC2_UNorm:
        case PixelFormat::kBC2_UNorm_sRGB:
        case PixelFormat::kBC3_Typeless:
        case PixelFormat::kBC3_UNorm:
        case PixelFormat::kBC3_UNorm_sRGB:
        case PixelFormat::kBC4_Typeless:
        case PixelFormat::kBC4_UNorm:
        case PixelFormat::kBC4_SNorm:
        case PixelFormat::kBC5_Typeless:
        case PixelFormat::kBC5_UNorm:
        case PixelFormat::kBC5_SNorm:
        case PixelFormat::kBC6H_Typeless:
        case PixelFormat::kBC6H_Uf16:
        case PixelFormat::kBC6H_Sf16:
        case PixelFormat::kBC7_Typeless:
        case PixelFormat::kBC7_UNorm:
        case PixelFormat::kBC7_UNorm_sRGB:
        case PixelFormat::kASTC_4x4_UNorm:
        case PixelFormat::kASTC_4x4_UNorm_sRGB:
            return 4;
        case PixelFormat::kASTC_6x6_UNorm:
        case PixelFormat::kASTC_6x6_UNorm_sRGB:
            return 6;
        case PixelFormat::kASTC_8x8_UNorm:
        case PixelFormat::kASTC_8x8_UNorm_sRGB:
            return 8;
        case PixelFormat::kASTC_10x10_UNorm:
        case PixelFormat::kASTC_10x10_UNorm_sRGB:
            return 10;
        default:
            return 1;
        }
    }

    PixelFormat GfxPixelFormat::tosRGB(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kR8G8B8A8_UNorm:
            return PixelFormat::kR8G8B8A8_UNorm_sRGB;
        case PixelFormat::kBC1_UNorm:
            return PixelFormat::kBC1_UNorm_sRGB;
        case PixelFormat::kBC2_UNorm:
            return PixelFormat::kBC2_UNorm_sRGB;
        case PixelFormat::kBC3_UNorm:
            return PixelFormat::kBC3_UNorm_sRGB;
        case PixelFormat::kB8G8R8A8_UNorm:
            return PixelFormat::kB8G8R8A8_UNorm_sRGB;
        case PixelFormat::kB8G8R8X8_UNorm:
            return PixelFormat::kB8G8R8X8_UNorm_sRGB;
        case PixelFormat::kBC7_UNorm:
            return PixelFormat::kBC7_UNorm_sRGB;
        case PixelFormat::kASTC_4x4_UNorm:
            return PixelFormat::kASTC_4x4_UNorm_sRGB;
        case PixelFormat::kASTC_6x6_UNorm:
            return PixelFormat::kASTC_6x6_UNorm_sRGB;
        case PixelFormat::kASTC_8x8_UNorm:
            return PixelFormat::kASTC_8x8_UNorm_sRGB;
        case PixelFormat::kASTC_10x10_UNorm:
            return PixelFormat::kASTC_10x10_UNorm_sRGB;
        default:
            return format;
        }
    }

    PixelFormat GfxPixelFormat::toNonsRGB(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kR8G8B8A8_UNorm_sRGB:
            return PixelFormat::kR8G8B8A8_UNorm;
        case PixelFormat::kBC1_UNorm_sRGB:
            return PixelFormat::kBC1_UNorm;
        case PixelFormat::kBC2_UNorm_sRGB:
            return PixelFormat::kBC2_UNorm;
        case PixelFormat::kBC3_UNorm_sRGB:
            return PixelFormat::kBC3_UNorm;
        case PixelFormat::kB8G8R8A8_UNorm_sRGB:
            return PixelFormat::kB8G8R8A8_UNorm;
        case PixelFormat::kB8G8R8X8_UNorm_sRGB:
            return PixelFormat::kB8G8R8X8_UNorm;
        case PixelFormat::kBC7_UNorm_sRGB:
            return PixelFormat::kBC7_UNorm;
        case PixelFormat::kASTC_4x4_UNorm_sRGB:
            return PixelFormat::kASTC_4x4_UNorm;
        case PixelFormat::kASTC_6x6_UNorm_sRGB:
            return PixelFormat::kASTC_6x6_UNorm;
        case PixelFormat::kASTC_8x8_UNorm_sRGB:
            return PixelFormat::kASTC_8x8_UNorm;
        case PixelFormat::kASTC_10x10_UNorm_sRGB:
            return PixelFormat::kASTC_10x10_UNorm;
        default:
            return format;
        }
    }

    PixelFormat GfxPixelFormat::makeTypeless(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kR32G32B32A32_Float:
        case PixelFormat::kR32G32B32A32_UInt:
        case PixelFormat::kR32G32B32A32_SInt:
            return PixelFormat::kR32G32B32A32_Typeless;
        case PixelFormat::kR32G32B32_Float:
        case PixelFormat::kR32G32B32_UInt:
        case PixelFormat::kR32G32B32_SInt:
            return PixelFormat::kR32G32B32_Typeless;
        case PixelFormat::kR16G16B16A16_Float:
        case PixelFormat::kR16G16B16A16_UNorm:
        case PixelFormat::kR16G16B16A16_UInt:
        case PixelFormat::kR16G16B16A16_SNorm:
        case PixelFormat::kR16G16B16A16_SInt:
            return PixelFormat::kR16G16B16A16_Typeless;
        case PixelFormat::kR32G32_Float:
        case PixelFormat::kR32G32_UInt:
        case PixelFormat::kR32G32_SInt:
            return PixelFormat::kR32G32_Typeless;
        case PixelFormat::kR10G10B10A2_UNorm:
        case PixelFormat::kR10G10B10A2_UInt:
            return PixelFormat::kR10G10B10A2_Typeless;
        case PixelFormat::kR8G8B8A8_UNorm:
        case PixelFormat::kR8G8B8A8_UNorm_sRGB:
        case PixelFormat::kR8G8B8A8_UInt:
        case PixelFormat::kR8G8B8A8_SNorm:
        case PixelFormat::kR8G8B8A8_SInt:
            return PixelFormat::kR8G8B8A8_Typeless;
        case PixelFormat::kR16G16_Float:
        case PixelFormat::kR16G16_UNorm:
        case PixelFormat::kR16G16_UInt:
        case PixelFormat::kR16G16_SNorm:
        case PixelFormat::kR16G16_SInt:
            return PixelFormat::kR16G16_Typeless;
        case PixelFormat::kD32_Float:
        case PixelFormat::kR32_Float:
        case PixelFormat::kR32_UInt:
        case PixelFormat::kR32_SInt:
            return PixelFormat::kR32_Typeless;
        case PixelFormat::kR8G8_UNorm:
        case PixelFormat::kR8G8_UInt:
        case PixelFormat::kR8G8_SNorm:
        case PixelFormat::kR8G8_SInt:
            return PixelFormat::kR8G8_Typeless;
        case PixelFormat::kR16_Float:
        case PixelFormat::kD16_UNorm:
        case PixelFormat::kR16_UNorm:
        case PixelFormat::kR16_UInt:
        case PixelFormat::kR16_SNorm:
        case PixelFormat::kR16_SInt:
            return PixelFormat::kR16_Typeless;
        case PixelFormat::kR8_UNorm:
        case PixelFormat::kR8_UInt:
        case PixelFormat::kR8_SNorm:
        case PixelFormat::kR8_SInt:
            return PixelFormat::kR8_Typeless;
        case PixelFormat::kBC1_UNorm:
        case PixelFormat::kBC1_UNorm_sRGB:
            return PixelFormat::kBC1_Typeless;
        case PixelFormat::kBC2_UNorm:
        case PixelFormat::kBC2_UNorm_sRGB:
            return PixelFormat::kBC2_Typeless;
        case PixelFormat::kBC3_UNorm:
        case PixelFormat::kBC3_UNorm_sRGB:
            return PixelFormat::kBC3_Typeless;
        case PixelFormat::kBC4_UNorm:
        case PixelFormat::kBC4_SNorm:
            return PixelFormat::kBC4_Typeless;
        case PixelFormat::kBC5_UNorm:
        case PixelFormat::kBC5_SNorm:
            return PixelFormat::kBC5_Typeless;
        case PixelFormat::kB8G8R8A8_UNorm:
        case PixelFormat::kB8G8R8A8_UNorm_sRGB:
            return PixelFormat::kB8G8R8A8_Typeless;
        case PixelFormat::kB8G8R8X8_UNorm:
        case PixelFormat::kB8G8R8X8_UNorm_sRGB:
            return PixelFormat::kB8G8R8X8_Typeless;
        case PixelFormat::kBC6H_Uf16:
        case PixelFormat::kBC6H_Sf16:
            return PixelFormat::kBC6H_Typeless;
        case PixelFormat::kBC7_UNorm:
        case PixelFormat::kBC7_UNorm_sRGB:
            return PixelFormat::kBC7_Typeless;
        case PixelFormat::kD24_UNorm_S8_UInt:
            return PixelFormat::kR24G8_Typeless;
        case PixelFormat::kD32_Float_S8X24_UInt:
            return PixelFormat::kR32G8X24_Typeless;
        default:
            return format;
        }
    }

    PixelFormat GfxPixelFormat::makeTypelessFloat(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kR32G32B32A32_Typeless:
            return PixelFormat::kR32G32B32A32_Float;
        case PixelFormat::kR32G32B32_Typeless:
            return PixelFormat::kR32G32B32_Float;
        case PixelFormat::kR16G16B16A16_Typeless:
            return PixelFormat::kR16G16B16A16_Float;
        case PixelFormat::kR32G32_Typeless:
            return PixelFormat::kR32G32_Float;
        case PixelFormat::kR16G16_Typeless:
            return PixelFormat::kR16G16_Float;
        case PixelFormat::kR32_Typeless:
            return PixelFormat::kR32_Float;
        case PixelFormat::kR16_Typeless:
            return PixelFormat::kR16_Float;
        default:
            return format;
        }
    }

    PixelFormat GfxPixelFormat::makeTypelessUNorm(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kR16G16B16A16_Typeless:
            return PixelFormat::kR16G16B16A16_UNorm;
        case PixelFormat::kR10G10B10A2_Typeless:
            return PixelFormat::kR10G10B10A2_UNorm;
        case PixelFormat::kR8G8B8A8_Typeless:
            return PixelFormat::kR8G8B8A8_UNorm;
        case PixelFormat::kR16G16_Typeless:
            return PixelFormat::kR16G16_UNorm;
        case PixelFormat::kR8G8_Typeless:
            return PixelFormat::kR8G8_UNorm;
        case PixelFormat::kR16_Typeless:
            return PixelFormat::kR16_UNorm;
        case PixelFormat::kR8_Typeless:
            return PixelFormat::kR8_UNorm;
        case PixelFormat::kBC1_Typeless:
            return PixelFormat::kBC1_UNorm;
        case PixelFormat::kBC2_Typeless:
            return PixelFormat::kBC2_UNorm;
        case PixelFormat::kBC3_Typeless:
            return PixelFormat::kBC3_UNorm;
        case PixelFormat::kBC4_Typeless:
            return PixelFormat::kBC4_UNorm;
        case PixelFormat::kBC5_Typeless:
            return PixelFormat::kBC5_UNorm;
        case PixelFormat::kB8G8R8A8_Typeless:
            return PixelFormat::kB8G8R8A8_UNorm;
        case PixelFormat::kB8G8R8X8_Typeless:
            return PixelFormat::kB8G8R8X8_UNorm;
        case PixelFormat::kBC7_Typeless:
            return PixelFormat::kBC7_UNorm;
        default:
            return format;
        }
    }

    PixelFormat GfxPixelFormat::findShaderResourceFormat(const PixelFormat format, bool sRGB)
    {
        if (sRGB)
        {
            switch (format)
            {
            case PixelFormat::kB8G8R8A8_Typeless:
                return PixelFormat::kB8G8R8A8_UNorm_sRGB;
            case PixelFormat::kR8G8B8A8_Typeless:
                return PixelFormat::kR8G8B8A8_UNorm_sRGB;
            case PixelFormat::kBC1_Typeless:
                return PixelFormat::kBC1_UNorm_sRGB;
            case PixelFormat::kBC2_Typeless:
                return PixelFormat::kBC2_UNorm_sRGB;
            case PixelFormat::kBC3_Typeless:
                return PixelFormat::kBC3_UNorm_sRGB;
            case PixelFormat::kBC7_Typeless:
                return PixelFormat::kBC7_UNorm_sRGB;
            default:
                break;
            }
        }
        else
        {
            switch (format)
            {
            case PixelFormat::kB8G8R8A8_Typeless:
                return PixelFormat::kB8G8R8A8_UNorm;
            case PixelFormat::kR8G8B8A8_Typeless:
                return PixelFormat::kR8G8B8A8_UNorm;
            case PixelFormat::kBC1_Typeless:
                return PixelFormat::kBC1_UNorm;
            case PixelFormat::kBC2_Typeless:
                return PixelFormat::kBC2_UNorm;
            case PixelFormat::kBC3_Typeless:
                return PixelFormat::kBC3_UNorm;
            case PixelFormat::kBC7_Typeless:
                return PixelFormat::kBC7_UNorm;
            default:
                break;
            }
        }
        switch (format)
        {
        case PixelFormat::kR24G8_Typeless:
            return PixelFormat::kR24_UNorm_X8_Typeless;
        case PixelFormat::kR32_Typeless:
            return PixelFormat::kR32_Float;
        case PixelFormat::kR16_Typeless:
            return PixelFormat::kR16_UNorm;
        case PixelFormat::kD16_UNorm:
            return PixelFormat::kR16_UNorm;
        case PixelFormat::kD24_UNorm_S8_UInt:
            return PixelFormat::kR24_UNorm_X8_Typeless;
        case PixelFormat::kD32_Float:
            return PixelFormat::kR32_Float;
        case PixelFormat::kD32_Float_S8X24_UInt:
            return PixelFormat::kR32_Float_X8X24_Typeless;
        case PixelFormat::kYUY2:
            return PixelFormat::kR8G8B8A8_UNorm;
        default:
            break;
        }
        return format;
    }

    PixelFormat GfxPixelFormat::findUnorderedAccessFormat(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kB8G8R8A8_Typeless:
            return PixelFormat::kB8G8R8A8_UNorm;
        case PixelFormat::kR8G8B8A8_Typeless:
            return PixelFormat::kR8G8B8A8_UNorm;
        case PixelFormat::kYUY2:
            return PixelFormat::kR8G8B8A8_UNorm;
        default:
            break;
        }
        return format;
    }

    PixelFormat GfxPixelFormat::findDepthStencilFormat(const PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kR24G8_Typeless:
        case PixelFormat::kR24_UNorm_X8_Typeless:
            return PixelFormat::kD24_UNorm_S8_UInt;
        case PixelFormat::kR32_Typeless:
            return PixelFormat::kD32_Float;
        case PixelFormat::kR16_Typeless:
            return PixelFormat::kD16_UNorm;
        default:
            break;
        }
        return format;
    }

    PixelFormat GfxPixelFormat::findUncompressedFormat(PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::kBC1_Typeless:
        case PixelFormat::kBC2_Typeless:
        case PixelFormat::kBC3_Typeless:
            return PixelFormat::kR8G8B8A8_Typeless;
        case PixelFormat::kBC1_UNorm:
        case PixelFormat::kBC2_UNorm:
        case PixelFormat::kBC3_UNorm:
            return PixelFormat::kR8G8B8A8_UNorm;
        case PixelFormat::kBC1_UNorm_sRGB:
        case PixelFormat::kBC2_UNorm_sRGB:
        case PixelFormat::kBC3_UNorm_sRGB:
            return PixelFormat::kR8G8B8A8_UNorm_sRGB;
        case PixelFormat::kBC4_Typeless:
            return PixelFormat::kR8_Typeless;
        case PixelFormat::kBC4_UNorm:
            return PixelFormat::kR8_UNorm;
        case PixelFormat::kBC4_SNorm:
            return PixelFormat::kR8_SNorm;
        case PixelFormat::kBC5_Typeless:
            return PixelFormat::kR16G16_Typeless;
        case PixelFormat::kBC5_UNorm:
            return PixelFormat::kR16G16_UNorm;
        case PixelFormat::kBC5_SNorm:
            return PixelFormat::kR16G16_SNorm;
        case PixelFormat::kBC7_Typeless:
        case PixelFormat::kBC6H_Typeless:
            return PixelFormat::kR16G16B16A16_Typeless;
        case PixelFormat::kBC7_UNorm:
        case PixelFormat::kBC6H_Uf16:
        case PixelFormat::kBC6H_Sf16:
            return PixelFormat::kR16G16B16A16_Float;
        case PixelFormat::kBC7_UNorm_sRGB:
            return PixelFormat::kR16G16B16A16_UNorm;
        case PixelFormat::kASTC_4x4_UNorm:
        case PixelFormat::kASTC_6x6_UNorm:
        case PixelFormat::kASTC_8x8_UNorm:
        case PixelFormat::kASTC_10x10_UNorm:
            return PixelFormat::kR8G8B8A8_UNorm;
        case PixelFormat::kASTC_4x4_UNorm_sRGB:
        case PixelFormat::kASTC_6x6_UNorm_sRGB:
        case PixelFormat::kASTC_8x8_UNorm_sRGB:
        case PixelFormat::kASTC_10x10_UNorm_sRGB:
            return PixelFormat::kR8G8B8A8_UNorm_sRGB;
        default:
            return format;
        }
    }

    static PixelFormatSampler PixelFormatSamplers[] =
        {
            {
                PixelFormat::kR32G32B32A32_Float,
                sizeof(FVec4),
                [](const void *ptr)
                {
                    return *(FVec4 *)ptr;
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(FVec4 *)ptr = value;
                },
            },
            {
                PixelFormat::kR32G32B32_Float,
                sizeof(FVec3),
                [](const void *ptr)
                {
                    return FVec4(*(FVec3 *)ptr, 1.0f);
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(FVec3 *)ptr = FVec3(value);
                },
            },
            {
                PixelFormat::kR16G16B16A16_Float,
                sizeof(Half4),
                [](const void *ptr)
                {
                    return ((Half4 *)ptr)->toFVec4();
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(Half4 *)ptr = Half4(value.x, value.y, value.z, value.w);
                },
            },
            {PixelFormat::kR16G16B16A16_UNorm,
             sizeof(RGBA16UNorm),
             [](const void *ptr)
             {
                 return ((RGBA16UNorm *)ptr)->toFVec4();
             },
             [](void *ptr, const FVec4 &value)
             {
                 *(RGBA16UNorm *)ptr = RGBA16UNorm(value.x, value.y, value.z, value.w);
             }},
            {
                PixelFormat::kR32G32_Float,
                sizeof(FVec2),
                [](const void *ptr)
                {
                    return FVec4(((FVec2 *)ptr)->x, ((FVec2 *)ptr)->y, 0.0f, 0.0f);
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(FVec2 *)ptr = FVec2(value.x, value.y);
                },
            },
            {
                PixelFormat::kR8G8B8A8_UNorm,
                sizeof(Color32),
                [](const void *ptr)
                {
                    return FVec4(Color(*(Color32 *)ptr));
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(Color32 *)ptr = Color32(value);
                },
            },
            {
                PixelFormat::kR8G8B8A8_UNorm_sRGB,
                sizeof(Color32),
                [](const void *ptr)
                {
                    return FVec4(Color::srgbToLinear(Color(*(Color32 *)ptr)));
                },
                [](void *ptr, const FVec4 &value)
                {
                    Color srgb = Color::linearToSrgb((const Color &)value);
                    *(Color32 *)ptr = Color32(srgb);
                },
            },
            {
                PixelFormat::kR8G8_UNorm,
                sizeof(u16),
                [](const void *ptr)
                {
                    const u8 *rg = (const u8 *)ptr;
                    return FVec4((float)rg[0] / kMaxU8, (float)rg[1] / kMaxU8, 0, 1);
                },
                [](void *ptr, const FVec4 &value)
                {
                    u8 *rg = (u8 *)ptr;
                    rg[0] = (u8)(value.x * kMaxU8);
                    rg[1] = (u8)(value.y * kMaxU8);
                },
            },
            {
                PixelFormat::kR16G16_Float,
                sizeof(Half2),
                [](const void *ptr)
                {
                    const FVec2 rg = ((Half2 *)ptr)->toFVec2();
                    return FVec4(rg.x, rg.y, 0, 1);
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(Half2 *)ptr = Half2(value.x, value.y);
                },
            },
            {
                PixelFormat::kR16G16_UNorm,
                sizeof(RG16UNorm),
                [](const void *ptr)
                {
                    const FVec2 rg = ((RG16UNorm *)ptr)->toFVec2();
                    return FVec4(rg.x, rg.y, 0, 1);
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(RG16UNorm *)ptr = RG16UNorm(value.x, value.y);
                },
            },
            {
                PixelFormat::kR32_Float,
                sizeof(float),
                [](const void *ptr)
                {
                    return FVec4(*(float *)ptr, 0, 0, 1);
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(float *)ptr = value.x;
                },
            },
            {
                PixelFormat::kR16_Float,
                sizeof(Half),
                [](const void *ptr)
                {
                    return FVec4(Float16Compressor::decompress(*(Half *)ptr), 0, 0, 1);
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(Half *)ptr = Float16Compressor::compress(value.x);
                },
            },
            {
                PixelFormat::kR16_UNorm,
                sizeof(u16),
                [](const void *ptr)
                {
                    return FVec4((float)*(u16 *)ptr / kMaxU16, 0, 0, 1);
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(u16 *)ptr = (u16)(value.x * kMaxU16);
                },
            },
            {
                PixelFormat::kR8_UNorm,
                sizeof(u8),
                [](const void *ptr)
                {
                    return FVec4((float)*(byte *)ptr / kMaxU8, 0, 0, 1);
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(byte *)ptr = (byte)(value.x * kMaxU8);
                },
            },
            {
                PixelFormat::kA8_UNorm,
                sizeof(u8),
                [](const void *ptr)
                {
                    return FVec4(0, 0, 0, (float)*(byte *)ptr / kMaxU8);
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(byte *)ptr = (byte)(value.w * kMaxU8);
                },
            },
            {
                PixelFormat::kR32_UInt,
                sizeof(u32),
                [](const void *ptr)
                {
                    return FVec4((float)*(u32 *)ptr, 0, 0, 1);
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(u32 *)ptr = (u32)value.x;
                },
            },
            {
                PixelFormat::kR32_SInt,
                sizeof(i32),
                [](const void *ptr)
                {
                    return FVec4((float)*(i32 *)ptr, 0, 0, 1);
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(i32 *)ptr = (i32)value.x;
                },
            },
            {
                PixelFormat::kR16_UInt,
                sizeof(u16),
                [](const void *ptr)
                {
                    return FVec4((float)*(u16 *)ptr, 0, 0, 1);
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(u16 *)ptr = (u16)value.x;
                },
            },
            {
                PixelFormat::kR16_SInt,
                sizeof(i16),
                [](const void *ptr)
                {
                    return FVec4((float)*(i16 *)ptr, 0, 0, 1);
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(i16 *)ptr = (i16)value.x;
                },
            },
            {
                PixelFormat::kR8_UInt,
                sizeof(u8),
                [](const void *ptr)
                {
                    return FVec4((float)*(u8 *)ptr, 0, 0, 1);
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(u8 *)ptr = (u8)value.x;
                },
            },
            {
                PixelFormat::kR8_SInt,
                sizeof(i8),
                [](const void *ptr)
                {
                    return FVec4((float)*(i8 *)ptr, 0, 0, 1);
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(i8 *)ptr = (i8)value.x;
                },
            },
            {
                PixelFormat::kB8G8R8A8_UNorm,
                sizeof(Color32),
                [](const void *ptr)
                {
                    const Color32 bgra = *(Color32 *)ptr;
                    return FVec4(Color(Color32(bgra.b, bgra.g, bgra.r, bgra.a)));
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(Color32 *)ptr = Color32(byte(value.z * kMaxU8), byte(value.y * kMaxU8), byte(value.x * kMaxU8), byte(value.w * kMaxU8));
                },
            },
            {
                PixelFormat::kB8G8R8A8_UNorm_sRGB,
                sizeof(Color32),
                [](const void *ptr)
                {
                    const Color32 bgra = *(Color32 *)ptr;
                    return FVec4(Color::srgbToLinear(Color(Color32(bgra.b, bgra.g, bgra.r, bgra.a))));
                },
                [](void *ptr, const FVec4 &value)
                {
                    Color srgb = Color::linearToSrgb((const Color &)value);
                    *(Color32 *)ptr = Color32(byte(srgb.b * kMaxU8), byte(srgb.g * kMaxU8), byte(srgb.r * kMaxU8), byte(srgb.a * kMaxU8));
                },
            },
            {
                PixelFormat::kB8G8R8X8_UNorm,
                sizeof(Color32),
                [](const void *ptr)
                {
                    const Color32 bgra = *(Color32 *)ptr;
                    return FVec4(Color(Color32(bgra.b, bgra.g, bgra.r, kMaxU8)));
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(Color32 *)ptr = Color32(byte(value.z * kMaxU8), byte(value.y * kMaxU8), byte(value.x * kMaxU8), kMaxU8);
                },
            },
            {
                PixelFormat::kB8G8R8X8_UNorm_sRGB,
                sizeof(Color32),
                [](const void *ptr)
                {
                    const Color32 bgra = *(Color32 *)ptr;
                    return FVec4(Color::srgbToLinear(Color(Color32(bgra.b, bgra.g, bgra.r, kMaxU8))));
                },
                [](void *ptr, const FVec4 &value)
                {
                    Color srgb = Color::linearToSrgb((const Color &)value);
                    *(Color32 *)ptr = Color32(byte(srgb.b * kMaxU8), byte(srgb.g * kMaxU8), byte(srgb.r * kMaxU8), kMaxU8);
                },
            },
            {
                PixelFormat::kR11G11B10_Float,
                sizeof(FloatR11G11B10),
                [](const void *ptr)
                {
                    const FVec3 rgb = ((FloatR11G11B10 *)ptr)->toFVec3();
                    return FVec4(rgb.x, rgb.y, rgb.z, 0.0f);
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(FloatR11G11B10 *)ptr = FloatR11G11B10(value.x, value.y, value.z);
                },
            },
            {
                PixelFormat::kR10G10B10A2_UNorm,
                sizeof(FloatR10G10B10A2),
                [](const void *ptr)
                {
                    return ((FloatR10G10B10A2 *)ptr)->toFVec4();
                },
                [](void *ptr, const FVec4 &value)
                {
                    *(FloatR10G10B10A2 *)ptr = FloatR10G10B10A2(value.x, value.y, value.z, value.w);
                },
            },
            {
                PixelFormat::kR8G8B8A8_UInt,
                sizeof(Color32),
                [](const void *ptr)
                {
                    u8 data[4];
                    TkMemory::memoryCopy(data, ptr, sizeof(data));
                    return FVec4(data[0], data[1], data[2], data[3]);
                },
                [](void *ptr, const FVec4 &value)
                {
                    u8 data[4] = {(u8)value.x, (u8)value.y, (u8)value.z, (u8)value.w};
                    TkMemory::memoryCopy(ptr, data, sizeof(data));
                },
            },
            {
                PixelFormat::kR8G8B8A8_SInt,
                sizeof(Color32),
                [](const void *ptr)
                {
                    i8 data[4];
                    TkMemory::memoryCopy(data, ptr, sizeof(data));
                    return FVec4(data[0], data[1], data[2], data[3]);
                },
                [](void *ptr, const FVec4 &value)
                {
                    i8 data[4] = {(i8)value.x, (i8)value.y, (i8)value.z, (i8)value.w};
                    TkMemory::memoryCopy(ptr, data, sizeof(data));
                },
            },
            {
                PixelFormat::kR16G16B16A16_UInt,
                sizeof(u16) * 4,
                [](const void *ptr)
                {
                    u16 data[4];
                    TkMemory::memoryCopy(data, ptr, sizeof(data));
                    return FVec4(data[0], data[1], data[2], data[3]);
                },
                [](void *ptr, const FVec4 &value)
                {
                    u16 data[4] = {(u16)value.x, (u16)value.y, (u16)value.z, (u16)value.w};
                    TkMemory::memoryCopy(ptr, data, sizeof(data));
                },
            },
            {
                PixelFormat::kR16G16B16A16_SInt,
                sizeof(i16) * 4,
                [](const void *ptr)
                {
                    i16 data[4];
                    TkMemory::memoryCopy(data, ptr, sizeof(data));
                    return FVec4(data[0], data[1], data[2], data[3]);
                },
                [](void *ptr, const FVec4 &value)
                {
                    i16 data[4] = {(i16)value.x, (i16)value.y, (i16)value.z, (i16)value.w};
                    TkMemory::memoryCopy(ptr, data, sizeof(data));
                },
            },
            {
                PixelFormat::kR32G32B32A32_UInt,
                sizeof(u32) * 4,
                [](const void *ptr)
                {
                    u32 data[4];
                    TkMemory::memoryCopy(data, ptr, sizeof(data));
                    return FVec4((float)data[0], (float)data[1], (float)data[2], (float)data[3]);
                },
                [](void *ptr, const FVec4 &value)
                {
                    u32 data[4] = {(u32)value.x, (u32)value.y, (u32)value.z, (u32)value.w};
                    TkMemory::memoryCopy(ptr, data, sizeof(data));
                },
            },
            {
                PixelFormat::kR32G32B32A32_SInt,
                sizeof(i32) * 4,
                [](const void *ptr)
                {
                    i32 data[4];
                    TkMemory::memoryCopy(data, ptr, sizeof(data));
                    return FVec4((float)data[0], (float)data[1], (float)data[2], (float)data[3]);
                },
                [](void *ptr, const FVec4 &value)
                {
                    i32 data[4] = {(i32)value.x, (i32)value.y, (i32)value.z, (i32)value.w};
                    TkMemory::memoryCopy(ptr, data, sizeof(data));
                },
            },
    };

    void PixelFormatSampler::store(void *data, i32 x, i32 y, i32 rowPitch, const Color &color) const
    {
        Write((byte *)data + rowPitch * y + PixelSize * x, (FVec4 &)color);
    }

    FVec4 PixelFormatSampler::sample(const void *data, i32 x) const
    {
        return Read((const byte *)data + x * PixelSize);
    }

    Color PixelFormatSampler::samplePoint(const void *data, const FVec2 &uv, const IVec2 &size, i32 rowPitch) const
    {
        const IVec2 end = size - 1;
        const IVec2 uvFloor(TkMath::min(TkMath::floorToInt(uv.x * size.x), end.x), TkMath::min(TkMath::floorToInt(uv.y * size.y), end.y));
        FVec4 result = Read((const byte *)data + rowPitch * uvFloor.y + PixelSize * uvFloor.x);
        return *(Color *)&result;
    }

    Color PixelFormatSampler::samplePoint(const void *data, i32 x, i32 y, i32 rowPitch) const
    {
        FVec4 result = Read((const byte *)data + rowPitch * y + PixelSize * x);
        return *(Color *)&result;
    }

    Color PixelFormatSampler::sampleLinear(const void *data, const FVec2 &uv, const IVec2 &size, i32 rowPitch) const
    {
        const IVec2 end = size - 1;
        const IVec2 uvFloor(TkMath::min(TkMath::floorToInt(uv.x * size.x), end.x), TkMath::min(TkMath::floorToInt(uv.y * size.y), end.y));
        const IVec2 uvNext(TkMath::min(uvFloor.x + 1, end.x), TkMath::min(uvFloor.y + 1, end.y));
        const FVec2 uvFraction(uv.x * size.y - uvFloor.x, uv.y * size.y - uvFloor.y);

        const FVec4 v00 = Read((const byte *)data + rowPitch * uvFloor.y + PixelSize * uvFloor.x);
        const FVec4 v01 = Read((const byte *)data + rowPitch * uvFloor.y + PixelSize * uvNext.x);
        const FVec4 v10 = Read((const byte *)data + rowPitch * uvNext.y + PixelSize * uvFloor.x);
        const FVec4 v11 = Read((const byte *)data + rowPitch * uvNext.y + PixelSize * uvNext.x);

        FVec4 result = FVec4::lerp(FVec4::lerp(v00, v01, uvFraction.x), FVec4::Lerp(v10, v11, uvFraction.x), uvFraction.y);
        return *(Color *)&result;
    }

    const PixelFormatSampler *PixelFormatSampler::get(PixelFormat format)
    {
        format = GfxPixelFormat::makeTypelessFloat(format);
        for (const auto &sampler : PixelFormatSamplers)
        {
            if (sampler.Format == format)
                return &sampler;
        }
        return nullptr;
    }

}