/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Matrix.hpp"

#include "toolkit/types/String.hpp"
#include "Transform.hpp"
#include "Quat.hpp"
#include "Vector2.hpp"

namespace toolkit
{

    static_assert(sizeof(Mat) == 4 * 4 * 4, "Invalid Mat type size.");

    const Mat Mat::Zero(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    const Mat Mat::Identity(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    Mat::Mat(const Mat3x3 &matrix)
    {
        TkMemory::memoryCopy(&m11, &matrix.m11, sizeof(FVec3));
        TkMemory::memoryCopy(&m21, &matrix.m21, sizeof(FVec3));
        TkMemory::memoryCopy(&m31, &matrix.m31, sizeof(FVec3));
        m14 = 0.0f;
        m24 = 0.0f;
        m34 = 0.0f;
        m44 = 1.0f;
    }

    TkString Mat::toString() const
    {
        return TkString::format("%s", *this);
    }

    float Mat::getDeterminant() const
    {
        const float temp1 = m33 * m44 - m34 * m43;
        const float temp2 = m32 * m44 - m34 * m42;
        const float temp3 = m32 * m43 - m33 * m42;
        const float temp4 = m31 * m44 - m34 * m41;
        const float temp5 = m31 * m43 - m33 * m41;
        const float temp6 = m31 * m42 - m32 * m41;
        return m11 * (m22 * temp1 - m23 * temp2 + m24 * temp3) - m12 * (m21 * temp1 - m23 * temp4 + m24 * temp5) + m13 * (m21 * temp2 - m22 * temp4 + m24 * temp6) - m14 * (m21 * temp3 - m22 * temp5 + m23 * temp6);
    }

    float Mat::rotDeterminant() const
    {
        return values[0][0] * (values[1][1] * values[2][2] - values[1][2] * values[2][1]) -
               values[1][0] * (values[0][1] * values[2][2] - values[0][2] * values[2][1]) +
               values[2][0] * (values[0][1] * values[1][2] - values[0][2] * values[1][1]);
    }

    void Mat::normalizeScale()
    {
        const float scaleX = 1.0f / FVec3(m11, m21, m31).length();
        const float scaleY = 1.0f / FVec3(m12, m22, m32).length();
        const float scaleZ = 1.0f / FVec3(m13, m23, m33).length();

        m11 *= scaleX;
        m21 *= scaleX;
        m31 *= scaleX;

        m12 *= scaleY;
        m22 *= scaleY;
        m32 *= scaleY;

        m13 *= scaleZ;
        m23 *= scaleZ;
        m33 *= scaleZ;
    }

    void Mat::decompose(float &yaw, float &pitch, float &roll) const
    {
        pitch = TkMath::asin(-m32);
        if (TkMath::cos(pitch) > 1e-12f)
        {
            roll = TkMath::atan2(m12, m22);
            yaw = TkMath::atan2(m31, m33);
        }
        else
        {
            roll = TkMath::atan2(-m21, m11);
            yaw = 0.0f;
        }
    }

    void Mat::decompose(FVec3 &scale, FVec3 &translation) const
    {
        // Get the translation
        translation = FVec3(m41, m42, m43);

        // Scaling is the length of the rows
        scale = FVec3(
            TkMath::sqrt(m11 * m11 + m12 * m12 + m13 * m13),
            TkMath::sqrt(m21 * m21 + m22 * m22 + m23 * m23),
            TkMath::sqrt(m31 * m31 + m32 * m32 + m33 * m33));
    }

    void Mat::decompose(Transform &transform) const
    {
        Mat3x3 rotationMat;
        FVec3 translation;
        decompose(transform.Scale, rotationMat, translation);
        transform.Translation = translation;
        Quat::rotationMatrix(rotationMat, transform.Orientation);
    }

    void Mat::decompose(FVec3 &scale, Quat &rotation, FVec3 &translation) const
    {
        Mat3x3 rotationMat;
        decompose(scale, rotationMat, translation);
        Quat::rotationMatrix(rotationMat, rotation);
    }

    void Mat::decompose(FVec3 &scale, Mat3x3 &rotation, FVec3 &translation) const
    {
        // Get the translation
        translation = FVec3(m41, m42, m43);

        // Scaling is the length of the rows
        scale = FVec3(
            TkMath::sqrt(m11 * m11 + m12 * m12 + m13 * m13),
            TkMath::sqrt(m21 * m21 + m22 * m22 + m23 * m23),
            TkMath::sqrt(m31 * m31 + m32 * m32 + m33 * m33));

        // If any of the scaling factors are zero, than the rotation matrix can not exist
        rotation = Mat3x3::Identity;
        if (scale.isAnyZero())
            return;

        // Calculate an perfect orthonormal matrix (no reflections)
        const auto at = FVec3(m31 / scale.z, m32 / scale.z, m33 / scale.z);
        const auto up = FVec3::cross(at, FVec3(m11 / scale.x, m12 / scale.x, m13 / scale.x));
        const auto right = FVec3::cross(up, at);
        rotation.setRight(right);
        rotation.setUp(up);
        rotation.setBackward(at);

        // In case of reflexions
        scale.x = FVec3::dot(right, getRight()) > 0.0f ? scale.x : -scale.x;
        scale.y = FVec3::dot(up, getUp()) > 0.0f ? scale.y : -scale.y;
        scale.z = FVec3::dot(at, getBackward()) > 0.0f ? scale.z : -scale.z;
    }

    bool Mat::operator==(const Mat &other) const
    {
        for (i32 i = 0; i < 16; i++)
        {
            if (TkMath::notNearEqual(other.raw[i], raw[i]))
                return false;
        }
        return true;
    }

    void Mat::add(const Mat &left, const Mat &right, Mat &result)
    {
        result.m11 = left.m11 + right.m11;
        result.m12 = left.m12 + right.m12;
        result.m13 = left.m13 + right.m13;
        result.m14 = left.m14 + right.m14;
        result.m21 = left.m21 + right.m21;
        result.m22 = left.m22 + right.m22;
        result.m23 = left.m23 + right.m23;
        result.m24 = left.m24 + right.m24;
        result.m31 = left.m31 + right.m31;
        result.m32 = left.m32 + right.m32;
        result.m33 = left.m33 + right.m33;
        result.m34 = left.m34 + right.m34;
        result.m41 = left.m41 + right.m41;
        result.m42 = left.m42 + right.m42;
        result.m43 = left.m43 + right.m43;
        result.m44 = left.m44 + right.m44;
    }

    void Mat::subtract(const Mat &left, const Mat &right, Mat &result)
    {
        result.m11 = left.m11 - right.m11;
        result.m12 = left.m12 - right.m12;
        result.m13 = left.m13 - right.m13;
        result.m14 = left.m14 - right.m14;
        result.m21 = left.m21 - right.m21;
        result.m22 = left.m22 - right.m22;
        result.m23 = left.m23 - right.m23;
        result.m24 = left.m24 - right.m24;
        result.m31 = left.m31 - right.m31;
        result.m32 = left.m32 - right.m32;
        result.m33 = left.m33 - right.m33;
        result.m34 = left.m34 - right.m34;
        result.m41 = left.m41 - right.m41;
        result.m42 = left.m42 - right.m42;
        result.m43 = left.m43 - right.m43;
        result.m44 = left.m44 - right.m44;
    }

    void Mat::multiply(const Mat &left, float right, Mat &result)
    {
        result.m11 = left.m11 * right;
        result.m12 = left.m12 * right;
        result.m13 = left.m13 * right;
        result.m14 = left.m14 * right;
        result.m21 = left.m21 * right;
        result.m22 = left.m22 * right;
        result.m23 = left.m23 * right;
        result.m24 = left.m24 * right;
        result.m31 = left.m31 * right;
        result.m32 = left.m32 * right;
        result.m33 = left.m33 * right;
        result.m34 = left.m34 * right;
        result.m41 = left.m41 * right;
        result.m42 = left.m42 * right;
        result.m43 = left.m43 * right;
        result.m44 = left.m44 * right;
    }

    void Mat::multiply(const Mat &left, const Mat &right, Mat &result)
    {
        result.m11 = left.m11 * right.m11 + left.m12 * right.m21 + left.m13 * right.m31 + left.m14 * right.m41;
        result.m12 = left.m11 * right.m12 + left.m12 * right.m22 + left.m13 * right.m32 + left.m14 * right.m42;
        result.m13 = left.m11 * right.m13 + left.m12 * right.m23 + left.m13 * right.m33 + left.m14 * right.m43;
        result.m14 = left.m11 * right.m14 + left.m12 * right.m24 + left.m13 * right.m34 + left.m14 * right.m44;
        result.m21 = left.m21 * right.m11 + left.m22 * right.m21 + left.m23 * right.m31 + left.m24 * right.m41;
        result.m22 = left.m21 * right.m12 + left.m22 * right.m22 + left.m23 * right.m32 + left.m24 * right.m42;
        result.m23 = left.m21 * right.m13 + left.m22 * right.m23 + left.m23 * right.m33 + left.m24 * right.m43;
        result.m24 = left.m21 * right.m14 + left.m22 * right.m24 + left.m23 * right.m34 + left.m24 * right.m44;
        result.m31 = left.m31 * right.m11 + left.m32 * right.m21 + left.m33 * right.m31 + left.m34 * right.m41;
        result.m32 = left.m31 * right.m12 + left.m32 * right.m22 + left.m33 * right.m32 + left.m34 * right.m42;
        result.m33 = left.m31 * right.m13 + left.m32 * right.m23 + left.m33 * right.m33 + left.m34 * right.m43;
        result.m34 = left.m31 * right.m14 + left.m32 * right.m24 + left.m33 * right.m34 + left.m34 * right.m44;
        result.m41 = left.m41 * right.m11 + left.m42 * right.m21 + left.m43 * right.m31 + left.m44 * right.m41;
        result.m42 = left.m41 * right.m12 + left.m42 * right.m22 + left.m43 * right.m32 + left.m44 * right.m42;
        result.m43 = left.m41 * right.m13 + left.m42 * right.m23 + left.m43 * right.m33 + left.m44 * right.m43;
        result.m44 = left.m41 * right.m14 + left.m42 * right.m24 + left.m43 * right.m34 + left.m44 * right.m44;
    }

    void Mat::divide(const Mat &left, float right, Mat &result)
    {
        TK_ASSERT(!TkMath::isZero(right));
        const float inv = 1.0f / right;

        result.m11 = left.m11 * inv;
        result.m12 = left.m12 * inv;
        result.m13 = left.m13 * inv;
        result.m14 = left.m14 * inv;
        result.m21 = left.m21 * inv;
        result.m22 = left.m22 * inv;
        result.m23 = left.m23 * inv;
        result.m24 = left.m24 * inv;
        result.m31 = left.m31 * inv;
        result.m32 = left.m32 * inv;
        result.m33 = left.m33 * inv;
        result.m34 = left.m34 * inv;
        result.m41 = left.m41 * inv;
        result.m42 = left.m42 * inv;
        result.m43 = left.m43 * inv;
        result.m44 = left.m44 * inv;
    }

    void Mat::divide(const Mat &left, const Mat &right, Mat &result)
    {
        result.m11 = left.m11 / right.m11;
        result.m12 = left.m12 / right.m12;
        result.m13 = left.m13 / right.m13;
        result.m14 = left.m14 / right.m14;
        result.m21 = left.m21 / right.m21;
        result.m22 = left.m22 / right.m22;
        result.m23 = left.m23 / right.m23;
        result.m24 = left.m24 / right.m24;
        result.m31 = left.m31 / right.m31;
        result.m32 = left.m32 / right.m32;
        result.m33 = left.m33 / right.m33;
        result.m34 = left.m34 / right.m34;
        result.m41 = left.m41 / right.m41;
        result.m42 = left.m42 / right.m42;
        result.m43 = left.m43 / right.m43;
        result.m44 = left.m44 / right.m44;
    }

    void Mat::negate(const Mat &value, Mat &result)
    {
        result.m11 = -value.m11;
        result.m12 = -value.m12;
        result.m13 = -value.m13;
        result.m14 = -value.m14;
        result.m21 = -value.m21;
        result.m22 = -value.m22;
        result.m23 = -value.m23;
        result.m24 = -value.m24;
        result.m31 = -value.m31;
        result.m32 = -value.m32;
        result.m33 = -value.m33;
        result.m34 = -value.m34;
        result.m41 = -value.m41;
        result.m42 = -value.m42;
        result.m43 = -value.m43;
        result.m44 = -value.m44;
    }

    void Mat::lerp(const Mat &start, const Mat &end, float amount, Mat &result)
    {
        result.m11 = TkMath::lerp(start.m11, end.m11, amount);
        result.m12 = TkMath::lerp(start.m12, end.m12, amount);
        result.m13 = TkMath::lerp(start.m13, end.m13, amount);
        result.m14 = TkMath::lerp(start.m14, end.m14, amount);
        result.m21 = TkMath::lerp(start.m21, end.m21, amount);
        result.m22 = TkMath::lerp(start.m22, end.m22, amount);
        result.m23 = TkMath::lerp(start.m23, end.m23, amount);
        result.m24 = TkMath::lerp(start.m24, end.m24, amount);
        result.m31 = TkMath::lerp(start.m31, end.m31, amount);
        result.m32 = TkMath::lerp(start.m32, end.m32, amount);
        result.m33 = TkMath::lerp(start.m33, end.m33, amount);
        result.m34 = TkMath::lerp(start.m34, end.m34, amount);
        result.m41 = TkMath::lerp(start.m41, end.m41, amount);
        result.m42 = TkMath::lerp(start.m42, end.m42, amount);
        result.m43 = TkMath::lerp(start.m43, end.m43, amount);
        result.m44 = TkMath::lerp(start.m44, end.m44, amount);
    }

    Mat Mat::transpose(const Mat &value)
    {
        Mat result;
        result.m11 = value.m11;
        result.m12 = value.m21;
        result.m13 = value.m31;
        result.m14 = value.m41;
        result.m21 = value.m12;
        result.m22 = value.m22;
        result.m23 = value.m32;
        result.m24 = value.m42;
        result.m31 = value.m13;
        result.m32 = value.m23;
        result.m33 = value.m33;
        result.m34 = value.m43;
        result.m41 = value.m14;
        result.m42 = value.m24;
        result.m43 = value.m34;
        result.m44 = value.m44;
        return result;
    }

    void Mat::transpose(const Mat &value, Mat &result)
    {
        Mat temp;
        temp.m11 = value.m11;
        temp.m12 = value.m21;
        temp.m13 = value.m31;
        temp.m14 = value.m41;
        temp.m21 = value.m12;
        temp.m22 = value.m22;
        temp.m23 = value.m32;
        temp.m24 = value.m42;
        temp.m31 = value.m13;
        temp.m32 = value.m23;
        temp.m33 = value.m33;
        temp.m34 = value.m43;
        temp.m41 = value.m14;
        temp.m42 = value.m24;
        temp.m43 = value.m34;
        temp.m44 = value.m44;
        result = temp;
    }

    void Mat::invert(const Mat &value, Mat &result)
    {
        const float b0 = value.m31 * value.m42 - value.m32 * value.m41;
        const float b1 = value.m31 * value.m43 - value.m33 * value.m41;
        const float b2 = value.m34 * value.m41 - value.m31 * value.m44;
        const float b3 = value.m32 * value.m43 - value.m33 * value.m42;
        const float b4 = value.m34 * value.m42 - value.m32 * value.m44;
        const float b5 = value.m33 * value.m44 - value.m34 * value.m43;

        const float d11 = value.m22 * b5 + value.m23 * b4 + value.m24 * b3;
        const float d12 = value.m21 * b5 + value.m23 * b2 + value.m24 * b1;
        const float d13 = value.m21 * -b4 + value.m22 * b2 + value.m24 * b0;
        const float d14 = value.m21 * b3 + value.m22 * -b1 + value.m23 * b0;

        float det = value.m11 * d11 - value.m12 * d12 + value.m13 * d13 - value.m14 * d14;
        if (TkMath::abs(det) <= 1e-12f)
        {
            result = Zero;
            return;
        }

        det = 1.0f / det;

        const float a0 = value.m11 * value.m22 - value.m12 * value.m21;
        const float a1 = value.m11 * value.m23 - value.m13 * value.m21;
        const float a2 = value.m14 * value.m21 - value.m11 * value.m24;
        const float a3 = value.m12 * value.m23 - value.m13 * value.m22;
        const float a4 = value.m14 * value.m22 - value.m12 * value.m24;
        const float a5 = value.m13 * value.m24 - value.m14 * value.m23;

        const float d21 = value.m12 * b5 + value.m13 * b4 + value.m14 * b3;
        const float d22 = value.m11 * b5 + value.m13 * b2 + value.m14 * b1;
        const float d23 = value.m11 * -b4 + value.m12 * b2 + value.m14 * b0;
        const float d24 = value.m11 * b3 + value.m12 * -b1 + value.m13 * b0;

        const float d31 = value.m42 * a5 + value.m43 * a4 + value.m44 * a3;
        const float d32 = value.m41 * a5 + value.m43 * a2 + value.m44 * a1;
        const float d33 = value.m41 * -a4 + value.m42 * a2 + value.m44 * a0;
        const float d34 = value.m41 * a3 + value.m42 * -a1 + value.m43 * a0;

        const float d41 = value.m32 * a5 + value.m33 * a4 + value.m34 * a3;
        const float d42 = value.m31 * a5 + value.m33 * a2 + value.m34 * a1;
        const float d43 = value.m31 * -a4 + value.m32 * a2 + value.m34 * a0;
        const float d44 = value.m31 * a3 + value.m32 * -a1 + value.m33 * a0;

        result.m11 = +d11 * det;
        result.m12 = -d21 * det;
        result.m13 = +d31 * det;
        result.m14 = -d41 * det;
        result.m21 = -d12 * det;
        result.m22 = +d22 * det;
        result.m23 = -d32 * det;
        result.m24 = +d42 * det;
        result.m31 = +d13 * det;
        result.m32 = -d23 * det;
        result.m33 = +d33 * det;
        result.m34 = -d43 * det;
        result.m41 = -d14 * det;
        result.m42 = +d24 * det;
        result.m43 = -d34 * det;
        result.m44 = +d44 * det;
    }

    void Mat::billboard(const FVec3 &objectPosition, const FVec3 &cameraPosition, const FVec3 &cameraUpFloat, const FVec3 &cameraForwardFloat, Mat &result)
    {
        FVec3 crossed;
        FVec3 final;
        FVec3 difference = cameraPosition - objectPosition;

        const float lengthSq = difference.lengthSquared();
        if (TkMath::isZero(lengthSq))
            difference = -cameraForwardFloat;
        else
            difference *= 1.0f / TkMath::sqrt(lengthSq);

        FVec3::cross(cameraUpFloat, difference, crossed);
        crossed.normalize();
        FVec3::cross(difference, crossed, final);

        result.m11 = crossed.x;
        result.m12 = crossed.y;
        result.m13 = crossed.z;
        result.m14 = 0.0f;

        result.m21 = final.x;
        result.m22 = final.y;
        result.m23 = final.z;
        result.m24 = 0.0f;

        result.m31 = difference.x;
        result.m32 = difference.y;
        result.m33 = difference.z;
        result.m34 = 0.0f;

        result.m41 = objectPosition.x;
        result.m42 = objectPosition.y;
        result.m43 = objectPosition.z;
        result.m44 = 1.0f;
    }

    void Mat::lookAt(const FVec3 &eye, const FVec3 &target, const FVec3 &up, Mat &result)
    {
        FVec3 xaxis, yaxis, zaxis;
        FVec3::subtract(target, eye, zaxis);
        zaxis.normalize();
        FVec3::cross(up, zaxis, xaxis);
        xaxis.normalize();
        FVec3::cross(zaxis, xaxis, yaxis);

        result.m11 = xaxis.x;
        result.m21 = xaxis.y;
        result.m31 = xaxis.z;

        result.m12 = yaxis.x;
        result.m22 = yaxis.y;
        result.m32 = yaxis.z;

        result.m13 = zaxis.x;
        result.m23 = zaxis.y;
        result.m33 = zaxis.z;

        result.m14 = 0.0f;
        result.m24 = 0.0f;
        result.m34 = 0.0f;

        result.m41 = -FVec3::dot(xaxis, eye);
        result.m42 = -FVec3::dot(yaxis, eye);
        result.m43 = -FVec3::dot(zaxis, eye);
        result.m44 = 1.0f;
    }

    void Mat::orthoOffCenter(float left, float right, float bottom, float top, float zNear, float zFar, Mat &result)
    {
        const float zRange = 1.0f / (zFar - zNear);

        result = Identity;
        result.m11 = 2.0f / (right - left);
        result.m22 = 2.0f / (top - bottom);
        result.m33 = zRange;
        result.m41 = (left + right) / (left - right);
        result.m42 = (top + bottom) / (bottom - top);
        result.m43 = -zNear * zRange;
    }

    void Mat::perspectiveFov(float fov, float aspect, float zNear, float zFar, Mat &result)
    {
        const float yScale = 1.0f / TkMath::tan(fov * 0.5f);
        const float xScale = yScale / aspect;

        const float halfWidth = zNear / xScale;
        const float halfHeight = zNear / yScale;

        perspectiveOffCenter(-halfWidth, halfWidth, -halfHeight, halfHeight, zNear, zFar, result);
    }

    void Mat::perspectiveOffCenter(float left, float right, float bottom, float top, float zNear, float zFar, Mat &result)
    {
        const float zRange = zFar / (zFar - zNear);

        result = Zero;
        result.m11 = 2.0f * zNear / (right - left);
        result.m22 = 2.0f * zNear / (top - bottom);
        result.m31 = (left + right) / (left - right);
        result.m32 = (top + bottom) / (bottom - top);
        result.m33 = zRange;
        result.m34 = 1.0f;
        result.m43 = -zNear * zRange;
    }

    void Mat::rotationX(float angle, Mat &result)
    {
        const float cosA = TkMath::cos(angle);
        const float sinA = TkMath::sin(angle);
        result = Identity;
        result.m22 = cosA;
        result.m23 = sinA;
        result.m32 = -sinA;
        result.m33 = cosA;
    }

    void Mat::rotationY(float angle, Mat &result)
    {
        const float cosA = TkMath::cos(angle);
        const float sinA = TkMath::sin(angle);
        result = Identity;
        result.m11 = cosA;
        result.m13 = -sinA;
        result.m31 = sinA;
        result.m33 = cosA;
    }

    void Mat::rotationZ(float angle, Mat &result)
    {
        const float cosA = TkMath::cos(angle);
        const float sinA = TkMath::sin(angle);
        result = Identity;
        result.m11 = cosA;
        result.m12 = sinA;
        result.m21 = -sinA;
        result.m22 = cosA;
    }

    void Mat::rotationAxis(const FVec3 &axis, float angle, Mat &result)
    {
        const float x = axis.x;
        const float y = axis.y;
        const float z = axis.z;
        const float cosA = TkMath::cos(angle);
        const float sinA = TkMath::sin(angle);
        const float xx = x * x;
        const float yy = y * y;
        const float zz = z * z;
        const float xy = x * y;
        const float xz = x * z;
        const float yz = y * z;

        result = Identity;
        result.m11 = xx + cosA * (1.0f - xx);
        result.m12 = xy - cosA * xy + sinA * z;
        result.m13 = xz - cosA * xz - sinA * y;
        result.m21 = xy - cosA * xy - sinA * z;
        result.m22 = yy + cosA * (1.0f - yy);
        result.m23 = yz - cosA * yz + sinA * x;
        result.m31 = xz - cosA * xz + sinA * y;
        result.m32 = yz - cosA * yz - sinA * x;
        result.m33 = zz + cosA * (1.0f - zz);
    }

    void Mat::rotationQuaternion(const Quat &rotation, Mat &result)
    {
        const float xx = rotation.x * rotation.x;
        const float yy = rotation.y * rotation.y;
        const float zz = rotation.z * rotation.z;
        const float xy = rotation.x * rotation.y;
        const float zw = rotation.z * rotation.w;
        const float zx = rotation.z * rotation.x;
        const float yw = rotation.y * rotation.w;
        const float yz = rotation.y * rotation.z;
        const float xw = rotation.x * rotation.w;

        result.m11 = 1.0f - 2.0f * (yy + zz);
        result.m12 = 2.0f * (xy + zw);
        result.m13 = 2.0f * (zx - yw);
        result.m14 = 0;

        result.m21 = 2.0f * (xy - zw);
        result.m22 = 1.0f - 2.0f * (zz + xx);
        result.m23 = 2.0f * (yz + xw);
        result.m24 = 0;

        result.m31 = 2.0f * (zx + yw);
        result.m32 = 2.0f * (yz - xw);
        result.m33 = 1.0f - 2.0f * (yy + xx);
        result.m34 = 0;

        result.m41 = 0;
        result.m42 = 0;
        result.m43 = 0;
        result.m44 = 1;
    }

    void Mat::rotationYawPitchRoll(float yaw, float pitch, float roll, Mat &result)
    {
        Quat quaternion;
        Quat::rotationYawPitchRoll(yaw, pitch, roll, quaternion);
        rotationQuaternion(quaternion, result);
    }

    Mat Mat::translation(const FVec3 &value)
    {
        Mat result = Identity;
        result.m41 = value.x;
        result.m42 = value.y;
        result.m43 = value.z;
        return result;
    }

    void Mat::translation(const FVec3 &value, Mat &result)
    {
        result = Identity;
        result.m41 = value.x;
        result.m42 = value.y;
        result.m43 = value.z;
    }

    void Mat::translation(float x, float y, float z, Mat &result)
    {
        result = Identity;
        result.m41 = x;
        result.m42 = y;
        result.m43 = z;
    }

    void Mat::skew(float angle, const FVec3 &rotationVec, const FVec3 &transVec, Mat &matrix)
    {
        // http://elckerlyc.ewi.utwente.nl/browser/Elckerlyc/Hmi/HmiMath/src/hmi/math/Mat3f.java
        const float MINIMAL_SKEW_ANGLE = 0.000001f;

        FVec3 e0 = rotationVec;
        FVec3 e1;
        FVec3::normalize(transVec, e1);

        const float rv1 = FVec3::dot(rotationVec, e1);
        e0 += rv1 * e1;
        const float rv0 = FVec3::dot(rotationVec, e0);
        const float cosa = TkMath::cos(angle);
        const float sina = TkMath::sin(angle);
        const float rr0 = rv0 * cosa - rv1 * sina;
        const float rr1 = rv0 * sina + rv1 * cosa;

        TK_ASSERT(rr0 >= MINIMAL_SKEW_ANGLE);

        const float d = rr1 / rr0 - rv1 / rv0;

        matrix = Identity;
        matrix.m11 = d * e1.x * e0.x + 1.0f;
        matrix.m12 = d * e1.x * e0.y;
        matrix.m13 = d * e1.x * e0.z;
        matrix.m21 = d * e1.y * e0.x;
        matrix.m22 = d * e1.y * e0.y + 1.0f;
        matrix.m23 = d * e1.y * e0.z;
        matrix.m31 = d * e1.z * e0.x;
        matrix.m32 = d * e1.z * e0.y;
        matrix.m33 = d * e1.z * e0.z + 1.0f;
    }

    void Mat::transformation(const FVec3 &scaling, const Quat &rotation, const FVec3 &translation, Mat &result)
    {
        // Equivalent to:
        // result =
        //    Mat.Scaling(scaling)
        //    *Mat.RotationX(rotation.x)
        //    *Mat.RotationY(rotation.y)
        //    *Mat.RotationZ(rotation.z)
        //    *Mat.Position(translation);

        // Rotation
        const float xx = rotation.x * rotation.x;
        const float yy = rotation.y * rotation.y;
        const float zz = rotation.z * rotation.z;
        const float xy = rotation.x * rotation.y;
        const float zw = rotation.z * rotation.w;
        const float zx = rotation.z * rotation.x;
        const float yw = rotation.y * rotation.w;
        const float yz = rotation.y * rotation.z;
        const float xw = rotation.x * rotation.w;
        result.m11 = 1.0f - 2.0f * (yy + zz);
        result.m12 = 2.0f * (xy + zw);
        result.m13 = 2.0f * (zx - yw);
        result.m21 = 2.0f * (xy - zw);
        result.m22 = 1.0f - 2.0f * (zz + xx);
        result.m23 = 2.0f * (yz + xw);
        result.m31 = 2.0f * (zx + yw);
        result.m32 = 2.0f * (yz - xw);
        result.m33 = 1.0f - 2.0f * (yy + xx);

        // Position
        result.m41 = translation.x;
        result.m42 = translation.y;
        result.m43 = translation.z;

        // Scale
        result.m11 *= scaling.x;
        result.m12 *= scaling.x;
        result.m13 *= scaling.x;
        result.m21 *= scaling.y;
        result.m22 *= scaling.y;
        result.m23 *= scaling.y;
        result.m31 *= scaling.z;
        result.m32 *= scaling.z;
        result.m33 *= scaling.z;

        result.m14 = 0.0f;
        result.m24 = 0.0f;
        result.m34 = 0.0f;
        result.m44 = 1.0f;
    }

    void Mat::affineTransformation(float scaling, const Quat &rotation, const FVec3 &translation, Mat &result)
    {
        result = Mat::scaling(scaling) * rotationQuaternion(rotation) * Mat::translation(translation);
    }

    void Mat::affineTransformation(float scaling, const FVec3 &rotationCenter, const Quat &rotation, const FVec3 &translation, Mat &result)
    {
        result = Mat::scaling(scaling) * Mat::translation(-rotationCenter) * rotationQuaternion(rotation) * Mat::translation(rotationCenter) * Mat::translation(translation);
    }

    void Mat::affineTransformation2D(float scaling, float rotation, const FVec2 &translation, Mat &result)
    {
        result = Mat::scaling(scaling, scaling, 1.0f) * Mat::rotationZ(rotation) * Mat::translation((FVec3)translation);
    }

    void Mat::affineTransformation2D(float scaling, const FVec2 &rotationCenter, float rotation, const FVec2 &translation, Mat &result)
    {
        result = Mat::scaling(scaling, scaling, 1.0f) * Mat::translation((FVec3)-rotationCenter) * rotationZ(rotation) * Mat::translation((FVec3)rotationCenter) * Mat::translation((FVec3)translation);
    }

    void Mat::transformation(const FVec3 &scalingCenter, const Quat &scalingRotation, const FVec3 &scaling, const FVec3 &rotationCenter, const Quat &rotation, const FVec3 &translation, Mat &result)
    {
        Mat sr;
        rotationQuaternion(scalingRotation, sr);
        result = Mat::translation(-scalingCenter) * transpose(sr) * Mat::scaling(scaling) * sr * Mat::translation(scalingCenter) * Mat::translation(-rotationCenter) * rotationQuaternion(rotation) * Mat::translation(rotationCenter) * Mat::translation(translation);
    }

    void Mat::transformation2D(const FVec2 &scalingCenter, float scalingRotation, const FVec2 &scaling, const FVec2 &rotationCenter, float rotation, const FVec2 &translation, Mat &result)
    {
        result = Mat::translation((FVec3)-scalingCenter) * rotationZ(-scalingRotation) * Mat::scaling((FVec3)scaling) * rotationZ(scalingRotation) * Mat::translation((FVec3)scalingCenter) * Mat::translation((FVec3)-rotationCenter) * rotationZ(rotation) * Mat::translation((FVec3)rotationCenter) * Mat::translation((FVec3)translation);
        result.m33 = 1.0f;
        result.m44 = 1.0f;
    }

    Mat Mat::createWorld(const FVec3 &position, const FVec3 &forward, const FVec3 &up)
    {
        Mat result;
        createWorld(position, forward, up, result);
        return result;
    }

    void Mat::createWorld(const FVec3 &position, const FVec3 &forward, const FVec3 &up, Mat &result)
    {
        FVec3 vector3, vector31, vector32;

        FVec3::normalize(forward, vector3);
        vector3 = vector3.getNegative();
        FVec3::normalize(FVec3::cross(up, vector3), vector31);
        FVec3::cross(vector3, vector31, vector32);

        result.m11 = vector31.x;
        result.m12 = vector31.y;
        result.m13 = vector31.z;
        result.m14 = 0.0f;

        result.m21 = vector32.x;
        result.m22 = vector32.y;
        result.m23 = vector32.z;
        result.m24 = 0.0f;

        result.m31 = vector3.x;
        result.m32 = vector3.y;
        result.m33 = vector3.z;
        result.m34 = 0.0f;

        result.m41 = position.x;
        result.m42 = position.y;
        result.m43 = position.z;
        result.m44 = 1.0f;
    }

    Mat Mat::createFromAxisAngle(const FVec3 &axis, float angle)
    {
        Mat result;
        createFromAxisAngle(axis, angle, result);
        return result;
    }

    void Mat::createFromAxisAngle(const FVec3 &axis, float angle, Mat &result)
    {
        const float x = axis.x;
        const float y = axis.y;
        const float z = axis.z;
        const float single = TkMath::sin(angle);
        const float single1 = TkMath::cos(angle);
        const float single2 = x * x;
        const float single3 = y * y;
        const float single4 = z * z;
        const float single5 = x * y;
        const float single6 = x * z;
        const float single7 = y * z;

        result.m11 = single2 + single1 * (1.0f - single2);
        result.m12 = single5 - single1 * single5 + single * z;
        result.m13 = single6 - single1 * single6 - single * y;
        result.m14 = 0.0f;

        result.m21 = single5 - single1 * single5 - single * z;
        result.m22 = single3 + single1 * (1.0f - single3);
        result.m23 = single7 - single1 * single7 + single * x;
        result.m24 = 0.0f;

        result.m31 = single6 - single1 * single6 + single * y;
        result.m32 = single7 - single1 * single7 - single * x;
        result.m33 = single4 + single1 * (1.0f - single4);
        result.m34 = 0.0f;

        result.m41 = 0.0f;
        result.m42 = 0.0f;
        result.m43 = 0.0f;
        result.m44 = 1.0f;
    }

    Vec3 Mat::transformVector(const Mat &m, const Vec3 &v)
    {
        return Vec3(
            m.values[0][0] * v.raw[0] + m.values[1][0] * v.raw[1] + m.values[2][0] * v.raw[2],
            m.values[0][1] * v.raw[0] + m.values[1][1] * v.raw[1] + m.values[2][1] * v.raw[2],
            m.values[0][2] * v.raw[0] + m.values[1][2] * v.raw[1] + m.values[2][2] * v.raw[2]);
    }

    FVec4 Mat::transformPosition(const Mat &m, const FVec3 &v)
    {
        return FVec4(
            m.values[0][0] * v.raw[0] + m.values[1][0] * v.raw[1] + m.values[2][0] * v.raw[2] + m.values[3][0],
            m.values[0][1] * v.raw[0] + m.values[1][1] * v.raw[1] + m.values[2][1] * v.raw[2] + m.values[3][1],
            m.values[0][2] * v.raw[0] + m.values[1][2] * v.raw[1] + m.values[2][2] * v.raw[2] + m.values[3][2],
            m.values[0][3] * v.raw[0] + m.values[1][3] * v.raw[1] + m.values[2][3] * v.raw[2] + m.values[3][3]);
    }

    FVec4 Mat::transformPosition(const Mat &m, const FVec4 &v)
    {
        return FVec4(
            m.values[0][0] * v.raw[0] + m.values[1][0] * v.raw[1] + m.values[2][0] * v.raw[2] + m.values[3][0] * v.raw[3],
            m.values[0][1] * v.raw[0] + m.values[1][1] * v.raw[1] + m.values[2][1] * v.raw[2] + m.values[3][1] * v.raw[3],
            m.values[0][2] * v.raw[0] + m.values[1][2] * v.raw[1] + m.values[2][2] * v.raw[2] + m.values[3][2] * v.raw[3],
            m.values[0][3] * v.raw[0] + m.values[1][3] * v.raw[1] + m.values[2][3] * v.raw[2] + m.values[3][3] * v.raw[3]);
    }

    void Mat3x4::setMatrix(const Mat &m)
    {
        const float *src = m.raw;
        float *dst = raw;
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
        dst[4] = src[4];
        dst[5] = src[5];
        dst[6] = src[6];
        dst[7] = src[7];
        dst[8] = src[8];
        dst[9] = src[9];
        dst[10] = src[10];
        dst[11] = src[11];
    }

    void Mat3x4::setMatrixTranspose(const Mat &m)
    {
        const float *src = m.raw;
        float *dst = raw;
        dst[0] = src[0];
        dst[1] = src[4];
        dst[2] = src[8];
        dst[3] = src[12];
        dst[4] = src[1];
        dst[5] = src[5];
        dst[6] = src[9];
        dst[7] = src[13];
        dst[8] = src[2];
        dst[9] = src[6];
        dst[10] = src[10];
        dst[11] = src[14];
    }

    const Mat3x3 Mat3x3::Zero(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    const Mat3x3 Mat3x3::Identity(
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f);

    Mat3x3::Mat3x3(const Mat &matrix)
    {
        memcpy(&m11, &matrix.m11, sizeof(FVec3));
        memcpy(&m21, &matrix.m21, sizeof(FVec3));
        memcpy(&m31, &matrix.m31, sizeof(FVec3));
    }

    TkString Mat3x3::toString() const
    {
        return TkString::format("%s", *this);
    }

    float Mat3x3::getDeterminant() const
    {
        return m11 * m22 * m33 + m12 * m23 * m31 + m13 * m21 * m32 - m13 * m22 * m31 - m12 * m21 * m33 - m11 * m23 * m32;
    }

    void Mat3x3::normalizeScale()
    {
        const float scaleX = 1.0f / FVec3(m11, m21, m31).length();
        const float scaleY = 1.0f / FVec3(m12, m22, m32).length();
        const float scaleZ = 1.0f / FVec3(m13, m23, m33).length();

        m11 *= scaleX;
        m21 *= scaleX;
        m31 *= scaleX;

        m12 *= scaleY;
        m22 *= scaleY;
        m32 *= scaleY;

        m13 *= scaleZ;
        m23 *= scaleZ;
        m33 *= scaleZ;
    }

    void Mat3x3::invert(const Mat3x3 &value, Mat3x3 &result)
    {
        const float d11 = value.m22 * value.m33 + value.m23 * -value.m32;
        const float d12 = value.m21 * value.m33 + value.m23 * -value.m31;
        const float d13 = value.m21 * value.m32 + value.m22 * -value.m31;

        float det = value.m11 * d11 - value.m12 * d12 + value.m13 * d13;
        if (TkMath::abs(det) < TkMath::ZeroTolerance)
        {
            result = Zero;
            return;
        }

        det = 1.0f / det;

        const float d21 = value.m12 * value.m33 + value.m13 * -value.m32;
        const float d22 = value.m11 * value.m33 + value.m13 * -value.m31;
        const float d23 = value.m11 * value.m32 + value.m12 * -value.m31;

        const float d31 = value.m12 * value.m23 - value.m13 * value.m22;
        const float d32 = value.m11 * value.m23 - value.m13 * value.m21;
        const float d33 = value.m11 * value.m22 - value.m12 * value.m21;

        result = Mat3x3(
            +d11 * det,
            -d21 * det,
            +d31 * det,
            -d12 * det,
            +d22 * det,
            -d32 * det,
            +d13 * det,
            -d23 * det,
            +d33 * det);
    }

    void Mat3x3::transpose(const Mat3x3 &value, Mat3x3 &result)
    {
        result = Mat3x3(
            value.m11,
            value.m21,
            value.m31,
            value.m12,
            value.m22,
            value.m32,
            value.m13,
            value.m23,
            value.m33);
    }

    void Mat3x3::add(const Mat3x3 &left, const Mat3x3 &right, Mat3x3 &result)
    {
        result.m11 = left.m11 + right.m11;
        result.m12 = left.m12 + right.m12;
        result.m13 = left.m13 + right.m13;
        result.m21 = left.m21 + right.m21;
        result.m22 = left.m22 + right.m22;
        result.m23 = left.m23 + right.m23;
        result.m31 = left.m31 + right.m31;
        result.m32 = left.m32 + right.m32;
        result.m33 = left.m33 + right.m33;
    }

    void Mat3x3::subtract(const Mat3x3 &left, const Mat3x3 &right, Mat3x3 &result)
    {
        result.m11 = left.m11 - right.m11;
        result.m12 = left.m12 - right.m12;
        result.m13 = left.m13 - right.m13;
        result.m21 = left.m21 - right.m21;
        result.m22 = left.m22 - right.m22;
        result.m23 = left.m23 - right.m23;
        result.m31 = left.m31 - right.m31;
        result.m32 = left.m32 - right.m32;
        result.m33 = left.m33 - right.m33;
    }

    void Mat3x3::multiply(const Mat3x3 &left, float right, Mat3x3 &result)
    {
        result.m11 = left.m11 * right;
        result.m12 = left.m12 * right;
        result.m13 = left.m13 * right;
        result.m21 = left.m21 * right;
        result.m22 = left.m22 * right;
        result.m23 = left.m23 * right;
        result.m31 = left.m31 * right;
        result.m32 = left.m32 * right;
        result.m33 = left.m33 * right;
    }

    void Mat3x3::multiply(const Mat3x3 &left, const Mat3x3 &right, Mat3x3 &result)
    {
        result = Mat3x3(
            left.m11 * right.m11 + left.m12 * right.m21 + left.m13 * right.m31,
            left.m11 * right.m12 + left.m12 * right.m22 + left.m13 * right.m32,
            left.m11 * right.m13 + left.m12 * right.m23 + left.m13 * right.m33,
            left.m21 * right.m11 + left.m22 * right.m21 + left.m23 * right.m31,
            left.m21 * right.m12 + left.m22 * right.m22 + left.m23 * right.m32,
            left.m21 * right.m13 + left.m22 * right.m23 + left.m23 * right.m33,
            left.m31 * right.m11 + left.m32 * right.m21 + left.m33 * right.m31,
            left.m31 * right.m12 + left.m32 * right.m22 + left.m33 * right.m32,
            left.m31 * right.m13 + left.m32 * right.m23 + left.m33 * right.m33);
    }

    void Mat3x3::divide(const Mat3x3 &left, float right, Mat3x3 &result)
    {
        TK_ASSERT(!TkMath::isZero(right));
        const float inv = 1.0f / right;

        result.m11 = left.m11 * inv;
        result.m12 = left.m12 * inv;
        result.m13 = left.m13 * inv;
        result.m21 = left.m21 * inv;
        result.m22 = left.m22 * inv;
        result.m23 = left.m23 * inv;
        result.m31 = left.m31 * inv;
        result.m32 = left.m32 * inv;
        result.m33 = left.m33 * inv;
    }

    void Mat3x3::divide(const Mat3x3 &left, const Mat3x3 &right, Mat3x3 &result)
    {
        result.m11 = left.m11 / right.m11;
        result.m12 = left.m12 / right.m12;
        result.m13 = left.m13 / right.m13;
        result.m21 = left.m21 / right.m21;
        result.m22 = left.m22 / right.m22;
        result.m23 = left.m23 / right.m23;
        result.m31 = left.m31 / right.m31;
        result.m32 = left.m32 / right.m32;
        result.m33 = left.m33 / right.m33;
    }

    void Mat3x3::translation2D(const FVec2 &translation, Mat3x3 &result)
    {
        result = Mat3x3(
            1, 0, 0,
            0, 1, 0,
            translation.x, translation.y, 1);
    }

    void Mat3x3::transform2DPoint(const FVec2 &point, const Mat3x3 &transform, FVec2 &result)
    {
        result = FVec2(
            point.x * transform.m11 + point.y * transform.m21 + transform.m31,
            point.x * transform.m12 + point.y * transform.m22 + transform.m32);
    }

    void Mat3x3::transform2DVector(const FVec2 &vector, const Mat3x3 &transform, FVec2 &result)
    {
        result = FVec2(
            vector.x * transform.m11 + vector.y * transform.m21,
            vector.x * transform.m12 + vector.y * transform.m22);
    }

    void Mat3x3::rotationQuaternion(const Quat &rotation, Mat3x3 &result)
    {
        const float xx = rotation.x * rotation.x;
        const float yy = rotation.y * rotation.y;
        const float zz = rotation.z * rotation.z;
        const float xy = rotation.x * rotation.y;
        const float zw = rotation.z * rotation.w;
        const float zx = rotation.z * rotation.x;
        const float yw = rotation.y * rotation.w;
        const float yz = rotation.y * rotation.z;
        const float xw = rotation.x * rotation.w;

        result.m11 = 1.0f - 2.0f * (yy + zz);
        result.m12 = 2.0f * (xy + zw);
        result.m13 = 2.0f * (zx - yw);

        result.m21 = 2.0f * (xy - zw);
        result.m22 = 1.0f - 2.0f * (zz + xx);
        result.m23 = 2.0f * (yz + xw);

        result.m31 = 2.0f * (zx + yw);
        result.m32 = 2.0f * (yz - xw);
        result.m33 = 1.0f - 2.0f * (yy + xx);
    }

    void Mat3x3::decompose(FVec3 &scale, Mat3x3 &rotation) const
    {
        // Scaling is the length of the rows
        scale = FVec3(
            TkMath::sqrt(m11 * m11 + m12 * m12 + m13 * m13),
            TkMath::sqrt(m21 * m21 + m22 * m22 + m23 * m23),
            TkMath::sqrt(m31 * m31 + m32 * m32 + m33 * m33));

        // If any of the scaling factors are zero, than the rotation matrix can not exist
        rotation = Identity;
        if (scale.isAnyZero())
            return;

        // Calculate an perfect orthonormal matrix (no reflections)
        const auto at = FVec3(m31 / scale.z, m32 / scale.z, m33 / scale.z);
        const auto up = FVec3::cross(at, FVec3(m11 / scale.x, m12 / scale.x, m13 / scale.x));
        const auto right = FVec3::cross(up, at);
        rotation.setRight(right);
        rotation.setUp(up);
        rotation.setBackward(at);

        // In case of reflexions
        scale.x = FVec3::dot(right, getRight()) > 0.0f ? scale.x : -scale.x;
        scale.y = FVec3::dot(up, getUp()) > 0.0f ? scale.y : -scale.y;
        scale.z = FVec3::dot(at, getBackward()) > 0.0f ? scale.z : -scale.z;
    }

    void Mat3x3::decompose(FVec3 &scale, Quat &rotation) const
    {
        Mat3x3 rotationMatrix;
        decompose(scale, rotationMatrix);
        Quat::rotationMatrix(rotationMatrix, rotation);
    }

    bool Mat3x3::operator==(const Mat3x3 &other) const
    {
        return TkMath::nearEqual(m11, other.m11) &&
               TkMath::nearEqual(m12, other.m12) &&
               TkMath::nearEqual(m13, other.m13) &&
               TkMath::nearEqual(m21, other.m21) &&
               TkMath::nearEqual(m22, other.m22) &&
               TkMath::nearEqual(m23, other.m23) &&
               TkMath::nearEqual(m31, other.m31) &&
               TkMath::nearEqual(m32, other.m32) &&
               TkMath::nearEqual(m33, other.m33);
    }
}