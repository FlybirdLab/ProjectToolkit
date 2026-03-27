#include "vector_test_utils.hpp"

using namespace math_test;

TEST(TransformTest, ConstructionQueriesAndDirections)
{
    EXPECT_TRUE(Transform::kIdentity.isIdentity());
    EXPECT_FALSE(Transform::kIdentity.toString().isEmpty());
    EXPECT_FLOAT_EQ(Transform::kIdentity.getDeterminant(), 1.0f);

    Quat rotation = Quat::euler(0.0f, 90.0f, 0.0f);
    Transform transform(FVec3(1.0f, 2.0f, 3.0f), rotation, FVec3(2.0f, 3.0f, 4.0f));
    EXPECT_FALSE(transform.isIdentity());
    EXPECT_FALSE(transform.isNanOrInfinity());

    expectVec3Near(transform.getRight(), FVec3(0.0f, 0.0f, -1.0f), 1e-4f);
    expectVec3Near(transform.getLeft(), FVec3(0.0f, 0.0f, 1.0f), 1e-4f);
    expectVec3Near(transform.getForward(), FVec3(1.0f, 0.0f, 0.0f), 1e-4f);
    expectVec3Near(transform.getBackward(), FVec3(-1.0f, 0.0f, 0.0f), 1e-4f);
}

TEST(TransformTest, RotationWorldAndRoundTripConversions)
{
    Quat rotation = Quat::euler(0.0f, 90.0f, 0.0f);
    Transform transform(FVec3(10.0f, 20.0f, 30.0f), rotation, FVec3(2.0f, 2.0f, 2.0f));

    Mat rotationMatrix = transform.getRotation();
    expectMatNear(rotationMatrix, Mat::rotationQuaternion(rotation), 1e-4f);

    Mat world = transform.getWorld();
    Transform decomposed;
    world.decompose(decomposed);
    expectTransformNear(decomposed, transform, 1e-4f);

    Vec3 worldPoint = transform.localToWorld(FVec3(1.0f, 0.0f, 0.0f));
    Vec3 localPoint = transform.worldToLocal(worldPoint);
    expectVec3Near(localPoint, FVec3(1.0f, 0.0f, 0.0f), 1e-4f);

    Vec3 worldVector = transform.localToWorldVector(FVec3::kForward);
    Vec3 localVector = transform.worldToLocalVector(worldVector);
    expectVec3Near(localVector, FVec3::kForward, 1e-4f);

    Quat worldRotation = Quat::euler(0.0f, 180.0f, 0.0f);
    Quat localRotation = transform.worldToLocal(worldRotation);
    expectQuatNear(rotation * localRotation, worldRotation, 1e-4f);
}

TEST(TransformTest, CompositionAdditionSubtractionAndLerp)
{
    Transform a(FVec3(1.0f, 2.0f, 3.0f), Quat::euler(0.0f, 90.0f, 0.0f), FVec3(2.0f, 2.0f, 2.0f));
    Transform b(FVec3(4.0f, 5.0f, 6.0f), Quat::euler(0.0f, 0.0f, 90.0f), FVec3(3.0f, 4.0f, 5.0f));

    Transform addedVector = a.add(FVec3(10.0f, 20.0f, 30.0f));
    expectVec3Near(addedVector.translation, FVec3(11.0f, 22.0f, 33.0f));
    expectQuatNear(addedVector.orientation, a.orientation);
    expectVec3Near(addedVector.scale, a.scale);

    Transform addedTransform = a.add(b);
    expectVec3Near(addedTransform.translation, FVec3(5.0f, 7.0f, 9.0f));
    expectVec3Near(addedTransform.scale, FVec3(6.0f, 8.0f, 10.0f));

    Transform localToWorld = a.localToWorld(b);
    Transform worldToLocal = a.worldToLocal(localToWorld);
    expectTransformNear(worldToLocal, b, 1e-4f);

    Transform subtracted = addedTransform.subtract(a);
    expectVec3Near(subtracted.translation, b.translation);
    expectVec3Near(subtracted.scale, b.scale, 1e-4f);

    Transform lerped = Transform::lerp(Transform::kIdentity, Transform(FVec3(10.0f, 20.0f, 30.0f), Quat::euler(0.0f, 90.0f, 0.0f), FVec3(2.0f, 3.0f, 4.0f)), 0.5f);
    expectVec3Near(lerped.translation, FVec3(5.0f, 10.0f, 15.0f));
    expectVec3Near(lerped.scale, FVec3(1.5f, 2.0f, 2.5f));

    Transform lerpedOut;
    Transform::lerp(Transform::kIdentity, Transform(FVec3(10.0f, 20.0f, 30.0f), Quat::euler(0.0f, 90.0f, 0.0f), FVec3(2.0f, 3.0f, 4.0f)), 0.5f, lerpedOut);
    expectTransformNear(lerpedOut, lerped, 1e-4f);
}

TEST(TransformTest, RotationAssignmentAndGridAlignment)
{
    Transform transform(FVec3(0.0f, 0.0f, 0.0f));
    Mat rotation = Mat::rotationQuaternion(Quat::euler(0.0f, 90.0f, 0.0f));
    transform.setRotation(rotation);
    expectQuatNear(transform.orientation, Quat::euler(0.0f, 90.0f, 0.0f), 1e-4f);

    Transform reference(FVec3(1.0f, 2.0f, 3.0f), Quat::kIdentity, FVec3::kOne);
    Transform aligned = Transform::alignRotationToNormalAndSnapToGrid(FVec3(1.2f, 2.6f, 3.8f), FVec3::kUp, FVec3::kZero, reference, FVec3(1.0f, 1.0f, 1.0f), FVec3(2.0f, 2.0f, 2.0f));
    EXPECT_TRUE(aligned.orientation.isNormalized());
    expectVec3Near(aligned.scale, FVec3(2.0f, 2.0f, 2.0f));
}

TEST(TransformTest, StabilityMemoryLayoutAndRepeatedRoundTrip)
{
    Transform transform(FVec3(3.0f, 4.0f, 5.0f), Quat::euler(10.0f, 20.0f, 30.0f), FVec3(2.0f, 2.0f, 2.0f));
    EXPECT_FALSE(transform.isNanOrInfinity());

    const FVec3 localPoint(1.0f, -2.0f, 3.0f);
    FVec3 current = localPoint;
    for (int i = 0; i < 16; ++i)
    {
        current = transform.worldToLocal(transform.localToWorld(current));
    }
    expectVec3Near(current, localPoint, 1e-4f);

    const Quat worldRotation = Quat::euler(15.0f, 45.0f, 5.0f);
    Quat localRotation;
    transform.worldToLocal(worldRotation, localRotation);
    expectQuatNear(transform.orientation * localRotation, worldRotation, 1e-4f);

    Transform zeroScale(FVec3::kZero, Quat::kIdentity, FVec3::kZero);
    EXPECT_FLOAT_EQ(zeroScale.getDeterminant(), 0.0f);
    EXPECT_FALSE(zeroScale.isNanOrInfinity());
}
