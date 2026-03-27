#include "vector_test_utils.hpp"

using namespace math_test;

TEST(MatrixTest, ConstantsLayoutAndAccessors)
{
    expectMatNear(Mat::kIdentity, Mat(1, 0, 0, 0,
                                      0, 1, 0, 0,
                                      0, 0, 1, 0,
                                      0, 0, 0, 1));
    EXPECT_TRUE(Mat::kIdentity.isIdentity());
    EXPECT_FALSE(Mat::kIdentity.toString().isEmpty());

    Mat matrix = Mat::kIdentity;
    matrix.setRight(FVec3(1.0f, 2.0f, 3.0f));
    matrix.setUp(FVec3(4.0f, 5.0f, 6.0f));
    matrix.setBackward(FVec3(7.0f, 8.0f, 9.0f));
    matrix.settranslation(FVec3(10.0f, 11.0f, 12.0f));
    expectVec3Near(matrix.getRight(), FVec3(1.0f, 2.0f, 3.0f));
    expectVec3Near(matrix.getUp(), FVec3(4.0f, 5.0f, 6.0f));
    expectVec3Near(matrix.getBackward(), FVec3(7.0f, 8.0f, 9.0f));
    expectVec3Near(matrix.getTranslation(), FVec3(10.0f, 11.0f, 12.0f));

    matrix.raw[0] = 3.0f;
    EXPECT_FLOAT_EQ(matrix.m11, 3.0f);
    EXPECT_FLOAT_EQ(matrix.values[0][0], 3.0f);

    Mat3x3 matrix3 = Mat3x3::kIdentity;
    matrix3.setRight(FVec3(3.0f, 0.0f, 0.0f));
    matrix3.setUp(FVec3(0.0f, 4.0f, 0.0f));
    matrix3.setBackward(FVec3(0.0f, 0.0f, 5.0f));
    expectVec3Near(matrix3.getScaleVector(), FVec3(3.0f, 4.0f, 5.0f));
    matrix3.raw[0] = 2.0f;
    EXPECT_FLOAT_EQ(matrix3.m11, 2.0f);
}

TEST(MatrixTest, ArithmeticInverseAndTranspose)
{
    const Mat a(1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16);
    const Mat b(16, 15, 14, 13,
                12, 11, 10, 9,
                8, 7, 6, 5,
                4, 3, 2, 1);

    Mat result;
    Mat::add(a, b, result);
    expectMatNear(result, Mat(17, 17, 17, 17,
                              17, 17, 17, 17,
                              17, 17, 17, 17,
                              17, 17, 17, 17));

    Mat::subtract(a, b, result);
    expectMatNear(result, Mat(-15, -13, -11, -9,
                              -7, -5, -3, -1,
                              1, 3, 5, 7,
                              9, 11, 13, 15));

    Mat::multiply(Mat::kIdentity, 2.0f, result);
    expectMatNear(result, Mat(2, 0, 0, 0,
                              0, 2, 0, 0,
                              0, 0, 2, 0,
                              0, 0, 0, 2));

    Mat::divide(result, 2.0f, result);
    expectMatNear(result, Mat::kIdentity);

    Mat translation = Mat::translation(FVec3(2.0f, 3.0f, 4.0f));
    Mat inverse = Mat::invert(translation);
    expectMatNear(translation * inverse, Mat::kIdentity, 1e-4f);

    Mat transposed = Mat::transpose(a);
    expectMatNear(transposed, Mat(1, 5, 9, 13,
                                  2, 6, 10, 14,
                                  3, 7, 11, 15,
                                  4, 8, 12, 16));

    EXPECT_NEAR(translation.getDeterminant(), 1.0f, 1e-5f);
}

TEST(MatrixTest, TransformCreationAndDecomposition)
{
    const Mat scaling = Mat::scaling(FVec3(2.0f, 3.0f, 4.0f));
    const Mat translation = Mat::translation(FVec3(5.0f, 6.0f, 7.0f));
    expectVec3Near(scaling.getScaleVector(), FVec3(2.0f, 3.0f, 4.0f));
    expectVec3Near(translation.getTranslation(), FVec3(5.0f, 6.0f, 7.0f));

    Mat rotX;
    Mat::rotationX(TkMath::PI_OVER_2, rotX);
    expectVec4Near(Mat::transformPosition(rotX, FVec3(0.0f, 1.0f, 0.0f)), FVec4(0.0f, 0.0f, 1.0f, 1.0f), 1e-4f);

    Mat rotY = Mat::rotationY(TkMath::PI_OVER_2);
    expectVec4Near(Mat::transformPosition(rotY, FVec3(0.0f, 0.0f, 1.0f)), FVec4(1.0f, 0.0f, 0.0f, 1.0f), 1e-4f);

    Mat rotZ = Mat::rotationZ(TkMath::PI_OVER_2);
    expectVec4Near(Mat::transformPosition(rotZ, FVec3(1.0f, 0.0f, 0.0f)), FVec4(0.0f, 1.0f, 0.0f, 1.0f), 1e-4f);

    Quat rotation = Quat::euler(0.0f, 90.0f, 0.0f);
    Mat fromQuat = Mat::rotationQuaternion(rotation);
    expectVec4Near(Mat::transformPosition(fromQuat, FVec3::kForward), FVec4(1.0f, 0.0f, 0.0f, 1.0f), 1e-4f);

    Mat transform;
    Mat::transformation(FVec3(2.0f, 3.0f, 4.0f), rotation, FVec3(5.0f, 6.0f, 7.0f), transform);
    FVec3 decomposedScale;
    Quat decomposedRotation;
    FVec3 decomposedTranslation;
    transform.decompose(decomposedScale, decomposedRotation, decomposedTranslation);
    expectVec3Near(decomposedScale, FVec3(2.0f, 3.0f, 4.0f), 1e-4f);
    expectQuatNear(decomposedRotation, rotation, 1e-4f);
    expectVec3Near(decomposedTranslation, FVec3(5.0f, 6.0f, 7.0f), 1e-4f);

    Transform decomposedTransform;
    transform.decompose(decomposedTransform);
    expectTransformNear(decomposedTransform, Transform(FVec3(5.0f, 6.0f, 7.0f), rotation, FVec3(2.0f, 3.0f, 4.0f)), 1e-4f);
}

TEST(MatrixTest, ProjectionWorldAndMat3Operations)
{
    Mat world = Mat::createWorld(FVec3(1.0f, 2.0f, 3.0f), FVec3::kForward, FVec3::kUp);
    expectVec3Near(world.getTranslation(), FVec3(1.0f, 2.0f, 3.0f));

    Mat axisAngle = Mat::createFromAxisAngle(FVec3::kUnitZ, TkMath::PI_OVER_2);
    expectVec4Near(Mat::transformPosition(axisAngle, FVec3::kRight), FVec4(0.0f, 1.0f, 0.0f, 1.0f), 1e-4f);

    Mat ortho;
    Mat::orthoOffCenter(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f, ortho);
    EXPECT_FALSE(ortho.isIdentity());

    Mat perspective;
    Mat::perspectiveFov(TkMath::PI_OVER_2, 1.0f, 0.1f, 100.0f, perspective);
    EXPECT_FALSE(perspective.isIdentity());

    Mat billboard;
    Mat::billboard(FVec3::kZero, FVec3(0.0f, 0.0f, -10.0f), FVec3::kUp, FVec3::kForward, billboard);
    EXPECT_FALSE(billboard.isIdentity());

    Mat3x3 trans2d = Mat3x3::translation2D(FVec2(3.0f, 4.0f));
    FVec2 point2d;
    Mat3x3::transform2DPoint(FVec2(1.0f, 2.0f), trans2d, point2d);
    expectVec2Near(point2d, FVec2(4.0f, 6.0f));

    FVec2 vector2d;
    Mat3x3::transform2DVector(FVec2(1.0f, 2.0f), trans2d, vector2d);
    expectVec2Near(vector2d, FVec2(1.0f, 2.0f));

    Mat3x3 rot3 = Mat3x3::rotationQuaternion(Quat::euler(0.0f, 90.0f, 0.0f));
    FVec3 scale3;
    Quat rotQuat;
    rot3.decompose(scale3, rotQuat);
    expectVec3Near(scale3, FVec3::kOne, 1e-4f);
    expectQuatNear(rotQuat, Quat::euler(0.0f, 90.0f, 0.0f), 1e-4f);
}

TEST(MatrixTest, StabilityRoundTripAndMemoryLayout)
{
    Mat matrix = Mat::kIdentity;
    matrix.raw[12] = 3.0f;
    matrix.raw[13] = 4.0f;
    matrix.raw[14] = 5.0f;
    expectVec3Near(matrix.getTranslation(), FVec3(3.0f, 4.0f, 5.0f));
    EXPECT_FLOAT_EQ(matrix.values[3][0], 3.0f);

    Mat3x3 matrix3 = Mat3x3::kIdentity;
    matrix3.raw[0] = 2.0f;
    matrix3.raw[4] = 3.0f;
    matrix3.raw[8] = 4.0f;
    expectVec3Near(matrix3.getScaleVector(), FVec3(2.0f, 3.0f, 4.0f));

    const Mat world = Mat::scaling(FVec3(2.0f, 3.0f, 4.0f)) * Mat::rotationQuaternion(Quat::euler(15.0f, 25.0f, 35.0f)) * Mat::translation(FVec3(7.0f, 8.0f, 9.0f));
    const Mat inverse = Mat::invert(world);
    expectMatNear(world * inverse, Mat::kIdentity, 1e-3f);
    expectMatNear(inverse * world, Mat::kIdentity, 1e-3f);

    for (int i = 0; i < 8; ++i)
    {
        const float angle = (i + 1) * 0.125f * TkMath::PI;
        const Mat rotation = Mat::createFromAxisAngle(FVec3::normalize(FVec3(1.0f, 2.0f, 3.0f)), angle);
        const Mat transpose = Mat::transpose(rotation);
        expectMatNear(rotation * transpose, Mat::kIdentity, 1e-3f);
    }
}
