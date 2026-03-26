/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Vector3.hpp"

#include "Vector2.hpp"
#include "Vector4.hpp"
#include "Quat.hpp"
#include "Matrix.hpp"
#include "Transform.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/utils/Color.hpp"

namespace toolkit
{

    // Float
    static_assert(sizeof(FVec3) == 12, "Invalid FVec3 type size.");

    template <>
    const FVec3 FVec3::Zero(0.0f);
    template <>
    const FVec3 FVec3::One(1.0f);
    template <>
    const FVec3 FVec3::Half(0.5f);
    template <>
    const FVec3 FVec3::UnitX(1.0f, 0.0f, 0.0f);
    template <>
    const FVec3 FVec3::UnitY(0.0f, 1.0f, 0.0f);
    template <>
    const FVec3 FVec3::UnitZ(0.0f, 0.0f, 1.0f);
    template <>
    const FVec3 FVec3::Up(0.0f, 1.0f, 0.0f);
    template <>
    const FVec3 FVec3::Down(0.0f, -1.0f, 0.0f);
    template <>
    const FVec3 FVec3::Left(-1.0f, 0.0f, 0.0f);
    template <>
    const FVec3 FVec3::Right(1.0f, 0.0f, 0.0f);
    template <>
    const FVec3 FVec3::Forward(0.0f, 0.0f, 1.0f);
    template <>
    const FVec3 FVec3::Backward(0.0f, 0.0f, -1.0f);
    template <>
    const FVec3 FVec3::Minimum(kMinF32);
    template <>
    const FVec3 FVec3::Maximum(kMaxF32);

    template <>
    FVec3::Vector3Base(const FVec2 &xy, float z)
        : x(xy.x), y(xy.y), z(z)
    {
    }

    template <>
    FVec3::Vector3Base(const DVec2 &xy, float z)
        : x(static_cast<float>(xy.x)), y(static_cast<float>(xy.y)), z(0)
    {
    }

    template <>
    FVec3::Vector3Base(const IVec2 &xy, float z)
        : x(static_cast<float>(xy.x)), y(static_cast<float>(xy.y)), z(z)
    {
    }

    template <>
    FVec3::Vector3Base(const IVec3 &xyz)
        : x(static_cast<float>(xyz.x)), y(static_cast<float>(xyz.y)), z(static_cast<float>(xyz.z))
    {
    }

    template <>
    FVec3::Vector3Base(const IVec4 &xyz)
        : x((float)xyz.x), y((float)xyz.y), z((float)xyz.z)
    {
    }

    template <>
    FVec3::Vector3Base(const FVec4 &xyz)
        : x((float)xyz.x), y((float)xyz.y), z((float)xyz.z)
    {
    }

    template <>
    FVec3::Vector3Base(const DVec4 &xyz)
        : x((float)xyz.x), y((float)xyz.y), z((float)xyz.z)
    {
    }

    template <>
    FVec3::Vector3Base(const Color &color)
        : x(color.r), y(color.g), z(color.b)
    {
    }

    template <>
    TkString FVec3::toString() const
    {
        return TkString::format("%s", *this);
    }

    template <>
    void FVec3::hermite(const FVec3 &value1, const FVec3 &tangent1, const FVec3 &value2, const FVec3 &tangent2, float amount, FVec3 &result)
    {
        const float squared = amount * amount;
        const float cubed = amount * squared;
        const float part1 = 2.0f * cubed - 3.0f * squared + 1.0f;
        const float part2 = -2.0f * cubed + 3.0f * squared;
        const float part3 = cubed - 2.0f * squared + amount;
        const float part4 = cubed - squared;
        result.x = value1.x * part1 + value2.x * part2 + tangent1.x * part3 + tangent2.x * part4;
        result.y = value1.y * part1 + value2.y * part2 + tangent1.y * part3 + tangent2.y * part4;
        result.z = value1.z * part1 + value2.z * part2 + tangent1.z * part3 + tangent2.z * part4;
    }

    template <>
    void FVec3::reflect(const FVec3 &vector, const FVec3 &normal, FVec3 &result)
    {
        const Real dot = vector.x * normal.x + vector.y * normal.y + vector.z * normal.z;
        result.x = vector.x - 2.0f * dot * normal.x;
        result.y = vector.y - 2.0f * dot * normal.y;
        result.z = vector.z - 2.0f * dot * normal.z;
    }

    template <>
    void FVec3::transform(const FVec3 &vector, const Quat &rotation, FVec3 &result)
    {
        const float x = rotation.x + rotation.x;
        const float y = rotation.y + rotation.y;
        const float z = rotation.z + rotation.z;
        const float wx = rotation.w * x;
        const float wy = rotation.w * y;
        const float wz = rotation.w * z;
        const float xx = rotation.x * x;
        const float xy = rotation.x * y;
        const float xz = rotation.x * z;
        const float yy = rotation.y * y;
        const float yz = rotation.y * z;
        const float zz = rotation.z * z;
        result = FVec3(
            vector.x * (1.0f - yy - zz) + vector.y * (xy - wz) + vector.z * (xz + wy),
            vector.x * (xy + wz) + vector.y * (1.0f - xx - zz) + vector.z * (yz - wx),
            vector.x * (xz - wy) + vector.y * (yz + wx) + vector.z * (1.0f - xx - yy));
    }

    template <>
    FVec3 FVec3::transform(const FVec3 &vector, const Quat &rotation)
    {
        const float x = rotation.x + rotation.x;
        const float y = rotation.y + rotation.y;
        const float z = rotation.z + rotation.z;
        const float wx = rotation.w * x;
        const float wy = rotation.w * y;
        const float wz = rotation.w * z;
        const float xx = rotation.x * x;
        const float xy = rotation.x * y;
        const float xz = rotation.x * z;
        const float yy = rotation.y * y;
        const float yz = rotation.y * z;
        const float zz = rotation.z * z;
        return {
            vector.x * (1.0f - yy - zz) + vector.y * (xy - wz) + vector.z * (xz + wy),
            vector.x * (xy + wz) + vector.y * (1.0f - xx - zz) + vector.z * (yz - wx),
            vector.x * (xz - wy) + vector.y * (yz + wx) + vector.z * (1.0f - xx - yy)};
    }

    template <>
    void FVec3::transform(const FVec3 &vector, const Mat &transform, FVec4 &result)
    {
        result = FVec4(
            vector.x * transform.m11 + vector.y * transform.m21 + vector.z * transform.m31 + transform.m41,
            vector.x * transform.m12 + vector.y * transform.m22 + vector.z * transform.m32 + transform.m42,
            vector.x * transform.m13 + vector.y * transform.m23 + vector.z * transform.m33 + transform.m43,
            vector.x * transform.m14 + vector.y * transform.m24 + vector.z * transform.m34 + transform.m44);
    }

    template <>
    void FVec3::transform(const FVec3 &vector, const Mat &transform, FVec3 &result)
    {
        result = FVec3(
            vector.x * transform.m11 + vector.y * transform.m21 + vector.z * transform.m31 + transform.m41,
            vector.x * transform.m12 + vector.y * transform.m22 + vector.z * transform.m32 + transform.m42,
            vector.x * transform.m13 + vector.y * transform.m23 + vector.z * transform.m33 + transform.m43);
    }

    template <>
    void FVec3::transform(const FVec3 &vector, const Mat3x3 &transform, FVec3 &result)
    {
        result = FVec3(
            vector.x * transform.m11 + vector.y * transform.m21 + vector.z * transform.m31,
            vector.x * transform.m12 + vector.y * transform.m22 + vector.z * transform.m32,
            vector.x * transform.m13 + vector.y * transform.m23 + vector.z * transform.m33);
    }

    template <>
    void FVec3::transform(const FVec3 &vector, const Transform &transform, FVec3 &result)
    {
#if USE_LARGE_WORLDS
        Vector3 tmp;
        transform.localToWorld(vector, tmp);
        result = tmp;
#else
        transform.localToWorld(vector, result);
#endif
    }

    template <>
    FVec3 FVec3::transform(const FVec3 &vector, const Mat &transform)
    {
        return {
            vector.x * transform.m11 + vector.y * transform.m21 + vector.z * transform.m31 + transform.m41,
            vector.x * transform.m12 + vector.y * transform.m22 + vector.z * transform.m32 + transform.m42,
            vector.x * transform.m13 + vector.y * transform.m23 + vector.z * transform.m33 + transform.m43};
    }

    template <>
    FVec3 FVec3::transform(const FVec3 &vector, const Transform &transform)
    {
        Vec3 result;
        transform.localToWorld(vector, result);
        return result;
    }

    template <>
    void FVec3::transformCoordinate(const FVec3 &coordinate, const Mat &transform, FVec3 &result)
    {
        FVec4 v;
        v.x = coordinate.x * transform.m11 + coordinate.y * transform.m21 + coordinate.z * transform.m31 + transform.m41;
        v.y = coordinate.x * transform.m12 + coordinate.y * transform.m22 + coordinate.z * transform.m32 + transform.m42;
        v.z = coordinate.x * transform.m13 + coordinate.y * transform.m23 + coordinate.z * transform.m33 + transform.m43;
        v.w = 1.0f / (coordinate.x * transform.m14 + coordinate.y * transform.m24 + coordinate.z * transform.m34 + transform.m44);
        result = FVec3(v.x * v.w, v.y * v.w, v.z * v.w);
    }

    template <>
    void FVec3::transformNormal(const FVec3 &normal, const Mat &transform, FVec3 &result)
    {
        result = FVec3(
            normal.x * transform.m11 + normal.y * transform.m21 + normal.z * transform.m31,
            normal.x * transform.m12 + normal.y * transform.m22 + normal.z * transform.m32,
            normal.x * transform.m13 + normal.y * transform.m23 + normal.z * transform.m33);
    }

    template <>
    FVec3 FVec3::project(const FVec3 &vector, const FVec3 &onNormal)
    {
        const float sqrMag = dot(onNormal, onNormal);
        if (sqrMag < TkMath::ZeroTolerance)
            return Zero;
        return onNormal * dot(vector, onNormal) / sqrMag;
    }

    template <>
    void FVec3::project(const FVec3 &vector, float x, float y, float width, float height, float minZ, float maxZ, const Mat &worldViewProjection, FVec3 &result)
    {
        FVec3 v;
        transformCoordinate(vector, worldViewProjection, v);
        result = FVec3((1.0f + v.x) * 0.5f * width + x, (1.0f - v.y) * 0.5f * height + y, v.z * (maxZ - minZ) + minZ);
    }

    template <>
    void FVec3::unproject(const FVec3 &vector, float x, float y, float width, float height, float minZ, float maxZ, const Mat &worldViewProjection, FVec3 &result)
    {
        Mat matrix{};
        Mat::invert(worldViewProjection, matrix);
        const FVec3 v((vector.x - x) / width * 2.0f - 1.0f, -((vector.y - y) / height * 2.0f - 1.0f), (vector.z - minZ) / (maxZ - minZ));
        transformCoordinate(v, matrix, result);
    }

    template <>
    void FVec3::createOrthonormalBasis(FVec3 &xAxis, FVec3 &yAxis, FVec3 &zAxis)
    {
        xAxis -= (xAxis | zAxis) / (zAxis | zAxis) * zAxis;
        yAxis -= (yAxis | zAxis) / (zAxis | zAxis) * zAxis;
        if (xAxis.lengthSquared() < TkMath::ZeroTolerance)
            xAxis = yAxis ^ zAxis;
        if (yAxis.lengthSquared() < TkMath::ZeroTolerance)
            yAxis = xAxis ^ zAxis;
        xAxis.normalize();
        yAxis.normalize();
        zAxis.normalize();
    }

    template <>
    void FVec3::findBestAxisVectors(FVec3 &firstAxis, FVec3 &secondAxis) const
    {
        const float absX = TkMath::abs(x);
        const float absY = TkMath::abs(y);
        const float absZ = TkMath::abs(z);
        if (absZ > absX && absZ > absY)
            firstAxis = FVec3(1, 0, 0);
        else
            firstAxis = FVec3(0, 0, 1);
        firstAxis = (firstAxis - *this * (firstAxis | *this)).getNormalized();
        secondAxis = firstAxis ^ *this;
    }

    template <>
    float FVec3::triangleArea(const FVec3 &v0, const FVec3 &v1, const FVec3 &v2)
    {
        return ((v2 - v0) ^ (v1 - v0)).length() * 0.5f;
    }

    template <>
    float FVec3::angle(const FVec3 &from, const FVec3 &to)
    {
        const float dot_ = TkMath::clamp(dot(normalize(from), normalize(to)), -1.0f, 1.0f);
        if (TkMath::abs(dot_) > 1.0f - TkMath::ZeroTolerance)
            return dot_ > 0.0f ? 0.0f : TkMath::PI;
        return TkMath::acos(dot_);
    }

    template <>
    FVec3 FVec3::snapToGrid(const FVec3 &pos, const FVec3 &gridSize)
    {
        return {TkMath::ceil((pos.x - (gridSize.x * 0.5f)) / gridSize.x) * gridSize.x,
                TkMath::ceil((pos.y - (gridSize.y * 0.5f)) / gridSize.y) * gridSize.y,
                TkMath::ceil((pos.z - (gridSize.z * 0.5f)) / gridSize.z) * gridSize.z};
    }

    template <>
    FVec3 FVec3::snapToGrid(const FVec3 &point, const FVec3 &gridSize, const Quat &gridOrientation, const FVec3 &gridOrigin, const FVec3 &offset)
    {
        return (gridOrientation * (gridOrientation.conjugated() * snapToGrid(point - gridOrigin, gridSize) + offset)) + gridOrigin;
    }

    // Double
    static_assert(sizeof(DVec3) == 24, "Invalid DVec3 type size.");

    template <>
    const DVec3 DVec3::Zero(0.0);
    template <>
    const DVec3 DVec3::One(1.0);
    template <>
    const DVec3 DVec3::Half(0.5);
    template <>
    const DVec3 DVec3::UnitX(1.0, 0.0, 0.0);
    template <>
    const DVec3 DVec3::UnitY(0.0, 1.0, 0.0);
    template <>
    const DVec3 DVec3::UnitZ(0.0, 0.0, 1.0);
    template <>
    const DVec3 DVec3::Up(0.0, 1.0, 0.0);
    template <>
    const DVec3 DVec3::Down(0.0, -1.0, 0.0);
    template <>
    const DVec3 DVec3::Left(-1.0, 0.0, 0.0);
    template <>
    const DVec3 DVec3::Right(1.0, 0.0, 0.0);
    template <>
    const DVec3 DVec3::Forward(0.0, 0.0, 1.0);
    template <>
    const DVec3 DVec3::Backward(0.0, 0.0, -1.0);
    template <>
    const DVec3 DVec3::Minimum(kMaxF64);
    template <>
    const DVec3 DVec3::Maximum(kMinF64);

    template <>
    DVec3::Vector3Base(const FVec2 &xy, double z)
        : x((double)xy.x), y((double)xy.y), z(z)
    {
    }

    template <>
    DVec3::Vector3Base(const DVec2 &xy, double z)
        : x(xy.x), y(xy.y), z(0)
    {
    }

    template <>
    DVec3::Vector3Base(const IVec2 &xy, double z)
        : x((double)xy.x), y((double)xy.y), z(z)
    {
    }

    template <>
    DVec3::Vector3Base(const IVec3 &xyz)
        : x((double)xyz.x), y((double)xyz.y), z((double)xyz.z)
    {
    }

    template <>
    DVec3::Vector3Base(const IVec4 &xyz)
        : x((double)xyz.x), y((double)xyz.y), z((double)xyz.z)
    {
    }

    template <>
    DVec3::Vector3Base(const FVec4 &xyz)
        : x((double)xyz.x), y((double)xyz.y), z((double)xyz.z)
    {
    }

    template <>
    DVec3::Vector3Base(const DVec4 &xyz)
        : x(xyz.x), y(xyz.y), z(xyz.z)
    {
    }

    template <>
    DVec3::Vector3Base(const Color &color)
        : x((double)color.r), y((double)color.g), z((double)color.b)
    {
    }

    template <>
    TkString DVec3::toString() const
    {
        return TkString::format("%s", *this);
    }

    template <>
    void DVec3::hermite(const DVec3 &value1, const DVec3 &tangent1, const DVec3 &value2, const DVec3 &tangent2, double amount, DVec3 &result)
    {
        const double squared = amount * amount;
        const double cubed = amount * squared;
        const double part1 = 2.0 * cubed - 3.0 * squared + 1.0;
        const double part2 = -2.0 * cubed + 3.0 * squared;
        const double part3 = cubed - 2.0 * squared + amount;
        const double part4 = cubed - squared;
        result.x = value1.x * part1 + value2.x * part2 + tangent1.x * part3 + tangent2.x * part4;
        result.y = value1.y * part1 + value2.y * part2 + tangent1.y * part3 + tangent2.y * part4;
        result.z = value1.z * part1 + value2.z * part2 + tangent1.z * part3 + tangent2.z * part4;
    }

    template <>
    void DVec3::reflect(const DVec3 &vector, const DVec3 &normal, DVec3 &result)
    {
        const double dot = vector.x * normal.x + vector.y * normal.y + vector.z * normal.z;
        result.x = vector.x - 2.0 * dot * normal.x;
        result.y = vector.y - 2.0 * dot * normal.y;
        result.z = vector.z - 2.0 * dot * normal.z;
    }

    template <>
    void DVec3::transform(const DVec3 &vector, const Quat &rotation, DVec3 &result)
    {
        const float x = rotation.x + rotation.x;
        const float y = rotation.y + rotation.y;
        const float z = rotation.z + rotation.z;
        const float wx = rotation.w * x;
        const float wy = rotation.w * y;
        const float wz = rotation.w * z;
        const float xx = rotation.x * x;
        const float xy = rotation.x * y;
        const float xz = rotation.x * z;
        const float yy = rotation.y * y;
        const float yz = rotation.y * z;
        const float zz = rotation.z * z;
        result = DVec3(
            vector.x * (1.0f - yy - zz) + vector.y * (xy - wz) + vector.z * (xz + wy),
            vector.x * (xy + wz) + vector.y * (1.0f - xx - zz) + vector.z * (yz - wx),
            vector.x * (xz - wy) + vector.y * (yz + wx) + vector.z * (1.0f - xx - yy));
    }

    template <>
    DVec3 DVec3::transform(const DVec3 &vector, const Quat &rotation)
    {
        const float x = rotation.x + rotation.x;
        const float y = rotation.y + rotation.y;
        const float z = rotation.z + rotation.z;
        const float wx = rotation.w * x;
        const float wy = rotation.w * y;
        const float wz = rotation.w * z;
        const float xx = rotation.x * x;
        const float xy = rotation.x * y;
        const float xz = rotation.x * z;
        const float yy = rotation.y * y;
        const float yz = rotation.y * z;
        const float zz = rotation.z * z;
        return {
            vector.x * static_cast<double>(1.0f - yy - zz) + vector.y * static_cast<double>(xy - wz) + vector.z * static_cast<double>(xz + wy),
            vector.x * static_cast<double>(xy + wz) + vector.y * static_cast<double>(1.0f - xx - zz) + vector.z * static_cast<double>(yz - wx),
            vector.x * static_cast<double>(xz - wy) + vector.y * static_cast<double>(yz + wx) + vector.z * static_cast<double>(1.0f - xx - yy)};
    }

    template <>
    void DVec3::transform(const DVec3 &vector, const Mat &transform, DVec4 &result)
    {
        result = DVec4(
            vector.x * transform.m11 + vector.y * transform.m21 + vector.z * transform.m31 + transform.m41,
            vector.x * transform.m12 + vector.y * transform.m22 + vector.z * transform.m32 + transform.m42,
            vector.x * transform.m13 + vector.y * transform.m23 + vector.z * transform.m33 + transform.m43,
            vector.x * transform.m14 + vector.y * transform.m24 + vector.z * transform.m34 + transform.m44);
    }

    template <>
    void DVec3::transform(const DVec3 &vector, const Mat &transform, DVec3 &result)
    {
        result = DVec3(
            vector.x * transform.m11 + vector.y * transform.m21 + vector.z * transform.m31 + transform.m41,
            vector.x * transform.m12 + vector.y * transform.m22 + vector.z * transform.m32 + transform.m42,
            vector.x * transform.m13 + vector.y * transform.m23 + vector.z * transform.m33 + transform.m43);
    }

    template <>
    void DVec3::transform(const DVec3 &vector, const Mat3x3 &transform, DVec3 &result)
    {
        result = DVec3(
            vector.x * transform.m11 + vector.y * transform.m21 + vector.z * transform.m31,
            vector.x * transform.m12 + vector.y * transform.m22 + vector.z * transform.m32,
            vector.x * transform.m13 + vector.y * transform.m23 + vector.z * transform.m33);
    }

    template <>
    void DVec3::transform(const DVec3 &vector, const Transform &transform, DVec3 &result)
    {
#if USE_LARGE_WORLDS
        transform.localToWorld(vector, result);
#else
        Vec3 tmp;
        transform.localToWorld(vector, tmp);
        result = tmp;
#endif
    }

    template <>
    DVec3 DVec3::transform(const DVec3 &vector, const Mat &transform)
    {
        return {
            vector.x * transform.m11 + vector.y * transform.m21 + vector.z * transform.m31 + transform.m41,
            vector.x * transform.m12 + vector.y * transform.m22 + vector.z * transform.m32 + transform.m42,
            vector.x * transform.m13 + vector.y * transform.m23 + vector.z * transform.m33 + transform.m43};
    }

    template <>
    DVec3 DVec3::transform(const DVec3 &vector, const Transform &transform)
    {
        Vec3 result;
        transform.localToWorld(vector, result);
        return result;
    }

    template <>
    void DVec3::transformCoordinate(const DVec3 &coordinate, const Mat &transform, DVec3 &result)
    {
        DVec4 v;
        v.x = coordinate.x * transform.m11 + coordinate.y * transform.m21 + coordinate.z * transform.m31 + transform.m41;
        v.y = coordinate.x * transform.m12 + coordinate.y * transform.m22 + coordinate.z * transform.m32 + transform.m42;
        v.z = coordinate.x * transform.m13 + coordinate.y * transform.m23 + coordinate.z * transform.m33 + transform.m43;
        v.w = 1.0 / (coordinate.x * transform.m14 + coordinate.y * transform.m24 + coordinate.z * transform.m34 + transform.m44);
        result = DVec3(v.x * v.w, v.y * v.w, v.z * v.w);
    }

    template <>
    void DVec3::transformNormal(const DVec3 &normal, const Mat &transform, DVec3 &result)
    {
        result = DVec3(
            normal.x * transform.m11 + normal.y * transform.m21 + normal.z * transform.m31,
            normal.x * transform.m12 + normal.y * transform.m22 + normal.z * transform.m32,
            normal.x * transform.m13 + normal.y * transform.m23 + normal.z * transform.m33);
    }

    template <>
    DVec3 DVec3::project(const DVec3 &vector, const DVec3 &onNormal)
    {
        const Real sqrMag = dot(onNormal, onNormal);
        if (sqrMag < TkMath::ZeroTolerance)
            return Zero;
        return onNormal * dot(vector, onNormal) / sqrMag;
    }

    template <>
    void DVec3::project(const DVec3 &vector, float x, float y, float width, float height, float minZ, float maxZ, const Mat &worldViewProjection, DVec3 &result)
    {
        DVec3 v;
        transformCoordinate(vector, worldViewProjection, v);
        result = DVec3((1.0f + v.x) * 0.5f * width + x, (1.0f - v.y) * 0.5f * height + y, v.z * (maxZ - minZ) + minZ);
    }

    template <>
    void DVec3::unproject(const DVec3 &vector, float x, float y, float width, float height, float minZ, float maxZ, const Mat &worldViewProjection, DVec3 &result)
    {
        Mat matrix{};
        Mat::invert(worldViewProjection, matrix);
        const DVec3 v((vector.x - x) / width * 2.0f - 1.0f, -((vector.y - y) / height * 2.0f - 1.0f), (vector.z - minZ) / (maxZ - minZ));
        transformCoordinate(v, matrix, result);
    }

    template <>
    void DVec3::createOrthonormalBasis(DVec3 &xAxis, DVec3 &yAxis, DVec3 &zAxis)
    {
        xAxis -= (xAxis | zAxis) / (zAxis | zAxis) * zAxis;
        yAxis -= (yAxis | zAxis) / (zAxis | zAxis) * zAxis;
        if (xAxis.lengthSquared() < TkMath::ZeroTolerance)
            xAxis = yAxis ^ zAxis;
        if (yAxis.lengthSquared() < TkMath::ZeroTolerance)
            yAxis = xAxis ^ zAxis;
        xAxis.normalize();
        yAxis.normalize();
        zAxis.normalize();
    }

    template <>
    void DVec3::findBestAxisVectors(DVec3 &firstAxis, DVec3 &secondAxis) const
    {
        const double absX = TkMathD::abs(x);
        const double absY = TkMathD::abs(y);
        const double absZ = TkMathD::abs(z);
        if (absZ > absX && absZ > absY)
            firstAxis = DVec3(1, 0, 0);
        else
            firstAxis = DVec3(0, 0, 1);
        firstAxis = (firstAxis - *this * (firstAxis | *this)).getNormalized();
        secondAxis = firstAxis ^ *this;
    }

    template <>
    double DVec3::triangleArea(const DVec3 &v0, const DVec3 &v1, const DVec3 &v2)
    {
        return ((v2 - v0) ^ (v1 - v0)).length() * 0.5;
    }

    template <>
    double DVec3::angle(const DVec3 &from, const DVec3 &to)
    {
        const double dot_ = TkMath::clamp(dot(normalize(from), normalize(to)), -1.0, 1.0);
        if (TkMathD::abs(dot_) > 1.0 - TkMath::ZeroTolerance)
            return dot_ > 0.0 ? 0.0 : TkMath::PI;
        return TkMath::acos(dot_);
    }

    template <>
    DVec3 DVec3::snapToGrid(const DVec3 &pos, const DVec3 &gridSize)
    {
        return {TkMath::ceil((pos.x - (gridSize.x * 0.5)) / gridSize.x) * gridSize.x,
                TkMath::ceil((pos.y - (gridSize.y * 0.5)) / gridSize.y) * gridSize.y,
                TkMath::ceil((pos.z - (gridSize.z * 0.5)) / gridSize.z) * gridSize.z};
    }

    template <>
    DVec3 DVec3::snapToGrid(const DVec3 &point, const DVec3 &gridSize, const Quat &gridOrientation, const DVec3 &gridOrigin, const DVec3 &offset)
    {
        return gridOrientation * (gridOrientation.conjugated() * snapToGrid(point - gridOrigin, gridSize) + offset) + gridOrigin;
    }

    // Int
    static_assert(sizeof(IVec3) == 12, "Invalid IVec3 type size.");

    template <>
    const IVec3 IVec3::Zero(0);
    template <>
    const IVec3 IVec3::One(1);
    template <>
    const IVec3 IVec3::Half(1);
    template <>
    const IVec3 IVec3::UnitX(1, 0, 0);
    template <>
    const IVec3 IVec3::UnitY(0, 1, 0);
    template <>
    const IVec3 IVec3::UnitZ(0, 0, 1);
    template <>
    const IVec3 IVec3::Up(0, 1, 0);
    template <>
    const IVec3 IVec3::Down(0, -1, 0);
    template <>
    const IVec3 IVec3::Left(-1, 0, 0);
    template <>
    const IVec3 IVec3::Right(1, 0, 0);
    template <>
    const IVec3 IVec3::Forward(0, 0, 1);
    template <>
    const IVec3 IVec3::Backward(0, 0, -1);
    template <>
    const IVec3 IVec3::Minimum(kMinI32);
    template <>
    const IVec3 IVec3::Maximum(kMaxI32);

    template <>
    IVec3::Vector3Base(const FVec2 &xy, i32 z)
        : x((i32)xy.x), y((i32)xy.y), z(z)
    {
    }

    template <>
    IVec3::Vector3Base(const DVec2 &xy, i32 z)
        : x((i32)xy.x), y((i32)xy.y), z(0)
    {
    }

    template <>
    IVec3::Vector3Base(const IVec2 &xy, i32 z)
        : x(xy.x), y(xy.y), z(z)
    {
    }

    template <>
    IVec3::Vector3Base(const IVec3 &xyz)
        : x(xyz.x), y(xyz.y), z(xyz.z)
    {
    }

    template <>
    IVec3::Vector3Base(const IVec4 &xyz)
        : x(xyz.x), y(xyz.y), z(xyz.z)
    {
    }

    template <>
    IVec3::Vector3Base(const FVec4 &xyz)
        : x((i32)xyz.x), y((i32)xyz.y), z((i32)xyz.z)
    {
    }

    template <>
    IVec3::Vector3Base(const DVec4 &xyz)
        : x((i32)xyz.x), y((i32)xyz.y), z((i32)xyz.z)
    {
    }

    template <>
    IVec3::Vector3Base(const Color &color)
        : x((i32)color.r), y((i32)color.g), z((i32)color.b)
    {
    }

    template <>
    TkString IVec3::toString() const
    {
        return TkString::format("%s", *this);
    }

    template <>
    void IVec3::hermite(const IVec3 &value1, const IVec3 &tangent1, const IVec3 &value2, const IVec3 &tangent2, i32 amount, IVec3 &result)
    {
        result = value1;
    }

    template <>
    void IVec3::reflect(const IVec3 &vector, const IVec3 &normal, IVec3 &result)
    {
        result = vector;
    }

    template <>
    void IVec3::transform(const IVec3 &vector, const Quat &rotation, IVec3 &result)
    {
        result = vector;
    }

    template <>
    IVec3 IVec3::transform(const IVec3 &vector, const Quat &rotation)
    {
        return vector;
    }

    template <>
    void IVec3::transform(const IVec3 &vector, const Mat &transform, IVec4 &result)
    {
        result = IVec4(vector);
    }

    template <>
    void IVec3::transform(const IVec3 &vector, const Mat &transform, IVec3 &result)
    {
        result = vector;
    }

    template <>
    void IVec3::transform(const IVec3 &vector, const Mat3x3 &transform, IVec3 &result)
    {
        result = vector;
    }

    template <>
    void IVec3::transform(const IVec3 &vector, const Transform &transform, IVec3 &result)
    {
        result = vector;
    }

    template <>
    IVec3 IVec3::transform(const IVec3 &vector, const Mat &transform)
    {
        return vector;
    }

    template <>
    IVec3 IVec3::transform(const IVec3 &vector, const Transform &transform)
    {
        return vector;
    }

    template <>
    void IVec3::transformCoordinate(const IVec3 &coordinate, const Mat &transform, IVec3 &result)
    {
        result = coordinate;
    }

    template <>
    void IVec3::transformNormal(const IVec3 &normal, const Mat &transform, IVec3 &result)
    {
        result = normal;
    }

    template <>
    IVec3 IVec3::project(const IVec3 &vector, const IVec3 &onNormal)
    {
        return Zero;
    }

    template <>
    void IVec3::project(const IVec3 &vector, float x, float y, float width, float height, float minZ, float maxZ, const Mat &worldViewProjection, IVec3 &result)
    {
        result = vector;
    }

    template <>
    void IVec3::unproject(const IVec3 &vector, float x, float y, float width, float height, float minZ, float maxZ, const Mat &worldViewProjection, IVec3 &result)
    {
        result = vector;
    }

    template <>
    void IVec3::createOrthonormalBasis(IVec3 &xAxis, IVec3 &yAxis, IVec3 &zAxis)
    {
    }

    template <>
    void IVec3::findBestAxisVectors(IVec3 &firstAxis, IVec3 &secondAxis) const
    {
    }

    template <>
    i32 IVec3::triangleArea(const IVec3 &v0, const IVec3 &v1, const IVec3 &v2)
    {
        return 0;
    }

    template <>
    i32 IVec3::angle(const IVec3 &from, const IVec3 &to)
    {
        return 0;
    }

    template <>
    IVec3 IVec3::snapToGrid(const IVec3 &pos, const IVec3 &gridSize)
    {
        return {(pos.x - gridSize.x / 2) / gridSize.x * gridSize.x,
                (pos.y - gridSize.y / 2) / gridSize.y * gridSize.y,
                (pos.z - gridSize.z / 2) / gridSize.z * gridSize.z};
    }

    template <>
    IVec3 IVec3::snapToGrid(const IVec3 &point, const IVec3 &gridSize, const Quat &gridOrientation, const IVec3 &gridOrigin, const IVec3 &offset)
    {
        return gridOrientation * (gridOrientation.conjugated() * snapToGrid(point - gridOrigin, gridSize) + offset) + gridOrigin;
    }

}