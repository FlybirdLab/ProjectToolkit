#include "vector_test_utils.hpp"

#include <limits>

using namespace math_test;

TEST(Vector2Test, ConstantsAndConstructors)
{
    expectVec2Near(FVec2::kZero, FVec2(0.0f, 0.0f));
    expectVec2Near(FVec2::kOne, FVec2(1.0f, 1.0f));
    expectVec2Near(FVec2::kHalf, FVec2(0.5f, 0.5f));
    expectVec2Near(FVec2::kUnitX, FVec2(1.0f, 0.0f));
    expectVec2Near(FVec2::kUnitY, FVec2(0.0f, 1.0f));

    const float raw[2] = {3.0f, -4.0f};
    expectVec2Near(FVec2(2.5f), FVec2(2.5f, 2.5f));
    expectVec2Near(FVec2(raw), FVec2(3.0f, -4.0f));
    expectVec2Near(FVec2(IVec3(7, 8, 9)), FVec2(7.0f, 8.0f));
    expectVec2Near(FVec2(FVec4(1.0f, 2.0f, 3.0f, 4.0f)), FVec2(1.0f, 2.0f));
    expectVec2Near(FVec2(Color(0.25f, 0.5f, 0.75f, 1.0f)), FVec2(0.25f, 0.5f));

    expectVec2Near(DVec2(FVec3(1.25f, -2.5f, 9.0f)), DVec2(1.25, -2.5));
    expectVec2Near(IVec2(DVec4(4.8, -7.2, 0.0, 1.0)), IVec2(4, -7));
}

TEST(Vector2Test, QueryHelpersAndStringConversion)
{
    const FVec2 value(3.0f, 4.0f);
    EXPECT_TRUE(FVec2::kZero.isZero());
    EXPECT_TRUE(FVec2::kOne.isOne());
    EXPECT_TRUE(FVec2(0.0f, 2.0f).isAnyZero());
    EXPECT_TRUE(FVec2(0.6f, 0.8f).isNormalized());
    EXPECT_FALSE(value.toString().isEmpty());

    EXPECT_FLOAT_EQ(value.length(), 5.0f);
    EXPECT_FLOAT_EQ(value.lengthSquared(), 25.0f);
    EXPECT_FLOAT_EQ(value.invLength(), 0.2f);
    EXPECT_FLOAT_EQ(value.averageArithmetic(), 3.5f);
    EXPECT_FLOAT_EQ(value.sumValues(), 7.0f);
    EXPECT_FLOAT_EQ(value.mulValues(), 12.0f);
    EXPECT_FLOAT_EQ(value.minValue(), 3.0f);
    EXPECT_FLOAT_EQ(value.maxValue(), 4.0f);
    expectVec2Near(value.getAbsolute(), FVec2(3.0f, 4.0f));
    expectVec2Near(FVec2(-3.0f, 4.0f).getAbsolute(), FVec2(3.0f, 4.0f));
    expectVec2Near(value.getNegative(), FVec2(-3.0f, -4.0f));
    expectVec2Near(value.getNormalized(), FVec2(0.6f, 0.8f));

    FVec2 normalized(3.0f, 4.0f);
    normalized.normalize();
    expectVec2Near(normalized, FVec2(0.6f, 0.8f));

    const FVec2 nanValue(std::numeric_limits<float>::quiet_NaN(), 0.0f);
    const FVec2 infValue(std::numeric_limits<float>::infinity(), 1.0f);
    EXPECT_TRUE(nanValue.isNaN());
    EXPECT_TRUE(infValue.isInfinity());
    EXPECT_TRUE(nanValue.isNanOrInfinity());
    EXPECT_TRUE(infValue.isNanOrInfinity());
}

TEST(Vector2Test, OperatorsComparisonsAndFreeFunctions)
{
    const FVec2 a(6.0f, 8.0f);
    const FVec2 b(2.0f, 4.0f);

    expectVec2Near(a + b, FVec2(8.0f, 12.0f));
    expectVec2Near(a - b, FVec2(4.0f, 4.0f));
    expectVec2Near(a * b, FVec2(12.0f, 32.0f));
    expectVec2Near(a / b, FVec2(3.0f, 2.0f));
    expectVec2Near(-a, FVec2(-6.0f, -8.0f));

    expectVec2Near(a + 1.0f, FVec2(7.0f, 9.0f));
    expectVec2Near(a - 1.0f, FVec2(5.0f, 7.0f));
    expectVec2Near(a * 0.5f, FVec2(3.0f, 4.0f));
    expectVec2Near(a / 2.0f, FVec2(3.0f, 4.0f));
    expectVec2Near(1.0f + b, FVec2(3.0f, 5.0f));
    expectVec2Near(10.0f - b, FVec2(8.0f, 6.0f));
    expectVec2Near(2.0f * b, FVec2(4.0f, 8.0f));
    expectVec2Near(16.0f / b, FVec2(8.0f, 4.0f));

    FVec2 c = a;
    c += b;
    expectVec2Near(c, FVec2(8.0f, 12.0f));
    c -= b;
    c *= b;
    c /= b;
    expectVec2Near(c, a);
    c += 1.0f;
    c -= 1.0f;
    c *= 2.0f;
    c /= 2.0f;
    expectVec2Near(c, a);

    EXPECT_TRUE(a == FVec2(6.0f, 8.0f));
    EXPECT_TRUE(a != b);
    EXPECT_TRUE(a > b);
    EXPECT_TRUE(a >= b);
    EXPECT_TRUE(b < a);
    EXPECT_TRUE(b <= a);

    EXPECT_TRUE(FVec2::nearEqual(FVec2(1.0f, 2.0f), FVec2(1.0f + 1e-7f, 2.0f - 1e-7f)));
    EXPECT_TRUE(FVec2::nearEqual(FVec2(1.0f, 2.0f), FVec2(1.01f, 2.01f), 0.02f));
    EXPECT_NE(getHash(a), 0u);
}

TEST(Vector2Test, StaticAlgorithms)
{
    const FVec2 a(3.5f, -2.25f);
    const FVec2 b(1.5f, 4.0f);
    FVec2 result;

    EXPECT_FLOAT_EQ(FVec2::dot(a, b), -3.75f);
    EXPECT_FLOAT_EQ(FVec2::cross(FVec2(1.0f, 0.0f), FVec2(0.0f, 1.0f)), 1.0f);

    FVec2::add(a, b, result);
    expectVec2Near(result, a + b);
    FVec2::subtract(a, b, result);
    expectVec2Near(result, a - b);
    FVec2::multiply(a, b, result);
    expectVec2Near(result, a * b);
    FVec2::divide(FVec2(9.0f, 12.0f), FVec2(3.0f, 4.0f), result);
    expectVec2Near(result, FVec2(3.0f, 3.0f));
    FVec2::min(a, b, result);
    expectVec2Near(result, FVec2::min(a, b));
    FVec2::max(a, b, result);
    expectVec2Near(result, FVec2::max(a, b));

    expectVec2Near(FVec2::mod(FVec2(5.5f, 7.25f), FVec2(2.0f, 3.0f)), FVec2(1.5f, 1.25f));
    expectVec2Near(FVec2::floor(FVec2(1.75f, -1.25f)), FVec2(1.0f, -2.0f));
    expectVec2Near(FVec2::frac(FVec2(1.75f, -1.25f)), FVec2(0.75f, -0.25f));
    expectVec2Near(FVec2::round(FVec2(1.49f, 1.5f)), FVec2(1.0f, 2.0f));
    expectVec2Near(FVec2::ceil(FVec2(1.1f, -1.9f)), FVec2(2.0f, -1.0f));
    expectVec2Near(FVec2::abs(FVec2(-3.0f, 4.0f)), FVec2(3.0f, 4.0f));
    expectVec2Near(FVec2::clamp(FVec2(-1.0f, 5.0f), FVec2(0.0f, 1.0f), FVec2(2.0f, 3.0f)), FVec2(0.0f, 3.0f));
    FVec2::clamp(FVec2(-1.0f, 5.0f), FVec2(0.0f, 1.0f), FVec2(2.0f, 3.0f), result);
    expectVec2Near(result, FVec2(0.0f, 3.0f));

    EXPECT_FLOAT_EQ(FVec2::distance(FVec2(1.0f, 2.0f), FVec2(4.0f, 6.0f)), 5.0f);
    EXPECT_FLOAT_EQ(FVec2::distanceSquared(FVec2(1.0f, 2.0f), FVec2(4.0f, 6.0f)), 25.0f);
    expectVec2Near(FVec2::normalize(FVec2(3.0f, 4.0f)), FVec2(0.6f, 0.8f));
    expectVec2Near(FVec2::lerp(FVec2::kZero, FVec2(10.0f, 20.0f), 0.25f), FVec2(2.5f, 5.0f));
    FVec2::lerp(FVec2::kZero, FVec2(10.0f, 20.0f), 0.75f, result);
    expectVec2Near(result, FVec2(7.5f, 15.0f));

    EXPECT_FLOAT_EQ(FVec2::triangleArea(FVec2(0.0f, 0.0f), FVec2(4.0f, 0.0f), FVec2(0.0f, 3.0f)), 6.0f);
    EXPECT_NEAR(FVec2::angle(FVec2::kUnitX, FVec2::kUnitY), TkMath::PI_OVER_2, 1e-5f);
}

TEST(Vector2Test, StabilityAndMemoryLayout)
{
    FVec2 zero = FVec2::kZero;
    zero.normalize();
    expectVec2Near(zero, FVec2::kZero);

    FVec2 rawBacked(1.0f, 2.0f);
    rawBacked.raw[0] = 7.0f;
    rawBacked.raw[1] = -3.0f;
    expectVec2Near(rawBacked, FVec2(7.0f, -3.0f));

    FVec2 setterProbe;
    setterProbe.x = 11.0f;
    setterProbe.y = 12.0f;
    EXPECT_FLOAT_EQ(setterProbe.raw[0], 11.0f);
    EXPECT_FLOAT_EQ(setterProbe.raw[1], 12.0f);

    EXPECT_TRUE(FVec2::nearEqual(FVec2::normalize(FVec2(1000.0f, 0.0f)), FVec2::kUnitX));
    EXPECT_FLOAT_EQ(FVec2::angle(FVec2::kUnitX, FVec2::kUnitX), 0.0f);
    EXPECT_NEAR(FVec2::angle(FVec2::kUnitX, -FVec2::kUnitX), TkMath::PI, 1e-5f);

    FVec2 iterative = FVec2(1.0f, 0.0f);
    for (int i = 0; i < 128; ++i)
    {
        iterative = FVec2::normalize(iterative + FVec2(0.001f, 0.002f));
    }
    EXPECT_TRUE(iterative.isNormalized());
}
