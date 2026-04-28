/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * Reference:
 *     https://msdn.microsoft.com/en-us/library/windows/desktop/bb173059(v=vs.85).aspx
 */

#pragma once

#include <toolkit/graphics/common.hpp>

namespace toolkit
{
    // Enumeration with all pixel formats used by the graphics pipeline.
    enum class PixelFormat : u32
    {
        kUnknown = 0,
        // A four-component, 128-bit typeless format that supports 32 bits per channel including alpha.
        kR32G32B32A32_Typeless = 1,
        // A four-component, 128-bit floating-point format that supports 32 bits per channel including alpha.
        kR32G32B32A32_Float = 2,
        // A four-component, 128-bit unsigned-integer format that supports 32 bits per channel including alpha.
        kR32G32B32A32_UInt = 3,
        // A four-component, 128-bit signed-integer format that supports 32 bits per channel including alpha.
        kR32G32B32A32_SInt = 4,
        // A three-component, 96-bit typeless format that supports 32 bits per color channel.
        kR32G32B32_Typeless = 5,
        // A three-component, 96-bit floating-point format that supports 32 bits per color channel.
        kR32G32B32_Float = 6,
        // A three-component, 96-bit unsigned-integer format that supports 32 bits per color channel.
        kR32G32B32_UInt = 7,
        // A three-component, 96-bit signed-integer format that supports 32 bits per color channel.
        kR32G32B32_SInt = 8,
        // A four-component, 64-bit typeless format that supports 16 bits per channel including alpha.
        kR16G16B16A16_Typeless = 9,
        // A four-component, 64-bit floating-point format that supports 16 bits per channel including alpha.
        kR16G16B16A16_Float = 10,
        // A four-component, 64-bit unsigned-normalized-integer format that supports 16 bits per channel including alpha.
        kR16G16B16A16_UNorm = 11,
        // A four-component, 64-bit unsigned-integer format that supports 16 bits per channel including alpha.
        kR16G16B16A16_UInt = 12,
        // A four-component, 64-bit signed-normalized-integer format that supports 16 bits per channel including alpha.
        kR16G16B16A16_SNorm = 13,
        // A four-component, 64-bit signed-integer format that supports 16 bits per channel including alpha.
        kR16G16B16A16_SInt = 14,
        // A two-component, 64-bit typeless format that supports 32 bits for the red channel and 32 bits for the green channel.
        kR32G32_Typeless = 15,
        // A two-component, 64-bit floating-point format that supports 32 bits for the red channel and 32 bits for the green channel.
        kR32G32_Float = 16,
        // A two-component, 64-bit unsigned-integer format that supports 32 bits for the red channel and 32 bits for the green channel.
        kR32G32_UInt = 17,
        // A two-component, 64-bit signed-integer format that supports 32 bits for the red channel and 32 bits for the green channel.
        kR32G32_SInt = 18,
        // A two-component, 64-bit typeless format that supports 32 bits for the red channel, 8 bits for the green channel, and 24 bits are unused.
        kR32G8X24_Typeless = 19,
        // A 32-bit floating-point component, and two unsigned-integer components (with an additional 32 bits). This format supports 32-bit depth, 8-bit stencil, and 24 bits are unused.
        kD32_Float_S8X24_UInt = 20,
        // A 32-bit floating-point component, and two typeless components (with an additional 32 bits). This format supports 32-bit red channel, 8 bits are unused, and 24 bits are unused.
        kR32_Float_X8X24_Typeless = 21,
        // A 32-bit typeless component, and two unsigned-integer components (with an additional 32 bits). This format has 32 bits unused, 8 bits for green channel, and 24 bits are unused.
        kX32_Typeless_G8X24_UInt = 22,
        // A four-component, 32-bit typeless format that supports 10 bits for each color and 2 bits for alpha.
        kR10G10B10A2_Typeless = 23,
        // A four-component, 32-bit unsigned-normalized-integer format that supports 10 bits for each color and 2 bits for alpha.
        kR10G10B10A2_UNorm = 24,
        // A four-component, 32-bit unsigned-integer format that supports 10 bits for each color and 2 bits for alpha.
        kR10G10B10A2_UInt = 25,
        // Three partial-precision floating-point numbers encoded into a single 32-bit value (a variant of s10e5, which is sign bit, 10-bit mantissa, and 5-bit biased (15) exponent). There are no sign bits, and there is a 5-bit biased (15) exponent for each channel, 6-bit mantissa for R and G, and a 5-bit mantissa for B.
        kR11G11B10_Float = 26,
        // A four-component, 32-bit typeless format that supports 8 bits per channel including alpha.
        kR8G8B8A8_Typeless = 27,
        // A four-component, 32-bit unsigned-normalized-integer format that supports 8 bits per channel including alpha.
        kR8G8B8A8_UNorm = 28,
        // A four-component, 32-bit unsigned-normalized integer sRGB format that supports 8 bits per channel including alpha.
        kR8G8B8A8_UNorm_sRGB = 29,
        // A four-component, 32-bit unsigned-integer format that supports 8 bits per channel including alpha.
        kR8G8B8A8_UInt = 30,
        // A four-component, 32-bit signed-normalized-integer format that supports 8 bits per channel including alpha.
        kR8G8B8A8_SNorm = 31,
        // A four-component, 32-bit signed-integer format that supports 8 bits per channel including alpha.
        kR8G8B8A8_SInt = 32,
        // A two-component, 32-bit typeless format that supports 16 bits for the red channel and 16 bits for the green channel.
        kR16G16_Typeless = 33,
        // A two-component, 32-bit floating-point format that supports 16 bits for the red channel and 16 bits for the green channel.
        kR16G16_Float = 34,
        // A two-component, 32-bit unsigned-normalized-integer format that supports 16 bits each for the green and red channels.
        kR16G16_UNorm = 35,
        // A two-component, 32-bit unsigned-integer format that supports 16 bits for the red channel and 16 bits for the green channel.
        kR16G16_UInt = 36,
        // A two-component, 32-bit signed-normalized-integer format that supports 16 bits for the red channel and 16 bits for the green channel.
        kR16G16_SNorm = 37,
        // A two-component, 32-bit signed-integer format that supports 16 bits for the red channel and 16 bits for the green channel.
        kR16G16_SInt = 38,
        // A single-component, 32-bit typeless format that supports 32 bits for the red channel.
        kR32_Typeless = 39,
        // A single-component, 32-bit floating-point format that supports 32 bits for depth.
        kD32_Float = 40,
        // A single-component, 32-bit floating-point format that supports 32 bits for the red channel.
        kR32_Float = 41,
        // A single-component, 32-bit unsigned-integer format that supports 32 bits for the red channel.
        kR32_UInt = 42,
        // A single-component, 32-bit signed-integer format that supports 32 bits for the red channel.
        kR32_SInt = 43,
        // A two-component, 32-bit typeless format that supports 24 bits for the red channel and 8 bits for the green channel.
        kR24G8_Typeless = 44,
        // A 32-bit z-buffer format that supports 24 bits for depth and 8 bits for stencil.
        kD24_UNorm_S8_UInt = 45,
        // A 32-bit format, that contains a 24 bit, single-component, unsigned-normalized integer, with an additional typeless 8 bits. This format has 24 bits red channel and 8 bits unused.
        kR24_UNorm_X8_Typeless = 46,
        // A 32-bit format, that contains a 24 bit, single-component, typeless format, with an additional 8 bit unsigned integer component. This format has 24 bits unused and 8 bits green channel.
        kX24_Typeless_G8_UInt = 47,
        // A two-component, 16-bit typeless format that supports 8 bits for the red channel and 8 bits for the green channel.
        kR8G8_Typeless = 48,
        // A two-component, 16-bit unsigned-normalized-integer format that supports 8 bits for the red channel and 8 bits for the green channel.
        kR8G8_UNorm = 49,
        // A two-component, 16-bit unsigned-integer format that supports 8 bits for the red channel and 8 bits for the green channel.
        kR8G8_UInt = 50,
        // A two-component, 16-bit signed-normalized-integer format that supports 8 bits for the red channel and 8 bits for the green channel.
        kR8G8_SNorm = 51,
        // A two-component, 16-bit signed-integer format that supports 8 bits for the red channel and 8 bits for the green channel.
        kR8G8_SInt = 52,
        // A single-component, 16-bit typeless format that supports 16 bits for the red channel.
        kR16_Typeless = 53,
        // A single-component, 16-bit floating-point format that supports 16 bits for the red channel.
        kR16_Float = 54,
        // A single-component, 16-bit unsigned-normalized-integer format that supports 16 bits for depth.
        kD16_UNorm = 55,
        // A single-component, 16-bit unsigned-normalized-integer format that supports 16 bits for the red channel.
        kR16_UNorm = 56,
        // A single-component, 16-bit unsigned-integer format that supports 16 bits for the red channel.
        kR16_UInt = 57,
        // A single-component, 16-bit signed-normalized-integer format that supports 16 bits for the red channel.
        kR16_SNorm = 58,
        // A single-component, 16-bit signed-integer format that supports 16 bits for the red channel.
        kR16_SInt = 59,
        // A single-component, 8-bit typeless format that supports 8 bits for the red channel.
        kR8_Typeless = 60,
        // A single-component, 8-bit unsigned-normalized-integer format that supports 8 bits for the red channel.
        kR8_UNorm = 61,
        // A single-component, 8-bit unsigned-integer format that supports 8 bits for the red channel.
        kR8_UInt = 62,
        // A single-component, 8-bit signed-normalized-integer format that supports 8 bits for the red channel.
        kR8_SNorm = 63,
        // A single-component, 8-bit signed-integer format that supports 8 bits for the red channel.
        kR8_SInt = 64,
        // A single-component, 8-bit unsigned-normalized-integer format for alpha only.
        kA8_UNorm = 65,
        // A single-component, 1-bit unsigned-normalized integer format that supports 1 bit for the red channel.
        kR1_UNorm = 66,
        // Three partial-precision floating-point numbers encoded into a single 32-bit value all sharing the same 5-bit exponent (variant of s10e5, which is sign bit, 10-bit mantissa, and 5-bit biased (15) exponent). There is no sign bit, and there is a shared 5-bit biased (15) exponent and a 9-bit mantissa for each channel.
        kR9G9B9E5_SharedExp = 67,
        // A four-component, 32-bit unsigned-normalized-integer format. This packed RGB format is analogous to the UYVY format. Each 32-bit block describes a pair of pixels: (R8, G8, B8) and (R8, G8, B8) where the R8/B8 values are repeated, and the G8 values are unique to each pixel. Width must be even.
        kR8G8_B8G8_UNorm = 68,
        // A four-component, 32-bit unsigned-normalized-integer format. This packed RGB format is analogous to the YUY2 format. Each 32-bit block describes a pair of pixels: (R8, G8, B8) and (R8, G8, B8) where the R8/B8 values are repeated, and the G8 values are unique to each pixel. Width must be even.
        kG8R8_G8B8_UNorm = 69,
        // Four-component typeless block-compression format
        kBC1_Typeless = 70,
        // Four-component block-compression format.
        kBC1_UNorm = 71,
        // Four-component block-compression format for sRGB data.
        kBC1_UNorm_sRGB = 72,
        // Four-component typeless block-compression format.
        kBC2_Typeless = 73,
        // Four-component block-compression format.
        kBC2_UNorm = 74,
        // Four-component block-compression format for sRGB data.
        kBC2_UNorm_sRGB = 75,
        // Four-component typeless block-compression format.
        kBC3_Typeless = 76,
        // Four-component block-compression format.
        kBC3_UNorm = 77,
        // Four-component block-compression format for sRGB data.
        kBC3_UNorm_sRGB = 78,
        // One-component typeless block-compression format.
        kBC4_Typeless = 79,
        // One-component block-compression format.
        kBC4_UNorm = 80,
        // One-component block-compression format.
        kBC4_SNorm = 81,
        // Two-component typeless block-compression format.
        kBC5_Typeless = 82,
        // Two-component block-compression format.
        kBC5_UNorm = 83,
        // Two-component block-compression format.
        kBC5_SNorm = 84,
        // A three-component, 16-bit unsigned-normalized-integer format that supports 5 bits for blue, 6 bits for green, and 5 bits for red.
        kB5G6R5_UNorm = 85,
        // A four-component, 16-bit unsigned-normalized-integer format that supports 5 bits for each color channel and 1-bit alpha.
        kB5G5R5A1_UNorm = 86,
        // A four-component, 32-bit unsigned-normalized-integer format that supports 8 bits for each color channel and 8-bit alpha.
        kB8G8R8A8_UNorm = 87,
        // A four-component, 32-bit unsigned-normalized-integer format that supports 8 bits for each color channel and 8 bits unused.
        kB8G8R8X8_UNorm = 88,
        // A four-component, 32-bit 2.8-biased fixed-point format that supports 10 bits for each color channel and 2-bit alpha.
        kR10G10B10_Xr_Bias_A2_UNorm = 89,
        // A four-component, 32-bit typeless format that supports 8 bits for each channel including alpha.
        kB8G8R8A8_Typeless = 90,
        // A four-component, 32-bit unsigned-normalized standard RGB format that supports 8 bits for each channel including alpha.
        kB8G8R8A8_UNorm_sRGB = 91,
        // A four-component, 32-bit typeless format that supports 8 bits for each color channel, and 8 bits are unused.
        kB8G8R8X8_Typeless = 92,
        // A four-component, 32-bit unsigned-normalized standard RGB format that supports 8 bits for each color channel, and 8 bits are unused.
        kB8G8R8X8_UNorm_sRGB = 93,
        // A three-component, HDR floating point format for color channels format that supports RGB channels. There is no support for an alpha channel.
        kBC6H_Typeless = 94,
        // A three-component, HDR unsigned floating point format for color channels format that supports RGB channels. There is no support for an alpha channel.
        kBC6H_Uf16 = 95,
        // A three-component, HDR signed floating point format for color channels format that supports RGB channels. There is no support for an alpha channel.
        kBC6H_Sf16 = 96,
        // A four-component, HDR floating point format for color channels format that supports RGBA channels.
        kBC7_Typeless = 97,
        // A four-component, HDR floating point format for color channels format that supports RGBA channels.
        kBC7_UNorm = 98,
        // A four-component, HDR floating point format for color channels format that supports RGBA channels.
        kBC7_UNorm_sRGB = 99,
        // A four-component ASTC (4x4 pixel block in 128 bits) block-compression format that supports RGBA channels.
        kASTC_4x4_UNorm = 100,
        // A four-component ASTC (4x4 pixel block in 128 bits) block-compression format that supports RGBA channels. Data in sRGB color space.
        kASTC_4x4_UNorm_sRGB = 101,
        // A four-component ASTC (6x6 pixel block in 128 bits) block-compression format that supports RGBA channels.
        kASTC_6x6_UNorm = 102,
        // A four-component ASTC (6x6 pixel block in 128 bits) block-compression format that supports RGBA channels. Data in sRGB color space.
        kASTC_6x6_UNorm_sRGB = 103,
        // A four-component ASTC (8x8 pixel block in 128 bits) block-compression format that supports RGBA channels.
        kASTC_8x8_UNorm = 104,
        // A four-component ASTC (8x8 pixel block in 128 bits) block-compression format that supports RGBA channels. Data in sRGB color space.
        kASTC_8x8_UNorm_sRGB = 105,
        // A four-component ASTC (10x10 pixel block in 128 bits) block-compression format that supports RGBA channels.
        kASTC_10x10_UNorm = 106,
        // A four-component ASTC (10x10 pixel block in 128 bits) block-compression format that supports RGBA channels. Data in sRGB color space.
        kASTC_10x10_UNorm_sRGB = 107,
        // Packed YUV 4:2:2 video texture format. The mapping to the view channel in shader is Y0->R8, U0->G8, Y1->B8, and V0->A8.
        kYUY2 = 108,
        // Packed YUV 4:2:0 video texture format. Texture uses separate views to access the luma (Y) plane separately from the chroma (UV) planes. For luminance data view, the mapping to the view channel is Y->R8. For chrominance data view, the mapping to the view channel is U->R8 and V->G8.
        kNV12 = 109,
        // The maximum format value (for internal use only).
        MAX
    };

    // Utils for pixel format.
    class GfxPixelFormat
    {
    public:
        // init pixel format cache.
        static void init();

    public:
        // Calculate the size of a in bytes.
        TK_FORCE_INLINE static i32 sizeInBytes(const PixelFormat format)
        {
            return sizeInBits(format) / 8;
        }
        // Calculate the size of a in bits.
        static i32 sizeInBits(PixelFormat format);

        // Calculate the size of the alpha channel in bits depending on the pixel format.
        static i32 alphaSizeInBits(PixelFormat format);

        // Determine whether the specified contains alpha channel.
        TK_FORCE_INLINE static bool hasAlpha(const PixelFormat format)
        {
            return alphaSizeInBits(format) != 0;
        }

        // Determine whether the specified is depth stencil.
        static bool isDepthStencil(PixelFormat format);
        // Determine whether the specified has stencil bits.
        static bool hasStencil(PixelFormat format);

        // Determine whether the specified is Typeless.
        static bool isTypeless(PixelFormat format, bool partialTypeless = true);
        // Return true if the is valid.
        static bool isValid(PixelFormat format);

        // Return true if the is a compressed format.
        static bool isCompressed(PixelFormat format);
        // Return true if the is a compressed format from BC formats family (BC1, BC2, BC3, BC4, BC5, BC6H, BC7).
        static bool isCompressedBC(PixelFormat format);
        // Return true if the is a compressed format from ASTC formats family (various block sizes).
        static bool isCompressedASTC(PixelFormat format);

        // Determine whether the specified is video.
        static bool isVideo(PixelFormat format);
        // Determine whether the specified is a sRGB format.
        static bool isSRGB(PixelFormat format);
        // Determine whether the specified is HDR (either 16 or 32bits Float)
        static bool isHDR(PixelFormat format);
        // Determine whether the specified format is in RGBA order.
        static bool isRgbAOrder(PixelFormat format);
        // Determine whether the specified format is in BGRA order.
        static bool isBGRAOrder(PixelFormat format);
        // Determine whether the specified format contains normalized data. It indicates that values stored in an integer format are to be mapped to the range [-1,1] (for signed values) or [0,1] (for unsigned values) when they are accessed and converted to floating point.
        static bool isNormalized(PixelFormat format);
        // Determine whether the specified format is integer data type (signed or unsigned).
        static bool isInteger(PixelFormat format);

        // Compute the format components count (number of R, G, B, A channels).
        static i32 computeComponentsCount(PixelFormat format);
        // Compute the amount of pixels per-axis stored in the a single block of the format (eg. 4 for BC-family). Return 1 for uncompressed formats.
        static i32 computeBlockSize(PixelFormat format);

        // Find the equivalent sRGB format to the provided format.
        static PixelFormat tosRGB(PixelFormat format);
        // Find the equivalent non sRGB format to the provided sRGB format.
        static PixelFormat toNonsRGB(PixelFormat format);

        // Convert the format to typeless.
        static PixelFormat makeTypeless(PixelFormat format);
        // Convert the typeless format to float.
        static PixelFormat makeTypelessFloat(PixelFormat format);
        // Convert the typeless format to unorm.
        static PixelFormat makeTypelessUNorm(PixelFormat format);

    public:
        static PixelFormat findShaderResourceFormat(PixelFormat format, bool sRGB);
        static PixelFormat findUnorderedAccessFormat(PixelFormat format);
        static PixelFormat findDepthStencilFormat(PixelFormat format);
        static PixelFormat findUncompressedFormat(PixelFormat format);
    };
}