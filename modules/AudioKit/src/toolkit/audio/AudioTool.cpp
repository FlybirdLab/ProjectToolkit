/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "AudioTool.hpp"

#include "toolkit/base/Assert.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/base/Math.hpp"

#define CONVERT_TO_MONO_AVG 1

namespace toolkit
{
    static void convertToMono8(const i8 *input, u8 *output, u32 numSamples, u32 numChannels)
    {
        for (u32 i = 0; i < numSamples; i++)
        {
            i16 sum = 0;
            for (u32 j = 0; j < numChannels; j++)
            {
                sum += *input;
                ++input;
            }

#if CONVERT_TO_MONO_AVG
            *output = (u8)(sum / numChannels);
#else
            *output = (u8)TkMath::clamp<i16>(sum, 0, kMaxU8);
#endif
            ++output;
        }
    }

    void ConvertToMono16(const i16 *input, i16 *output, u32 numSamples, u32 numChannels)
    {
        for (u32 i = 0; i < numSamples; i++)
        {
            i32 sum = 0;
            for (u32 j = 0; j < numChannels; j++)
            {
                sum += *input;
                ++input;
            }

#if CONVERT_TO_MONO_AVG
            *output = (i16)(sum / numChannels);
#else
            *output = (i16)TkMath::clamp<i32>(sum, MIN_i16, MAX_i16);
#endif
            ++output;
        }
    }

    static void convert32To24Bits(const i32 input, u8 *output)
    {
        const u32 valToEncode = *(u32 *)&input;
        output[0] = (valToEncode >> 8) & 0x000000FF;
        output[1] = (valToEncode >> 16) & 0x000000FF;
        output[2] = (valToEncode >> 24) & 0x000000FF;
    }

    static void convertToMono24(const u8 *input, u8 *output, u32 numSamples, u32 numChannels)
    {
        for (u32 i = 0; i < numSamples; i++)
        {
            i64 sum = 0;
            for (u32 j = 0; j < numChannels; j++)
            {
                sum += AudioTool::convert24To32Bits(input);
                input += 3;
            }

#if CONVERT_TO_MONO_AVG
            const i32 val = (i32)(sum / numChannels);
#else
            const i32 val = (i32)TkMath::clamp<i64>(sum, MIN_i16, MAX_i16);
#endif
            convert32To24Bits(val, output);
            output += 3;
        }
    }

    static void convertToMono32(const i32 *input, i32 *output, u32 numSamples, u32 numChannels)
    {
        for (u32 i = 0; i < numSamples; i++)
        {
            i64 sum = 0;
            for (u32 j = 0; j < numChannels; j++)
            {
                sum += *input;
                ++input;
            }

#if CONVERT_TO_MONO_AVG
            *output = (i32)(sum / numChannels);
#else
            *output = (i32)TkMath::clamp<i64>(sum, MIN_i16, MAX_i16);
#endif
            ++output;
        }
    }

    static void convert8To32Bits(const i8 *input, i32 *output, u32 numSamples)
    {
        for (u32 i = 0; i < numSamples; i++)
        {
            const i8 val = input[i];
            output[i] = val << 24;
        }
    }

    static void convert16To32Bits(const i16 *input, i32 *output, u32 numSamples)
    {
        for (u32 i = 0; i < numSamples; i++)
            output[i] = input[i] << 16;
    }

    static void convert24To32Bits(const u8 *input, i32 *output, u32 numSamples)
    {
        for (u32 i = 0; i < numSamples; i++)
        {
            output[i] = AudioTool::convert24To32Bits(input);
            input += 3;
        }
    }

    static void convert32To8Bits(const i32 *input, u8 *output, u32 numSamples)
    {
        for (u32 i = 0; i < numSamples; i++)
            output[i] = (i8)(input[i] >> 24);
    }

    static void convert32To16Bits(const i32 *input, i16 *output, u32 numSamples)
    {
        for (u32 i = 0; i < numSamples; i++)
            output[i] = (i16)(input[i] >> 16);
    }

    static void convert32To24Bits(const i32 *input, u8 *output, u32 numSamples)
    {
        for (u32 i = 0; i < numSamples; i++)
        {
            convert32To24Bits(input[i], output);
            output += 3;
        }
    }

    void AudioTool::convertToMono(const byte *input, byte *output, u32 bitDepth, u32 numSamples, u32 numChannels)
    {
        switch (bitDepth)
        {
        case 8:
            convertToMono8((i8 *)input, output, numSamples, numChannels);
            break;
        case 16:
            ConvertToMono16((i16 *)input, (i16 *)output, numSamples, numChannels);
            break;
        case 24:
            convertToMono24(input, output, numSamples, numChannels);
            break;
        case 32:
            convertToMono32((i32 *)input, (i32 *)output, numSamples, numChannels);
            break;
        default:
            TK_ASSERT(false);
            break;
        }
    }

    void AudioTool::convertBitDepth(const byte *input, u32 inBitDepth, byte *output, u32 outBitDepth, u32 numSamples)
    {
        i32 *srcBuffer = nullptr;

        const bool needTempBuffer = inBitDepth != 32;
        if (needTempBuffer)
            srcBuffer = (i32 *)TkMemory::allocate(TkMemory::getDefault(), numSamples * sizeof(i32));
        else
            srcBuffer = (i32 *)input;

        // Convert it to a temporary 32-bit buffer and then use that to convert to actual requested bit depth.
        // It could be more efficient to convert directly from source to requested depth without a temporary buffer,
        // at the cost of additional complexity. If this method ever becomes a performance issue consider that.

        switch (inBitDepth)
        {
        case 8:
            convert8To32Bits((i8 *)input, srcBuffer, numSamples);
            break;
        case 16:
            convert16To32Bits((i16 *)input, srcBuffer, numSamples);
            break;
        case 24:
            ::toolkit::convert24To32Bits(input, srcBuffer, numSamples);
            break;
        case 32:
            // Do nothing
            break;
        default:
            TK_ASSERT(false);
            break;
        }

        switch (outBitDepth)
        {
        case 8:
            convert32To8Bits(srcBuffer, output, numSamples);
            break;
        case 16:
            convert32To16Bits(srcBuffer, (i16 *)output, numSamples);
            break;
        case 24:
            convert32To24Bits(srcBuffer, output, numSamples);
            break;
        case 32:
            TkMemory::memoryCopy(output, srcBuffer, numSamples * sizeof(i32));
            break;
        default:
            TK_ASSERT(false);
            break;
        }

        if (needTempBuffer)
        {
            TkMemory::free(TkMemory::getDefault(), srcBuffer);
            srcBuffer = nullptr;
        }
    }

    void AudioTool::convertToFloat(const byte *input, u32 inBitDepth, float *output, u32 numSamples)
    {
        if (inBitDepth == 8)
        {
            for (u32 i = 0; i < numSamples; i++)
            {
                const i8 sample = *(i8 *)input;
                output[i] = sample * (1.0f / 127.0f);
                input++;
            }
        }
        else if (inBitDepth == 16)
        {
            for (u32 i = 0; i < numSamples; i++)
            {
                const i16 sample = *(i16 *)input;
                output[i] = sample * (1.0f / 32767.0f);
                input += 2;
            }
        }
        else if (inBitDepth == 24)
        {
            for (u32 i = 0; i < numSamples; i++)
            {
                const i32 sample = convert24To32Bits(input);
                output[i] = sample * (1.0f / 2147483647.0f);
                input += 3;
            }
        }
        else if (inBitDepth == 32)
        {
            for (u32 i = 0; i < numSamples; i++)
            {
                const i32 sample = *(i32 *)input;
                output[i] = sample * (1.0f / 2147483647.0f);
                input += 4;
            }
        }
        else
        {
            TK_ASSERT(false);
        }
    }

    void AudioTool::convertFromFloat(const float *input, i32 *output, u32 numSamples)
    {
        for (u32 i = 0; i < numSamples; i++)
        {
            float sample = *(float *)input;
            sample = TkMath::clamp(sample, -1.0f + TkMath::ZeroTolerance, +1.0f - TkMath::ZeroTolerance);
            output[i] = static_cast<i32>(sample * 2147483648.0f);
            input++;
        }
    }

}