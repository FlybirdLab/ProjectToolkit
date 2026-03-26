/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "Transform.hpp"

#include "Matrix.hpp"
#include "toolkit/types/String.hpp"

namespace toolkit
{

    Transform Transform::Identity(Vec3(0, 0, 0));
    Transform::Transform(const Vec3 &position, const Mat3x3 &rotationScale)
        : Translation(position)
    {
        rotationScale.decompose(Scale, Orientation);
    }

    TkString Transform::toString() const
    {
        return TkString::format("%s", *this);
    }

    bool Transform::isIdentity() const
    {
        return Translation.isZero() && Orientation.isIdentity() && Scale.isOne();
    }

    bool Transform::isNanOrInfinity() const
    {
        return Translation.isNanOrInfinity() || Orientation.isNanOrInfinity() || Scale.isNanOrInfinity();
    }

    Mat Transform::getRotation() const
    {
        Mat result{};
        Mat::rotationQuaternion(Orientation, result);
        return result;
    }

    void Transform::getRotation(Mat &result) const
    {
        Mat::rotationQuaternion(Orientation, result);
    }

    void Transform::setRotation(const Mat &value)
    {
        Quat::rotationMatrix(value, Orientation);
    }

    Mat Transform::getWorld() const
    {
        Mat result{};
        Mat::transformation(Scale, Orientation, Translation, result);
        return result;
    }

    void Transform::getWorld(Mat &result) const
    {
        Mat::transformation(Scale, Orientation, Translation, result);
    }

    Transform Transform::add(const Vec3 &translation) const
    {
        Transform result;
        result.Orientation = Orientation;
        result.Scale = Scale;
        Vec3::add(Translation, translation, result.Translation);
        return result;
    }

    Transform Transform::add(const Transform &other) const
    {
        Transform result;
        Quat::multiply(Orientation, other.Orientation, result.Orientation);
        result.Orientation.normalize();
        FVec3::multiply(Scale, other.Scale, result.Scale);
        Vec3::add(Translation, other.Translation, result.Translation);
        return result;
    }

    Transform Transform::subtract(const Transform &other) const
    {
        Transform result;
        Vec3::subtract(Translation, other.Translation, result.Translation);
        const Quat invRotation = other.Orientation.conjugated();
        Quat::multiply(Orientation, invRotation, result.Orientation);
        result.Orientation.normalize();
        FVec3::divide(Scale, other.Scale, result.Scale);
        return result;
    }

    void Transform::localToWorld(const Transform &other, Transform &result) const
    {
        // Quat::multiply(Orientation, other.Orientation, result.Orientation);
        const float a = Orientation.y * other.Orientation.z - Orientation.z * other.Orientation.y;
        const float b = Orientation.z * other.Orientation.x - Orientation.x * other.Orientation.z;
        const float c = Orientation.x * other.Orientation.y - Orientation.y * other.Orientation.x;
        const float d = Orientation.x * other.Orientation.x + Orientation.y * other.Orientation.y + Orientation.z * other.Orientation.z;
        result.Orientation.x = Orientation.x * other.Orientation.w + other.Orientation.x * Orientation.w + a;
        result.Orientation.y = Orientation.y * other.Orientation.w + other.Orientation.y * Orientation.w + b;
        result.Orientation.z = Orientation.z * other.Orientation.w + other.Orientation.z * Orientation.w + c;
        result.Orientation.w = Orientation.w * other.Orientation.w - d;

        // result.Orientation.normalize();
        const float length = result.Orientation.length();
        if (length > TkMath::ZeroTolerance)
        {
            const float inv = 1.0f / length;
            result.Orientation.x *= inv;
            result.Orientation.y *= inv;
            result.Orientation.z *= inv;
            result.Orientation.w *= inv;
        }

        // Vec3::multiply(Scale, other.Scale, result.Scale);
        result.Scale = FVec3(Scale.x * other.Scale.x, Scale.y * other.Scale.y, Scale.z * other.Scale.z);

        // Vec3 tmp; Vec3::multiply(other.Translation, Scale, tmp);
        auto tmp = Vec3(other.Translation.x * Scale.x, other.Translation.y * Scale.y, other.Translation.z * Scale.z);

        // Vec3::transform(tmp, Orientation, tmp);
        const float x = Orientation.x + Orientation.x;
        const float y = Orientation.y + Orientation.y;
        const float z = Orientation.z + Orientation.z;
        const float wx = Orientation.w * x;
        const float wy = Orientation.w * y;
        const float wz = Orientation.w * z;
        const float xx = Orientation.x * x;
        const float xy = Orientation.x * y;
        const float xz = Orientation.x * z;
        const float yy = Orientation.y * y;
        const float yz = Orientation.y * z;
        const float zz = Orientation.z * z;
        tmp = Vec3(
            tmp.x * (1.0f - yy - zz) + tmp.y * (xy - wz) + tmp.z * (xz + wy),
            tmp.x * (xy + wz) + tmp.y * (1.0f - xx - zz) + tmp.z * (yz - wx),
            tmp.x * (xz - wy) + tmp.y * (yz + wx) + tmp.z * (1.0f - xx - yy));

        // Vec3::add(tmp, Translation, result.Translation);
        result.Translation = Vec3(tmp.x + Translation.x, tmp.y + Translation.y, tmp.z + Translation.z);
    }

    void Transform::localToWorldVector(const Vec3 &vector, Vec3 &result) const
    {
        Vec3 tmp = vector * Scale;
        Vec3::transform(tmp, Orientation, result);
    }

    void Transform::localToWorld(const Vec3 &point, Vec3 &result) const
    {
        Vec3 tmp = point * Scale;
        Vec3::transform(tmp, Orientation, tmp);
        Vec3::add(tmp, Translation, result);
    }

    void Transform::worldToLocal(const Transform &other, Transform &result) const
    {
        Vec3 invScale = Scale;
        if (invScale.x != 0.0f)
            invScale.x = 1.0f / invScale.x;
        if (invScale.y != 0.0f)
            invScale.y = 1.0f / invScale.y;
        if (invScale.z != 0.0f)
            invScale.z = 1.0f / invScale.z;
        const Quat invRotation = Orientation.conjugated();
        Quat::multiply(invRotation, other.Orientation, result.Orientation);
        result.Orientation.normalize();
        FVec3::multiply(other.Scale, invScale, result.Scale);
        const Vec3 tmp = other.Translation - Translation;
        Vec3::transform(tmp, invRotation, result.Translation);
        Vec3::multiply(result.Translation, invScale, result.Translation);
    }

    void Transform::worldToLocal(const Vec3 &point, Vec3 &result) const
    {
        Vec3 invScale = Scale;
        if (invScale.x != 0.0f)
            invScale.x = 1.0f / invScale.x;
        if (invScale.y != 0.0f)
            invScale.y = 1.0f / invScale.y;
        if (invScale.z != 0.0f)
            invScale.z = 1.0f / invScale.z;
        const Quat invRotation = Orientation.conjugated();
        result = point - Translation;
        Vec3::transform(result, invRotation, result);
        result *= invScale;
    }

    void Transform::worldToLocalVector(const Vec3 &vector, Vec3 &result) const
    {
        FVec3 invScale = Scale;
        if (invScale.x != 0.0f)
            invScale.x = 1.0f / invScale.x;
        if (invScale.y != 0.0f)
            invScale.y = 1.0f / invScale.y;
        if (invScale.z != 0.0f)
            invScale.z = 1.0f / invScale.z;
        const Quat invRotation = Orientation.conjugated();
        Vec3::transform(vector, invRotation, result);
        result *= invScale;
    }

    void Transform::worldToLocal(const Quat &rotation, Quat &result) const
    {
        Quat orientation = Orientation;
        orientation.conjugate();
        Quat::multiply(orientation, rotation, orientation);
        orientation.normalize();
        result = orientation;
    }

    FVec3 Transform::getRight() const
    {
        return FVec3::transform(FVec3::Right, Orientation);
    }

    FVec3 Transform::getLeft() const
    {
        return FVec3::transform(FVec3::Left, Orientation);
    }

    FVec3 Transform::getUp() const
    {
        return FVec3::transform(FVec3::Up, Orientation);
    }

    FVec3 Transform::getDown() const
    {
        return FVec3::transform(FVec3::Down, Orientation);
    }

    FVec3 Transform::getForward() const
    {
        return FVec3::transform(FVec3::Forward, Orientation);
    }

    FVec3 Transform::getBackward() const
    {
        return FVec3::transform(FVec3::Backward, Orientation);
    }

    Transform Transform::lerp(const Transform &t1, const Transform &t2, float amount)
    {
        Transform result;
        Vec3::lerp(t1.Translation, t2.Translation, amount, result.Translation);
        Quat::slerp(t1.Orientation, t2.Orientation, amount, result.Orientation);
        FVec3::lerp(t1.Scale, t2.Scale, amount, result.Scale);
        return result;
    }

    void Transform::lerp(const Transform &t1, const Transform &t2, float amount, Transform &result)
    {
        Vec3::lerp(t1.Translation, t2.Translation, amount, result.Translation);
        Quat::slerp(t1.Orientation, t2.Orientation, amount, result.Orientation);
        FVec3::lerp(t1.Scale, t2.Scale, amount, result.Scale);
    }

    Transform Transform::alignRotationToNormalAndSnapToGrid(const Vec3 &point, const Vec3 &normal, const Vec3 &normalOffset, const Transform &relativeTo, const Vec3 &gridSize, const FVec3 &scale)
    {
        Quat rot = Quat::GetRotationFromNormal(normal, relativeTo);
        return Transform(Vec3::snapToGrid(point, gridSize, rot, relativeTo.Translation, normalOffset), rot, scale);
    }
}