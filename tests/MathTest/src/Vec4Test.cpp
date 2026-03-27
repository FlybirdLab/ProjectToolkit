#include "vector_test_utils.hpp"

#include <limits>

using namespace math_test;

TEST(Vector4Test, ConstantsConstructorsAndQueries)
{
    expectVec4Near(FVec4::kZero, FVec4(0.0f, 0.0f, 0.0f, 0.0f));
    expectVec4Near(FVec4::kOne, FVec4(1.0f, 1.0f, 1.0f, 1.0f));
    expectVec4Near(FVec4::kHalf, FVec4(0.5f, 0.5f, 0.5f, 0.5f));
    expectVec4Near(FVec4::kUnitX, FVec4(1.0f, 0.0f, 0.0f, 0.0f));
    expectVec4Near(FVec4::kUnitW, FVec4(0.0f, 0.0f, 0.0f, 1.0f));

    const float raw[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    expectVec4Near(FVec4(2.0f), FVec4(2.0f, 2.0f, 2.0f, 2.0f));
    expectVec4Near(FVec4(raw), FVec4(1.0f, 2.0f, 3.0f, 4.0f));
    expectVec4Near(FVec4(FVec2(5.0f, 6.0f), 7.0f, 8.0f), FVec4(5.0f, 6.0f, 7.0f, 8.0f));
    expectVec4Near(FVec4(FVec2(1.0f, 2.0f), FVec2(3.0f, 4.0f)), FVec4(1.0f, 2.0f, 3.0f, 4.0f));
    expectVec4Near(FVec4(FVec3(1.0f, 2.0f, 3.0f), 4.0f), FVec4(1.0f, 2.0f, 3.0f, 4.0f));
    expectVec4Near(FVec4(IVec3(1, 2, 3), 4.0f), FVec4(1.0f, 2.0f, 3.0f, 4.0f));
    expectVec4Near(FVec4(Color(0.25f, 0.5f, 0.75f, 1.0f)), FVec4(0.25f, 0.5f, 0.75f, 1.0f));
    expectVec4Near(FVec4(Rectangle(1.0f, 2.0f, 3.0f, 4.0f)), FVec4(1.0f, 2.0f, 3.0f, 4.0f));

    const FVec4 value(2.0f, 3.0f, 6.0f, 9.0f);
    EXPECT_TRUE(FVec4::kZero.isZero());
    EXPECT_TRUE(FVec4::kOne.isOne());
    EXPECT_TRUE(FVec4(0.0f, 1.0f, 2.0f, 3.0f).isAnyZero());
    EXPECT_FALSE(value.toString().isEmpty());

    EXPECT_FLOAT_EQ(value.averageArithmetic(), 5.0f);
    EXPECT_FLOAT_EQ(value.sumValues(), 20.0f);
    EXPECT_FLOAT_EQ(value.minValue(), 2.0f);
    EXPECT_FLOAT_EQ(value.maxValue(), 9.0f);
    expectVec4Near(value.getAbsolute(), value);
    expectVec4Near(FVec4(-2.0f, 3.0f, -6.0f, 9.0f).getAbsolute(), value);
    expectVec4Near(value.getNegative(), FVec4(-2.0f, -3.0f, -6.0f, -9.0f));

    const FVec4 nanValue(std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.0f, 0.0f);
    const FVec4 infValue(std::numeric_limits<float>::infinity(), 1.0f, 2.0f, 3.0f);
    EXPECT_TRUE(nanValue.isNaN());
    EXPECT_TRUE(infValue.isInfinity());
    EXPECT_TRUE(nanValue.isNanOrInfinity());
    EXPECT_TRUE(infValue.isNanOrInfinity());
}

TEST(Vector4Test, OperatorsComparisonsAndAlgorithms)
{
    const FVec4 a(8.0f, 12.0f, 16.0f, 20.0f);
    const FVec4 b(2.0f, 3.0f, 4.0f, 5.0f);
    FVec4 result;

    expectVec4Near(a + b, FVec4(10.0f, 15.0f, 20.0f, 25.0f));
    expectVec4Near(a - b, FVec4(6.0f, 9.0f, 12.0f, 15.0f));
    expectVec4Near(a * b, FVec4(16.0f, 36.0f, 64.0f, 100.0f));
    expectVec4Near(a / b, FVec4(4.0f, 4.0f, 4.0f, 4.0f));
    expectVec4Near(-a, FVec4(-8.0f, -12.0f, -16.0f, -20.0f));

    expectVec4Near(a + 1.0f, FVec4(9.0f, 13.0f, 17.0f, 21.0f));
    expectVec4Near(a - 1.0f, FVec4(7.0f, 11.0f, 15.0f, 19.0f));
    expectVec4Near(a * 0.5f, FVec4(4.0f, 6.0f, 8.0f, 10.0f));
    expectVec4Near(a / 2.0f, FVec4(4.0f, 6.0f, 8.0f, 10.0f));
    expectVec4Near(1.0f + b, FVec4(3.0f, 4.0f, 5.0f, 6.0f));
    expectVec4Near(10.0f - b, FVec4(8.0f, 7.0f, 6.0f, 5.0f));
    expectVec4Near(2.0f * b, FVec4(4.0f, 6.0f, 8.0f, 10.0f));
    expectVec4Near(20.0f / b, FVec4(10.0f, 20.0f / 3.0f, 5.0f, 4.0f));

    FVec4 c = a;
    c += b;
    c -= b;
    c *= b;
    c /= b;
    c += 1.0f;
    c -= 1.0f;
    c *= 2.0f;
    c /= 2.0f;
    expectVec4Near(c, a);

    EXPECT_TRUE(a == FVec4(8.0f, 12.0f, 16.0f, 20.0f));
    EXPECT_TRUE(a != b);
    EXPECT_TRUE(a > b);
    EXPECT_TRUE(a >= b);
    EXPECT_TRUE(b < a);
    EXPECT_TRUE(b <= a);
    EXPECT_TRUE(FVec4::nearEqual(FVec4(1.0f, 2.0f, 3.0f, 4.0f), FVec4(1.0f + 1e-7f, 2.0f, 3.0f, 4.0f - 1e-7f)));
    EXPECT_TRUE(FVec4::nearEqual(FVec4(1.0f, 2.0f, 3.0f, 4.0f), FVec4(1.01f, 2.01f, 3.01f, 4.01f), 0.02f));
    EXPECT_NE(getHash(a), 0u);

    FVec4::add(a, b, result);
    expectVec4Near(result, a + b);
    FVec4::subtract(a, b, result);
    expectVec4Near(result, a - b);
    FVec4::multiply(a, b, result);
    expectVec4Near(result, a * b);
    FVec4::divide(a, b, result);
    expectVec4Near(result, a / b);

    expectVec4Near(FVec4::mod(FVec4(5.5f, 7.25f, 9.75f, 11.5f), FVec4(2.0f, 3.0f, 4.0f, 5.0f)), FVec4(1.5f, 1.25f, 1.75f, 1.5f));
    expectVec4Near(FVec4::floor(FVec4(1.75f, -1.25f, 3.99f, -4.01f)), FVec4(1.0f, -2.0f, 3.0f, -5.0f));
    expectVec4Near(FVec4::frac(FVec4(1.75f, -1.25f, 3.99f, -4.01f)), FVec4(0.75f, -0.25f, 0.99f, -0.01f), 1e-4f);
    expectVec4Near(FVec4::round(FVec4(1.49f, 1.5f, -1.6f, 2.4f)), FVec4(1.0f, 2.0f, -2.0f, 2.0f));
    expectVec4Near(FVec4::ceil(FVec4(1.1f, -1.9f, 2.01f, -4.99f)), FVec4(2.0f, -1.0f, 3.0f, -4.0f));
    expectVec4Near(FVec4::abs(FVec4(-3.0f, 4.0f, -5.0f, 6.0f)), FVec4(3.0f, 4.0f, 5.0f, 6.0f));
    expectVec4Near(FVec4::clamp(FVec4(-1.0f, 5.0f, 7.0f, -3.0f), FVec4(0.0f, 1.0f, 2.0f, -2.0f), FVec4(2.0f, 3.0f, 6.0f, 1.0f)), FVec4(0.0f, 3.0f, 6.0f, -2.0f));
    FVec4::clamp(FVec4(-1.0f, 5.0f, 7.0f, -3.0f), FVec4(0.0f, 1.0f, 2.0f, -2.0f), FVec4(2.0f, 3.0f, 6.0f, 1.0f), result);
    expectVec4Near(result, FVec4(0.0f, 3.0f, 6.0f, -2.0f));

    expectVec4Near(FVec4::lerp(FVec4::kZero, FVec4(10.0f, 20.0f, 30.0f, 40.0f), 0.25f), FVec4(2.5f, 5.0f, 7.5f, 10.0f));
    FVec4::lerp(FVec4::kZero, FVec4(10.0f, 20.0f, 30.0f, 40.0f), 0.75f, result);
    expectVec4Near(result, FVec4(7.5f, 15.0f, 22.5f, 30.0f));
}

TEST(Vector4Test, MatrixTransformAndOtherSpecializations)
{
    const Mat translation = Mat::translation(FVec3(10.0f, 20.0f, 30.0f));
    expectVec4Near(FVec4::transform(FVec4(1.0f, 2.0f, 3.0f, 1.0f), translation), FVec4(11.0f, 22.0f, 33.0f, 1.0f));

    expectVec4Near(DVec4(FVec3(1.25f, -2.5f, 3.75f), 4.5), DVec4(1.25, -2.5, 3.75, 4.5));
    expectVec4Near(IVec4(DVec2(4.8, -7.2), DVec2(2.2, -3.9)), IVec4(4, -7, 2, -3));
}

TEST(Vector4Test, StabilityAndLayout)
{
    FVec4 rawBacked(1.0f, 2.0f, 3.0f, 4.0f);
    rawBacked.raw[0] = -1.0f;
    rawBacked.raw[1] = -2.0f;
    rawBacked.raw[2] = 5.0f;
    rawBacked.raw[3] = 6.0f;
    expectVec4Near(rawBacked, FVec4(-1.0f, -2.0f, 5.0f, 6.0f));

    FVec4 columnProbe;
    columnProbe.x = 7.0f;
    columnProbe.y = 8.0f;
    columnProbe.z = 9.0f;
    columnProbe.w = 10.0f;
    EXPECT_FLOAT_EQ(columnProbe.raw[0], 7.0f);
    EXPECT_FLOAT_EQ(columnProbe.raw[3], 10.0f);

    expectVec4Near(FVec4::lerp(FVec4::kZero, FVec4::kOne, 0.0f), FVec4::kZero);
    expectVec4Near(FVec4::lerp(FVec4::kZero, FVec4::kOne, 1.0f), FVec4::kOne);

    FVec4 iterative = FVec4::kZero;
    for (int i = 0; i < 32; ++i)
    {
        iterative = FVec4::lerp(iterative, FVec4(1.0f, 2.0f, 3.0f, 4.0f), 0.25f);
    }
    expectVec4Near(iterative, FVec4(1.0f, 2.0f, 3.0f, 4.0f), 1e-3f);
}
