/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

using namespace audio_test;

TEST(AudioToolTest, ConvertToMonoAveragesChannels)
{
    const byte input8[] = {10, 30, 100, 50};
    byte output8[2] = {};
    AudioTool::convertToMono(input8, output8, 8, 2, 2);
    EXPECT_EQ(output8[0], 20);
    EXPECT_EQ(output8[1], 75);

    const i16 input16[] = {1000, 3000, -2000, 2000};
    i16 output16[2] = {};
    AudioTool::convertToMono(reinterpret_cast<const byte *>(input16), reinterpret_cast<byte *>(output16), 16, 2, 2);
    EXPECT_EQ(output16[0], 2000);
    EXPECT_EQ(output16[1], 0);

    const byte input24[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x80, 0x00, 0x00, 0x80};
    byte output24[6] = {};
    AudioTool::convertToMono(input24, output24, 24, 2, 2);
    EXPECT_EQ(AudioTool::convert24To32Bits(output24), 0);
    EXPECT_EQ(AudioTool::convert24To32Bits(output24 + 3), kMinI32);

    const i32 input32[] = {10, 30, -100, 100};
    i32 output32[2] = {};
    AudioTool::convertToMono(reinterpret_cast<const byte *>(input32), reinterpret_cast<byte *>(output32), 32, 2, 2);
    EXPECT_EQ(output32[0], 20);
    EXPECT_EQ(output32[1], 0);
}

TEST(AudioToolTest, ConvertBitDepthCoversCommonPaths)
{
    const i8 input8[] = {-128, 0, 127};
    i16 out16[3] = {};
    AudioTool::convertBitDepth(reinterpret_cast<const byte *>(input8), 8, reinterpret_cast<byte *>(out16), 16, 3);
    EXPECT_EQ(out16[0], -32768);
    EXPECT_EQ(out16[1], 0);
    EXPECT_EQ(out16[2], 32512);

    const i16 input16[] = {-32768, 0, 32767};
    i32 out32[3] = {};
    AudioTool::convertBitDepth(reinterpret_cast<const byte *>(input16), 16, reinterpret_cast<byte *>(out32), 32, 3);
    EXPECT_EQ(out32[0], kMinI32);
    EXPECT_EQ(out32[1], 0);
    EXPECT_EQ(out32[2], 2147418112);

    const byte input24[] = {0, 0, 0, 0, 0, 0x80};
    i32 out24[2] = {};
    AudioTool::convertBitDepth(input24, 24, reinterpret_cast<byte *>(out24), 32, 2);
    EXPECT_EQ(out24[0], 0);
    EXPECT_EQ(out24[1], kMinI32);

    const i32 input32[] = {kMinI32, 0, 2147418112};
    i16 out16From32[3] = {};
    AudioTool::convertBitDepth(reinterpret_cast<const byte *>(input32), 32, reinterpret_cast<byte *>(out16From32), 16, 3);
    EXPECT_EQ(out16From32[0], -32768);
    EXPECT_EQ(out16From32[1], 0);
    EXPECT_EQ(out16From32[2], 32767);

    i32 copy32[3] = {};
    AudioTool::convertBitDepth(reinterpret_cast<const byte *>(input32), 32, reinterpret_cast<byte *>(copy32), 32, 3);
    EXPECT_EQ(copy32[0], kMinI32);
    EXPECT_EQ(copy32[1], 0);
    EXPECT_EQ(copy32[2], 2147418112);

    i8 out8From32[3] = {};
    AudioTool::convertBitDepth(reinterpret_cast<const byte *>(input32), 32, reinterpret_cast<byte *>(out8From32), 8, 3);
    EXPECT_EQ(out8From32[0], -128);
    EXPECT_EQ(out8From32[1], 0);
    EXPECT_EQ(out8From32[2], 127);

    byte out24From32[9] = {};
    AudioTool::convertBitDepth(reinterpret_cast<const byte *>(input32), 32, out24From32, 24, 3);
    EXPECT_EQ(AudioTool::convert24To32Bits(out24From32), kMinI32);
    EXPECT_EQ(AudioTool::convert24To32Bits(out24From32 + 3), 0);
    EXPECT_EQ(AudioTool::convert24To32Bits(out24From32 + 6), 2147418112);
}

TEST(AudioToolTest, ConvertToFloatAndBackUsesExpectedScaling)
{
    const i8 input8[] = {-127, 0, 127};
    float out8[3] = {};
    AudioTool::convertToFloat(reinterpret_cast<const byte *>(input8), 8, out8, 3);
    EXPECT_NEAR(out8[0], -1.0f, 1e-6f);
    EXPECT_NEAR(out8[1], 0.0f, 1e-6f);
    EXPECT_NEAR(out8[2], 1.0f, 1e-6f);

    const i16 input16[] = {-32767, 0, 32767};
    float out16[3] = {};
    AudioTool::convertToFloat(reinterpret_cast<const byte *>(input16), 16, out16, 3);
    EXPECT_NEAR(out16[0], -1.0f, 1e-6f);
    EXPECT_NEAR(out16[1], 0.0f, 1e-6f);
    EXPECT_NEAR(out16[2], 1.0f, 1e-6f);

    const byte input24[] = {0, 0, 0x80, 0, 0, 0};
    float out24[2] = {};
    AudioTool::convertToFloat(input24, 24, out24, 2);
    EXPECT_NEAR(out24[0], -1.0f, 1e-6f);
    EXPECT_NEAR(out24[1], 0.0f, 1e-6f);

    const i32 input32[] = {kMinI32, 0, kMaxI32};
    float out32[3] = {};
    AudioTool::convertToFloat(reinterpret_cast<const byte *>(input32), 32, out32, 3);
    EXPECT_NEAR(out32[0], -1.0f, 1e-6f);
    EXPECT_NEAR(out32[1], 0.0f, 1e-6f);
    EXPECT_NEAR(out32[2], 1.0f, 1e-4f);

    const float inputFloat[] = {0.0f, 0.25f, -0.5f};
    i32 outFloat[3] = {};
    AudioTool::convertFromFloat(inputFloat, outFloat, 3);
    EXPECT_EQ(outFloat[0], 0);
    EXPECT_EQ(outFloat[1], 536870912);
    EXPECT_EQ(outFloat[2], -1073741824);
}

TEST(AudioToolTest, Convert24To32BitsHelperIsConsistent)
{
    const byte input[] = {0x00, 0x00, 0x80};
    EXPECT_EQ(AudioTool::convert24To32Bits(input), kMinI32);
}

TEST(AudioToolTest, NullAndZeroInputsAreNoOps)
{
    byte output[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    AudioTool::convertToMono(nullptr, output, 16, 2, 2);
    AudioTool::convertToMono(output, nullptr, 16, 2, 2);
    AudioTool::convertToMono(output, output, 16, 2, 0);
    AudioTool::convertBitDepth(nullptr, 16, output, 32, 2);
    AudioTool::convertBitDepth(output, 16, nullptr, 32, 2);
    AudioTool::convertToFloat(nullptr, 16, reinterpret_cast<float *>(output), 2);
    AudioTool::convertFromFloat(nullptr, reinterpret_cast<i32 *>(output), 2);

    EXPECT_EQ(output[0], 1);
    EXPECT_EQ(output[7], 8);
}

TEST(AudioToolTest, ConvertFromFloatClampsAndHandlesNonFinite)
{
    const float input[] = {-2.0f, -1.0f, 1.0f, 2.0f, std::numeric_limits<float>::quiet_NaN()};
    i32 output[5] = {};

    AudioTool::convertFromFloat(input, output, 5);

    EXPECT_EQ(output[0], kMinI32);
    EXPECT_EQ(output[1], kMinI32);
    EXPECT_EQ(output[2], kMaxI32);
    EXPECT_EQ(output[3], kMaxI32);
    EXPECT_EQ(output[4], 0);
}
