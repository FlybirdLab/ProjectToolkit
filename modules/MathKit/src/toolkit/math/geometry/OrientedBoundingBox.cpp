/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "OrientedBoundingBox.hpp"

#include "BoundingSphere.hpp"
#include "BoundingBox.hpp"
#include "Ray.hpp"
#include "toolkit/math/Matrix.hpp"
#include "toolkit/types/String.hpp"

namespace toolkit {

    OrientedBoundingBox::OrientedBoundingBox(const BoundingBox& bb)
    {
        const Vec3 center = bb.minimum + (bb.maximum - bb.minimum) * 0.5f;
        extents = bb.maximum - center;
        transformation = Transform(center);
    }


    OrientedBoundingBox::OrientedBoundingBox(const Vec3& extents, const Mat& t)
        : extents(extents)
    {
        t.decompose(transformation);
    }


    OrientedBoundingBox::OrientedBoundingBox(const Vec3& extents, const Mat3x3& rotationScale, const Vec3& translation)
        : extents(extents)
        , transformation(translation, rotationScale)
    {
    }


    OrientedBoundingBox::OrientedBoundingBox(const Vec3& minimum, const Vec3& maximum)
    {
        const Vec3 center = minimum + (maximum - minimum) * 0.5f;
        extents = maximum - center;
        transformation = Transform(center);
    }


    OrientedBoundingBox::OrientedBoundingBox(Vec3 points[], i32 pointCount)
    {
        TK_ASSERT(points && pointCount > 0);
        Vec3 minimum = points[0];
        Vec3 maximum = points[0];
        for (i32 i = 1; i < pointCount; i++)
        {
            Vec3::min(minimum, points[i], minimum);
            Vec3::max(maximum, points[i], maximum);
        }
        const Vec3 center = minimum + (maximum - minimum) * 0.5f;
        extents = maximum - center;
        transformation = Transform(center);
    }


    TkString OrientedBoundingBox::toString() const
    {
        return TkString::format("%s", *this);
    }


    void OrientedBoundingBox::getCorners(FVec3 corners[8]) const
    {
        const FVec3 xv = transformation.localToWorldVector(Vec3((float)extents.x, 0, 0));
        const FVec3 yv = transformation.localToWorldVector(Vec3(0, (float)extents.y, 0));
        const FVec3 zv = transformation.localToWorldVector(Vec3(0, 0, (float)extents.z));

        const FVec3 center = transformation.Translation;

        corners[0] = center + xv + yv + zv;
        corners[1] = center + xv + yv - zv;
        corners[2] = center - xv + yv - zv;
        corners[3] = center - xv + yv + zv;
        corners[4] = center + xv - yv + zv;
        corners[5] = center + xv - yv - zv;
        corners[6] = center - xv - yv - zv;
        corners[7] = center - xv - yv + zv;
    }


    void OrientedBoundingBox::getCorners(DVec3 corners[8]) const
    {
        const DVec3 xv = transformation.localToWorldVector(Vec3((float)extents.x, 0, 0));
        const DVec3 yv = transformation.localToWorldVector(Vec3(0, (float)extents.y, 0));
        const DVec3 zv = transformation.localToWorldVector(Vec3(0, 0, (float)extents.z));

        const DVec3 center = transformation.Translation;

        corners[0] = center + xv + yv + zv;
        corners[1] = center + xv + yv - zv;
        corners[2] = center - xv + yv - zv;
        corners[3] = center - xv + yv + zv;
        corners[4] = center + xv - yv + zv;
        corners[5] = center + xv - yv - zv;
        corners[6] = center - xv - yv - zv;
        corners[7] = center - xv - yv + zv;
    }


    Vec3 OrientedBoundingBox::getSize() const
    {
        const Vec3 xv = transformation.localToWorldVector(Vec3(extents.x * 2, 0, 0));
        const Vec3 yv = transformation.localToWorldVector(Vec3(0, extents.y * 2, 0));
        const Vec3 zv = transformation.localToWorldVector(Vec3(0, 0, extents.z * 2));
        return Vec3(xv.length(), yv.length(), zv.length());
    }


    Vec3 OrientedBoundingBox::getSizeSquared() const
    {
        const Vec3 xv = transformation.localToWorldVector(Vec3(extents.x * 2, 0, 0));
        const Vec3 yv = transformation.localToWorldVector(Vec3(0, extents.y * 2, 0));
        const Vec3 zv = transformation.localToWorldVector(Vec3(0, 0, extents.z * 2));
        return Vec3(xv.lengthSquared(), yv.lengthSquared(), zv.lengthSquared());
    }


    BoundingBox OrientedBoundingBox::getBoundingBox() const
    {
        BoundingBox result;
        Vec3 corners[8];
        getCorners(corners);
        BoundingBox::fromPoints(corners, 8, result);
        return result;
    }


    void OrientedBoundingBox::getBoundingBox(BoundingBox& result) const
    {
        Vec3 corners[8];
        getCorners(corners);

        BoundingBox::fromPoints(corners, 8, result);
    }


    void OrientedBoundingBox::transform(const Mat& matrix)
    {
        Transform transform;
        matrix.decompose(transform);
        transformation = transform.localToWorld(transformation);
    }


    void OrientedBoundingBox::transform(const Transform& transform)
    {
        transformation = transform.localToWorld(transformation);
    }


    ContainmentType OrientedBoundingBox::contains(const Vec3& point, real* distance) const
    {
        // Transform the point into the obb coordinates
        Vec3 locPoint;
        transformation.worldToLocal(point, locPoint);
        locPoint.x = TkMath::abs(locPoint.x);
        locPoint.y = TkMath::abs(locPoint.y);
        locPoint.z = TkMath::abs(locPoint.z);

        if (distance)
        {
            // Get minimum distance to edge in local space
            Vec3 tmp;
            Vec3::subtract(extents, locPoint, tmp);
            const real minDstToEdgeLocal = tmp.getAbsolute().minValue();

            // Transform distance to world space
            Vec3 dstVec = Vec3::UnitX * minDstToEdgeLocal;
            transformation.localToWorldVector(dstVec, dstVec);
            *distance = dstVec.length();
        }

        // Simple axes-aligned BB check
        if (locPoint.x < extents.x && locPoint.y < extents.y && locPoint.z < extents.z)
            return ContainmentType::kContains;
        if (Vec3::nearEqual(locPoint, extents))
            return ContainmentType::kIntersects;
        return ContainmentType::kDisjoint;
    }


    ContainmentType OrientedBoundingBox::contains(const BoundingSphere& sphere, bool ignoreScale) const
    {
        // Transform sphere center into the obb coordinates
        Vec3 locCenter;
        transformation.worldToLocal(sphere.center, locCenter);

        real locRadius;
        if (ignoreScale)
        {
            locRadius = sphere.radius;
        }
        else
        {
            // Transform sphere radius into the obb coordinates
            Vec3 vRadius = Vec3::UnitX * sphere.radius;
            transformation.localToWorldVector(vRadius, vRadius);
            locRadius = vRadius.length();
        }

        // Perform regular BoundingBox to BoundingSphere containment check
        const Vec3 minusextents = -extents;
        Vec3 vector;
        Vec3::clamp(locCenter, minusextents, extents, vector);
        const real distance = Vec3::distanceSquared(locCenter, vector);

        if (distance > locRadius * locRadius)
            return ContainmentType::kDisjoint;
        if (minusextents.x + locRadius <= locCenter.x && locCenter.x <= extents.x - locRadius && (extents.x - minusextents.x > locRadius && minusextents.y + locRadius <= locCenter.y) && (locCenter.y <= extents.y - locRadius && extents.y - minusextents.y > locRadius && (minusextents.z + locRadius <= locCenter.z && locCenter.z <= extents.z - locRadius && extents.z - minusextents.z > locRadius)))
            return ContainmentType::kContains;
        return ContainmentType::kIntersects;
    }


    bool OrientedBoundingBox::intersects(const Ray& ray, Vec3& point) const
    {
        // Put ray in box space
        Ray bRay;
        transformation.worldToLocalVector(ray.direction, bRay.direction);
        transformation.worldToLocal(ray.position, bRay.position);

        // Perform a regular ray to BoundingBox check
        const BoundingBox bb(-extents, extents);
        const bool intersects = TkCollision::rayIntersectsBox(bRay, bb, point);

        // Put the result intersection back to world
        if (intersects)
            transformation.localToWorld(point, point);

        return intersects;
    }


    bool OrientedBoundingBox::intersects(const Ray& ray, real& distance) const
    {
        Vec3 point;
        const bool result = intersects(ray, point);
        distance = Vec3::distance(ray.position, point);
        return result;
    }


    bool OrientedBoundingBox::intersects(const Ray& ray, real& distance, Vec3& normal) const
    {
        // Put ray in box space
        Ray bRay;
        transformation.worldToLocalVector(ray.direction, bRay.direction);
        transformation.worldToLocal(ray.position, bRay.position);

        // Perform a regular ray to BoundingBox check
        const BoundingBox bb(-extents, extents);
        if (TkCollision::rayIntersectsBox(bRay, bb, distance, normal))
        {
            // Put the result intersection back to world
            transformation.localToWorldVector(normal, normal);
            normal.normalize();
            return true;
        }

        return false;
    }
}