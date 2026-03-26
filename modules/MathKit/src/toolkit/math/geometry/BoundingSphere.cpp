/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "BoundingSphere.hpp"

#include "BoundingBox.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/math/Matrix.hpp"

namespace toolkit {

    const BoundingSphere BoundingSphere::Empty(Vec3(0, 0, 0), 0);

    TkString BoundingSphere::toString() const
    {
        return TkString::format("%s", *this);
    }


    bool BoundingSphere::intersects(const Ray& ray) const
    {
        real distance;
        return TkCollision::rayIntersectsSphere(ray, *this, distance);
    }


    bool BoundingSphere::intersects(const Ray& ray, real& distance) const
    {
        return TkCollision::rayIntersectsSphere(ray, *this, distance);
    }


    bool BoundingSphere::intersects(const Ray& ray, real& distance, Vec3& normal) const
    {
        return TkCollision::rayIntersectsSphere(ray, *this, distance, normal);
    }


    bool BoundingSphere::intersects(const Ray& ray, Vec3& point) const
    {
        return TkCollision::rayIntersectsSphere(ray, *this, point);
    }


    PlaneIntersectionType BoundingSphere::intersects(const Plane& plane) const
    {
        return TkCollision::planeIntersectsSphere(plane, *this);
    }


    bool BoundingSphere::intersects(const Vec3& vertex1, const Vec3& vertex2, const Vec3& vertex3) const
    {
        return TkCollision::sphereIntersectsTriangle(*this, vertex1, vertex2, vertex3);
    }


    bool BoundingSphere::intersects(const BoundingBox& box) const
    {
        return TkCollision::boxIntersectsSphere(box, *this);
    }


    bool BoundingSphere::intersects(const BoundingSphere& sphere) const
    {
        const real radiisum = radius + sphere.radius;
        const real x = center.x - sphere.center.x;
        const real y = center.y - sphere.center.y;
        const real z = center.z - sphere.center.z;
        return x * x + y * y + z * z <= radiisum * radiisum;
    }


    ContainmentType BoundingSphere::contains(const Vec3& point) const
    {
        return TkCollision::sphereContainsPoint(*this, point);
    }


    ContainmentType BoundingSphere::contains(const Vec3& vertex1, const Vec3& vertex2, const Vec3& vertex3) const
    {
        return TkCollision::sphereContainsTriangle(*this, vertex1, vertex2, vertex3);
    }


    ContainmentType BoundingSphere::contains(const BoundingBox& box) const
    {
        return TkCollision::sphereContainsBox(*this, box);
    }


    ContainmentType BoundingSphere::contains(const BoundingSphere& sphere) const
    {
        return TkCollision::sphereContainsSphere(*this, sphere);
    }


    BoundingBox BoundingSphere::getBoundingBox() const
    {
        BoundingBox result;
        BoundingBox::fromSphere(*this, result);
        return result;
    }


    void BoundingSphere::getBoundingBox(BoundingBox& result) const
    {
        BoundingBox::fromSphere(*this, result);
    }


    void BoundingSphere::fromPoints(const FVec3* points, i32 pointsCount, BoundingSphere& result)
    {
        TK_ASSERT(points && pointsCount > 0);

        // Find the center of all points
        FVec3 center = FVec3::Zero;
        for (i32 i = 0; i < pointsCount; i++)
            FVec3::add(points[i], center, center);
        center /= (float)pointsCount;

        // Find the radius of the sphere
        float radius = 0.0f;
        for (i32 i = 0; i < pointsCount; i++)
        {
            const float distance = FVec3::distanceSquared(center, points[i]);
            if (distance > radius)
                radius = distance;
        }

        // Construct the sphere
        result.center = center;
        result.radius = TkMath::sqrt(radius);
    }


    void BoundingSphere::fromPoints(const DVec3* points, i32 pointsCount, BoundingSphere& result)
    {
        TK_ASSERT(points && pointsCount > 0);

        // Find the center of all points
        DVec3 center = DVec3::Zero;
        for (i32 i = 0; i < pointsCount; i++)
            DVec3::add(points[i], center, center);
        center /= (double)pointsCount;

        // Find the radius of the sphere
        double radius = 0.0;
        for (i32 i = 0; i < pointsCount; i++)
        {
            const double distance = DVec3::distanceSquared(center, points[i]);
            if (distance > radius)
                radius = distance;
        }

        // Construct the sphere
        result.center = center;
        result.radius = (real)TkMath::sqrt(radius);
    }


    void BoundingSphere::fromBox(const BoundingBox& box, BoundingSphere& result)
    {
        if (box.minimum.isNanOrInfinity() || box.maximum.isNanOrInfinity())
        {
            result = Empty;
            return;
        }
        const real x = box.maximum.x - box.minimum.x;
        const real y = box.maximum.y - box.minimum.y;
        const real z = box.maximum.z - box.minimum.z;
        result.center.x = box.minimum.x + x * 0.5f;
        result.center.y = box.minimum.y + y * 0.5f;
        result.center.z = box.minimum.z + z * 0.5f;
        result.radius = TkMath::sqrt(x * x + y * y + z * z) * 0.5f;
    }


    void BoundingSphere::merge(const BoundingSphere& value1, const BoundingSphere& value2, BoundingSphere& result)
    {
        // Pre-exit if one of the bounding sphere by assuming that a merge with an empty sphere is equivalent at taking the non-empty sphere
        if (value1 == Empty)
        {
            result = value2;
            return;
        }
        if (value2 == Empty)
        {
            result = value1;
            return;
        }

        const Vec3 difference = value2.center - value1.center;
        const real length = difference.length();
        const real radius = value1.radius;
        const real radius2 = value2.radius;

        if (radius + radius2 >= length)
        {
            if (radius - radius2 >= length)
            {
                result = value1;
                return;
            }

            if (radius2 - radius >= length)
            {
                result = value2;
                return;
            }
        }

        const Vec3 vector = difference * (1.0f / length);
        const real min = TkMath::min(-radius, length - radius2);
        const real max = (TkMath::max(radius, length + radius2) - min) * 0.5f;

        result.center = value1.center + vector * (max + min);
        result.radius = max;
    }


    void BoundingSphere::merge(const BoundingSphere& value1, const Vec3& value2, BoundingSphere& result)
    {
        const Vec3 difference = value2 - value1.center;
        const real length = difference.length();
        const real radius = value1.radius;
        if (radius >= length)
        {
            result = value1;
            return;
        }

        const Vec3 vector = difference * (1.0f / length);
        const real min = TkMath::min(-radius, length);
        const real max = (TkMath::max(radius, length) - min) * 0.5f;

        result.center = value1.center + vector * (max + min);
        result.radius = max;
    }


    void BoundingSphere::transform(const BoundingSphere& sphere, const Mat& matrix, BoundingSphere& result)
    {
        Vec3::transform(sphere.center, matrix, result.center);
        result.radius = sphere.radius * matrix.getScaleVector().getAbsolute().maxValue();
    }

}