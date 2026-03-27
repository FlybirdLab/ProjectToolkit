#include "vector_test_utils.hpp"

using namespace math_test;

TEST(ColorTest, ConstructionFactoriesQueriesAndConversions)
{
    expectColorNear(Color(0.25f), Color(0.25f, 0.25f, 0.25f, 0.25f));
    expectColorNear(Color(FVec3(0.1f, 0.2f, 0.3f), 0.4f), Color(0.1f, 0.2f, 0.3f, 0.4f));
    expectColorNear(Color(FVec4(0.5f, 0.6f, 0.7f, 0.8f)), Color(0.5f, 0.6f, 0.7f, 0.8f));
    expectColorNear(Color(Color(0.1f, 0.2f, 0.3f, 0.4f), 1.0f), Color(0.1f, 0.2f, 0.3f, 1.0f));

    expectColorNear(Color::fromBytes(64, 128, 255, 32), Color(64.0f / 255.0f, 128.0f / 255.0f, 1.0f, 32.0f / 255.0f), 1e-5f);
    expectColorNear(Color::fromRGB(0x3366CC, 0.5f), Color(0x33 / 255.0f, 0x66 / 255.0f, 0xCC / 255.0f, 0.5f), 1e-5f);
    expectColorNear(Color::fromARGB(0x80402010), Color(0x40 / 255.0f, 0x20 / 255.0f, 0x10 / 255.0f, 0x80 / 255.0f), 1e-5f);
    expectColorNear(Color::fromRGBA(0x40201080), Color(0x40 / 255.0f, 0x20 / 255.0f, 0x10 / 255.0f, 0x80 / 255.0f), 1e-5f);

    bool isValid = false;
    expectColorNear(Color::fromHex("#ABC", isValid), Color::fromBytes(0xAA, 0xBB, 0xCC), 1e-5f);
    EXPECT_TRUE(isValid);
    expectColorNear(Color::fromHex("11223344", isValid), Color::fromBytes(0x11, 0x22, 0x33, 0x44), 1e-5f);
    EXPECT_TRUE(isValid);
    Color invalidHex = Color::fromHex("ZZ", isValid);
    EXPECT_FALSE(isValid);
    expectColorNear(invalidHex, Color::fromBytes(0, 0, 0), 1e-5f);

    expectColorNear(Color::fromHSV(0.0f, 1.0f, 1.0f, 0.5f), Color(1.0f, 0.0f, 0.0f, 0.5f), 1e-4f);
    expectColorNear(Color::fromHSV(FVec3(120.0f, 1.0f, 1.0f), 1.0f), Color(0.0f, 1.0f, 0.0f, 1.0f), 1e-4f);
    expectVec3Near(Color(1.0f, 0.0f, 0.0f, 1.0f).toHSV(), FVec3(0.0f, 1.0f, 1.0f), 1e-4f);

    const Color color(0.25f, 0.5f, 0.75f, 1.0f);
    EXPECT_FALSE(color.toString().isEmpty());
    EXPECT_EQ(color.toHexString(), TkString("3F7FBFFF", 8));
    EXPECT_FALSE(color.isTransparent());
    EXPECT_FALSE(color.hasOpacity());
    EXPECT_TRUE(Color::Transparent.isTransparent());
    EXPECT_TRUE(Color(0.0f, 0.0f, 0.0f, 0.5f).hasOpacity());
    EXPECT_TRUE(Color::nearEqual(color, Color(0.25f, 0.5f, 0.75f, 1.0f)));
    EXPECT_NE(getHash(color), 0u);

    expectVec3Near(color.toFVec3(), FVec3(0.25f, 0.5f, 0.75f));
    expectVec4Near(color.toFVec4(), FVec4(0.25f, 0.5f, 0.75f, 1.0f));
}

TEST(ColorTest, OperatorsInterpolationAndColorSpaceHelpers)
{
    const Color a(0.2f, 0.4f, 0.6f, 0.8f);
    const Color b(0.1f, 0.2f, 0.3f, 0.4f);

    expectColorNear(a + b, Color(0.3f, 0.6f, 0.9f, 1.2f), 1e-5f);
    expectColorNear(a - b, Color(0.1f, 0.2f, 0.3f, 0.4f), 1e-5f);
    expectColorNear(a * b, Color(0.02f, 0.08f, 0.18f, 0.32f), 1e-5f);
    expectColorNear(a + 0.1f, Color(0.3f, 0.5f, 0.7f, 0.9f), 1e-5f);
    expectColorNear(a - 0.1f, Color(0.1f, 0.3f, 0.5f, 0.7f), 1e-5f);
    expectColorNear(a * 2.0f, Color(0.4f, 0.8f, 1.2f, 1.6f), 1e-5f);
    expectColorNear(a / 2.0f, Color(0.1f, 0.2f, 0.3f, 0.4f), 1e-5f);

    Color mutated = a;
    mutated += b;
    mutated -= b;
    mutated *= b;
    expectColorNear(mutated, Color(0.02f, 0.08f, 0.18f, 0.32f), 1e-5f);
    mutated = a;
    mutated *= 2.0f;
    expectColorNear(mutated, Color(0.4f, 0.8f, 1.2f, 1.6f), 1e-5f);

    Color lerped;
    Color::lerp(Color::Transparent, Color::White, 0.25f, lerped);
    expectColorNear(lerped, Color(0.25f, 0.25f, 0.25f, 0.25f), 1e-5f);
    expectColorNear(Color::lerp(Color::Transparent, Color::White, 0.75f), Color(0.75f, 0.75f, 0.75f, 0.75f), 1e-5f);

    expectColorNear(a.rgbMultiplied(2.0f), Color(0.4f, 0.8f, 1.2f, 0.8f), 1e-5f);
    expectColorNear(a.rgbMultiplied(Color(2.0f, 0.5f, 1.5f, 1.0f)), Color(0.4f, 0.2f, 0.9f, 0.8f), 1e-5f);
    expectColorNear(a.alphaMultiplied(0.5f), Color(0.2f, 0.4f, 0.6f, 0.4f), 1e-5f);

    const Color linear(0.25f, 0.5f, 0.75f, 1.0f);
    const Color srgb = Color::linearToSrgb(linear);
    const Color roundTrip = Color::srgbToLinear(srgb);
    expectColorNear(roundTrip, linear, 1e-4f);

    const Color randomColor = Color::random();
    EXPECT_GE(randomColor.r, 0.0f);
    EXPECT_LE(randomColor.r, 1.0f);
    EXPECT_GE(randomColor.g, 0.0f);
    EXPECT_LE(randomColor.g, 1.0f);
    EXPECT_GE(randomColor.b, 0.0f);
    EXPECT_LE(randomColor.b, 1.0f);
    EXPECT_FLOAT_EQ(randomColor.a, 1.0f);
}

TEST(Color32Test, ConstructionQueriesPackingAndInterpolation)
{
    expectColor32Eq(Color32::Transparent, Color32(0, 0, 0, 0));
    expectColor32Eq(Color32::Black, Color32(0, 0, 0, 255));
    expectColor32Eq(Color32::White, Color32(255, 255, 255, 255));

    Color32 direct(1, 2, 3, 4);
    expectColor32Eq(direct, Color32(1, 2, 3, 4));
    expectColor32Eq(Color32(Color(0.5f, 0.25f, 1.0f, 0.75f)), Color32(127, 63, 255, 191));
    expectColor32Eq(Color32(FVec4(0.5f, 0.25f, 1.0f, 0.75f)), Color32(127, 63, 255, 191));

    EXPECT_FALSE(direct.toString().isEmpty());
    EXPECT_EQ(Color32(0x12, 0x34, 0xAB, 0xCD).toHexString(), TkString("1234AB", 6));
    EXPECT_FALSE(Color32::White.isTransparent());
    EXPECT_TRUE(Color32::Transparent.isTransparent());
    EXPECT_FALSE(Color32::White.hasOpacity());
    EXPECT_TRUE(Color32(1, 2, 3, 4).hasOpacity());
    EXPECT_EQ(Color32(0x12, 0x34, 0x56, 0x78).getAsABGR(), 0x12345678u);
    EXPECT_EQ(Color32(0x12, 0x34, 0x56, 0x78).getAsBGRA(), 0x78123456u);
    EXPECT_EQ(Color32(0x12, 0x34, 0x56, 0x78).getAsARGB(), 0x56341278u);
    EXPECT_EQ(Color32(0x12, 0x34, 0x56, 0x78).getAsRGB(), 0x123456u);
    EXPECT_EQ(Color32(0x12, 0x34, 0x56, 0x78).getAsRGBA(), 0x78563412u);
    EXPECT_NE(getHash(Color32::White), 0u);

    expectColor32Eq(Color32::fromRGB(0x123456, 0x78), Color32(0x12, 0x34, 0x56, 0x78));
    expectColor32Eq(Color32::fromRGBA(0.5f, 0.25f, 1.0f, 0.75f), Color32(127, 63, 255, 191));
    expectColor32Eq(Color32::Lerp(Color32(0, 0, 0, 0), Color32(100, 200, 50, 255), 0.5f), Color32(50, 100, 25, 127));

    expectColor32Eq(Color32(10, 20, 30, 40) + Color32(1, 2, 3, 4), Color32(11, 22, 33, 44));
    expectColor32Eq(Color32(10, 20, 30, 40) - Color32(1, 2, 3, 4), Color32(9, 18, 27, 36));
    expectColor32Eq(Color32(10, 20, 30, 40) * Color32(2, 3, 4, 5), Color32(20, 60, 120, 200));
    expectColor32Eq(Color32(10, 20, 30, 40) * 2.0f, Color32(20, 40, 60, 80));

    Color32 mutated(10, 20, 30, 40);
    mutated += Color32(1, 2, 3, 4);
    mutated -= Color32(1, 2, 3, 4);
    mutated *= Color32(2, 3, 4, 5);
    expectColor32Eq(mutated, Color32(20, 60, 120, 200));

    const Color32 randomColor = Color32::random();
    EXPECT_EQ(randomColor.a, 255);
}
