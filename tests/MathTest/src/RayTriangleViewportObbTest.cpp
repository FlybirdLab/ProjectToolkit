#include "vector_test_utils.hpp"

using namespace math_test;

TEST(RayTest, IdentityQueriesIntersectionsAndPickRay)
{
    expectRayNear(Ray::kIdentity, Ray(FVec3::kZero, FVec3::kForward));
    EXPECT_FALSE(Ray::kIdentity.toString().isEmpty());
    expectVec3Near(Ray::kIdentity.getPoint(5.0f), FVec3(0.0f, 0.0f, 5.0f));
    EXPECT_TRUE(Ray::nearEqual(Ray::kIdentity, Ray(FVec3::kZero, FVec3::kForward)));

    const Ray ray(FVec3(-5.0f, 0.0f, 0.0f), FVec3::kUnitX);
    EXPECT_TRUE(ray.intersects(FVec3::kZero));

    Vec3 lineHit;
    EXPECT_TRUE(ray.intersects(Ray(FVec3(0.0f, -1.0f, 0.0f), FVec3::kUnitY), lineHit));
    expectVec3Near(lineHit, FVec3::kZero, 1e-4f);

    real distance = 0.0f;
    EXPECT_TRUE(ray.intersects(Plane(FVec3::kUnitX, 0.0f), distance));
    EXPECT_NEAR(distance, 5.0f, 1e-4f);

    Vec3 point;
    EXPECT_TRUE(ray.intersects(Plane(FVec3::kUnitX, 0.0f), point));
    expectVec3Near(point, FVec3::kZero, 1e-4f);

    EXPECT_TRUE(ray.intersects(FVec3(-1.0f, -1.0f, 0.0f), FVec3(1.0f, -1.0f, 0.0f), FVec3(0.0f, 1.0f, 0.0f), distance));
    EXPECT_TRUE(ray.intersects(BoundingBox(FVec3(-1.0f, -1.0f, -1.0f), FVec3(1.0f, 1.0f, 1.0f)), distance));
    EXPECT_TRUE(ray.intersects(BoundingSphere(FVec3::kZero, 1.0f), distance));

    Viewport viewport(0.0f, 0.0f, 100.0f, 100.0f);
    const Ray pick = Ray::getPickRay(50.0f, 50.0f, viewport, Mat::kIdentity);
    EXPECT_TRUE(pick.direction.isNormalized());
}

TEST(TriangleTest, NormalAndIntersectionOverloads)
{
    Triangle triangle(FVec3(-1.0f, -1.0f, 0.0f), FVec3(1.0f, -1.0f, 0.0f), FVec3(0.0f, 1.0f, 0.0f));
    expectVec3Near(triangle.getNormal(), FVec3::kUnitZ, 1e-4f);

    const Ray ray(FVec3(0.0f, 0.0f, -5.0f), FVec3::kForward);
    real distance = 0.0f;
    Vec3 normal;
    Vec3 point;
    EXPECT_TRUE(triangle.intersects(ray));
    EXPECT_TRUE(triangle.intersects(ray, distance));
    EXPECT_NEAR(distance, 5.0f, 1e-4f);
    EXPECT_TRUE(triangle.intersects(ray, distance, normal));
    expectVec3Near(normal, FVec3::kUnitZ, 1e-4f);
    EXPECT_TRUE(triangle.intersects(ray, point));
    expectVec3Near(point, FVec3::kZero, 1e-4f);
}

TEST(ViewportTest, BoundsAspectProjectAndUnproject)
{
    Viewport viewport(FVec2(100.0f, 200.0f));
    EXPECT_NEAR(viewport.getAspectRatio(), 0.5f, 1e-5f);
    expectRectNear(viewport.getBounds(), Rectangle(0.0f, 0.0f, 100.0f, 200.0f));

    viewport.setBounds(Rectangle(10.0f, 20.0f, 80.0f, 40.0f));
    expectRectNear(viewport.getBounds(), Rectangle(10.0f, 20.0f, 80.0f, 40.0f));
    EXPECT_TRUE(viewport == Viewport(Rectangle(10.0f, 20.0f, 80.0f, 40.0f)));
    EXPECT_FALSE(viewport.toString().isEmpty());

    Vec3 projected;
    viewport.project(FVec3(0.0f, 0.0f, 0.5f), Mat::kIdentity, projected);
    expectVec3Near(projected, FVec3(50.0f, 40.0f, 0.5f), 1e-4f);

    Vec3 unprojected;
    viewport.unproject(projected, Mat::kIdentity, unprojected);
    expectVec3Near(unprojected, FVec3(0.0f, 0.0f, 0.5f), 1e-4f);
}

TEST(OrientedBoundingBoxTest, ConstructorsQueriesMutationAndIntersections)
{
    BoundingBox aabb(FVec3(-1.0f, -2.0f, -3.0f), FVec3(1.0f, 2.0f, 3.0f));
    OrientedBoundingBox fromAabb(aabb);
    expectVec3Near(fromAabb.getCenter(), FVec3::kZero);
    expectVec3Near(fromAabb.getSizeUnscaled(), FVec3(2.0f, 4.0f, 6.0f));
    expectVec3Near(fromAabb.getSize(), FVec3(2.0f, 4.0f, 6.0f));
    expectVec3Near(fromAabb.getSizeSquared(), FVec3(4.0f, 16.0f, 36.0f));
    EXPECT_FALSE(fromAabb.toString().isEmpty());

    Vec3 points[] = {FVec3(-2.0f, -2.0f, -2.0f), FVec3(2.0f, 2.0f, 2.0f)};
    OrientedBoundingBox fromPoints(points, 2);
    expectVec3Near(fromPoints.extents, FVec3(2.0f, 2.0f, 2.0f));

    OrientedBoundingBox centered = OrientedBoundingBox::createCentered(FVec3(10.0f, 20.0f, 30.0f), FVec3(4.0f, 6.0f, 8.0f));
    expectVec3Near(centered.getCenter(), FVec3(10.0f, 20.0f, 30.0f));
    expectVec3Near(centered.getSizeUnscaled(), FVec3(4.0f, 6.0f, 8.0f));

    OrientedBoundingBox transformed(FVec3::kOne, Transform(FVec3(1.0f, 2.0f, 3.0f), Quat::euler(0.0f, 90.0f, 0.0f), FVec3::kOne));
    BoundingBox bb = transformed.getBoundingBox();
    EXPECT_TRUE(bb.contains(transformed.getCenter()) != ContainmentType::kDisjoint);

    centered.scale(2.0f);
    expectVec3Near(centered.getSizeUnscaled(), FVec3(8.0f, 12.0f, 16.0f));
    centered.scale(FVec3(0.5f, 0.5f, 0.5f));
    expectVec3Near(centered.getSizeUnscaled(), FVec3(4.0f, 6.0f, 8.0f));
    centered.translate(FVec3(1.0f, 2.0f, 3.0f));
    expectVec3Near(centered.getCenter(), FVec3(11.0f, 22.0f, 33.0f));

    centered.transform(Mat::translation(FVec3(1.0f, 0.0f, 0.0f)));
    centered.transform(Transform(FVec3(0.0f, 1.0f, 0.0f)));
    expectVec3Near(centered.getCenter(), FVec3(12.0f, 23.0f, 33.0f), 1e-4f);

    EXPECT_EQ(fromAabb.contains(FVec3::kZero), ContainmentType::kContains);
    EXPECT_EQ(fromAabb.contains(BoundingSphere(FVec3::kZero, 0.5f)), ContainmentType::kContains);

    const Ray ray(FVec3(-5.0f, 0.0f, 0.0f), FVec3::kUnitX);
    Vec3 hitPoint;
    real distance = 0.0f;
    Vec3 normal;
    EXPECT_TRUE(fromAabb.intersects(ray));
    EXPECT_TRUE(fromAabb.intersects(ray, hitPoint));
    EXPECT_TRUE(fromAabb.intersects(ray, distance));
    EXPECT_TRUE(fromAabb.intersects(ray, distance, normal));
    EXPECT_NEAR(distance, 4.0f, 1e-4f);
    EXPECT_TRUE(normal.isNormalized());
}
