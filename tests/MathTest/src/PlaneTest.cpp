#include "vector_test_utils.hpp"

using namespace math_test;

TEST(PlaneTest, ConstructionQueriesMutationAndTransforms)
{
    Plane plane(FVec3::kUnitY, -2.0f);
    EXPECT_FALSE(plane.toString().isEmpty());
    expectVec3Near(plane.pointOnPlane(), FVec3(0.0f, 2.0f, 0.0f));

    Plane fromPoint(FVec3(0.0f, 2.0f, 0.0f), FVec3::kUnitY);
    expectPlaneNear(fromPoint, plane);

    Plane fromTriangle(FVec3::kZero, FVec3::kUnitX, FVec3::kUnitZ);
    EXPECT_TRUE(fromTriangle.normal.isNormalized());

    Plane scaled = plane * 2.0f;
    expectPlaneNear(scaled, Plane(FVec3(0.0f, 2.0f, 0.0f), -4.0f));
    expectPlaneNear(2.0f * plane, scaled);
    EXPECT_TRUE(plane == Plane(FVec3::kUnitY, -2.0f));
    EXPECT_TRUE(plane != Plane(FVec3::kUnitX, -2.0f));
    EXPECT_TRUE(Plane::nearEqual(plane, Plane(FVec3::kUnitY, -2.0f)));

    Plane nonUnit(FVec3(0.0f, 2.0f, 0.0f), -4.0f);
    nonUnit.normalize();
    expectPlaneNear(nonUnit, plane);
    expectPlaneNear(Plane::normalize(Plane(FVec3(0.0f, 2.0f, 0.0f), -4.0f)), plane);
    Plane normalizedOut;
    Plane::normalize(Plane(FVec3(0.0f, 2.0f, 0.0f), -4.0f), normalizedOut);
    expectPlaneNear(normalizedOut, plane);

    Plane negated = plane.negated();
    plane.negate();
    expectPlaneNear(plane, negated);

    Plane moved(FVec3::kUnitY, -2.0f);
    moved.translate(FVec3(0.0f, 3.0f, 0.0f));
    expectPlaneNear(moved, Plane(FVec3::kUnitY, -5.0f));
    expectPlaneNear(Plane::translated(Plane(FVec3::kUnitY, -2.0f), FVec3(0.0f, 3.0f, 0.0f)), moved);
    expectPlaneNear(Plane::translated(Plane(FVec3::kUnitY, -2.0f), 0.0f, 3.0f, 0.0f), moved);

    expectVec3Near(Plane::intersection(Plane(FVec3::kUnitX, 0.0f), Plane(FVec3::kUnitY, 0.0f), Plane(FVec3::kUnitZ, 0.0f)), FVec3::kZero, 1e-4f);

    Quat rotation = Quat::euler(0.0f, 0.0f, 90.0f);
    Plane rotated = Plane::transform(Plane(FVec3::kUnitX, -1.0f), rotation);
    expectVec3Near(rotated.normal, FVec3::kUnitY, 1e-4f);

    Plane translatedByMatrix = Plane::transform(Plane(FVec3::kUnitY, -2.0f), Mat::translation(FVec3(0.0f, 3.0f, 0.0f)));
    expectPlaneNear(translatedByMatrix, Plane(FVec3::kUnitY, -5.0f), 1e-4f);
}

TEST(PlaneTest, DotAndIntersectionHelpers)
{
    const Plane plane(FVec3::kUnitY, -2.0f);
    EXPECT_EQ(plane.intersects(FVec3(0.0f, 5.0f, 0.0f)), PlaneIntersectionType::kFront);
    EXPECT_EQ(plane.intersects(FVec3(0.0f, 0.0f, 0.0f)), PlaneIntersectionType::kBack);
    EXPECT_EQ(plane.intersects(FVec3(0.0f, 2.0f, 0.0f)), PlaneIntersectionType::kIntersecting);

    real dot = 0.0f;
    Plane::dot(plane, FVec4(0.0f, 2.0f, 0.0f, 1.0f), dot);
    EXPECT_NEAR(dot, 0.0f, 1e-4f);
    EXPECT_NEAR(Plane::dot(plane, FVec4(0.0f, 5.0f, 0.0f, 1.0f)), 3.0f, 1e-4f);
    Plane::dotCoordinate(plane, FVec3(0.0f, 5.0f, 0.0f), dot);
    EXPECT_NEAR(dot, 3.0f, 1e-4f);
    EXPECT_NEAR(Plane::dotCoordinate(plane, FVec3(0.0f, 0.0f, 0.0f)), -2.0f, 1e-4f);
    Plane::dotNormal(plane, FVec3(1.0f, 5.0f, 7.0f), dot);
    EXPECT_NEAR(dot, 5.0f, 1e-4f);
    EXPECT_NEAR(Plane::dotNormal(plane, FVec3(1.0f, 5.0f, 7.0f)), 5.0f, 1e-4f);
}
