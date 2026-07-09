#include "vector_test_utils.hpp"

using namespace math_test;

TEST(BoundingBoxTest, ConstantsPropertiesAndCorners)
{
    expectBoundingBoxNear(BoundingBox::kZero, BoundingBox(FVec3::kZero));
    EXPECT_FALSE(BoundingBox::kZero.toString().isEmpty());

    BoundingBox box(FVec3(-1.0f, -2.0f, -3.0f), FVec3(4.0f, 6.0f, 8.0f));
    expectVec3Near(box.getCenter(), FVec3(1.5f, 2.0f, 2.5f));
    expectVec3Near(box.getSize(), FVec3(5.0f, 8.0f, 11.0f));
    EXPECT_FLOAT_EQ(box.getVolume(), 440.0f);

    FVec3 corners[8];
    box.getCorners(corners);
    expectVec3Near(corners[0], FVec3(-1.0f, 6.0f, 8.0f));
    expectVec3Near(corners[7], FVec3(-1.0f, -2.0f, -3.0f));

    box.setCenter(FVec3(10.0f, 20.0f, 30.0f));
    expectVec3Near(box.getCenter(), FVec3(10.0f, 20.0f, 30.0f));
    expectVec3Near(box.getSize(), FVec3(5.0f, 8.0f, 11.0f));

    box.setSize(FVec3(2.0f, 4.0f, 6.0f));
    expectVec3Near(box.getCenter(), FVec3(10.0f, 20.0f, 30.0f));
    expectVec3Near(box.getSize(), FVec3(2.0f, 4.0f, 6.0f));
}

TEST(BoundingBoxTest, CreationMergeAndTransform)
{
    const FVec3 points[] = {
        FVec3(-2.0f, 1.0f, 3.0f),
        FVec3(4.0f, -5.0f, 6.0f),
        FVec3(0.5f, 2.0f, -7.0f)};
    BoundingBox fromPoints;
    BoundingBox::fromPoints(points, 3, fromPoints);
    expectBoundingBoxNear(fromPoints, BoundingBox(FVec3(-2.0f, -5.0f, -7.0f), FVec3(4.0f, 2.0f, 6.0f)));

    const DVec3 dpoints[] = {
        DVec3(-1.0, -2.0, -3.0),
        DVec3(2.0, 3.5, 4.0)};
    BoundingBox fromDPoints;
    BoundingBox::fromPoints(dpoints, 2, fromDPoints);
    expectBoundingBoxNear(fromDPoints, BoundingBox(FVec3(-1.0f, -2.0f, -3.0f), FVec3(2.0f, 3.5f, 4.0f)));

    BoundingSphere sphere(FVec3(1.0f, 2.0f, 3.0f), 2.5f);
    expectBoundingBoxNear(BoundingBox::fromSphere(sphere), BoundingBox(FVec3(-1.5f, -0.5f, 0.5f), FVec3(3.5f, 4.5f, 5.5f)));

    BoundingBox merged(FVec3(-1.0f, -1.0f, -1.0f), FVec3(1.0f, 1.0f, 1.0f));
    merged.merge(FVec3(3.0f, 0.0f, 0.0f));
    merged.merge(BoundingBox(FVec3(-2.0f, -3.0f, -4.0f), FVec3(-1.5f, -2.5f, 0.0f)));
    expectBoundingBoxNear(merged, BoundingBox(FVec3(-2.0f, -3.0f, -4.0f), FVec3(3.0f, 1.0f, 1.0f)));

    expectBoundingBoxNear(merged.makeOffsetted(FVec3(10.0f, 20.0f, 30.0f)),
                          BoundingBox(FVec3(8.0f, 17.0f, 26.0f), FVec3(13.0f, 21.0f, 31.0f)));
    expectBoundingBoxNear(BoundingBox::makeScaled(merged, 2.0f),
                          BoundingBox(FVec3(-4.5f, -5.0f, -6.5f), FVec3(5.5f, 3.0f, 3.5f)));

    const Mat translation = Mat::translation(FVec3(5.0f, 6.0f, 7.0f));
    expectBoundingBoxNear(BoundingBox::transform(BoundingBox(FVec3(-1.0f, -1.0f, -1.0f), FVec3(1.0f, 1.0f, 1.0f)), translation),
                          BoundingBox(FVec3(4.0f, 5.0f, 6.0f), FVec3(6.0f, 7.0f, 8.0f)));
}

TEST(BoundingBoxTest, IntersectionContainmentDistanceAndStability)
{
    const BoundingBox box(FVec3(-1.0f, -1.0f, -1.0f), FVec3(1.0f, 1.0f, 1.0f));
    const Ray ray(FVec3(-5.0f, 0.0f, 0.0f), FVec3::kUnitX);
    real distance = 0.0f;
    EXPECT_TRUE(box.intersects(ray, distance));
    EXPECT_NEAR(distance, 4.0f, 1e-4f);

    FVec3 hitPoint;
    EXPECT_TRUE(box.intersects(ray, hitPoint));
    expectVec3Near(hitPoint, FVec3(-1.0f, 0.0f, 0.0f), 1e-4f);

    const Plane plane(FVec3::kUnitX, 0.0f);
    EXPECT_EQ(box.intersects(plane), PlaneIntersectionType::kIntersecting);

    EXPECT_EQ(box.contains(FVec3::kZero), ContainmentType::kContains);
    EXPECT_EQ(box.contains(BoundingBox(FVec3(-0.5f, -0.5f, -0.5f), FVec3(0.5f, 0.5f, 0.5f))), ContainmentType::kContains);
    EXPECT_EQ(box.contains(BoundingSphere(FVec3::kZero, 0.5f)), ContainmentType::kContains);
    EXPECT_TRUE(box.intersects(BoundingBox(FVec3(0.5f, 0.5f, 0.5f), FVec3(2.0f, 2.0f, 2.0f))));
    EXPECT_TRUE(box.intersects(BoundingSphere(FVec3(1.5f, 0.0f, 0.0f), 0.75f)));

    EXPECT_NEAR(box.distance(FVec3(3.0f, 0.0f, 0.0f)), 2.0f, 1e-4f);
    EXPECT_NEAR(box.distance(BoundingBox(FVec3(4.0f, -1.0f, -1.0f), FVec3(6.0f, 1.0f, 1.0f))), 3.0f, 1e-4f);

    EXPECT_TRUE(BoundingBox::nearEqual(box, BoundingBox(FVec3(-1.0f, -1.0f, -1.0f), FVec3(1.0f, 1.0f, 1.0f))));
}
