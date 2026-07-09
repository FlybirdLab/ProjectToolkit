/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Plane.hpp"

#include "toolkit/math/Vector3.hpp"
#include "toolkit/math/Vector4.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/math/Quat.hpp"
#include "toolkit/math/Matrix.hpp"

namespace toolkit
{

    Plane::Plane(const Vec3 &point1, const Vec3 &point2, const Vec3 &point3)
    {
        Vec3 cross;

        const Vec3 t1 = point2 - point1;
        const Vec3 t2 = point3 - point1;

        Vec3::cross(t1, t2, cross);
        const real invPyth = cross.invertedLength();

        normal = cross * invPyth;
        d = -(normal.x * point1.x + normal.y * point1.y + normal.z * point1.z);
    }

    TkString Plane::toString() const
    {
        return TkString::format("%s", *this);
    }

    void Plane::normalize()
    {
        const real length = normal.length();
        if (!TkMath::isZero(length))
        {
            const real rcp = 1.0f / length;
            normal *= rcp;
            d *= rcp;
        }
    }

    Vec3 Plane::intersection(const Plane &inPlane1, const Plane &inPlane2, const Plane &inPlane3)
    {
        // TODO: convet into cros products, dot products etc. ???

        const real bc1 = inPlane1.normal.y * inPlane3.normal.z - inPlane3.normal.y * inPlane1.normal.z;
        const real bc2 = inPlane2.normal.y * inPlane1.normal.z - inPlane1.normal.y * inPlane2.normal.z;
        const real bc3 = inPlane3.normal.y * inPlane2.normal.z - inPlane2.normal.y * inPlane3.normal.z;

        const real ad1 = inPlane1.normal.x * inPlane3.d - inPlane3.normal.x * inPlane1.d;
        const real ad2 = inPlane2.normal.x * inPlane1.d - inPlane1.normal.x * inPlane2.d;
        const real ad3 = inPlane3.normal.x * inPlane2.d - inPlane2.normal.x * inPlane3.d;

        const real x = -(inPlane1.d * bc3 + inPlane2.d * bc1 + inPlane3.d * bc2);
        const real y = -(inPlane1.normal.z * ad3 + inPlane2.normal.z * ad1 + inPlane3.normal.z * ad2);
        const real z = +(inPlane1.normal.y * ad3 + inPlane2.normal.y * ad1 + inPlane3.normal.y * ad2);
        const real w = -(inPlane1.normal.x * bc3 + inPlane2.normal.x * bc1 + inPlane3.normal.x * bc2);

        // better to have detectable invalid values than to have reaaaaaaally big values
        if (w > -kNormalEpsilon && w < kNormalEpsilon)
        {
            return Vec3(NAN);
        }
        return Vec3(x / w, y / w, z / w);
    }

    void Plane::multiply(const Plane &value, real scale, Plane &result)
    {
        result.normal.x = value.normal.x * scale;
        result.normal.y = value.normal.y * scale;
        result.normal.z = value.normal.z * scale;
        result.d = value.d * scale;
    }

    Plane Plane::multiply(const Plane &value, real scale)
    {
        return Plane(value.normal * scale, value.d * scale);
    }

    void Plane::dot(const Plane &left, const Vec4 &right, real &result)
    {
        result = left.normal.x * right.x + left.normal.y * right.y + left.normal.z * right.z + left.d * right.w;
    }

    real Plane::dot(const Plane &left, const Vec4 &right)
    {
        return left.normal.x * right.x + left.normal.y * right.y + left.normal.z * right.z + left.d * right.w;
    }

    void Plane::dotCoordinate(const Plane &left, const Vec3 &right, real &result)
    {
        result = left.normal.x * right.x + left.normal.y * right.y + left.normal.z * right.z + left.d;
    }

    real Plane::dotCoordinate(const Plane &left, const Vec3 &right)
    {
        return left.normal.x * right.x + left.normal.y * right.y + left.normal.z * right.z + left.d;
    }

    void Plane::dotNormal(const Plane &left, const Vec3 &right, real &result)
    {
        result = left.normal.x * right.x + left.normal.y * right.y + left.normal.z * right.z;
    }

    real Plane::dotNormal(const Plane &left, const Vec3 &right)
    {
        return left.normal.x * right.x + left.normal.y * right.y + left.normal.z * right.z;
    }

    void Plane::normalize(const Plane &plane, Plane &result)
    {
        const real magnitude = 1.0f / TkMath::sqrt(plane.normal.x * plane.normal.x + plane.normal.y * plane.normal.y + plane.normal.z * plane.normal.z);
        result.normal.x = plane.normal.x * magnitude;
        result.normal.y = plane.normal.y * magnitude;
        result.normal.z = plane.normal.z * magnitude;
        result.d = plane.d * magnitude;
    }

    Plane Plane::normalize(const Plane &plane)
    {
        const real magnitude = 1.0f / TkMath::sqrt(plane.normal.x * plane.normal.x + plane.normal.y * plane.normal.y + plane.normal.z * plane.normal.z);
        return Plane(plane.normal * magnitude, plane.d * magnitude);
    }

    void Plane::transform(const Plane &plane, const Quat &rotation, Plane &result)
    {
        const real x2 = rotation.x + rotation.x;
        const real y2 = rotation.y + rotation.y;
        const real z2 = rotation.z + rotation.z;
        const real wx = rotation.w * x2;
        const real wy = rotation.w * y2;
        const real wz = rotation.w * z2;
        const real xx = rotation.x * x2;
        const real xy = rotation.x * y2;
        const real xz = rotation.x * z2;
        const real yy = rotation.y * y2;
        const real yz = rotation.y * z2;
        const real zz = rotation.z * z2;

        const real x = plane.normal.x;
        const real y = plane.normal.y;
        const real z = plane.normal.z;

        result.normal.x = x * (1.0f - yy - zz) + y * (xy - wz) + z * (xz + wy);
        result.normal.y = x * (xy + wz) + y * (1.0f - xx - zz) + z * (yz - wx);
        result.normal.z = x * (xz - wy) + y * (yz + wx) + z * (1.0f - xx - yy);
        result.d = plane.d;
    }

    Plane Plane::transform(const Plane &plane, const Quat &rotation)
    {
        Plane result;
        transform(plane, rotation, result);
        return result;
    }

    void Plane::transform(const Plane &plane, const Mat &transformation, Plane &result)
    {
        const real x = plane.normal.x;
        const real y = plane.normal.y;
        const real z = plane.normal.z;
        const real d = plane.d;

        Mat inverse;
        Mat::invert(transformation, inverse);

        result.normal.x = x * inverse.m11 + y * inverse.m12 + z * inverse.m13 + d * inverse.m14;
        result.normal.y = x * inverse.m21 + y * inverse.m22 + z * inverse.m23 + d * inverse.m24;
        result.normal.z = x * inverse.m31 + y * inverse.m32 + z * inverse.m33 + d * inverse.m34;
        result.d = x * inverse.m41 + y * inverse.m42 + z * inverse.m43 + d * inverse.m44;
    }

    Plane Plane::transform(const Plane &plane, const Mat &transformation)
    {
        Plane result;
        transform(plane, transformation, result);
        return result;
    }

}