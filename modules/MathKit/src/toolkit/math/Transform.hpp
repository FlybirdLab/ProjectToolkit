/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "Vector3.hpp"
#include "Quat.hpp"

namespace toolkit
{

    class Mat;

    // Describes transformation in a 3D space.
    class TK_API Transform
    {
    public:
        // An identity transform.
        static Transform kIdentity;

    public:
        Transform() = default;
        Transform(const Vec3 &position);
        Transform(const Vec3 &position, const Quat &rotation);
        Transform(const Vec3 &position, const Quat &rotation, const FVec3 &scale);
        Transform(const Vec3 &position, const Mat3x3 &rotationscale);

    public:
        TkString toString() const;

        // Checks if transform is an identity transformation.
        bool isIdentity() const;
        // Returns true if transform has one or more components equal to +/- infinity or NaN.
        bool isNanOrInfinity() const;

        // Calculates the determinant of this transformation.
        TK_FORCE_INLINE float getDeterminant() const
        {
            return scale.x * scale.y * scale.z;
        }

    public:
        // Gets rotation matrix (from orientation).
        Mat getRotation() const;
        // Gets rotation matrix (from orientation).
        void getRotation(Mat &result) const;
        // Sets rotation matrix (from orientation).
        void setRotation(const Mat &value);
        // Gets world matrix that describes transformation as a 4 by 4 matrix.
        Mat getWorld() const;
        // Gets world matrix that describes transformation as a 4 by 4 matrix.
        void getWorld(Mat &result) const;

    public:
        // Adds translation to this transform.
        Transform add(const Vec3 &translation) const;
        // Adds transformation to this transform.
        Transform add(const Transform &other) const;
        // Subtracts transformation from this transform.
        Transform subtract(const Transform &other) const;
        // Performs transformation of the given transform in local space to the world space of this transform.
        Transform localToWorld(const Transform &other) const
        {
            Transform result;
            localToWorld(other, result);
            return result;
        }
        // Performs transformation of the given transform in local space to the world space of this transform.
        void localToWorld(const Transform &other, Transform &result) const;
        // Performs transformation of the given point in local space to the world space of this transform.
        Vec3 localToWorld(const Vec3 &point) const
        {
            Vec3 result;
            localToWorld(point, result);
            return result;
        }

        // Performs transformation of the given vector in local space to the world space of this transform.
        void localToWorldVector(const Vec3 &vector, Vec3 &result) const;

        // Performs transformation of the given vector in local space to the world space of this transform.
        Vec3 localToWorldVector(const Vec3 &vector) const
        {
            Vec3 result;
            localToWorldVector(vector, result);
            return result;
        }

        // Performs transformation of the given point in local space to the world space of this transform.
        void localToWorld(const Vec3 &point, Vec3 &result) const;

        // Performs transformation of the given transform in local space to the world space of this transform.
        Transform worldToLocal(const Transform &other) const
        {
            Transform result;
            worldToLocal(other, result);
            return result;
        }

        // Performs transformation of the given transform in world space to the local space of this transform.
        void worldToLocal(const Transform &other, Transform &result) const;

        // Performs transformation of the given point in world space to the local space of this transform.
        void worldToLocal(const Vec3 &point, Vec3 &result) const;

        // Performs transformation of the given point in world space to the local space of this transform.
        Vec3 worldToLocal(const Vec3 &point) const
        {
            Vec3 result;
            worldToLocal(point, result);
            return result;
        }

        // Performs transformation of the given vector in world space to the local space of this transform.
        void worldToLocalVector(const Vec3 &vector, Vec3 &result) const;

        // Performs transformation of the given vector in world space to the local space of this transform.
        Vec3 worldToLocalVector(const Vec3 &vector) const
        {
            Vec3 result;
            worldToLocalVector(vector, result);
            return result;
        }

        // Performs transformation of the given rotation in world space to the local space of this transform.
        void worldToLocal(const Quat &rotation, Quat &result) const;

        // Performs transformation of the given rotation in world space to the local space of this transform.
        Quat worldToLocal(const Quat &rotation) const
        {
            Quat result{};
            worldToLocal(rotation, result);
            return result;
        }

        // Combines the functions: <br/>
        static Transform alignRotationToNormalAndSnapToGrid(const Vec3 &point, const Vec3 &normal, const Vec3 &normalOffset, const Transform &relativeTo, const Vec3 &gridSize, const FVec3 &scale = FVec3::kOne);

    public:
        TK_FORCE_INLINE Transform operator*(const Transform &other) const
        {
            return localToWorld(other);
        }

        TK_FORCE_INLINE Transform operator+(const Transform &other) const
        {
            return add(other);
        }

        TK_FORCE_INLINE Transform operator-(const Transform &other) const
        {
            return subtract(other);
        }

        TK_FORCE_INLINE Transform operator+(const Vec3 &other) const
        {
            return add(other);
        }

        TK_FORCE_INLINE bool operator==(const Transform &other) const
        {
            return translation == other.translation && orientation == other.orientation && scale == other.scale;
        }

        TK_FORCE_INLINE bool operator!=(const Transform &other) const
        {
            return translation != other.translation || orientation != other.orientation || scale != other.scale;
        }

    public:
        static bool nearEqual(const Transform &a, const Transform &b);
        static bool nearEqual(const Transform &a, const Transform &b, float epsilon);

    public:
        FVec3 getRight() const;
        FVec3 getLeft() const;
        FVec3 getUp() const;
        FVec3 getDown() const;
        FVec3 getForward() const;
        FVec3 getBackward() const;

        static Transform lerp(const Transform &t1, const Transform &t2, float amount);
        static void lerp(const Transform &t1, const Transform &t2, float amount, Transform &result);

    public:
        // The translation vector of the transform.
        Vec3 translation;
        // The rotation of the transform.
        Quat orientation{};
        // The scale vector of the transform.
        FVec3 scale;
    };

    TK_FORCE_INLINE static bool nearEqual(const Transform &a, const Transform &b)
    {
        return Transform::nearEqual(a, b);
    }

    template <>
    struct TIsPODType<Transform>
    {
        enum
        {
            Value = true
        };
    };
}

namespace toolkit
{
    template <>
    inline psize formatCustom<>(FormatBuffer &buf, const toolkit::Transform &t)
    {
        const psize beginUsed = buf.getUsed();
        Formatter::format(buf, "translation:%s orientation:%s scale:%s", t.translation, t.orientation, t.scale);
        return buf.getUsed() - beginUsed - 1;
    }
}
