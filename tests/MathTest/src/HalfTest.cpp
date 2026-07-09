#include "vector_test_utils.hpp"

using namespace math_test;

TEST(HalfTest, Float16CompressionAndHalfVectors)
{
    const Half half = Float16Compressor::compress(1.5f);
    EXPECT_NEAR(Float16Compressor::decompress(half), 1.5f, 1e-3f);

    expectVec2Near(Half2::Zero.toFVec2(), FVec2::kZero, 1e-5f);
    expectVec3Near(Half3::Zero.toFVec3(), FVec3::kZero, 1e-5f);
    expectVec4Near(Half4::Zero.toFVec4(), FVec4::kZero, 1e-5f);

    Half2 half2(1.25f, -2.5f);
    expectVec2Near(half2.toFVec2(), FVec2(1.25f, -2.5f), 1e-3f);
    expectVec2Near(Half2(FVec2(3.0f, 4.0f)).toFVec2(), FVec2(3.0f, 4.0f), 1e-3f);

    Half3 half3(1.25f, -2.5f, 3.75f);
    expectVec3Near(half3.toFVec3(), FVec3(1.25f, -2.5f, 3.75f), 1e-3f);
    expectVec3Near(Half3(FVec3(4.0f, 5.0f, 6.0f)).toFVec3(), FVec3(4.0f, 5.0f, 6.0f), 1e-3f);

    Half4 fromVec(FVec4(1.0f, 2.0f, 3.0f, 4.0f));
    expectVec4Near(fromVec.toFVec4(), FVec4(1.0f, 2.0f, 3.0f, 4.0f), 1e-3f);
    expectVec3Near(fromVec.toFVec3(), FVec3(1.0f, 2.0f, 3.0f), 1e-3f);
    expectVec2Near(fromVec.toFVec2(), FVec2(1.0f, 2.0f), 1e-3f);

    Half4 fromColor(Color(0.25f, 0.5f, 0.75f, 1.0f));
    expectVec4Near(fromColor.toFVec4(), FVec4(0.25f, 0.5f, 0.75f, 1.0f), 1e-3f);

    Half4 fromRect(Rectangle(1.0f, 2.0f, 3.0f, 4.0f));
    expectVec4Near(fromRect.toFVec4(), FVec4(1.0f, 2.0f, 3.0f, 4.0f), 1e-3f);
}

TEST(HalfTest, PackedFormatsAndUNorms)
{
    FloatR10G10B10A2 packed1010102(0.25f, 0.5f, 0.75f, 1.0f);
    EXPECT_EQ((u32)packed1010102, packed1010102.value);
    expectVec3Near(packed1010102.toFVec3(), FVec3(0.25f, 0.5f, 0.75f), 2e-3f);
    expectVec4Near(packed1010102.toFVec4(), FVec4(0.25f, 0.5f, 0.75f, 1.0f), 2e-3f);
    expectVec4Near(FloatR10G10B10A2(FVec4(0.1f, 0.2f, 0.3f, 0.0f)).toFVec4(), FVec4(0.1f, 0.2f, 0.3f, 0.0f), 2e-3f);
    expectVec3Near(FloatR10G10B10A2(FVec3(0.2f, 0.4f, 0.6f), 1.0f).toFVec3(), FVec3(0.2f, 0.4f, 0.6f), 2e-3f);
    const float rgbaValues[4] = {0.9f, 0.8f, 0.7f, 1.0f};
    expectVec4Near(FloatR10G10B10A2(rgbaValues).toFVec4(), FVec4(0.9f, 0.8f, 0.7f, 1.0f), 2e-3f);

    FloatR11G11B10 packed111110(1.0f, 2.0f, 4.0f);
    EXPECT_EQ((u32)packed111110, packed111110.value);
    expectVec3Near(packed111110.toFVec3(), FVec3(1.0f, 2.0f, 4.0f), 5e-2f);
    expectVec3Near(FloatR11G11B10(FVec3(0.25f, 0.5f, 0.75f)).toFVec3(), FVec3(0.25f, 0.5f, 0.75f), 5e-2f);
    expectVec3Near(FloatR11G11B10(FVec4(0.3f, 0.6f, 0.9f, 1.0f)).toFVec3(), FVec3(0.3f, 0.6f, 0.9f), 5e-2f);
    expectVec3Near(FloatR11G11B10(Color(0.4f, 0.5f, 0.6f, 1.0f)).toFVec3(), FVec3(0.4f, 0.5f, 0.6f), 5e-2f);
    const float rgbValues[3] = {0.2f, 0.4f, 0.8f};
    expectVec3Near(FloatR11G11B10(rgbValues).toFVec3(), FVec3(0.2f, 0.4f, 0.8f), 5e-2f);

    expectVec2Near(RG16UNorm(0.25f, 0.75f).toFVec2(), FVec2(0.25f, 0.75f), 2e-5f);
    expectVec4Near(RGBA16UNorm(0.1f, 0.2f, 0.3f, 0.4f).toFVec4(), FVec4(0.1f, 0.2f, 0.3f, 0.4f), 2e-5f);
}
