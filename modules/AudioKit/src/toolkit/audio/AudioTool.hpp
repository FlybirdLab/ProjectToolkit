/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/audio/common.hpp>

namespace toolkit
{
    // Audio data importing and processing utilities.
    class TK_API AudioTool
    {
    public:
        // Converts a set of audio samples using multiple channels into a set of mono samples.
        static void convertToMono(const byte *input, byte *output, u32 bitDepth, u32 numSamples, u32 numChannels);

        // Converts a set of audio samples of a certain bit depth to a new bit depth.
        static void convertBitDepth(const byte *input, u32 inBitDepth, byte *output, u32 outBitDepth, u32 numSamples);

        // Converts a set of audio samples of a certain bit depth to a set of floating point samples in range [-1, 1].
        static void convertToFloat(const byte *input, u32 inBitDepth, float *output, u32 numSamples);

        // Converts a set of audio samples of floating point samples in range [-1, 1] to a 32-bit depth PCM data.
        static void convertFromFloat(const float *input, i32 *output, u32 numSamples);

        // Converts a 24-bit signed integer into a 32-bit signed integer.
        TK_FORCE_INLINE static i32 convert24To32Bits(const byte *input)
        {
            const u32 value = (static_cast<u32>(input[2]) << 24) | (static_cast<u32>(input[1]) << 16) | (static_cast<u32>(input[0]) << 8);
            return static_cast<i32>(value);
        }
    };

}
