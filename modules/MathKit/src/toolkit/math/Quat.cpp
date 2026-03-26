/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Quat.hpp"

#include "Vector3.hpp"
#include "Vector4.hpp"
#include "Matrix.hpp"
#include "toolkit/base/Math.hpp"
#include "Transform.hpp"
#include "toolkit/types/String.hpp"

namespace toolkit
{

    Quat Quat::Zero(0, 0, 0, 0);
    Quat Quat::One(1, 1, 1, 1);
    Quat Quat::Identity(0, 0, 0, 1);
    Quat::Quat(const FVec4 &value)
        : x(value.x), y(value.y), z(value.z), w(value.w)
    {
    }

    TkString Quat::toString() const
    {
        return TkString::format("%s", *this);
    }

    float Quat::getAngle() const
    {
        const float length = x * x + y * y + z * z;
        if (TkMath::isZero(length))
            return 0.0f;
        return 2.0f * acosf(TkMath::clamp(w, -1.0f, 1.0f));
    }

    FVec3 Quat::getAxis() const
    {
        const float length = x * x + y * y + z * z;
        if (TkMath::isZero(length))
            return FVec3::UnitX;
        const float inv = 1.0f / TkMath::sqrt(length);
        return {x * inv, y * inv, z * inv};
    }

    FVec3 Quat::getEuler() const
    {
        FVec3 result;
        const float sqw = w * w;
        const float sqx = x * x;
        const float sqy = y * y;
        const float sqz = z * z;
        const float unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
        const float test = x * w - y * z;

        if (test > 0.499995f * unit)
        {
            // singularity at north pole

            // yaw pitch roll
            result.y = 2.0f * TkMath::atan2(y, x);
            result.x = TkMath::PI_OVER_2;
            result.z = 0;
        }
        else if (test < -0.499995f * unit)
        {
            // singularity at south pole

            // yaw pitch roll
            result.y = -2.0f * TkMath::atan2(y, x);
            result.x = -TkMath::PI_OVER_2;
            result.z = 0;
        }
        else
        {
            // yaw pitch roll
            const Quat q = Quat(w, z, x, y);
            result.y = TkMath::atan2(2.0f * q.x * q.w + 2.0f * q.y * q.z, 1 - 2.0f * (q.z * q.z + q.w * q.w));
            result.x = TkMath::asin(2.0f * (q.x * q.z - q.w * q.y));
            result.z = TkMath::atan2(2.0f * q.x * q.y + 2.0f * q.z * q.w, 1 - 2.0f * (q.y * q.y + q.z * q.z));
        }

        return TkMath::unwindDegrees(result * TkMath::RadiansToDegrees);
    }

    void Quat::multiply(const Quat &other)
    {
        const float a = y * other.z - z * other.y;
        const float b = z * other.x - x * other.z;
        const float c = x * other.y - y * other.x;
        const float d = x * other.x + y * other.y + z * other.z;
        x = x * other.w + other.x * w + a;
        y = y * other.w + other.y * w + b;
        z = z * other.w + other.z * w + c;
        w = w * other.w - d;
    }

    FVec3 Quat::operator*(const FVec3 &vector) const
    {
        return FVec3::transform(vector, *this);
    }

    void Quat::add(const Quat &left, const Quat &right, Quat &result)
    {
        result.x = left.x + right.x;
        result.y = left.y + right.y;
        result.z = left.z + right.z;
        result.w = left.w + right.w;
    }

    void Quat::subtract(const Quat &left, const Quat &right, Quat &result)
    {
        result.x = left.x - right.x;
        result.y = left.y - right.y;
        result.z = left.z - right.z;
        result.w = left.w - right.w;
    }

    void Quat::multiply(const Quat &left, const Quat &right, Quat &result)
    {
        const float a = left.y * right.z - left.z * right.y;
        const float b = left.z * right.x - left.x * right.z;
        const float c = left.x * right.y - left.y * right.x;
        const float d = left.x * right.x + left.y * right.y + left.z * right.z;
        result.x = left.x * right.w + right.x * left.w + a;
        result.y = left.y * right.w + right.y * left.w + b;
        result.z = left.z * right.w + right.z * left.w + c;
        result.w = left.w * right.w - d;
    }

    void Quat::negate(const Quat &value, Quat &result)
    {
        result.x = -value.x;
        result.y = -value.y;
        result.z = -value.z;
        result.w = -value.w;
    }

    void Quat::lerp(const Quat &start, const Quat &end, float amount, Quat &result)
    {
        const float inverse = 1.0f - amount;
        if (dot(start, end) >= 0.0f)
        {
            result.x = inverse * start.x + amount * end.x;
            result.y = inverse * start.y + amount * end.y;
            result.z = inverse * start.z + amount * end.z;
            result.w = inverse * start.w + amount * end.w;
        }
        else
        {
            result.x = inverse * start.x - amount * end.x;
            result.y = inverse * start.y - amount * end.y;
            result.z = inverse * start.z - amount * end.z;
            result.w = inverse * start.w - amount * end.w;
        }
        result.normalize();
    }

    void Quat::rotationAxis(const FVec3 &axis, float angle, Quat &result)
    {
        FVec3 normalized;
        FVec3::normalize(axis, normalized);

        const float half = angle * 0.5f;
        const float sinHalf = TkMath::sin(half);
        const float cosHalf = TkMath::cos(half);

        result.x = normalized.x * sinHalf;
        result.y = normalized.y * sinHalf;
        result.z = normalized.z * sinHalf;
        result.w = cosHalf;
    }

    void Quat::rotationCosAxis(const FVec3 &axis, float cos, Quat &result)
    {
        FVec3 normalized;
        FVec3::normalize(axis, normalized);

        const float cosHalf2 = (1.0f + cos) * 0.5f;
        const float sinHalf2 = 1.0f - cosHalf2;
        const float cosHalf = TkMath::sqrt(cosHalf2);
        const float sinHalf = TkMath::sqrt(sinHalf2);

        result.x = normalized.x * sinHalf;
        result.y = normalized.y * sinHalf;
        result.z = normalized.z * sinHalf;
        result.w = cosHalf;
    }

    void Quat::rotationMatrix(const Mat &matrix, Quat &result)
    {
        float sqrtV;
        float half;
        const float scale = matrix.m11 + matrix.m22 + matrix.m33;

        if (scale > 0.0f)
        {
            sqrtV = TkMath::sqrt(scale + 1.0f);
            result.w = sqrtV * 0.5f;
            sqrtV = 0.5f / sqrtV;

            result.x = (matrix.m23 - matrix.m32) * sqrtV;
            result.y = (matrix.m31 - matrix.m13) * sqrtV;
            result.z = (matrix.m12 - matrix.m21) * sqrtV;
        }
        else if (matrix.m11 >= matrix.m22 && matrix.m11 >= matrix.m33)
        {
            sqrtV = TkMath::sqrt(1.0f + matrix.m11 - matrix.m22 - matrix.m33);
            half = 0.5f / sqrtV;

            result = Quat(
                0.5f * sqrtV,
                (matrix.m12 + matrix.m21) * half,
                (matrix.m13 + matrix.m31) * half,
                (matrix.m23 - matrix.m32) * half);
        }
        else if (matrix.m22 > matrix.m33)
        {
            sqrtV = TkMath::sqrt(1.0f + matrix.m22 - matrix.m11 - matrix.m33);
            half = 0.5f / sqrtV;

            result = Quat(
                (matrix.m21 + matrix.m12) * half,
                0.5f * sqrtV,
                (matrix.m32 + matrix.m23) * half,
                (matrix.m31 - matrix.m13) * half);
        }
        else
        {
            sqrtV = TkMath::sqrt(1.0f + matrix.m33 - matrix.m11 - matrix.m22);
            half = 0.5f / sqrtV;

            result = Quat(
                (matrix.m31 + matrix.m13) * half,
                (matrix.m32 + matrix.m23) * half,
                0.5f * sqrtV,
                (matrix.m12 - matrix.m21) * half);
        }

        result.normalize();
    }

    void Quat::rotationMatrix(const Mat3x3 &matrix, Quat &result)
    {
        float sqrtV;
        float half;
        const float scale = matrix.m11 + matrix.m22 + matrix.m33;

        if (scale > 0.0f)
        {
            sqrtV = TkMath::sqrt(scale + 1.0f);
            result.w = sqrtV * 0.5f;
            sqrtV = 0.5f / sqrtV;

            result.x = (matrix.m23 - matrix.m32) * sqrtV;
            result.y = (matrix.m31 - matrix.m13) * sqrtV;
            result.z = (matrix.m12 - matrix.m21) * sqrtV;
        }
        else if (matrix.m11 >= matrix.m22 && matrix.m11 >= matrix.m33)
        {
            sqrtV = TkMath::sqrt(1.0f + matrix.m11 - matrix.m22 - matrix.m33);
            half = 0.5f / sqrtV;

            result = Quat(
                0.5f * sqrtV,
                (matrix.m12 + matrix.m21) * half,
                (matrix.m13 + matrix.m31) * half,
                (matrix.m23 - matrix.m32) * half);
        }
        else if (matrix.m22 > matrix.m33)
        {
            sqrtV = TkMath::sqrt(1.0f + matrix.m22 - matrix.m11 - matrix.m33);
            half = 0.5f / sqrtV;

            result = Quat(
                (matrix.m21 + matrix.m12) * half,
                0.5f * sqrtV,
                (matrix.m32 + matrix.m23) * half,
                (matrix.m31 - matrix.m13) * half);
        }
        else
        {
            sqrtV = TkMath::sqrt(1.0f + matrix.m33 - matrix.m11 - matrix.m22);
            half = 0.5f / sqrtV;

            result = Quat(
                (matrix.m31 + matrix.m13) * half,
                (matrix.m32 + matrix.m23) * half,
                0.5f * sqrtV,
                (matrix.m12 - matrix.m21) * half);
        }

        result.normalize();
    }

    void Quat::lookAt(const FVec3 &eye, const FVec3 &target, const FVec3 &up, Quat &result)
    {
        Mat matrix{};
        Mat::lookAt(eye, target, up, matrix);
        rotationMatrix(matrix, result);
    }

    void Quat::rotationLookAt(const FVec3 &forward, const FVec3 &up, Quat &result)
    {
        lookAt(FVec3::Zero, forward, up, result);
    }

    void Quat::billboard(const FVec3 &objectPosition, const FVec3 &cameraPosition, const FVec3 &cameraUpVector, const FVec3 &cameraForwardVector, Quat &result)
    {
        Mat matrix{};
        Mat::billboard(objectPosition, cameraPosition, cameraUpVector, cameraForwardVector, matrix);
        rotationMatrix(matrix, result);
    }

    Quat Quat::fromDirection(const FVec3 &direction)
    {
        TK_CHECK_RETURN_DEBUG(direction.isNormalized(), Quat::Identity);
        Quat orientation;
        if (FVec3::dot(direction, FVec3::Up) >= 0.999f)
        {
            rotationAxis(FVec3::Left, TkMath::PI_OVER_2, orientation);
        }
        else if (FVec3::dot(direction, FVec3::Down) >= 0.999f)
        {
            rotationAxis(FVec3::Right, TkMath::PI_OVER_2, orientation);
        }
        else
        {
            FVec3 right, up;
            FVec3::cross(direction, FVec3::Up, right);
            FVec3::cross(right, direction, up);
            lookRotation(direction, up, orientation);
        }
        return orientation;
    }

    void Quat::lookRotation(const FVec3 &forward, const FVec3 &up, Quat &result)
    {
        FVec3 forwardNorm = forward;
        forwardNorm.normalize();
        FVec3 rightNorm;
        FVec3::cross(up, forwardNorm, rightNorm);
        rightNorm.normalize();
        FVec3 upNorm;
        FVec3::cross(forwardNorm, rightNorm, upNorm);

#define m00 rightNorm.x
#define m01 rightNorm.y
#define m02 rightNorm.z
#define m10 upNorm.x
#define m11 upNorm.y
#define m12 upNorm.z
#define m20 forwardNorm.x
#define m21 forwardNorm.y
#define m22 forwardNorm.z

        const float sum = m00 + m11 + m22;
        if (sum > 0)
        {
            const float num = TkMath::sqrt(sum + 1);
            const float invNumHalf = 0.5f / num;
            result.x = (m12 - m21) * invNumHalf;
            result.y = (m20 - m02) * invNumHalf;
            result.z = (m01 - m10) * invNumHalf;
            result.w = num * 0.5f;
        }
        else if (m00 >= m11 && m00 >= m22)
        {
            const float num = TkMath::sqrt(1 + m00 - m11 - m22);
            const float invNumHalf = 0.5f / num;
            result.x = 0.5f * num;
            result.y = (m01 + m10) * invNumHalf;
            result.z = (m02 + m20) * invNumHalf;
            result.w = (m12 - m21) * invNumHalf;
        }
        else if (m11 > m22)
        {
            const float num = TkMath::sqrt(1 + m11 - m00 - m22);
            const float invNumHalf = 0.5f / num;
            result.x = (m10 + m01) * invNumHalf;
            result.y = 0.5f * num;
            result.z = (m21 + m12) * invNumHalf;
            result.w = (m20 - m02) * invNumHalf;
        }
        else
        {
            const float num = TkMath::sqrt(1 + m22 - m00 - m11);
            const float invNumHalf = 0.5f / num;
            result.x = (m20 + m02) * invNumHalf;
            result.y = (m21 + m12) * invNumHalf;
            result.z = 0.5f * num;
            result.w = (m01 - m10) * invNumHalf;
        }

#undef m00
#undef m01
#undef m02
#undef m10
#undef m11
#undef m12
#undef m20
#undef m21
#undef m22
    }

    void Quat::getRotationFromTo(const FVec3 &from, const FVec3 &to, Quat &result, const FVec3 &fallbackAxis)
    {
        // Based on Stan Melax's article in Game Programming Gems

        FVec3 v0 = from;
        FVec3 v1 = to;
        v0.normalize();
        v1.normalize();

        // If dot == 1, vectors are the same
        const float d = FVec3::dot(v0, v1);
        if (d >= 1.0f)
        {
            result = Identity;
            return;
        }

        if (d < 1e-6f - 1.0f)
        {
            if (fallbackAxis != FVec3::Zero)
            {
                // Rotate 180 degrees about the fallback axis
                rotationAxis(fallbackAxis, TkMath::PI, result);
            }
            else
            {
                // Generate an axis
                FVec3 axis = FVec3::cross(FVec3::UnitX, from);
                if (axis.lengthSquared() < TkMath::ZeroTolerance) // Pick another if colinear
                    axis = FVec3::cross(FVec3::UnitY, from);
                axis.normalize();
                rotationAxis(axis, TkMath::PI, result);
            }
        }
        else
        {
            const float s = TkMath::sqrt((1 + d) * 2);
            const float invS = 1 / s;

            FVec3 c;
            FVec3::cross(v0, v1, c);

            result.x = c.x * invS;
            result.y = c.y * invS;
            result.z = c.z * invS;
            result.w = s * 0.5f;
            result.normalize();
        }
    }

    void Quat::findBetween(const FVec3 &from, const FVec3 &to, Quat &result)
    {
        // http://lolengine.net/blog/2014/02/24/quaternion-from-two-vectors-final
        const float normFromNormTo = TkMath::sqrt(from.lengthSquared() * to.lengthSquared());
        if (normFromNormTo < TkMath::ZeroTolerance)
        {
            result = Identity;
            return;
        }
        const float w = normFromNormTo + FVec3::dot(from, to);
        if (w < 1.e-6f * normFromNormTo)
        {
            result = TkMath::abs(from.x) > TkMath::abs(from.z)
                         ? Quat(-from.y, from.x, 0.0f, 0.0f)
                         : Quat(0.0f, -from.z, from.y, 0.0f);
        }
        else
        {
            const FVec3 cross = FVec3::cross(from, to);
            result = Quat(cross.x, cross.y, cross.z, w);
        }
        result.normalize();
    }

    void Quat::slerp(const Quat &start, const Quat &end, float amount, Quat &result)
    {
        float opposite;
        float inverse;
        const float dot = Quat::dot(start, end);

        if (TkMath::abs(dot) > 1.0f - TkMath::ZeroTolerance)
        {
            inverse = 1.0f - amount;
            opposite = amount * TkMath::sign(dot);
        }
        else
        {
            const float acos1 = TkMath::acos(TkMath::abs(dot));
            const float invSin = 1.0f / TkMath::sin(acos1);
            inverse = TkMath::sin((1.0f - amount) * acos1) * invSin;
            opposite = TkMath::sin(amount * acos1) * invSin * TkMath::sign(dot);
        }

        result.x = inverse * start.x + opposite * end.x;
        result.y = inverse * start.y + opposite * end.y;
        result.z = inverse * start.z + opposite * end.z;
        result.w = inverse * start.w + opposite * end.w;
    }

    Quat Quat::euler(float x, float y, float z)
    {
        const float halfRoll = z * (TkMath::DegreesToRadians * 0.5f);
        const float halfPitch = x * (TkMath::DegreesToRadians * 0.5f);
        const float halfYaw = y * (TkMath::DegreesToRadians * 0.5f);

        const float sinRollOver2 = TkMath::sin(halfRoll);
        const float cosRollOver2 = TkMath::cos(halfRoll);
        const float sinPitchOver2 = TkMath::sin(halfPitch);
        const float cosPitchOver2 = TkMath::cos(halfPitch);
        const float sinYawOver2 = TkMath::sin(halfYaw);
        const float cosYawOver2 = TkMath::cos(halfYaw);

        return Quat(
            cosYawOver2 * sinPitchOver2 * cosRollOver2 + sinYawOver2 * cosPitchOver2 * sinRollOver2,
            sinYawOver2 * cosPitchOver2 * cosRollOver2 - cosYawOver2 * sinPitchOver2 * sinRollOver2,
            cosYawOver2 * cosPitchOver2 * sinRollOver2 - sinYawOver2 * sinPitchOver2 * cosRollOver2,
            cosYawOver2 * cosPitchOver2 * cosRollOver2 + sinYawOver2 * sinPitchOver2 * sinRollOver2);
    }

    Quat Quat::euler(const FVec3 &euler)
    {
        const float halfRoll = euler.z * (TkMath::DegreesToRadians * 0.5f);
        const float halfPitch = euler.x * (TkMath::DegreesToRadians * 0.5f);
        const float halfYaw = euler.y * (TkMath::DegreesToRadians * 0.5f);

        const float sinRollOver2 = TkMath::sin(halfRoll);
        const float cosRollOver2 = TkMath::cos(halfRoll);
        const float sinPitchOver2 = TkMath::sin(halfPitch);
        const float cosPitchOver2 = TkMath::cos(halfPitch);
        const float sinYawOver2 = TkMath::sin(halfYaw);
        const float cosYawOver2 = TkMath::cos(halfYaw);

        return Quat(
            cosYawOver2 * sinPitchOver2 * cosRollOver2 + sinYawOver2 * cosPitchOver2 * sinRollOver2,
            sinYawOver2 * cosPitchOver2 * cosRollOver2 - cosYawOver2 * sinPitchOver2 * sinRollOver2,
            cosYawOver2 * cosPitchOver2 * sinRollOver2 - sinYawOver2 * sinPitchOver2 * cosRollOver2,
            cosYawOver2 * cosPitchOver2 * cosRollOver2 + sinYawOver2 * sinPitchOver2 * sinRollOver2);
    }

    void Quat::rotationYawPitchRoll(float yaw, float pitch, float roll, Quat &result)
    {
        const float halfRoll = roll * 0.5f;
        const float halfPitch = pitch * 0.5f;
        const float halfYaw = yaw * 0.5f;

        const float sinRollOver2 = TkMath::sin(halfRoll);
        const float cosRollOver2 = TkMath::cos(halfRoll);
        const float sinPitchOver2 = TkMath::sin(halfPitch);
        const float cosPitchOver2 = TkMath::cos(halfPitch);
        const float sinYawOver2 = TkMath::sin(halfYaw);
        const float cosYawOver2 = TkMath::cos(halfYaw);

        result.w = cosYawOver2 * cosPitchOver2 * cosRollOver2 + sinYawOver2 * sinPitchOver2 * sinRollOver2;
        result.x = cosYawOver2 * sinPitchOver2 * cosRollOver2 + sinYawOver2 * cosPitchOver2 * sinRollOver2;
        result.y = sinYawOver2 * cosPitchOver2 * cosRollOver2 - cosYawOver2 * sinPitchOver2 * sinRollOver2;
        result.z = cosYawOver2 * cosPitchOver2 * sinRollOver2 - sinYawOver2 * sinPitchOver2 * cosRollOver2;
    }

    Quat Quat::GetRotationFromNormal(const Vec3 &normal, const Transform &reference)
    {
        FVec3 up = reference.getUp();
        const real dot = Vec3::dot(normal, up);
        if (TkMath::nearEqual(TkMath::abs(dot), 1))
            up = reference.getRight();
        return Quat::lookRotation(normal, up);
    }

}