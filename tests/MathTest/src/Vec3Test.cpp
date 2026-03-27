#include "vector_test_utils.hpp"

#include <limits>

using namespace math_test;

TEST(Vector3Test, ConstantsConstructorsAndQueries)
{
    expectVec3Near(FVec3::kZero, FVec3(0.0f, 0.0f, 0.0f));
    expectVec3Near(FVec3::kOne, FVec3(1.0f, 1.0f, 1.0f));
    expectVec3Near(FVec3::kHalf, FVec3(0.5f, 0.5f, 0.5f));
    expectVec3Near(FVec3::kUnitX, FVec3(1.0f, 0.0f, 0.0f));
    expectVec3Near(FVec3::kUp, FVec3(0.0f, 1.0f, 0.0f));
    expectVec3Near(FVec3::kForward, FVec3(0.0f, 0.0f, 1.0f));

    const float raw[3] = {1.0f, 2.0f, 3.0f};
    expectVec3Near(FVec3(2.0f), FVec3(2.0f, 2.0f, 2.0f));
    expectVec3Near(FVec3(raw), FVec3(1.0f, 2.0f, 3.0f));
    expectVec3Near(FVec3(FVec2(4.0f, 5.0f), 6.0f), FVec3(4.0f, 5.0f, 6.0f));
    expectVec3Near(FVec3(IVec3(7, 8, 9)), FVec3(7.0f, 8.0f, 9.0f));
    expectVec3Near(FVec3(FVec4(1.0f, 2.0f, 3.0f, 4.0f)), FVec3(1.0f, 2.0f, 3.0f));
    expectVec3Near(FVec3(Color(0.25f, 0.5f, 0.75f, 1.0f)), FVec3(0.25f, 0.5f, 0.75f));

    const FVec3 value(2.0f, 3.0f, 6.0f);
    EXPECT_TRUE(FVec3::kZero.isZero());
    EXPECT_TRUE(FVec3::kOne.isOne());
    EXPECT_TRUE(FVec3(0.0f, 1.0f, 2.0f).isAnyZero());
    EXPECT_TRUE(FVec3(0.0f, 0.6f, 0.8f).isNormalized());
    EXPECT_FALSE(value.toString().isEmpty());

    EXPECT_FLOAT_EQ(value.lengthSquared(), 49.0f);
    EXPECT_FLOAT_EQ(value.length(), 7.0f);
    EXPECT_FLOAT_EQ(value.invertedLength(), 1.0f / 7.0f);
    EXPECT_NEAR(value.averageArithmetic(), 11.0f / 3.0f, 1e-5f);
    EXPECT_FLOAT_EQ(value.sumValues(), 11.0f);
    EXPECT_FLOAT_EQ(value.minValue(), 2.0f);
    EXPECT_FLOAT_EQ(value.maxValue(), 6.0f);
    expectVec3Near(value.getAbsolute(), value);
    expectVec3Near(FVec3(-2.0f, 3.0f, -6.0f).getAbsolute(), value);
    expectVec3Near(value.getNegative(), FVec3(-2.0f, -3.0f, -6.0f));
    expectVec3Near(FVec3(0.0f, 3.0f, 4.0f).getNormalized(), FVec3(0.0f, 0.6f, 0.8f));

    FVec3 normalized(0.0f, 3.0f, 4.0f);
    normalized.normalize();
    expectVec3Near(normalized, FVec3(0.0f, 0.6f, 0.8f));
    normalized = FVec3(0.0f, 3.0f, 4.0f);
    normalized.normalizeFast();
    expectVec3Near(normalized, FVec3(0.0f, 0.6f, 0.8f), 1e-4f);

    const FVec3 nanValue(std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.0f);
    const FVec3 infValue(std::numeric_limits<float>::infinity(), 1.0f, 2.0f);
    EXPECT_TRUE(nanValue.isNaN());
    EXPECT_TRUE(infValue.isInfinity());
    EXPECT_TRUE(nanValue.isNanOrInfinity());
    EXPECT_TRUE(infValue.isNanOrInfinity());
}

TEST(Vector3Test, OperatorsAndAlgorithms)
{
    const FVec3 a(6.0f, 8.0f, 10.0f);
    const FVec3 b(2.0f, 4.0f, 5.0f);
    FVec3 result;

    expectVec3Near(a + b, FVec3(8.0f, 12.0f, 15.0f));
    expectVec3Near(a - b, FVec3(4.0f, 4.0f, 5.0f));
    expectVec3Near(a * b, FVec3(12.0f, 32.0f, 50.0f));
    expectVec3Near(a / b, FVec3(3.0f, 2.0f, 2.0f));
    expectVec3Near(-a, FVec3(-6.0f, -8.0f, -10.0f));
    expectVec3Near(1.0f + b, FVec3(3.0f, 5.0f, 6.0f));
    expectVec3Near(10.0f - b, FVec3(8.0f, 6.0f, 5.0f));
    expectVec3Near(2.0f * b, FVec3(4.0f, 8.0f, 10.0f));
    expectVec3Near(20.0f / b, FVec3(10.0f, 5.0f, 4.0f));

    FVec3 c = a;
    c += b;
    c -= b;
    c *= b;
    c /= b;
    c += 1.0f;
    c -= 1.0f;
    c *= 2.0f;
    c /= 2.0f;
    expectVec3Near(c, a);

    EXPECT_TRUE(a == FVec3(6.0f, 8.0f, 10.0f));
    EXPECT_TRUE(a != b);
    EXPECT_TRUE(a > b);
    EXPECT_TRUE(a >= b);
    EXPECT_TRUE(b < a);
    EXPECT_TRUE(b <= a);
    EXPECT_TRUE(FVec3::nearEqual(FVec3(1.0f, 2.0f, 3.0f), FVec3(1.0f + 1e-7f, 2.0f, 3.0f - 1e-7f)));
    EXPECT_TRUE(FVec3::nearEqual(FVec3(1.0f, 2.0f, 3.0f), FVec3(1.01f, 2.01f, 3.01f), 0.02f));
    EXPECT_NE(getHash(a), 0u);

    EXPECT_FLOAT_EQ(FVec3::dot(a, b), 94.0f);
    expectVec3Near(FVec3::cross(FVec3::kUnitX, FVec3::kUnitY), FVec3::kUnitZ);
    expectVec3Near(FVec3::kUnitX ^ FVec3::kUnitY, FVec3::kUnitZ);
    EXPECT_FLOAT_EQ(FVec3::kUnitX | FVec3::kUnitY, 0.0f);

    FVec3::add(a, b, result);
    expectVec3Near(result, a + b);
    FVec3::subtract(a, b, result);
    expectVec3Near(result, a - b);
    FVec3::multiply(a, b, result);
    expectVec3Near(result, a * b);
    FVec3::divide(a, b, result);
    expectVec3Near(result, a / b);
    FVec3::min(a, b, result);
    expectVec3Near(result, FVec3::min(a, b));
    FVec3::max(a, b, result);
    expectVec3Near(result, FVec3::max(a, b));

    expectVec3Near(FVec3::mod(FVec3(5.5f, 7.25f, 9.75f), FVec3(2.0f, 3.0f, 4.0f)), FVec3(1.5f, 1.25f, 1.75f));
    expectVec3Near(FVec3::floor(FVec3(1.75f, -1.25f, 3.99f)), FVec3(1.0f, -2.0f, 3.0f));
    expectVec3Near(FVec3::frac(FVec3(1.75f, -1.25f, 3.99f)), FVec3(0.75f, -0.25f, 0.99f), 1e-4f);
    expectVec3Near(FVec3::round(FVec3(1.49f, 1.5f, -1.6f)), FVec3(1.0f, 2.0f, -2.0f));
    expectVec3Near(FVec3::ceil(FVec3(1.1f, -1.9f, 2.01f)), FVec3(2.0f, -1.0f, 3.0f));
    expectVec3Near(FVec3::abs(FVec3(-3.0f, 4.0f, -5.0f)), FVec3(3.0f, 4.0f, 5.0f));
    expectVec3Near(FVec3::clamp(FVec3(-1.0f, 5.0f, 7.0f), FVec3(0.0f, 1.0f, 2.0f), FVec3(2.0f, 3.0f, 6.0f)), FVec3(0.0f, 3.0f, 6.0f));
    FVec3::clamp(FVec3(-1.0f, 5.0f, 7.0f), FVec3(0.0f, 1.0f, 2.0f), FVec3(2.0f, 3.0f, 6.0f), result);
    expectVec3Near(result, FVec3(0.0f, 3.0f, 6.0f));

    expectVec3Near(FVec3::clampLength(FVec3(3.0f, 4.0f, 0.0f), 2.0f), FVec3(1.2f, 1.6f, 0.0f));
    expectVec3Near(FVec3::clampLength(FVec3(3.0f, 0.0f, 0.0f), 5.0f, 6.0f), FVec3(5.0f, 0.0f, 0.0f));
    FVec3::clampLength(FVec3(3.0f, 4.0f, 0.0f), 0.0f, 2.0f, result);
    expectVec3Near(result, FVec3(1.2f, 1.6f, 0.0f));

    EXPECT_FLOAT_EQ(FVec3::distance(FVec3(1.0f, 2.0f, 3.0f), FVec3(4.0f, 6.0f, 3.0f)), 5.0f);
    EXPECT_FLOAT_EQ(FVec3::distanceSquared(FVec3(1.0f, 2.0f, 3.0f), FVec3(4.0f, 6.0f, 3.0f)), 25.0f);
    expectVec3Near(FVec3::normalize(FVec3(0.0f, 3.0f, 4.0f)), FVec3(0.0f, 0.6f, 0.8f));
    expectVec3Near(FVec3::normalizeFast(FVec3(0.0f, 3.0f, 4.0f)), FVec3(0.0f, 0.6f, 0.8f), 1e-4f);
    FVec3::normalize(FVec3(0.0f, 3.0f, 4.0f), result);
    expectVec3Near(result, FVec3(0.0f, 0.6f, 0.8f));

    expectVec3Near(FVec3::lerp(FVec3::kZero, FVec3(10.0f, 20.0f, 30.0f), 0.25f), FVec3(2.5f, 5.0f, 7.5f));
    FVec3::lerp(FVec3::kZero, FVec3(10.0f, 20.0f, 30.0f), 0.75f, result);
    expectVec3Near(result, FVec3(7.5f, 15.0f, 22.5f));
    FVec3::smoothStep(FVec3::kZero, FVec3(10.0f, 20.0f, 30.0f), 0.5f, result);
    expectVec3Near(result, FVec3(5.0f, 10.0f, 15.0f));
    expectVec3Near(FVec3::moveTowards(FVec3::kZero, FVec3(10.0f, 0.0f, 0.0f), 2.5f), FVec3(2.5f, 0.0f, 0.0f));

    FVec3::hermite(FVec3::kZero, FVec3::kUnitX, FVec3(10.0f, 0.0f, 0.0f), FVec3::kUnitY, 0.0f, result);
    expectVec3Near(result, FVec3::kZero);
    FVec3::hermite(FVec3::kZero, FVec3::kUnitX, FVec3(10.0f, 0.0f, 0.0f), FVec3::kUnitY, 1.0f, result);
    expectVec3Near(result, FVec3(10.0f, 0.0f, 0.0f));

    FVec3::reflect(FVec3(1.0f, -1.0f, 0.0f), FVec3::kUnitY, result);
    expectVec3Near(result, FVec3(1.0f, 1.0f, 0.0f));
    expectVec3Near(FVec3::project(FVec3(3.0f, 4.0f, 0.0f), FVec3::kUnitX), FVec3(3.0f, 0.0f, 0.0f));
    expectVec3Near(FVec3::projectOnPlane(FVec3(3.0f, 4.0f, 5.0f), FVec3::kUnitY), FVec3(3.0f, 0.0f, 5.0f));

    EXPECT_FLOAT_EQ(FVec3::triangleArea(FVec3::kZero, FVec3(4.0f, 0.0f, 0.0f), FVec3(0.0f, 3.0f, 0.0f)), 6.0f);
    EXPECT_NEAR(FVec3::angle(FVec3::kUnitX, FVec3::kUnitY), TkMath::PI_OVER_2, 1e-5f);
    expectVec3Near(FVec3::snapToGrid(FVec3(1.1f, 2.6f, 5.9f), FVec3(2.0f, 2.0f, 2.0f)), FVec3(2.0f, 2.0f, 6.0f));

    FVec3 xAxis(1.0f, 1.0f, 0.0f);
    FVec3 yAxis(0.0f, 1.0f, 1.0f);
    FVec3 zAxis = FVec3::kUnitZ;
    FVec3::createOrthonormalBasis(xAxis, yAxis, zAxis);
    EXPECT_NEAR(xAxis.length(), 1.0f, 1e-4f);
    EXPECT_NEAR(yAxis.length(), 1.0f, 1e-4f);
    EXPECT_NEAR(zAxis.length(), 1.0f, 1e-4f);
    EXPECT_NEAR(FVec3::dot(xAxis, zAxis), 0.0f, 1e-4f);
    EXPECT_NEAR(FVec3::dot(yAxis, zAxis), 0.0f, 1e-4f);

    FVec3 firstAxis;
    FVec3 secondAxis;
    FVec3::kUnitY.findBestAxisVectors(firstAxis, secondAxis);
    EXPECT_NEAR(FVec3::dot(firstAxis, FVec3::kUnitY), 0.0f, 1e-4f);
    EXPECT_NEAR(FVec3::dot(secondAxis, FVec3::kUnitY), 0.0f, 1e-4f);
    EXPECT_NEAR(FVec3::dot(firstAxis, secondAxis), 0.0f, 1e-4f);
}

TEST(Vector3Test, TransformationsAndProjectionRoundTrip)
{
    FVec3 result;
    Quat rotation;
    Quat::rotationAxis(FVec3::kUnitZ, TkMath::PI_OVER_2, rotation);

    FVec3::transform(FVec3::kUnitX, rotation, result);
    expectVec3Near(result, FVec3::kUnitY, 1e-4f);
    expectVec3Near(FVec3::transform(FVec3::kUnitX, rotation), FVec3::kUnitY, 1e-4f);

    const Mat translation = Mat::translation(FVec3(10.0f, 20.0f, 30.0f));
    FVec3::transform(FVec3(1.0f, 2.0f, 3.0f), translation, result);
    expectVec3Near(result, FVec3(11.0f, 22.0f, 33.0f));

    FVec4 result4;
    FVec3::transform(FVec3(1.0f, 2.0f, 3.0f), translation, result4);
    expectVec4Near(result4, FVec4(11.0f, 22.0f, 33.0f, 1.0f));

    Mat3x3 rotation3x3;
    Mat3x3::rotationQuaternion(rotation, rotation3x3);
    FVec3::transform(FVec3::kUnitX, rotation3x3, result);
    expectVec3Near(result, FVec3::kUnitY, 1e-4f);

    Transform transform(FVec3(5.0f, 6.0f, 7.0f));
    FVec3::transform(FVec3(1.0f, 2.0f, 3.0f), transform, result);
    expectVec3Near(result, FVec3(6.0f, 8.0f, 10.0f));
    expectVec3Near(FVec3::transform(FVec3(1.0f, 2.0f, 3.0f), transform), FVec3(6.0f, 8.0f, 10.0f));

    FVec3::transformCoordinate(FVec3(1.0f, 2.0f, 3.0f), translation, result);
    expectVec3Near(result, FVec3(11.0f, 22.0f, 33.0f));
    FVec3::transformNormal(FVec3::kUnitX, translation, result);
    expectVec3Near(result, FVec3::kUnitX);

    const Mat identity = Mat::kIdentity;
    const FVec3 point(0.25f, -0.5f, 0.75f);
    FVec3 projected;
    FVec3 unprojected;
    FVec3::project(point, 0.0f, 0.0f, 100.0f, 200.0f, 0.0f, 1.0f, identity, projected);
    FVec3::unproject(projected, 0.0f, 0.0f, 100.0f, 200.0f, 0.0f, 1.0f, identity, unprojected);
    expectVec3Near(unprojected, point, 1e-4f);

    expectVec3Near(FVec3::snapToGrid(FVec3(2.1f, 0.2f, 0.2f), FVec3(1.0f, 1.0f, 1.0f), Quat::kIdentity, FVec3::kZero, FVec3::kUnitX), FVec3(3.0f, 0.0f, 0.0f));
}

TEST(Vector3Test, StabilityAndLayout)
{
    FVec3 zero = FVec3::kZero;
    zero.normalize();
    expectVec3Near(zero, FVec3::kZero);

    FVec3 rawBacked(1.0f, 2.0f, 3.0f);
    rawBacked.raw[0] = -7.0f;
    rawBacked.raw[1] = 8.0f;
    rawBacked.raw[2] = 9.0f;
    expectVec3Near(rawBacked, FVec3(-7.0f, 8.0f, 9.0f));

    FVec3 minClamped;
    FVec3::clampLength(FVec3(0.0f, 3.0f, 4.0f), 10.0f, 20.0f, minClamped);
    EXPECT_NEAR(minClamped.length(), 10.0f, 1e-4f);

    expectVec3Near(FVec3::project(FVec3(1.0f, 2.0f, 3.0f), FVec3::kZero), FVec3::kZero);

    Quat rotation = Quat::euler(0.0f, 90.0f, 0.0f);
    expectVec3Near(rotation * FVec3::kForward, FVec3::kRight, 1e-4f);

    FVec3 iterative = FVec3::kUnitX;
    for (int i = 0; i < 64; ++i)
    {
        iterative = FVec3::normalizeFast(iterative + FVec3(0.001f, -0.002f, 0.003f));
    }
    EXPECT_TRUE(iterative.isNormalized());
}
