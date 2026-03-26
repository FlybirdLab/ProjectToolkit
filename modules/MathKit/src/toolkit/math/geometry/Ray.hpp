/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/debug/Debug.hpp>

#include <toolkit/math/Vector3.hpp>
#include <toolkit/math/geometry/CollisionsHelper.hpp>

namespace toolkit {

    struct Viewport;

    // Represents a three dimensional line based on a point in space and a direction.
    struct TK_API Ray
    {
        // The position in three dimensional space where the ray starts.
        Vec3 position;
        // The normalized direction in which the ray points.
        Vec3 direction;

        // Identity ray (at zero origin pointing forwards).
        static Ray Identity;

        Ray() = default;
        Ray(const Vec3& p, const Vec3& d)
            : position(p)
            , direction(d)
        {
            TK_CHECK_DEBUG(direction.isNormalized());
        }

        TkString toString() const;

        TK_FORCE_INLINE bool operator==(const Ray& other) const
        {
            return position == other.position && direction == other.direction;
        }

        TK_FORCE_INLINE bool operator!=(const Ray& other) const
        {
            return position != other.position || direction != other.direction;
        }

        static bool nearEqual(const Ray& a, const Ray& b)
        {
            return Vec3::nearEqual(a.position, b.position) && Vec3::nearEqual(a.direction, b.direction);
        }

        static bool nearEqual(const Ray& a, const Ray& b, real epsilon)
        {
            return Vec3::nearEqual(a.position, b.position, epsilon) && Vec3::nearEqual(a.direction, b.direction, epsilon);
        }

        // Gets a point at distance long ray.
        Vec3 getPoint(real distance) const;

        // Determines if there is an intersection between ray and a point.
        bool intersects(const Vec3& point) const
        {
            return TkCollision::rayIntersectsPoint(*this, point);
        }

        // Determines if there is an intersection between two rays.
        bool intersects(const Ray& ray) const
        {
            Vec3 point;
            return TkCollision::rayIntersectsRay(*this, ray, point);
        }

        // Determines if there is an intersection between ray and a <see cref="Ray" />.
        bool intersects(const Ray& ray, Vec3& point) const
        {
            return TkCollision::rayIntersectsRay(*this, ray, point);
        }

        // Determines if there is an intersection between ray and a <see cref="Plane" />.
        bool intersects(const Plane& plane) const
        {
            real distance;
            return TkCollision::rayIntersectsPlane(*this, plane, distance);
        }

        // Determines if there is an intersection between ray and a <see cref="Plane" />.
        bool intersects(const Plane& plane, real& distance) const
        {
            return TkCollision::rayIntersectsPlane(*this, plane, distance);
        }

        // Determines if there is an intersection between ray and a <see cref="Plane" />.
        bool intersects(const Plane& plane, Vec3& point) const
        {
            return TkCollision::rayIntersectsPlane(*this, plane, point);
        }

        // Determines if there is an intersection between ray and a triangle.
        bool intersects(const Vec3& vertex1, const Vec3& vertex2, const Vec3& vertex3) const
        {
            real distance;
            return TkCollision::rayIntersectsTriangle(*this, vertex1, vertex2, vertex3, distance);
        }

        // Determines if there is an intersection between ray and a triangle.
        bool intersects(const Vec3& vertex1, const Vec3& vertex2, const Vec3& vertex3, real& distance) const
        {
            return TkCollision::rayIntersectsTriangle(*this, vertex1, vertex2, vertex3, distance);
        }

        // Determines if there is an intersection between ray and a triangle.
        bool intersects(const Vec3& vertex1, const Vec3& vertex2, const Vec3& vertex3, Vec3& point) const
        {
            return TkCollision::rayIntersectsTriangle(*this, vertex1, vertex2, vertex3, point);
        }

        // Determines if there is an intersection between ray and a <see cref="BoundingBox" />.
        bool intersects(const BoundingBox& box) const
        {
            real distance;
            return TkCollision::rayIntersectsBox(*this, box, distance);
        }

        // Determines if there is an intersection between ray and a <see cref="BoundingBox" />.
        bool intersects(const BoundingBox& box, real& distance) const
        {
            return TkCollision::rayIntersectsBox(*this, box, distance);
        }

        // Determines if there is an intersection between ray and a <see cref="BoundingBox" />.
        bool intersects(const BoundingBox& box, Vec3& point) const
        {
            return TkCollision::rayIntersectsBox(*this, box, point);
        }

        // Determines if there is an intersection between ray and a <see cref="BoundingSphere" />.
        bool intersects(const BoundingSphere& sphere) const
        {
            real distance;
            return TkCollision::rayIntersectsSphere(*this, sphere, distance);
        }

        // Determines if there is an intersection between ray and a <see cref="BoundingSphere" />.
        bool intersects(const BoundingSphere& sphere, real& distance) const
        {
            return TkCollision::rayIntersectsSphere(*this, sphere, distance);
        }

        // Determines if there is an intersection between ray and a <see cref="BoundingSphere" />.
        bool intersects(const BoundingSphere& sphere, Vec3& point) const
        {
            return TkCollision::rayIntersectsSphere(*this, sphere, point);
        }

        // Calculates a world space ray from 2d screen coordinates.
        static Ray getPickRay(float x, float y, const Viewport& viewport, const Mat& vp);
    };

    template<>
    struct TIsPODType<Ray>
    {
        enum { Value = true };
    };

}

namespace toolkit
{
    template <>
    inline psize formatCustom<>(FormatBuffer &buf, const toolkit::Ray &t)
    {
        const psize beginUsed = buf.getUsed();
        Formatter::format(buf, "Position:{} Direction:{}", t.position, t.direction);
        return buf.getUsed() - beginUsed - 1;
    }
}
