#include "vector_test_utils.hpp"

using namespace math_test;

TEST(CollisionsTest, ClosestPointAndDistanceHelpers)
{
    FVec2 cp2;
    TkCollision::closestPointPointLine(FVec2(2.0f, 3.0f), FVec2::kZero, FVec2(4.0f, 0.0f), cp2);
    expectVec2Near(cp2, FVec2(2.0f, 0.0f));
    expectVec2Near(TkCollision::closestPointPointLine(FVec2(2.0f, 3.0f), FVec2::kZero, FVec2(4.0f, 0.0f)), FVec2(2.0f, 0.0f));

    FVec3 cp3;
    TkCollision::closestPointPointLine(FVec3(2.0f, 3.0f, 0.0f), FVec3::kZero, FVec3(4.0f, 0.0f, 0.0f), cp3);
    expectVec3Near(cp3, FVec3(2.0f, 0.0f, 0.0f));
    expectVec3Near(TkCollision::closestPointPointLine(FVec3(2.0f, 3.0f, 0.0f), FVec3::kZero, FVec3(4.0f, 0.0f, 0.0f)), FVec3(2.0f, 0.0f, 0.0f));

    const Triangle tri(FVec3::kZero, FVec3(4.0f, 0.0f, 0.0f), FVec3(0.0f, 4.0f, 0.0f));
    TkCollision::closestPointPointTriangle(FVec3(1.0f, 1.0f, 2.0f), tri.v0, tri.v1, tri.v2, cp3);
    expectVec3Near(cp3, FVec3(1.0f, 1.0f, 0.0f), 1e-4f);
    expectVec3Near(TkCollision::closestPointPointTriangle(FVec3(1.0f, 1.0f, 2.0f), tri.v0, tri.v1, tri.v2), FVec3(1.0f, 1.0f, 0.0f), 1e-4f);

    const Plane plane(FVec3::kUnitY, -2.0f);
    TkCollision::closestPointPlanePoint(plane, FVec3(1.0f, 5.0f, 3.0f), cp3);
    expectVec3Near(cp3, FVec3(1.0f, 2.0f, 3.0f), 1e-4f);
    expectVec3Near(TkCollision::closestPointPlanePoint(plane, FVec3(1.0f, 5.0f, 3.0f)), FVec3(1.0f, 2.0f, 3.0f), 1e-4f);

    const BoundingBox box(FVec3(-1.0f, -1.0f, -1.0f), FVec3(1.0f, 1.0f, 1.0f));
    TkCollision::closestPointBoxPoint(box, FVec3(4.0f, 0.0f, 0.0f), cp3);
    expectVec3Near(cp3, FVec3(1.0f, 0.0f, 0.0f));
    expectVec3Near(TkCollision::closestPointBoxPoint(box, FVec3(4.0f, 0.0f, 0.0f)), FVec3(1.0f, 0.0f, 0.0f));

    const Rectangle rect(10.0f, 20.0f, 30.0f, 40.0f);
    TkCollision::closestPointRectanglePoint(rect, FVec2(100.0f, 10.0f), cp2);
    expectVec2Near(cp2, FVec2(40.0f, 20.0f));
    expectVec2Near(TkCollision::closestPointRectanglePoint(rect, FVec2(100.0f, 10.0f)), FVec2(40.0f, 20.0f));

    const BoundingSphere sphere(FVec3::kZero, 2.0f);
    TkCollision::closestPointSpherePoint(sphere, FVec3(5.0f, 0.0f, 0.0f), cp3);
    expectVec3Near(cp3, FVec3(2.0f, 0.0f, 0.0f), 1e-4f);
    expectVec3Near(TkCollision::closestPointSpherePoint(sphere, FVec3(5.0f, 0.0f, 0.0f)), FVec3(2.0f, 0.0f, 0.0f), 1e-4f);

    TkCollision::closestPointSphereSphere(BoundingSphere(FVec3::kZero, 2.0f), BoundingSphere(FVec3(5.0f, 0.0f, 0.0f), 1.0f), cp3);
    expectVec3Near(cp3, FVec3(2.0f, 0.0f, 0.0f), 1e-4f);
    expectVec3Near(TkCollision::closestPointSphereSphere(BoundingSphere(FVec3::kZero, 2.0f), BoundingSphere(FVec3(5.0f, 0.0f, 0.0f), 1.0f)), FVec3(2.0f, 0.0f, 0.0f), 1e-4f);

    EXPECT_NEAR(TkCollision::distancePlanePoint(plane, FVec3(0.0f, 5.0f, 0.0f)), 3.0f, 1e-4f);
    EXPECT_NEAR(TkCollision::distanceBoxPoint(box, FVec3(4.0f, 0.0f, 0.0f)), 3.0f, 1e-4f);
    EXPECT_NEAR(TkCollision::distanceBoxBox(box, BoundingBox(FVec3(4.0f, -1.0f, -1.0f), FVec3(5.0f, 1.0f, 1.0f))), 3.0f, 1e-4f);
    EXPECT_NEAR(TkCollision::distanceSpherePoint(sphere, FVec3(5.0f, 0.0f, 0.0f)), 3.0f, 1e-4f);
    EXPECT_NEAR(TkCollision::distanceSphereSphere(sphere, BoundingSphere(FVec3(10.0f, 0.0f, 0.0f), 3.0f)), 5.0f, 1e-4f);
}

TEST(CollisionsTest, RayPlaneTriangleBoxSphereHelpers)
{
    const Ray ray(FVec3(-5.0f, 0.0f, 0.0f), FVec3::kUnitX);
    EXPECT_TRUE(TkCollision::rayIntersectsPoint(ray, FVec3::kZero));

    Vec3 point;
    EXPECT_TRUE(TkCollision::rayIntersectsRay(ray, Ray(FVec3::kZero, FVec3::kUnitY), point));
    expectVec3Near(point, FVec3::kZero, 1e-4f);

    real distance = 0.0f;
    EXPECT_TRUE(TkCollision::rayIntersectsPlane(ray, Plane(FVec3::kUnitX, 0.0f), distance));
    EXPECT_NEAR(distance, 5.0f, 1e-4f);
    EXPECT_TRUE(TkCollision::rayIntersectsPlane(ray, Plane(FVec3::kUnitX, 0.0f), point));
    expectVec3Near(point, FVec3::kZero, 1e-4f);

    Vec3 normal;
    EXPECT_TRUE(TkCollision::rayIntersectsTriangle(Ray(FVec3(0.0f, 0.0f, -5.0f), FVec3::kForward), FVec3(-1.0f, -1.0f, 0.0f), FVec3(1.0f, -1.0f, 0.0f), FVec3(0.0f, 1.0f, 0.0f), distance));
    EXPECT_TRUE(TkCollision::rayIntersectsTriangle(Ray(FVec3(0.0f, 0.0f, -5.0f), FVec3::kForward), FVec3(-1.0f, -1.0f, 0.0f), FVec3(1.0f, -1.0f, 0.0f), FVec3(0.0f, 1.0f, 0.0f), distance, normal));
    EXPECT_TRUE(TkCollision::rayIntersectsTriangle(Ray(FVec3(0.0f, 0.0f, -5.0f), FVec3::kForward), FVec3(-1.0f, -1.0f, 0.0f), FVec3(1.0f, -1.0f, 0.0f), FVec3(0.0f, 1.0f, 0.0f), point));

    const BoundingBox box(FVec3(-1.0f, -1.0f, -1.0f), FVec3(1.0f, 1.0f, 1.0f));
    EXPECT_TRUE(TkCollision::rayIntersectsBox(ray, box, distance));
    EXPECT_TRUE(TkCollision::rayIntersectsBox(ray, box, distance, normal));
    EXPECT_TRUE(TkCollision::rayIntersectsBox(ray, box, point));

    const BoundingSphere sphere(FVec3::kZero, 1.0f);
    EXPECT_TRUE(TkCollision::rayIntersectsSphere(ray, sphere, distance));
    EXPECT_TRUE(TkCollision::rayIntersectsSphere(ray, sphere, distance, normal));
    EXPECT_TRUE(TkCollision::rayIntersectsSphere(ray, sphere, point));
}

TEST(CollisionsTest, PlaneBoxSphereContainmentFrustumAndLineHelpers)
{
    const Plane plane(FVec3::kUnitY, -2.0f);
    EXPECT_EQ(TkCollision::planeIntersectsPoint(plane, FVec3(0.0f, 5.0f, 0.0f)), PlaneIntersectionType::kFront);
    EXPECT_TRUE(TkCollision::planeIntersectsPlane(Plane(FVec3::kUnitX, 0.0f), Plane(FVec3::kUnitY, 0.0f)));

    Ray line;
    EXPECT_TRUE(TkCollision::planeIntersectsPlane(Plane(FVec3::kUnitX, 0.0f), Plane(FVec3::kUnitY, 0.0f), line));
    EXPECT_TRUE(line.direction.isNormalized());

    EXPECT_EQ(TkCollision::planeIntersectsTriangle(plane, FVec3::kZero, FVec3(1.0f, 5.0f, 0.0f), FVec3(-1.0f, 0.0f, 0.0f)), PlaneIntersectionType::kIntersecting);
    EXPECT_EQ(TkCollision::planeIntersectsBox(plane, BoundingBox(FVec3(-1.0f, 1.0f, -1.0f), FVec3(1.0f, 3.0f, 1.0f))), PlaneIntersectionType::kIntersecting);
    EXPECT_EQ(TkCollision::planeIntersectsSphere(plane, BoundingSphere(FVec3(0.0f, 2.0f, 0.0f), 1.0f)), PlaneIntersectionType::kIntersecting);

    const BoundingBox box(FVec3(-1.0f, -1.0f, -1.0f), FVec3(1.0f, 1.0f, 1.0f));
    const BoundingSphere sphere(FVec3::kZero, 1.0f);
    EXPECT_TRUE(TkCollision::boxIntersectsBox(box, BoundingBox(FVec3(0.5f, 0.5f, 0.5f), FVec3(2.0f, 2.0f, 2.0f))));
    EXPECT_TRUE(TkCollision::boxIntersectsSphere(box, sphere));
    EXPECT_TRUE(TkCollision::sphereIntersectsTriangle(sphere, FVec3(-1.0f, 0.0f, 0.0f), FVec3(1.0f, 0.0f, 0.0f), FVec3(0.0f, 1.0f, 0.0f)));
    EXPECT_TRUE(TkCollision::sphereIntersectsSphere(sphere, BoundingSphere(FVec3(1.5f, 0.0f, 0.0f), 1.0f)));

    EXPECT_EQ(TkCollision::boxContainsPoint(box, FVec3::kZero), ContainmentType::kContains);
    EXPECT_EQ(TkCollision::boxContainsBox(box, BoundingBox(FVec3(-0.5f, -0.5f, -0.5f), FVec3(0.5f, 0.5f, 0.5f))), ContainmentType::kContains);
    EXPECT_EQ(TkCollision::boxContainsSphere(box, BoundingSphere(FVec3::kZero, 0.5f)), ContainmentType::kContains);
    EXPECT_EQ(TkCollision::sphereContainsPoint(sphere, FVec3::kZero), ContainmentType::kContains);
    EXPECT_EQ(TkCollision::sphereContainsTriangle(BoundingSphere(FVec3::kZero, 5.0f), FVec3(-1.0f, 0.0f, 0.0f), FVec3(1.0f, 0.0f, 0.0f), FVec3(0.0f, 1.0f, 0.0f)), ContainmentType::kContains);
    EXPECT_EQ(TkCollision::sphereContainsBox(BoundingSphere(FVec3::kZero, 5.0f), box), ContainmentType::kContains);
    EXPECT_EQ(TkCollision::sphereContainsSphere(BoundingSphere(FVec3::kZero, 5.0f), sphere), ContainmentType::kContains);

    Mat projection;
    Mat::perspectiveFov(TkMath::PI_OVER_2, 1.0f, 1.0f, 10.0f, projection);
    BoundingFrustum frustum(projection);
    EXPECT_TRUE(TkCollision::frustumIntersectsBox(frustum, BoundingBox(FVec3(-0.5f, -0.5f, 2.0f), FVec3(0.5f, 0.5f, 3.0f))));
    EXPECT_EQ(TkCollision::frustumContainsBox(frustum, BoundingBox(FVec3(-0.5f, -0.5f, 2.0f), FVec3(0.5f, 0.5f, 3.0f))), ContainmentType::kContains);

    EXPECT_TRUE(TkCollision::lineIntersectsLine(FVec2::kZero, FVec2(2.0f, 2.0f), FVec2(0.0f, 2.0f), FVec2(2.0f, 0.0f)));
    EXPECT_TRUE(TkCollision::lineIntersectsRect(FVec2(-1.0f, 0.0f), FVec2(3.0f, 0.0f), Rectangle(0.0f, -1.0f, 2.0f, 2.0f)));

    expectVec2Near(TkCollision::lineHitsBox(FVec3(-2.0f, 0.0f, 0.0f), FVec3(2.0f, 0.0f, 0.0f), FVec3(-1.0f, -1.0f, -1.0f), FVec3(1.0f, 1.0f, 1.0f)), FVec2(0.25f, 0.75f), 1e-4f);
    EXPECT_TRUE(TkCollision::isPointInTriangle(FVec2(0.25f, 0.25f), FVec2::kZero, FVec2(1.0f, 0.0f), FVec2(0.0f, 1.0f)));
}
