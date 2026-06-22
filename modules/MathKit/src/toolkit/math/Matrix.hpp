/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/math/Vector3.hpp>
#include <toolkit/math/Vector4.hpp>

namespace toolkit
{

    class Transform;

    class TK_API Mat
    {
    public:
        // A matrix with all of its components set to zero.
        static const Mat kZero;
        // The identity matrix.
        static const Mat kIdentity;

    public:
        Mat() = default;
        Mat(float m11, float m12, float m13, float m14,
            float m21, float m22, float m23, float m24,
            float m31, float m32, float m33, float m34,
            float m41, float m42, float m43, float m44)
            : m11(m11), m12(m12), m13(m13), m14(m14), m21(m21), m22(m22), m23(m23), m24(m24), m31(m31), m32(m32), m33(m33), m34(m34), m41(m41), m42(m42), m43(m43), m44(m44)
        {
        }
        Mat(float values[16])
        {
            TkMemory::memoryCopy(raw, values, sizeof(float) * 16);
        }
        Mat(float values[4][4])
        {
            TkMemory::memoryCopy(raw, values, sizeof(float) * 16);
        }
        explicit Mat(const Mat3x3 &matrix);

    public:
        TkString toString() const;

        // Gets the up FVec3 of the matrix; that is m21, m22, and m23.
        FVec3 getUp() const
        {
            return {m21, m22, m23};
        }

        // Sets FVec3 of the matrix; that is m21, m22, and m23.
        void setUp(const FVec3 &value)
        {
            m21 = value.x;
            m22 = value.y;
            m23 = value.z;
        }

        // Gets the down FVec3 of the matrix; that is -m21, -m22, and -m23.
        FVec3 getDown() const
        {
            return -FVec3(m21, m22, m23);
        }

        // Sets the down FVec3 of the matrix; that is -m21, -m22, and -m23.
        void setDown(const FVec3 &value)
        {
            m21 = -value.x;
            m22 = -value.y;
            m23 = -value.z;
        }

        // Gets the right FVec3 of the matrix; that is m11, m12, and m13.
        FVec3 getRight() const
        {
            return {m11, m12, m13};
        }

        // Sets the right FVec3 of the matrix; that is m11, m12, and m13.
        void setRight(const FVec3 &value)
        {
            m11 = value.x;
            m12 = value.y;
            m13 = value.z;
        }

        // Gets the left FVec3 of the matrix; that is -m11, -m12, and -m13.
        FVec3 getLeft() const
        {
            return -FVec3{m11, m12, m13};
        }

        // Sets the left FVec3 of the matrix; that is -m11, -m12, and -m13.
        void setLeft(const FVec3 &value)
        {
            m11 = -value.x;
            m12 = -value.y;
            m13 = -value.z;
        }

        // Gets the forward FVec3 of the matrix; that is -m31, -m32, and -m33.
        FVec3 getForward() const
        {
            return -FVec3(m31, m32, m33);
        }

        // Sets the forward FVec3 of the matrix; that is -m31, -m32, and -m33.
        void setForward(const FVec3 &value)
        {
            m31 = -value.x;
            m32 = -value.y;
            m33 = -value.z;
        }

        // Gets the backward FVec3 of the matrix; that is m31, m32, and m33.
        FVec3 getBackward() const
        {
            return {m31, m32, m33};
        }

        // Sets the backward FVec3 of the matrix; that is m31, m32, and m33.
        void setBackward(const FVec3 &value)
        {
            m31 = value.x;
            m32 = value.y;
            m33 = value.z;
        }

        // Gets the first row in the matrix; that is m11, m12, m13, and m14.
        FVec4 getRow1() const
        {
            return {m11, m12, m13, m14};
        }

        // Sets the first row in the matrix; that is m11, m12, m13, and m14.
        void setRow1(const FVec4 &value)
        {
            m11 = value.x;
            m12 = value.y;
            m13 = value.z;
            m14 = value.w;
        }

        // Gets the second row in the matrix; that is m21, m22, m23, and m24.
        FVec4 getRow2() const
        {
            return {m21, m22, m23, m24};
        }

        // Sets the second row in the matrix; that is m21, m22, m23, and m24.
        void setRow2(const FVec4 &value)
        {
            m21 = value.x;
            m22 = value.y;
            m23 = value.z;
            m24 = value.w;
        }

        // Gets the third row in the matrix; that is m31, m32, m33, and m34.
        FVec4 getRow3() const
        {
            return {m31, m32, m33, m34};
        }

        // Sets the third row in the matrix; that is m31, m32, m33, and m34.
        void setRow3(const FVec4 &value)
        {
            m31 = value.x;
            m32 = value.y;
            m33 = value.z;
            m34 = value.w;
        }

        // Gets the fourth row in the matrix; that is m41, m42, m43, and m44.
        FVec4 getRow4() const
        {
            return {m41, m42, m43, m44};
        }

        // Sets the fourth row in the matrix; that is m41, m42, m43, and m44.
        void setRow4(const FVec4 &value)
        {
            m41 = value.x;
            m42 = value.y;
            m43 = value.z;
            m44 = value.w;
        }

        // Gets the first column in the matrix; that is m11, m21, m31, and m41.
        FVec4 getColumn1() const
        {
            return {m11, m21, m31, m41};
        }

        // Sets the first column in the matrix; that is m11, m21, m31, and m41.
        void setColumn1(const FVec4 &value)
        {
            m11 = value.x;
            m21 = value.y;
            m31 = value.z;
            m41 = value.w;
        }

        // Gets the second column in the matrix; that is m12, m22, m32, and m42.
        FVec4 getColumn2() const
        {
            return {m12, m22, m32, m42};
        }

        // Sets the second column in the matrix; that is m12, m22, m32, and m42.
        void setColumn2(const FVec4 &value)
        {
            m12 = value.x;
            m22 = value.y;
            m32 = value.z;
            m42 = value.w;
        }

        // Gets the third column in the matrix; that is m13, m23, m33, and m43.
        FVec4 getColumn3() const
        {
            return {m13, m23, m33, m43};
        }

        // Sets the third column in the matrix; that is m13, m23, m33, and m43.
        void setColumn3(const FVec4 &value)
        {
            m13 = value.x;
            m23 = value.y;
            m33 = value.z;
            m43 = value.w;
        }

        // Gets the fourth column in the matrix; that is m14, m24, m34, and m44.
        FVec4 getColumn4() const
        {
            return {m14, m24, m34, m44};
        }

        // Sets the fourth column in the matrix; that is m14, m24, m34, and m44.
        void setColumn4(const FVec4 &value)
        {
            m14 = value.x;
            m24 = value.y;
            m34 = value.z;
            m44 = value.w;
        }

        // Sets part of the first row in the matrix; that is m11, m12, m13.
        void setX(const FVec3 &value)
        {
            m11 = value.x;
            m12 = value.y;
            m13 = value.z;
        }

        // Sets part of the second row in the matrix; that is m21, m22, m23.
        void setY(const FVec3 &value)
        {
            m21 = value.x;
            m22 = value.y;
            m23 = value.z;
        }

        // Sets part of the third row in the matrix; that is m31, m32, m33.
        void setZ(const FVec3 &value)
        {
            m31 = value.x;
            m32 = value.y;
            m33 = value.z;
        }

        // Gets the translation of the matrix; that is m41, m42, and m43.
        FVec3 getTranslation() const
        {
            return {m41, m42, m43};
        }

        // Sets the translation of the matrix; that is m41, m42, and m43.
        void settranslation(const FVec3 &value)
        {
            m41 = value.x;
            m42 = value.y;
            m43 = value.z;
        }

        // Gets the scale of the matrix; that is m11, m22, and m33.
        FVec3 getScaleVector() const
        {
            return {m11, m22, m33};
        }

        // Sets the scale of the matrix; that is m11, m22, and m33.
        void setScaleVector(const FVec3 &value)
        {
            m11 = value.x;
            m22 = value.y;
            m33 = value.z;
        }

        // Gets a value indicating whether this instance is an identity matrix.
        bool isIdentity() const
        {
            return *this == kIdentity;
        }

        // Calculates the determinant of the matrix.
        float getDeterminant() const;

        // Calculates determinant of the rotation 3x3 matrix.
        float rotDeterminant() const;

        // Inverts the matrix.
        void invert()
        {
            invert(*this, *this);
        }

        // Transposes the matrix.
        void transpose()
        {
            transpose(*this, *this);
        }

        // Removes any scaling from the matrix by performing the normalization (each row magnitude is 1). Does not modify the 4th row with translation vector.
        void normalizeScale();

    public:
        // Decomposes a rotation matrix with the specified yaw, pitch, roll.
        void decompose(float &yaw, float &pitch, float &roll) const;
        // Decomposes a matrix into a scale, rotation, and translation.
        void decompose(FVec3 &scale, FVec3 &translation) const;
        // Decomposes a matrix into a scale, rotation, and translation.
        void decompose(Transform &transform) const;
        // Decomposes a matrix into a scale, rotation, and translation.
        void decompose(FVec3 &scale, Quat &rotation, FVec3 &translation) const;
        // Decomposes a matrix into a scale, rotation, and translation.
        void decompose(FVec3 &scale, Mat3x3 &rotation, FVec3 &translation) const;

    public:
        Mat operator*(const float scale) const
        {
            Mat result;
            multiply(*this, scale, result);
            return result;
        }

        Mat operator*(const Mat &other) const
        {
            Mat result;
            multiply(*this, other, result);
            return result;
        }

        Mat &operator+=(const Mat &other)
        {
            add(*this, other, *this);
            return *this;
        }

        Mat &operator-=(const Mat &other)
        {
            subtract(*this, other, *this);
            return *this;
        }

        Mat &operator*=(const Mat &other)
        {
            const Mat tmp = *this;
            multiply(tmp, other, *this);
            return *this;
        }

        Mat &operator*=(const float scale)
        {
            multiply(*this, scale, *this);
            return *this;
        }

        bool operator==(const Mat &other) const;
        bool operator!=(const Mat &other) const
        {
            return !operator==(other);
        }

    public:
        // Calculates the sum of two matrices.
        static void add(const Mat &left, const Mat &right, Mat &result);
        // Calculates the difference between two matrices.
        static void subtract(const Mat &left, const Mat &right, Mat &result);
        // Scales a matrix by the given value.
        static void multiply(const Mat &left, float right, Mat &result);
        // Calculates the product of two matrices.
        static Mat multiply(const Mat &left, const Mat &right)
        {
            Mat result;
            multiply(left, right, result);
            return result;
        }
        // Calculates the product of two matrices.
        static void multiply(const Mat &left, const Mat &right, Mat &result);
        // Scales a matrix by the given value.
        static void divide(const Mat &left, float right, Mat &result);
        // Calculates the quotient of two matrices.
        static void divide(const Mat &left, const Mat &right, Mat &result);
        // Negates a matrix.
        static void negate(const Mat &value, Mat &result);
        // Performs a linear interpolation between two matrices.
        static void lerp(const Mat &start, const Mat &end, float amount, Mat &result);
        // Performs a cubic interpolation between two matrices.
        static void SmoothStep(const Mat &start, const Mat &end, float amount, Mat &result)
        {
            amount = TkMath::smoothStep(amount);
            lerp(start, end, amount, result);
        }
        // Calculates the transpose of the specified matrix.
        static Mat transpose(const Mat &value);
        // Calculates the transpose of the specified matrix.
        static void transpose(const Mat &value, Mat &result);
        // Calculates the inverse of the specified matrix.
        static Mat invert(const Mat &value)
        {
            Mat result;
            invert(value, result);
            return result;
        }
        // Calculates the inverse of the specified matrix.
        static void invert(const Mat &value, Mat &result);
        // Creates a left-handed spherical billboard that rotates around a specified object position.
        static void billboard(const FVec3 &objectPosition, const FVec3 &cameraPosition, const FVec3 &cameraUpFloat, const FVec3 &cameraForwardFloat, Mat &result);
        // Creates a left-handed, look-at matrix.
        static void lookAt(const FVec3 &eye, const FVec3 &target, const FVec3 &up, Mat &result);
        // Creates a left-handed, orthographic projection matrix.
        static void ortho(float width, float height, float zNear, float zFar, Mat &result)
        {
            const float halfWidth = width * 0.5f;
            const float halfHeight = height * 0.5f;
            orthoOffCenter(-halfWidth, halfWidth, -halfHeight, halfHeight, zNear, zFar, result);
        }
        // Creates a left-handed, customized orthographic projection matrix.
        static void orthoOffCenter(float left, float right, float bottom, float top, float zNear, float zFar, Mat &result);
        // Creates a left-handed, perspective projection matrix.
        static void perspective(float width, float height, float zNear, float zFar, Mat &result)
        {
            const float halfWidth = width * 0.5f;
            const float halfHeight = height * 0.5f;
            perspectiveOffCenter(-halfWidth, halfWidth, -halfHeight, halfHeight, zNear, zFar, result);
        }
        // Creates a left-handed, perspective projection matrix based on a field of view.
        static void perspectiveFov(float fov, float aspect, float zNear, float zFar, Mat &result);
        // Creates a left-handed, customized perspective projection matrix.
        static void perspectiveOffCenter(float left, float right, float bottom, float top, float zNear, float zFar, Mat &result);
        // Creates a matrix that scales along the x-axis, y-axis, and y-axis.
        static Mat scaling(const FVec3 &scale)
        {
            return scaling(scale.x, scale.y, scale.z);
        }
        // Creates a matrix that scales along the x-axis, y-axis, and y-axis.
        static void scaling(const FVec3 &scale, Mat &result)
        {
            scaling(scale.x, scale.y, scale.z, result);
        }
        // Creates a matrix that scales along the x-axis, y-axis, and y-axis.
        static Mat scaling(float x, float y, float z)
        {
            Mat result = kIdentity;
            result.m11 = x;
            result.m22 = y;
            result.m33 = z;
            return result;
        }
        // Creates a matrix that scales along the x-axis, y-axis, and y-axis.
        static void scaling(float x, float y, float z, Mat &result)
        {
            result = kIdentity;
            result.m11 = x;
            result.m22 = y;
            result.m33 = z;
        }
        // Creates a matrix that uniformly scales along all three axis.
        static Mat scaling(float scale)
        {
            Mat result = kIdentity;
            result.m11 = result.m22 = result.m33 = scale;
            return result;
        }
        // Creates a matrix that uniformly scales along all three axis.
        static void scaling(float scale, Mat &result)
        {
            result = kIdentity;
            result.m11 = result.m22 = result.m33 = scale;
        }
        // Creates a matrix that rotates around the x-axis. Angle of rotation in radians. Angles are measured clockwise when looking along the rotation axis toward the origin.
        static Mat rotationX(float angle)
        {
            Mat result;
            rotationX(angle, result);
            return result;
        }
        // Creates a matrix that rotates around the x-axis. Angle of rotation in radians. Angles are measured clockwise when looking along the rotation axis toward the origin.
        static void rotationX(float angle, Mat &result);
        // Creates a matrix that rotates around the y-axis. Angle of rotation in radians. Angles are measured clockwise when looking along the rotation axis toward the origin.
        static Mat rotationY(float angle)
        {
            Mat result;
            rotationY(angle, result);
            return result;
        }
        // Creates a matrix that rotates around the y-axis. Angle of rotation in radians. Angles are measured clockwise when looking along the rotation axis toward the origin.
        static void rotationY(float angle, Mat &result);
        // Creates a matrix that rotates around the z-axis. Angle of rotation in radians. Angles are measured clockwise when looking along the rotation axis toward the origin.
        static Mat rotationZ(float angle)
        {
            Mat result;
            rotationZ(angle, result);
            return result;
        }
        // Creates a matrix that rotates around the z-axis. Angle of rotation in radians. Angles are measured clockwise when looking along the rotation axis toward the origin.
        static void rotationZ(float angle, Mat &result);
        // Creates a matrix that rotates around an arbitrary axis (normalized). Angle of rotation in radians. Angles are measured clockwise when looking along the rotation axis toward the origin.
        static Mat rotationAxis(const FVec3 &axis, float angle)
        {
            Mat result;
            rotationAxis(axis, angle, result);
            return result;
        }
        // Creates a matrix that rotates around an arbitrary axis (normalized). Angle of rotation in radians. Angles are measured clockwise when looking along the rotation axis toward the origin.
        static void rotationAxis(const FVec3 &axis, float angle, Mat &result);
        // Creates a rotation matrix from a quaternion
        static Mat rotationQuaternion(const Quat &rotation)
        {
            Mat result;
            rotationQuaternion(rotation, result);
            return result;
        }
        // Creates a rotation matrix from a quaternion.
        static void rotationQuaternion(const Quat &rotation, Mat &result);
        // Creates a rotation matrix with a specified yaw, pitch, and roll.
        static Mat rotationYawPitchRoll(float yaw, float pitch, float roll)
        {
            Mat result;
            rotationYawPitchRoll(yaw, pitch, roll, result);
            return result;
        }
        // Creates a rotation matrix with a specified yaw, pitch, and roll.
        static void rotationYawPitchRoll(float yaw, float pitch, float roll, Mat &result);
        // Creates a translation matrix using the specified offsets.
        static Mat translation(const FVec3 &value);
        // Creates a translation matrix using the specified offsets.
        static void translation(const FVec3 &value, Mat &result);
        // Creates a translation matrix using the specified offsets.
        static void translation(float x, float y, float z, Mat &result);
        // Creates a skew/shear matrix by means of a translation vector, a rotation vector, and a rotation angle.
        static void skew(float angle, const FVec3 &rotationVec, const FVec3 &transVec, Mat &matrix);
        // Creates a matrix that contains both the X, Y and Z rotation, as well as scaling and translation.
        static void transformation(const FVec3 &scaling, const Quat &rotation, const FVec3 &translation, Mat &result);
        // Creates a 3D affine transformation matrix.
        static void affineTransformation(float scaling, const Quat &rotation, const FVec3 &translation, Mat &result);
        // Creates a 3D affine transformation matrix.
        static void affineTransformation(float scaling, const FVec3 &rotationCenter, const Quat &rotation, const FVec3 &translation, Mat &result);
        // Creates a 2D affine transformation matrix.
        static void affineTransformation2D(float scaling, float rotation, const FVec2 &translation, Mat &result);
        // Creates a 2D affine transformation matrix.
        static void affineTransformation2D(float scaling, const FVec2 &rotationCenter, float rotation, const FVec2 &translation, Mat &result);
        // Creates a transformation matrix.
        static void transformation(const FVec3 &scalingCenter, const Quat &scalingRotation, const FVec3 &scaling, const FVec3 &rotationCenter, const Quat &rotation, const FVec3 &translation, Mat &result);
        // Creates a 2D transformation matrix.
        static void transformation2D(const FVec2 &scalingCenter, float scalingRotation, const FVec2 &scaling, const FVec2 &rotationCenter, float rotation, const FVec2 &translation, Mat &result);
        // Creates a world matrix with the specified parameters.
        static Mat createWorld(const FVec3 &position, const FVec3 &forward, const FVec3 &up);
        // Creates a world matrix with the specified parameters.
        static void createWorld(const FVec3 &position, const FVec3 &forward, const FVec3 &up, Mat &result);
        // Creates a new Mat that rotates around an arbitrary vector.
        static Mat createFromAxisAngle(const FVec3 &axis, float angle);
        // Creates a new Mat that rotates around an arbitrary vector.
        static void createFromAxisAngle(const FVec3 &axis, float angle, Mat &result);

        static Vec3 transformVector(const Mat &m, const Vec3 &v);
        static FVec4 transformPosition(const Mat &m, const FVec3 &v);
        static FVec4 transformPosition(const Mat &m, const FVec4 &v);

    public:
        union
        {
            struct
            {
                float m11;
                float m12;
                float m13;
                float m14;
                float m21;
                float m22;
                float m23;
                float m24;
                float m31;
                float m32;
                float m33;
                float m34;
                float m41;
                float m42;
                float m43;
                float m44;
            };

            float values[4][4];
            float raw[16];
        };
    };

    template <>
    struct TIsPODType<Mat>
    {
        enum
        {
            Value = true
        };
    };

    // Helper matrix for optimized float3x4 package of transformation matrices.
    class TK_API Mat3x4
    {
    public:
        void setMatrix(const Mat &m);
        void setMatrixTranspose(const Mat &m);

    public:
        union
        {
            float values[3][4];
            float raw[12];
        };
    };

    template <>
    struct TIsPODType<Mat3x4>
    {
        enum
        {
            Value = true
        };
    };

    // Represents a 3x3 mathematical matrix.
    struct TK_API Mat3x3
    {
    public:
        // A matrix with all of its components set to zero.
        static const Mat3x3 kZero;
        // The identity matrix.
        static const Mat3x3 kIdentity;

    public:
        Mat3x3() = default;
        Mat3x3(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33)
            : m11(m11), m12(m12), m13(m13), m21(m21), m22(m22), m23(m23), m31(m31), m32(m32), m33(m33)
        {
        }
        Mat3x3(float values[9])
        {
            TkMemory::memoryCopy(raw, values, sizeof(float) * 9);
        }
        Mat3x3(float values[3][3])
        {
            TkMemory::memoryCopy(raw, values, sizeof(float) * 9);
        }
        explicit Mat3x3(const Mat &matrix);

    public:
        TkString toString() const;

        // Gets the up FVec3 of the matrix; that is m21, m22, and m23.
        FVec3 getUp() const
        {
            return {m21, m22, m23};
        }
        // Sets FVec3 of the matrix; that is m21, m22, and m23.
        void setUp(const FVec3 &value)
        {
            m21 = value.x;
            m22 = value.y;
            m23 = value.z;
        }
        // Gets the down FVec3 of the matrix; that is -m21, -m22, and -m23.
        FVec3 getDown() const
        {
            return -FVec3(m21, m22, m23);
        }
        // Sets the down FVec3 of the matrix; that is -m21, -m22, and -m23.
        void setDown(const FVec3 &value)
        {
            m21 = -value.x;
            m22 = -value.y;
            m23 = -value.z;
        }
        // Gets the right FVec3 of the matrix; that is m11, m12, and m13.
        FVec3 getRight() const
        {
            return {m11, m12, m13};
        }
        // Sets the right FVec3 of the matrix; that is m11, m12, and m13.
        void setRight(const FVec3 &value)
        {
            m11 = value.x;
            m12 = value.y;
            m13 = value.z;
        }
        // Gets the left FVec3 of the matrix; that is -m11, -m12, and -m13.
        FVec3 getLeft() const
        {
            return -FVec3(m11, m12, m13);
        }
        // Sets the left FVec3 of the matrix; that is -m11, -m12, and -m13.
        void setLeft(const FVec3 &value)
        {
            m11 = -value.x;
            m12 = -value.y;
            m13 = -value.z;
        }

        // Gets the forward FVec3 of the matrix; that is -m31, -m32, and -m33.
        FVec3 getForward() const
        {
            return -FVec3(m31, m32, m33);
        }

        // Sets the forward FVec3 of the matrix; that is -m31, -m32, and -m33.
        void setForward(const FVec3 &value)
        {
            m31 = -value.x;
            m32 = -value.y;
            m33 = -value.z;
        }

        // Gets the backward FVec3 of the matrix; that is m31, m32, and m33.
        FVec3 getBackward() const
        {
            return {m31, m32, m33};
        }

        // Sets the backward FVec3 of the matrix; that is m31, m32, and m33.
        void setBackward(const FVec3 &value)
        {
            m31 = value.x;
            m32 = value.y;
            m33 = value.z;
        }

        // Gets the first row in the matrix; that is m11, m12 and m13.
        FVec3 getRow1() const
        {
            return {m11, m12, m13};
        }

        // Sets the first row in the matrix; that is m11, m12 and m13.
        void setRow1(const FVec3 &value)
        {
            m11 = value.x;
            m12 = value.y;
            m13 = value.z;
        }

        // Gets the second row in the matrix; that is m21, m22 and m23.
        FVec3 getRow2() const
        {
            return {m21, m22, m23};
        }

        // Sets the second row in the matrix; that is m21, m22 and m23.
        void setRow2(const FVec3 &value)
        {
            m21 = value.x;
            m22 = value.y;
            m23 = value.z;
        }

        // Gets the third row in the matrix; that is m31, m32 and m33.
        FVec3 getRow3() const
        {
            return {m31, m32, m33};
        }

        // Sets the third row in the matrix; that is m31, m32 and m33.
        void setRow3(const FVec3 &value)
        {
            m31 = value.x;
            m32 = value.y;
            m33 = value.z;
        }

        // Gets the first column in the matrix; that is m11, m21 and m31.
        FVec3 getColumn1() const
        {
            return {m11, m21, m31};
        }

        // Sets the first column in the matrix; that is m11, m21 and m31.
        void setColumn1(const FVec3 &value)
        {
            m11 = value.x;
            m21 = value.y;
            m31 = value.z;
        }

        // Gets the second column in the matrix; that is m12, m22 and m32.
        FVec3 getColumn2() const
        {
            return {m12, m22, m32};
        }

        // Sets the second column in the matrix; that is m12, m22 and m32.
        void setColumn2(const FVec3 &value)
        {
            m12 = value.x;
            m22 = value.y;
            m32 = value.z;
        }

        // Gets the third column in the matrix; that is m13, m23 and m33.
        FVec3 getColumn3() const
        {
            return {m13, m23, m33};
        }

        // Sets the third column in the matrix; that is m13, m23 and m33.
        void setColumn3(const FVec3 &value)
        {
            m13 = value.x;
            m23 = value.y;
            m33 = value.z;
        }

        // Gets the scale of the matrix; that is m11, m22, and m33.
        FVec3 getScaleVector() const
        {
            return {m11, m22, m33};
        }

        // Sets the scale of the matrix; that is m11, m22, and m33.
        void setScaleVector(const FVec3 &value)
        {
            m11 = value.x;
            m22 = value.y;
            m33 = value.z;
        }

        // Gets a value indicating whether this instance is an identity Mat3x3.
        bool isIdentity() const
        {
            return *this == kIdentity;
        }

        // Calculates the determinant of the Mat3x3.
        float getDeterminant() const;

        // Inverts the Mat3x3.
        void invert()
        {
            invert(*this, *this);
        }

        // Transposes the Mat3x3.
        void transpose()
        {
            transpose(*this, *this);
        }

        // Removes any scaling from the matrix by performing the normalization (each row magnitude is 1).
        void normalizeScale();

    public:
        // Calculates the inverse of the specified Mat3x3.
        static void invert(const Mat3x3 &value, Mat3x3 &result);
        // Calculates the inverse of the specified Mat3x3.
        static Mat3x3 invert(const Mat3x3 &value)
        {
            Mat3x3 result;
            invert(value, result);
            return result;
        }

        // Calculates the transpose of the specified Mat3x3.
        static void transpose(const Mat3x3 &value, Mat3x3 &result);

        // Calculates the transpose of the specified Mat3x3.
        static Mat3x3 transpose(const Mat3x3 &value)
        {
            Mat3x3 result;
            transpose(value, result);
            return result;
        }

    public:
        // Determines the sum of two matrices.
        static void add(const Mat3x3 &left, const Mat3x3 &right, Mat3x3 &result);

        // Determines the sum of two matrices.
        static Mat3x3 add(const Mat3x3 &left, const Mat3x3 &right)
        {
            Mat3x3 result;
            add(left, right, result);
            return result;
        }

        // Determines the difference between two matrices.
        static void subtract(const Mat3x3 &left, const Mat3x3 &right, Mat3x3 &result);

        // Determines the difference between two matrices.
        static Mat3x3 subtract(const Mat3x3 &left, const Mat3x3 &right)
        {
            Mat3x3 result;
            subtract(left, right, result);
            return result;
        }

        // Scales a Mat3x3 by the given value.
        static void multiply(const Mat3x3 &left, float right, Mat3x3 &result);

        // Scales a Mat3x3 by the given value.
        static Mat3x3 multiply(const Mat3x3 &left, float right)
        {
            Mat3x3 result;
            multiply(left, right, result);
            return result;
        }

        // Determines the product of two matrices.
        static void multiply(const Mat3x3 &left, const Mat3x3 &right, Mat3x3 &result);

        // Determines the product of two matrices.
        static Mat3x3 multiply(const Mat3x3 &left, const Mat3x3 &right)
        {
            Mat3x3 result;
            multiply(left, right, result);
            return result;
        }

        // Scales a Mat3x3 by the given value.
        static void divide(const Mat3x3 &left, float right, Mat3x3 &result);

        // Scales a Mat3x3 by the given value.
        static Mat3x3 divide(const Mat3x3 &left, float right)
        {
            Mat3x3 result;
            divide(left, right, result);
            return result;
        }

        // Determines the quotient of two matrices.
        static void divide(const Mat3x3 &left, const Mat3x3 &right, Mat3x3 &result);

        // Determines the quotient of two matrices.
        static Mat3x3 divide(const Mat3x3 &left, const Mat3x3 &right)
        {
            Mat3x3 result;
            divide(left, right, result);
            return result;
        }

        // Creates 2D translation matrix.
        static void translation2D(const FVec2 &translation, Mat3x3 &result);

        // Creates 2D translation matrix.
        static Mat3x3 translation2D(const FVec2 &translation)
        {
            Mat3x3 result;
            translation2D(translation, result);
            return result;
        }

        // Transforms given point by the matrix (in 2D).
        static void transform2DPoint(const FVec2 &point, const Mat3x3 &transform, FVec2 &result);

        // Transforms given vector by the matrix (in 2D).
        static void transform2DVector(const FVec2 &vector, const Mat3x3 &transform, FVec2 &result);

        // Creates a rotation matrix from a quaternion
        static Mat3x3 rotationQuaternion(const Quat &rotation)
        {
            Mat3x3 result;
            rotationQuaternion(rotation, result);
            return result;
        }

        // Creates a rotation matrix from a quaternion
        static void rotationQuaternion(const Quat &rotation, Mat3x3 &result);
        // Decomposes a matrix into a scale and rotation.
        void decompose(FVec3 &scale, Mat3x3 &rotation) const;
        // Decomposes a matrix into a scale and rotation.
        void decompose(FVec3 &scale, Quat &rotation) const;

    public:
        // Tests for equality between two objects.
        bool operator==(const Mat3x3 &other) const;

        // Tests for equality between two objects.
        TK_FORCE_INLINE bool operator!=(const Mat3x3 &other) const
        {
            return !operator==(other);
        }

    public:
        union
        {
            struct
            {
                float m11;
                float m12;
                float m13;
                float m21;
                float m22;
                float m23;
                float m31;
                float m32;
                float m33;
            };

            float values[3][3];
            float raw[9];
        };
    };

    template <>
    struct TIsPODType<Mat3x3>
    {
        enum
        {
            Value = true
        };
    };

    // Represent a 4x4 mathematical matrix using f64-precision floating-point values.
    struct TK_API DMat4x4
    {
    public:
        union
        {
            struct
            {
                f64 m11;
                f64 m12;
                f64 m13;
                f64 m14;
                f64 m21;
                f64 m22;
                f64 m23;
                f64 m24;
                f64 m31;
                f64 m32;
                f64 m33;
                f64 m34;
                f64 m41;
                f64 m42;
                f64 m43;
                f64 m44;
            };

            f64 values[4][4];
            f64 raw[16];
        };

    public:
        DMat4x4() = default;
        DMat4x4(const Mat &matrix);

    public:
        // Inverts the matrix.
        void invert()
        {
            invert(*this, *this);
        }

        // Calculate the inverse of the specified matrix.
        static DMat4x4 invert(const DMat4x4 &value)
        {
            DMat4x4 result;
            invert(value, result);
            return result;
        }

        // Calculate the inverse of the specified matrix.
        static void invert(const DMat4x4 &value, DMat4x4 &result);

        // Create a left-handed, look-at matrix.
        static void lookAt(const DVec3 &eye, const DVec3 &target, const DVec3 &up, DMat4x4 &result);

        // Calculates the product of two matrices.
        static void multiply(const DMat4x4 &left, const DMat4x4 &right, DMat4x4 &result);

        // Creates a matrix that contains both the X, Y and Z rotation, as well as scaling and translation.
        static void transformation(const FVec3 &scaling, const Quat &rotation, const Vec3 &translation, DMat4x4 &result);

    public:
        DMat4x4 operator*(const DMat4x4 &other) const
        {
            DMat4x4 result;
            multiply(*this, other, result);
            return result;
        }
    };

    template <>
    struct TIsPODType<DMat4x4>
    {
        enum
        {
            Value = true
        };
    };
}

FORMAT_CUSTOM(toolkit::Mat,
              "[m11:%g m12:%g m13:%g m14:%g] [m21:%g m22:%g m23:%g m24:%g] [m31:%g m32:%g m33:%g m34:%g] [m41:%g m42:%g m43:%g m44:%g]",
              t.m11, t.m12, t.m13, t.m14, t.m21, t.m22, t.m23, t.m24, t.m31, t.m32, t.m33, t.m34, t.m41, t.m42, t.m43, t.m44);

FORMAT_CUSTOM(toolkit::Mat3x3,
              "[m11:%g m12:%g m13:%g] [m21:%g m22:%g m23:%g] [m31:%g m32:%g m33:%g]",
              t.m11, t.m12, t.m13, t.m21, t.m22, t.m23, t.m31, t.m32, t.m33);
