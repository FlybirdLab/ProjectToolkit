#include "vector_test_utils.hpp"

using namespace math_test;

TEST(BoundingFrustumTest, MatrixPlanesCornersAndDerivedBounds)
{
    Mat projection;
    Mat::perspectiveFov(TkMath::PI_OVER_2, 1.0f, 1.0f, 10.0f, projection);

    BoundingFrustum frustum(projection);
    EXPECT_FALSE(frustum.toString().isEmpty());
    expectMatNear(frustum.getMatrix(), projection, 1e-5f);

    const Plane nearPlane = frustum.getNear();
    const Plane farPlane = frustum.getFar();
    EXPECT_TRUE(nearPlane.normal.isNormalized());
    EXPECT_TRUE(farPlane.normal.isNormalized());
    EXPECT_TRUE(frustum.getPlane(99) == Plane());

    FVec3 corners[8];
    frustum.getCorners(corners);
    EXPECT_NEAR(corners[0].z, 1.0f, 1e-3f);
    EXPECT_NEAR(corners[4].z, 10.0f, 1e-2f);

    BoundingBox box;
    frustum.getBox(box);
    EXPECT_TRUE(box.contains(FVec3(0.0f, 0.0f, 1.0f)) != ContainmentType::kDisjoint);
    EXPECT_TRUE(box.contains(corners[4]) != ContainmentType::kDisjoint);

    BoundingSphere sphere;
    frustum.getSphere(sphere);
    EXPECT_TRUE(sphere.contains(FVec3(0.0f, 0.0f, 1.0f)) != ContainmentType::kDisjoint);
    EXPECT_TRUE(sphere.contains(corners[4]) != ContainmentType::kDisjoint);

    EXPECT_NEAR(frustum.getWidthAtDepth(1.0f), 2.0f, 1e-3f);
    EXPECT_NEAR(frustum.getHeightAtDepth(1.0f), 2.0f, 1e-3f);
    EXPECT_FALSE(frustum.isOrthographic());
}

TEST(BoundingFrustumTest, ContainmentIntersectionAndOrthographicStability)
{
    Mat projection;
    Mat::perspectiveFov(TkMath::PI_OVER_2, 1.0f, 1.0f, 10.0f, projection);
    BoundingFrustum frustum;
    frustum.setMatrix(Mat::kIdentity, projection);

    EXPECT_EQ(frustum.contains(FVec3(0.0f, 0.0f, 5.0f)), ContainmentType::kContains);
    EXPECT_EQ(frustum.contains(FVec3(100.0f, 100.0f, 5.0f)), ContainmentType::kDisjoint);

    const BoundingBox insideBox(FVec3(-0.5f, -0.5f, 2.0f), FVec3(0.5f, 0.5f, 3.0f));
    const BoundingBox outsideBox(FVec3(50.0f, 50.0f, 50.0f), FVec3(55.0f, 55.0f, 55.0f));
    EXPECT_EQ(frustum.contains(insideBox), ContainmentType::kContains);
    EXPECT_FALSE(frustum.intersects(outsideBox));

    const BoundingSphere insideSphere(FVec3(0.0f, 0.0f, 4.0f), 0.5f);
    const BoundingSphere crossingSphere(FVec3(0.0f, 0.0f, 0.75f), 0.5f);
    const BoundingSphere outsideSphere(FVec3(50.0f, 0.0f, 5.0f), 1.0f);
    EXPECT_EQ(frustum.contains(insideSphere), ContainmentType::kContains);
    EXPECT_EQ(frustum.contains(crossingSphere), ContainmentType::kIntersects);
    EXPECT_FALSE(frustum.intersects(outsideSphere));

    Mat ortho;
    Mat::orthoOffCenter(-2.0f, 2.0f, -3.0f, 3.0f, 1.0f, 9.0f, ortho);
    BoundingFrustum orthoFrustum(ortho);
    EXPECT_TRUE(orthoFrustum.isOrthographic());

    FVec3 orthoCorners[8];
    orthoFrustum.getCorners(orthoCorners);
    const float nearWidth = FVec3::distance(orthoCorners[0], orthoCorners[3]);
    const float nearHeight = FVec3::distance(orthoCorners[0], orthoCorners[1]);
    EXPECT_NEAR(nearWidth, 4.0f, 1e-3f);
    EXPECT_NEAR(nearHeight, 6.0f, 1e-3f);

    EXPECT_NEAR(orthoFrustum.getWidthAtDepth(5.0f), 0.0f, 1e-3f);
    EXPECT_NEAR(orthoFrustum.getHeightAtDepth(5.0f), 0.0f, 1e-3f);
}
