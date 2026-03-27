#include "vector_test_utils.hpp"

using namespace math_test;

TEST(BoundingSphereTest, BasicsBoundingBoxAndString)
{
    expectBoundingSphereNear(BoundingSphere::kEmpty, BoundingSphere(FVec3::kZero, 0.0f));

    BoundingSphere sphere(FVec3(1.0f, 2.0f, 3.0f), 4.0f);
    EXPECT_FALSE(sphere.toString().isEmpty());

    BoundingBox box = sphere.getBoundingBox();
    expectBoundingBoxNear(box, BoundingBox(FVec3(-3.0f, -2.0f, -1.0f), FVec3(5.0f, 6.0f, 7.0f)));

    BoundingBox boxOut;
    sphere.getBoundingBox(boxOut);
    expectBoundingBoxNear(boxOut, box);
}

TEST(BoundingSphereTest, CreationMergeTransformAndNearEqual)
{
    const FVec3 points[] = {
        FVec3(-1.0f, 0.0f, 0.0f),
        FVec3(1.0f, 0.0f, 0.0f),
        FVec3(0.0f, 2.0f, 0.0f)};
    BoundingSphere fromPoints;
    BoundingSphere::fromPoints(points, 3, fromPoints);
    expectVec3Near(fromPoints.center, FVec3(0.0f, 2.0f / 3.0f, 0.0f), 1e-4f);
    EXPECT_NEAR(fromPoints.radius, 4.0f / 3.0f, 1e-4f);
    for (const FVec3 &point : points)
    {
        EXPECT_TRUE(fromPoints.contains(point) != ContainmentType::kDisjoint);
    }

    BoundingSphere fromBox;
    BoundingSphere::fromBox(BoundingBox(FVec3(-1.0f, -2.0f, -3.0f), FVec3(3.0f, 2.0f, 1.0f)), fromBox);
    expectVec3Near(fromBox.center, FVec3(1.0f, 0.0f, -1.0f));
    EXPECT_NEAR(fromBox.radius, TkMath::sqrt(48.0f) * 0.5f, 1e-4f);

    BoundingSphere merged;
    BoundingSphere::merge(BoundingSphere(FVec3(-2.0f, 0.0f, 0.0f), 1.0f), BoundingSphere(FVec3(3.0f, 0.0f, 0.0f), 2.0f), merged);
    EXPECT_TRUE(merged.contains(BoundingSphere(FVec3(-2.0f, 0.0f, 0.0f), 1.0f)) != ContainmentType::kDisjoint);
    EXPECT_TRUE(merged.contains(BoundingSphere(FVec3(3.0f, 0.0f, 0.0f), 2.0f)) != ContainmentType::kDisjoint);

    BoundingSphere expanded;
    BoundingSphere::merge(BoundingSphere(FVec3::kZero, 1.0f), FVec3(5.0f, 0.0f, 0.0f), expanded);
    EXPECT_TRUE(expanded.contains(FVec3(5.0f, 0.0f, 0.0f)) != ContainmentType::kDisjoint);

    BoundingSphere transformed;
    BoundingSphere::transform(BoundingSphere(FVec3(1.0f, 2.0f, 3.0f), 2.0f), Mat::scaling(FVec3(2.0f, 3.0f, 4.0f)) * Mat::translation(FVec3(10.0f, 20.0f, 30.0f)), transformed);
    expectVec3Near(transformed.center, FVec3(12.0f, 26.0f, 42.0f), 1e-4f);
    EXPECT_NEAR(transformed.radius, 8.0f, 1e-4f);

    EXPECT_TRUE(BoundingSphere::nearEqual(BoundingSphere(FVec3(1.0f, 2.0f, 3.0f), 4.0f), BoundingSphere(FVec3(1.0f, 2.0f, 3.0f), 4.0f)));
}

TEST(BoundingSphereTest, IntersectionsContainmentAndRobustness)
{
    const BoundingSphere sphere(FVec3::kZero, 2.0f);
    const Ray ray(FVec3(-5.0f, 0.0f, 0.0f), FVec3::kUnitX);
    real distance = 0.0f;
    EXPECT_TRUE(sphere.intersects(ray, distance));
    EXPECT_NEAR(distance, 3.0f, 1e-4f);

    FVec3 hitPoint;
    EXPECT_TRUE(sphere.intersects(ray, hitPoint));
    expectVec3Near(hitPoint, FVec3(-2.0f, 0.0f, 0.0f), 1e-4f);

    EXPECT_EQ(sphere.intersects(Plane(FVec3::kUnitX, 0.0f)), PlaneIntersectionType::kIntersecting);
    EXPECT_TRUE(sphere.intersects(FVec3(-1.0f, 0.0f, 0.0f), FVec3(1.0f, 0.0f, 0.0f), FVec3(0.0f, 1.0f, 0.0f)));
    EXPECT_TRUE(sphere.intersects(BoundingBox(FVec3(1.5f, -0.5f, -0.5f), FVec3(3.0f, 0.5f, 0.5f))));
    EXPECT_TRUE(sphere.intersects(BoundingSphere(FVec3(3.5f, 0.0f, 0.0f), 2.0f)));

    EXPECT_EQ(sphere.contains(FVec3::kZero), ContainmentType::kContains);
    EXPECT_EQ(sphere.contains(BoundingSphere(FVec3::kZero, 1.0f)), ContainmentType::kContains);
    EXPECT_EQ(sphere.contains(BoundingBox(FVec3(-0.5f, -0.5f, -0.5f), FVec3(0.5f, 0.5f, 0.5f))), ContainmentType::kContains);

    BoundingSphere repeated = sphere;
    for (int i = 0; i < 32; ++i)
    {
        BoundingSphere::merge(repeated, FVec3(2.0f, 0.0f, 0.0f), repeated);
    }
    EXPECT_TRUE(repeated.contains(FVec3(2.0f, 0.0f, 0.0f)) != ContainmentType::kDisjoint);
    EXPECT_FALSE(repeated.center.isNanOrInfinity());
}
