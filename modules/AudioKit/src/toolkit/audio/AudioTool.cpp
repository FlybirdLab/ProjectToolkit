/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "AudioTool.hpp"

#include "toolkit/base/Assert.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/base/Math.hpp"
#include <cmath>

#define CONVERT_TO_MONO_AVG 1

namespace toolkit
{
    template <typename T>
    static T loadSample(const byte *input)
    {
        T value{};
        TkMemory::memoryCopy(&value, input, sizeof(T));
        return value;
    }

    template <typename T>
    static void storeSample(byte *output, T value)
    {
        TkMemory::memoryCopy(output, &value, sizeof(T));
    }

    static void convertToMono8(const byte *input, byte *output, u32 numSamples, u32 numChannels)
    {
        for (u32 i = 0; i < numSamples; i++)
        {
            i16 sum = 0;
            for (u32 j = 0; j < numChannels; j++)
            {
                sum += static_cast<i8>(*input);
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

    static void convertToMono16(const byte *input, byte *output, u32 numSamples, u32 numChannels)
    {
        for (u32 i = 0; i < numSamples; i++)
        {
            i32 sum = 0;
            for (u32 j = 0; j < numChannels; j++)
            {
                sum += loadSample<i16>(input);
                input += sizeof(i16);
            }

#if CONVERT_TO_MONO_AVG
            storeSample<i16>(output, static_cast<i16>(sum / static_cast<i32>(numChannels)));
#else
            storeSample<i16>(output, static_cast<i16>(TkMath::clamp<i32>(sum, kMinI16, kMaxI16)));
#endif
            output += sizeof(i16);
        }
    }

    static void convert32To24Bits(const i32 input, u8 *output)
    {
        const u32 valToEncode = static_cast<u32>(input);
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
            constexpr i64 MIN_i24 = -8388608;
            constexpr i64 MAX_i24 = 8388607;
            const i32 val = (i32)TkMath::clamp<i64>(sum, MIN_i24, MAX_i24);
#endif
            convert32To24Bits(val, output);
            output += 3;
        }
    }

    static void convertToMono32(const byte *input, byte *output, u32 numSamples, u32 numChannels)
    {
        for (u32 i = 0; i < numSamples; i++)
        {
            i64 sum = 0;
            for (u32 j = 0; j < numChannels; j++)
            {
                sum += loadSample<i32>(input);
                input += sizeof(i32);
            }

#if CONVERT_TO_MONO_AVG
            storeSample<i32>(output, static_cast<i32>(sum / static_cast<i64>(numChannels)));
#else
            storeSample<i32>(output, static_cast<i32>(TkMath::clamp<i64>(sum, kMinI32, kMaxI32)));
#endif
            output += sizeof(i32);
        }
    }

    static void convert8To32Bits(const byte *input, i32 *output, u32 numSamples)
    {
        for (u32 i = 0; i < numSamples; i++)
        {
            const i8 val = static_cast<i8>(input[i]);
            output[i] = val << 24;
        }
    }

    static void convert16To32Bits(const byte *input, i32 *output, u32 numSamples)
    {
        for (u32 i = 0; i < numSamples; i++)
        {
            output[i] = loadSample<i16>(input) << 16;
            input += sizeof(i16);
        }
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

    static void convert32To16Bits(const i32 *input, byte *output, u32 numSamples)
    {
        for (u32 i = 0; i < numSamples; i++)
        {
            storeSample<i16>(output, static_cast<i16>(input[i] >> 16));
            output += sizeof(i16);
        }
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
        if (input == nullptr || output == nullptr || numSamples == 0 || numChannels == 0)
            return;

        switch (bitDepth)
        {
        case 8:
            convertToMono8(input, output, numSamples, numChannels);
            break;
        case 16:
            convertToMono16(input, output, numSamples, numChannels);
            break;
        case 24:
            convertToMono24(input, output, numSamples, numChannels);
            break;
        case 32:
            convertToMono32(input, output, numSamples, numChannels);
            break;
        default:
            TK_ASSERT(false);
            break;
        }
    }

    void AudioTool::convertBitDepth(const byte *input, u32 inBitDepth, byte *output, u32 outBitDepth, u32 numSamples)
    {
        if (input == nullptr || output == nullptr || numSamples == 0)
            return;

        i32 *srcBuffer = nullptr;

        const bool needTempBuffer = true;
        if (needTempBuffer)
            srcBuffer = (i32 *)TkMemory::allocate(TkMemory::getDefault(), numSamples * sizeof(i32));
        if (needTempBuffer && srcBuffer == nullptr)
            return;

        // Convert it to a temporary 32-bit buffer and then use that to convert to actual requested bit depth.
        // It could be more efficient to convert directly from source to requested depth without a temporary buffer,
        // at the cost of additional complexity. If this method ever becomes a performance issue consider that.

        switch (inBitDepth)
        {
        case 8:
            convert8To32Bits(input, srcBuffer, numSamples);
            break;
        case 16:
            convert16To32Bits(input, srcBuffer, numSamples);
            break;
        case 24:
            ::toolkit::convert24To32Bits(input, srcBuffer, numSamples);
            break;
        case 32:
            for (u32 i = 0; i < numSamples; ++i)
            {
                srcBuffer[i] = loadSample<i32>(input);
                input += sizeof(i32);
            }
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
            convert32To16Bits(srcBuffer, output, numSamples);
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
        if (input == nullptr || output == nullptr || numSamples == 0)
            return;

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
                const i16 sample = loadSample<i16>(input);
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
                const i32 sample = loadSample<i32>(input);
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
        if (input == nullptr || output == nullptr || numSamples == 0)
            return;

        for (u32 i = 0; i < numSamples; i++)
        {
            float sample = input[i];
            if (!std::isfinite(sample))
                sample = 0.0f;
            sample = TkMath::clamp(sample, -1.0f, 1.0f);
            output[i] = sample >= 1.0f ? kMaxI32 : static_cast<i32>(sample * 2147483648.0f);
        }
    }

}
