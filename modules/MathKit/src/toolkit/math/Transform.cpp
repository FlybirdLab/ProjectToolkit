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

    Transform Transform::kIdentity(Vec3(0, 0, 0));

    Transform::Transform(const Vec3 &position)
        : translation(position), orientation(0.0f, 0.0f, 0.0f, 1.0f), scale(1.0f)
    {
    }

    Transform::Transform(const Vec3 &position, const Quat &rotation)
        : translation(position), orientation(rotation), scale(1.0f)
    {
    }

    Transform::Transform(const Vec3 &position, const Quat &rotation, const FVec3 &scale)
        : translation(position), orientation(rotation), scale(scale)
    {
    }

    Transform::Transform(const Vec3 &position, const Mat3x3 &rotationScale) : translation(position)
    {
        rotationScale.decompose(scale, orientation);
    }

    TkString Transform::toString() const
    {
        return TkString::format("%s", *this);
    }

    bool Transform::isIdentity() const
    {
        return translation.isZero() && orientation.isIdentity() && scale.isOne();
    }

    bool Transform::isNanOrInfinity() const
    {
        return translation.isNanOrInfinity() || orientation.isNanOrInfinity() || scale.isNanOrInfinity();
    }

    Mat Transform::getRotation() const
    {
        Mat result{};
        Mat::rotationQuaternion(orientation, result);
        return result;
    }

    void Transform::getRotation(Mat &result) const
    {
        Mat::rotationQuaternion(orientation, result);
    }

    void Transform::setRotation(const Mat &value)
    {
        Quat::rotationMatrix(value, orientation);
    }

    Mat Transform::getWorld() const
    {
        Mat result{};
        Mat::transformation(scale, orientation, translation, result);
        return result;
    }

    void Transform::getWorld(Mat &result) const
    {
        Mat::transformation(scale, orientation, translation, result);
    }

    Transform Transform::add(const Vec3 &translation) const
    {
        Transform result;
        result.orientation = orientation;
        result.scale = scale;
        Vec3::add(this->translation, translation, result.translation);
        return result;
    }

    Transform Transform::add(const Transform &other) const
    {
        Transform result;
        Quat::multiply(orientation, other.orientation, result.orientation);
        result.orientation.normalize();
        FVec3::multiply(scale, other.scale, result.scale);
        Vec3::add(this->translation, other.translation, result.translation);
        return result;
    }

    Transform Transform::subtract(const Transform &other) const
    {
        Transform result;
        Vec3::subtract(this->translation, other.translation, result.translation);
        const Quat invRotation = other.orientation.conjugated();
        Quat::multiply(orientation, invRotation, result.orientation);
        result.orientation.normalize();
        FVec3::divide(scale, other.scale, result.scale);
        return result;
    }

    void Transform::localToWorld(const Transform &other, Transform &result) const
    {
        // Quat::multiply(orientation, other.orientation, result.orientation);
        const float a = orientation.y * other.orientation.z - orientation.z * other.orientation.y;
        const float b = orientation.z * other.orientation.x - orientation.x * other.orientation.z;
        const float c = orientation.x * other.orientation.y - orientation.y * other.orientation.x;
        const float d = orientation.x * other.orientation.x + orientation.y * other.orientation.y + orientation.z * other.orientation.z;
        result.orientation.x = orientation.x * other.orientation.w + other.orientation.x * orientation.w + a;
        result.orientation.y = orientation.y * other.orientation.w + other.orientation.y * orientation.w + b;
        result.orientation.z = orientation.z * other.orientation.w + other.orientation.z * orientation.w + c;
        result.orientation.w = orientation.w * other.orientation.w - d;

        // result.orientation.normalize();
        const float length = result.orientation.length();
        if (length > TkMath::ZeroTolerance)
        {
            const float inv = 1.0f / length;
            result.orientation.x *= inv;
            result.orientation.y *= inv;
            result.orientation.z *= inv;
            result.orientation.w *= inv;
        }

        // Vec3::multiply(scale, other.scale, result.scale);
        result.scale = FVec3(scale.x * other.scale.x, scale.y * other.scale.y, scale.z * other.scale.z);

        // Vec3 tmp; Vec3::multiply(other.Translation, scale, tmp);
        auto tmp = Vec3(other.translation.x * scale.x, other.translation.y * scale.y, other.translation.z * scale.z);

        // Vec3::transform(tmp, orientation, tmp);
        const float x = orientation.x + orientation.x;
        const float y = orientation.y + orientation.y;
        const float z = orientation.z + orientation.z;
        const float wx = orientation.w * x;
        const float wy = orientation.w * y;
        const float wz = orientation.w * z;
        const float xx = orientation.x * x;
        const float xy = orientation.x * y;
        const float xz = orientation.x * z;
        const float yy = orientation.y * y;
        const float yz = orientation.y * z;
        const float zz = orientation.z * z;
        tmp = Vec3(
            tmp.x * (1.0f - yy - zz) + tmp.y * (xy - wz) + tmp.z * (xz + wy),
            tmp.x * (xy + wz) + tmp.y * (1.0f - xx - zz) + tmp.z * (yz - wx),
            tmp.x * (xz - wy) + tmp.y * (yz + wx) + tmp.z * (1.0f - xx - yy));

        // Vec3::add(tmp, translation, result.Translation);
        result.translation = Vec3(tmp.x + translation.x, tmp.y + translation.y, tmp.z + translation.z);
    }

    void Transform::localToWorldVector(const Vec3 &vector, Vec3 &result) const
    {
        Vec3 tmp = vector * scale;
        Vec3::transform(tmp, orientation, result);
    }

    void Transform::localToWorld(const Vec3 &point, Vec3 &result) const
    {
        Vec3 tmp = point * scale;
        Vec3::transform(tmp, orientation, tmp);
        Vec3::add(tmp, translation, result);
    }

    void Transform::worldToLocal(const Transform &other, Transform &result) const
    {
        Vec3 invScale = scale;
        if (invScale.x != 0.0f)
            invScale.x = 1.0f / invScale.x;
        if (invScale.y != 0.0f)
            invScale.y = 1.0f / invScale.y;
        if (invScale.z != 0.0f)
            invScale.z = 1.0f / invScale.z;
        const Quat invRotation = orientation.conjugated();
        Quat::multiply(invRotation, other.orientation, result.orientation);
        result.orientation.normalize();
        FVec3::multiply(other.scale, invScale, result.scale);
        const Vec3 tmp = other.translation - translation;
        Vec3::transform(tmp, invRotation, result.translation);
        Vec3::multiply(result.translation, invScale, result.translation);
    }

    void Transform::worldToLocal(const Vec3 &point, Vec3 &result) const
    {
        Vec3 invScale = scale;
        if (invScale.x != 0.0f)
            invScale.x = 1.0f / invScale.x;
        if (invScale.y != 0.0f)
            invScale.y = 1.0f / invScale.y;
        if (invScale.z != 0.0f)
            invScale.z = 1.0f / invScale.z;
        const Quat invRotation = orientation.conjugated();
        result = point - translation;
        Vec3::transform(result, invRotation, result);
        result *= invScale;
    }

    void Transform::worldToLocalVector(const Vec3 &vector, Vec3 &result) const
    {
        FVec3 invScale = scale;
        if (invScale.x != 0.0f)
            invScale.x = 1.0f / invScale.x;
        if (invScale.y != 0.0f)
            invScale.y = 1.0f / invScale.y;
        if (invScale.z != 0.0f)
            invScale.z = 1.0f / invScale.z;
        const Quat invRotation = orientation.conjugated();
        Vec3::transform(vector, invRotation, result);
        result *= invScale;
    }

    void Transform::worldToLocal(const Quat &rotation, Quat &result) const
    {
        Quat orientation = this->orientation;
        orientation.conjugate();
        Quat::multiply(orientation, rotation, orientation);
        orientation.normalize();
        result = orientation;
    }

    bool Transform::nearEqual(const Transform &a, const Transform &b)
    {
        return Vec3::nearEqual(a.translation, b.translation) && Quat::nearEqual(a.orientation, b.orientation) && FVec3::nearEqual(a.scale, b.scale);
    }

    bool Transform::nearEqual(const Transform &a, const Transform &b, float epsilon)
    {
        return Vec3::nearEqual(a.translation, b.translation, epsilon) && Quat::nearEqual(a.orientation, b.orientation, epsilon) && FVec3::nearEqual(a.scale, b.scale, epsilon);
    }

    FVec3 Transform::getRight() const
    {
        return FVec3::transform(FVec3::kRight, orientation);
    }

    FVec3 Transform::getLeft() const
    {
        return FVec3::transform(FVec3::kLeft, orientation);
    }

    FVec3 Transform::getUp() const
    {
        return FVec3::transform(FVec3::kUp, orientation);
    }

    FVec3 Transform::getDown() const
    {
        return FVec3::transform(FVec3::kDown, orientation);
    }

    FVec3 Transform::getForward() const
    {
        return FVec3::transform(FVec3::kForward, orientation);
    }

    FVec3 Transform::getBackward() const
    {
        return FVec3::transform(FVec3::kBackward, orientation);
    }

    Transform Transform::lerp(const Transform &t1, const Transform &t2, float amount)
    {
        Transform result;
        Vec3::lerp(t1.translation, t2.translation, amount, result.translation);
        Quat::slerp(t1.orientation, t2.orientation, amount, result.orientation);
        FVec3::lerp(t1.scale, t2.scale, amount, result.scale);
        return result;
    }

    void Transform::lerp(const Transform &t1, const Transform &t2, float amount, Transform &result)
    {
        Vec3::lerp(t1.translation, t2.translation, amount, result.translation);
        Quat::slerp(t1.orientation, t2.orientation, amount, result.orientation);
        FVec3::lerp(t1.scale, t2.scale, amount, result.scale);
    }

    Transform Transform::alignRotationToNormalAndSnapToGrid(const Vec3 &point, const Vec3 &normal, const Vec3 &normalOffset, const Transform &relativeTo, const Vec3 &gridSize, const FVec3 &scale)
    {
        Quat rot = Quat::GetRotationFromNormal(normal, relativeTo);
        return Transform(Vec3::snapToGrid(point, gridSize, rot, relativeTo.translation, normalOffset), rot, scale);
    }
}