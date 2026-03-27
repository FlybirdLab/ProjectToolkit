#include "vector_test_utils.hpp"

#include <limits>

using namespace math_test;

TEST(QuatTest, ConstantsQueriesAndBasicMutation)
{
    expectQuatNear(Quat::kIdentity, Quat(0.0f, 0.0f, 0.0f, 1.0f));
    EXPECT_TRUE(Quat::kIdentity.isIdentity());
    EXPECT_TRUE(Quat::kIdentity.isNormalized());
    EXPECT_FALSE(Quat::kIdentity.toString().isEmpty());

    Quat q(FVec4(1.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_FLOAT_EQ(q.lengthSquared(), 30.0f);
    EXPECT_NEAR(q.length(), TkMath::sqrt(30.0f), 1e-5f);

    Quat conjugated = q.conjugated();
    EXPECT_FLOAT_EQ(conjugated.x, -1.0f);
    EXPECT_FLOAT_EQ(conjugated.w, 4.0f);
    q.conjugate();
    EXPECT_FLOAT_EQ(q.x, -1.0f);
    EXPECT_FLOAT_EQ(q.y, -2.0f);
    EXPECT_FLOAT_EQ(q.z, -3.0f);
    EXPECT_FLOAT_EQ(q.w, 4.0f);

    Quat scaled = Quat::kIdentity;
    scaled.multiply(2.0f);
    EXPECT_FLOAT_EQ(scaled.w, 2.0f);
    scaled.normalize();
    expectQuatNear(scaled, Quat::kIdentity);

    const Quat nanQuat(std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.0f, 1.0f);
    const Quat infQuat(std::numeric_limits<float>::infinity(), 0.0f, 0.0f, 1.0f);
    EXPECT_TRUE(nanQuat.isNaN());
    EXPECT_TRUE(infQuat.isInfinity());
    EXPECT_TRUE(nanQuat.isNanOrInfinity());
    EXPECT_TRUE(infQuat.isNanOrInfinity());
}

TEST(QuatTest, AlgebraInterpolationAndAngles)
{
    Quat rotX90;
    Quat::rotationAxis(FVec3::kUnitX, TkMath::PI_OVER_2, rotX90);
    Quat rotY90;
    Quat::rotationAxis(FVec3::kUnitY, TkMath::PI_OVER_2, rotY90);

    Quat combined = rotY90 * rotX90;
    EXPECT_TRUE(combined.isNormalized());
    expectVec3Near(rotY90 * FVec3::kForward, FVec3::kRight, 1e-4f);

    Quat sum;
    Quat::add(rotX90, rotY90, sum);
    EXPECT_FLOAT_EQ(sum.x, rotX90.x + rotY90.x);
    EXPECT_FLOAT_EQ(sum.y, rotX90.y + rotY90.y);

    Quat diff;
    Quat::subtract(rotX90, rotY90, diff);
    EXPECT_FLOAT_EQ(diff.x, rotX90.x - rotY90.x);

    Quat scaled;
    Quat::multiply(rotX90, 0.5f, scaled);
    EXPECT_NEAR(scaled.x, rotX90.x * 0.5f, 1e-5f);

    Quat mulStatic;
    Quat::multiply(rotY90, rotX90, mulStatic);
    expectQuatNear(mulStatic, combined);

    Quat negated;
    Quat::negate(rotX90, negated);
    EXPECT_FLOAT_EQ(negated.x, -rotX90.x);
    EXPECT_FLOAT_EQ(negated.w, -rotX90.w);

    Quat lerped = Quat::lerp(Quat::kIdentity, rotY90, 0.5f);
    EXPECT_TRUE(lerped.isNormalized());
    Quat slerped;
    Quat::slerp(Quat::kIdentity, rotY90, 0.5f, slerped);
    EXPECT_TRUE(slerped.isNormalized());
    EXPECT_NEAR(Quat::angleBetween(Quat::kIdentity, rotY90), 90.0f, 1e-3f);

    Quat inverse = Quat::invert(rotY90);
    expectQuatNear(inverse * rotY90, Quat::kIdentity, 1e-4f);
}

TEST(QuatTest, CreationAndRoundTrips)
{
    Quat axis;
    Quat::rotationAxis(FVec3::kUnitZ, TkMath::PI_OVER_2, axis);
    EXPECT_NEAR(axis.getAngle(), TkMath::PI_OVER_2, 1e-4f);
    expectVec3Near(axis.getAxis(), FVec3::kUnitZ, 1e-4f);

    Quat cosAxis;
    Quat::rotationCosAxis(FVec3::kUnitZ, 0.0f, cosAxis);
    expectQuatNear(cosAxis, axis, 1e-4f);

    Quat euler = Quat::euler(0.0f, 90.0f, 0.0f);
    expectVec3Near(euler * FVec3::kForward, FVec3::kRight, 1e-4f);

    Quat yawPitchRoll = Quat::rotationYawPitchRoll(TkMath::PI_OVER_2, 0.0f, 0.0f);
    expectQuatNear(yawPitchRoll, euler, 1e-4f);

    Mat rotationMatrix = Mat::rotationQuaternion(axis);
    Quat fromMatrix = Quat::rotationMatrix(rotationMatrix);
    expectQuatNear(fromMatrix, axis, 1e-4f);

    Mat3x3 rotation3 = Mat3x3::rotationQuaternion(axis);
    Quat fromMatrix3;
    Quat::rotationMatrix(rotation3, fromMatrix3);
    expectQuatNear(fromMatrix3, axis, 1e-4f);

    Quat look = Quat::lookRotation(FVec3::kForward, FVec3::kUp);
    expectQuatNear(look, Quat::kIdentity, 1e-4f);

    Quat fromDirection = Quat::fromDirection(FVec3::kForward);
    expectQuatNear(fromDirection, Quat::kIdentity, 1e-4f);

    Quat fromTo = Quat::getRotationFromTo(FVec3::kForward, FVec3::kRight, FVec3::kUp);
    expectVec3Near(fromTo * FVec3::kForward, FVec3::kRight, 1e-4f);

    Quat between = Quat::findBetween(FVec3::kForward, FVec3::kRight);
    expectVec3Near(between * FVec3::kForward, FVec3::kRight, 1e-4f);
}

TEST(QuatTest, LookAtBillboardAndReferenceBasedRotation)
{
    Quat lookAt;
    Quat::lookAt(FVec3::kZero, FVec3::kForward, FVec3::kUp, lookAt);
    expectQuatNear(lookAt, Quat::kIdentity, 1e-4f);

    Quat rotationLookAt;
    Quat::rotationLookAt(FVec3::kForward, FVec3::kUp, rotationLookAt);
    expectQuatNear(rotationLookAt, Quat::kIdentity, 1e-4f);

    Quat billboard;
    Quat::billboard(FVec3::kZero, FVec3(0.0f, 0.0f, -10.0f), FVec3::kUp, FVec3::kForward, billboard);
    EXPECT_TRUE(billboard.isNormalized());

    Transform reference(FVec3::kZero, Quat::kIdentity, FVec3::kOne);
    Quat normalRotation = Quat::GetRotationFromNormal(FVec3::kUp, reference);
    expectVec3Near(normalRotation * FVec3::kForward, FVec3::kUp, 1e-4f);
}

TEST(QuatTest, StabilityRobustnessAndMemoryLayout)
{
    Quat rawBacked(1.0f, 2.0f, 3.0f, 4.0f);
    rawBacked.raw[0] = -1.0f;
    rawBacked.raw[1] = 5.0f;
    rawBacked.raw[2] = -7.0f;
    rawBacked.raw[3] = 9.0f;
    EXPECT_FLOAT_EQ(rawBacked.x, -1.0f);
    EXPECT_FLOAT_EQ(rawBacked.y, 5.0f);
    EXPECT_FLOAT_EQ(rawBacked.z, -7.0f);
    EXPECT_FLOAT_EQ(rawBacked.w, 9.0f);

    Quat zero(0.0f, 0.0f, 0.0f, 0.0f);
    zero.normalize();
    EXPECT_FLOAT_EQ(zero.x, 0.0f);
    EXPECT_FLOAT_EQ(zero.y, 0.0f);
    EXPECT_FLOAT_EQ(zero.z, 0.0f);
    EXPECT_FLOAT_EQ(zero.w, 0.0f);
    zero.invert();
    EXPECT_FLOAT_EQ(zero.x, 0.0f);
    EXPECT_FLOAT_EQ(zero.y, 0.0f);
    EXPECT_FLOAT_EQ(zero.z, 0.0f);
    EXPECT_FLOAT_EQ(zero.w, 0.0f);

    Quat accumulated = Quat::kIdentity;
    const Quat step = Quat::euler(0.0f, 10.0f, 0.0f);
    for (int i = 0; i < 36; ++i)
    {
        accumulated *= step;
        accumulated.normalize();
    }
    EXPECT_TRUE(accumulated.isNormalized());
    expectQuatNear(accumulated, Quat::kIdentity, 1e-3f);

    const Quat opposite = Quat::findBetween(FVec3::kForward, FVec3::kBackward);
    expectVec3Near(opposite * FVec3::kForward, FVec3::kBackward, 1e-4f);
}
