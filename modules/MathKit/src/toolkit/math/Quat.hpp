/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/math/common.hpp>

#include <toolkit/base/Math.hpp>
#include <toolkit/base/Format.hpp>

namespace toolkit
{

    class Mat;
    struct Mat3x3;

    // Represents a four dimensional mathematical quaternion. Euler angles are stored in: pitch, yaw, roll order (x, y, z).
    class TK_API Quat
    {
    public:
        // Equality tolerance factor used when comparing quaternions via dot operation.
        static constexpr real kTolerance = 0.9999999f;
        // Quat with all components equal 0.
        static Quat kZero;
        // Quat with all components equal 1.
        static Quat kOne;
        // Identity quaternion (represents no rotation).
        static Quat kIdentity;

    public:
        Quat() = default;
        Quat(const float x, const float y, const float z, const float w)
            : x(x), y(y), z(z), w(w) {}
        explicit Quat(const FVec4 &value);

    public:
        TkString toString() const;

        // Gets a value indicating whether this instance is equivalent to the identity quaternion.
        bool isIdentity() const
        {
            return TkMath::isZero(x) && TkMath::isZero(y) && TkMath::isZero(z) && TkMath::isOne(w);
        }

        // Gets a value indicting whether this instance is normalized.
        bool isNormalized() const
        {
            return TkMath::abs((x * x + y * y + z * z + w * w) - 1.0f) < 1e-4f;
        }

        // Returns true if quaternion has one or more components is not a number (NaN).
        bool isNaN() const
        {
            return isnan(x) || isnan(y) || isnan(z) || isnan(w);
        }

        // Returns true if quaternion has one or more components equal to +/- infinity.
        bool isInfinity() const
        {
            return isinf(x) || isinf(y) || isinf(z) || isinf(w);
        }

        // Returns true if quaternion has one or more components equal to +/- infinity or NaN.
        bool isNanOrInfinity() const
        {
            return isInfinity() || isNaN();
        }

        // Gets the angle of the quaternion.
        float getAngle() const;

        // Gets the axis components of the quaternion.
        FVec3 getAxis() const;

        // Calculates the length of the quaternion.
        float length() const
        {
            return TkMath::sqrt(x * x + y * y + z * z + w * w);
        }

        // Calculates the squared length of the quaternion.
        float lengthSquared() const
        {
            return x * x + y * y + z * z + w * w;
        }

        // Gets the euler angle (pitch, yaw, roll) in degrees.
        FVec3 getEuler() const;

        // Conjugates the quaternion
        void conjugate();
        // Gets the conjugated quaternion.
        Quat conjugated() const
        {
            return {-x, -y, -z, w};
        }

        // Conjugates and renormalizes the quaternion.
        void invert();
        // Converts the quaternion into a unit quaternion.
        void normalize();

        // Scales a quaternion by the given value.
        void multiply(float scale);
        // Multiplies a quaternion by another.
        void multiply(const Quat &other);

    public:
        // Adds two quaternions.
        inline Quat operator+(const Quat &b) const
        {
            return {x + b.x, y + b.y, z + b.z, w + b.w};
        }

        // Subtracts two quaternions.
        inline Quat operator-(const Quat &b) const
        {
            return {x - b.x, y - b.y, z - b.z, w - b.w};
        }

        // Multiplies two quaternions.
        inline Quat operator*(const Quat &b) const
        {
            Quat result{};
            multiply(*this, b, result);
            return result;
        }

        // Adds two quaternions.
        inline Quat &operator+=(const Quat &b)
        {
            x += b.x;
            y += b.y;
            z += b.z;
            w += b.w;
            return *this;
        }

        // Subtracts two quaternions.
        inline Quat &operator-=(const Quat &b)
        {
            x -= b.x;
            y -= b.y;
            z -= b.z;
            w -= b.w;
            return *this;
        }

        // Multiplies two quaternions.
        inline Quat &operator*=(const Quat &b)
        {
            multiply(b);
            return *this;
        }

        // Scales a quaternion by the given value.
        inline Quat operator*(float scale) const
        {
            Quat q = *this;
            q.multiply(scale);
            return q;
        }

        // Transforms a vector by the given rotation.
        FVec3 operator*(const FVec3 &vector) const;

        // Scales a quaternion by the given value.
        TK_FORCE_INLINE Quat &operator*=(float scale)
        {
            multiply(scale);
            return *this;
        }

        // Determines whether the specified <see cref="Quat" /> is equal to this instance.
        TK_FORCE_INLINE bool operator==(const Quat &other) const
        {
            return dot(*this, other) > kTolerance;
        }

        // Determines whether the specified <see cref="Quat" /> is equal to this instance.
        TK_FORCE_INLINE bool operator!=(const Quat &other) const
        {
            return !(*this == other);
        }

    public:
        // Determines whether the specified <see cref="Quat" /> structures are equal.
        static bool nearEqual(const Quat &a, const Quat &b)
        {
            return dot(a, b) > kTolerance;
        }

        // Determines whether the specified <see cref="Quat" /> structures are equal.
        static bool nearEqual(const Quat &a, const Quat &b, float epsilon)
        {
            return dot(a, b) > 1.0f - epsilon;
        }

        // Calculates the inverse of the specified quaternion.
        static Quat invert(const Quat &value)
        {
            Quat result;
            invert(value, result);
            return result;
        }

        // Calculates the inverse of the specified quaternion.
        static void invert(const Quat &value, Quat &result)
        {
            result = value;
            result.invert();
        }

        // Calculates the dot product of two quaternions.
        static float dot(const Quat &left, const Quat &right)
        {
            return left.x * right.x + left.y * right.y + left.z * right.z + left.w * right.w;
        }

        // Calculates the angle between two quaternions.
        static float angleBetween(const Quat &a, const Quat &b)
        {
            const float dot_ = dot(a, b);
            return dot_ > kTolerance ? 0 : TkMath::acos(TkMath::min(TkMath::abs(dot_), 1.0f)) * 2.0f * 57.29578f;
        }

        // Adds two quaternions.
        static void add(const Quat &left, const Quat &right, Quat &result);
        // Subtracts two quaternions.
        static void subtract(const Quat &left, const Quat &right, Quat &result);
        // Scales a quaternion by the given value.
        static void multiply(const Quat &value, float scale, Quat &result);

        // Multiplies a quaternion by another.
        static void multiply(const Quat &left, const Quat &right, Quat &result);
        // Reverses the direction of a given quaternion.
        static void negate(const Quat &value, Quat &result);

        // Performs a linear interpolation between two quaternions.
        static void lerp(const Quat &start, const Quat &end, float amount, Quat &result);
        // Performs a linear interpolation between two quaternion.
        static Quat lerp(const Quat &start, const Quat &end, float amount);

        // Creates a quaternion given an angle cosine (radians in range [-1,1]) and an axis of rotation (normalized).
        static void rotationAxis(const FVec3 &axis, float angle, Quat &result);
        // Creates a quaternion given an angle cosine (radians in range [-1,1]) and an axis of rotation (normalized).
        static void rotationCosAxis(const FVec3 &axis, float cos, Quat &result);
        // Creates a quaternion given a rotation matrix.
        static void rotationMatrix(const Mat &matrix, Quat &result);
        // Creates a quaternion given a rotation matrix.
        static void rotationMatrix(const Mat3x3 &matrix, Quat &result);

        // Creates a left-handed, look-at quaternion.
        static void lookAt(const FVec3 &eye, const FVec3 &target, const FVec3 &up, Quat &result);

        // Creates a left-handed, look-at quaternion.
        static void rotationLookAt(const FVec3 &forward, const FVec3 &up, Quat &result);

        // Creates a left-handed spherical billboard that rotates around a specified object position.
        static void billboard(const FVec3 &objectPosition, const FVec3 &cameraPosition, const FVec3 &cameraUpFloat, const FVec3 &cameraForwardFloat, Quat &result);
        // Calculates the orientation from the direction vector.
        static Quat fromDirection(const FVec3 &direction);

        // Creates a rotation with the specified forward and upwards directions.
        static void lookRotation(const FVec3 &forward, const FVec3 &up, Quat &result);
        // Creates a rotation with the specified forward and upwards directions.
        static Quat lookRotation(const FVec3 &forward, const FVec3 &up);

        // Gets the shortest arc quaternion to rotate this vector to the destination vector.
        static void getRotationFromTo(const FVec3 &from, const FVec3 &to, Quat &result, const FVec3 &fallbackAxis);
        // Gets the shortest arc quaternion to rotate this vector to the destination vector.
        static Quat getRotationFromTo(const FVec3 &from, const FVec3 &to, const FVec3 &fallbackAxis);

        // Gets the quaternion that will rotate vector from into vector to, around their plan perpendicular axis.The input vectors don't need to be normalized.
        static void findBetween(const FVec3 &from, const FVec3 &to, Quat &result);
        // Gets the quaternion that will rotate vector from into vector to, around their plan perpendicular axis.The input vectors don't need to be normalized.
        static Quat findBetween(const FVec3 &from, const FVec3 &to);
        // Creates a quaternion given a rotation matrix.
        static Quat rotationMatrix(const Mat &matrix);

        // Interpolates between two quaternions, using spherical linear interpolation.
        static void slerp(const Quat &start, const Quat &end, float amount, Quat &result);
        // Creates a quaternion given a yaw, pitch, and roll value (in degrees).
        static Quat euler(float x, float y, float z);
        // Creates a quaternion given a yaw, pitch, and roll value in order: X:roll, Y:pitch, Z:yaw (in degrees).
        static Quat euler(const FVec3 &euler);

        // Creates a quaternion given a yaw, pitch, and roll value (in radians).
        static Quat rotationYawPitchRoll(float yaw, float pitch, float roll);
        // Creates a quaternion given a yaw, pitch, and roll value (in radians).
        static void rotationYawPitchRoll(float yaw, float pitch, float roll, Quat &result);
        // Gets rotation from a normal in relation to a transform. This function is especially useful for axis aligned faces, and with <seealso cref="Physics::RayCast"/>.
        static Quat GetRotationFromNormal(const Vec3 &normal, const Transform &reference);

    public:
        union
        {
            struct
            {
                float x;
                float y;
                float z;
                float w;
            };
            float raw[4];
        };
    };

    // Scales a quaternion by the given value
    inline Quat operator*(float a, const Quat &b)
    {
        return b * a;
    }

    TK_FORCE_INLINE static bool nearEqual(const Quat &a, const Quat &b)
    {
        return Quat::nearEqual(a, b);
    }

    template <>
    struct TIsPODType<Quat>
    {
        enum
        {
            Value = true
        };
    };

}

FORMAT_CUSTOM(toolkit::Quat, "X:%g Y:%g Z:%g W:%g", t.x, t.y, t.z, t.w);
