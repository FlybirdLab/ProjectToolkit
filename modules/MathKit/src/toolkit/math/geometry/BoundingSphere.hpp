/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/math/Vector3.hpp>
#include <toolkit/base/Math.hpp>
#include <toolkit/math/geometry/Collisions.hpp>

namespace toolkit
{

    // Represents a bounding sphere in three dimensional space.
    class TK_API BoundingSphere
    {
    public:
        // An empty bounding sphere (Center = 0 and radius = 0).
        static const BoundingSphere kEmpty;

    public:
        BoundingSphere() = default;
        BoundingSphere(const Vec3 &c, real r)
            : center(c), radius(r)
        {
        }

    public:
        TkString toString() const;

    public:
        TK_FORCE_INLINE bool operator==(const BoundingSphere &other) const
        {
            return center == other.center && radius == other.radius;
        }

        TK_FORCE_INLINE bool operator!=(const BoundingSphere &other) const
        {
            return center != other.center || radius != other.radius;
        }

    public:
        static bool nearEqual(const BoundingSphere &a, const BoundingSphere &b)
        {
            return Vec3::nearEqual(a.center, b.center) && TkMath::nearEqual(a.radius, b.radius);
        }

        static bool nearEqual(const BoundingSphere &a, const BoundingSphere &b, real epsilon)
        {
            return Vec3::nearEqual(a.center, b.center, epsilon) && TkMath::nearEqual(a.radius, b.radius, epsilon);
        }

        // Determines if there is an intersection between sphere and a rat.
        bool intersects(const Ray &ray) const;
        // Determines if there is an intersection between sphere and a rat.
        bool intersects(const Ray &ray, real &distance) const;
        // Determines if there is an intersection between sphere and a rat.
        bool intersects(const Ray &ray, real &distance, Vec3 &normal) const;
        // Determines if there is an intersection between sphere and a rat.
        bool intersects(const Ray &ray, Vec3 &point) const;
        // Determines if there is an intersection between sphere and a plane.
        PlaneIntersectionType intersects(const Plane &plane) const;
        // Determines if there is an intersection between sphere and a triangle.
        bool intersects(const Vec3 &vertex1, const Vec3 &vertex2, const Vec3 &vertex3) const;
        // Determines if there is an intersection between sphere and a box.
        bool intersects(const BoundingBox &box) const;
        // Determines if there is an intersection between two spheres.
        bool intersects(const BoundingSphere &sphere) const;

        // Determines whether sphere contains a point.
        ContainmentType contains(const Vec3 &point) const;
        // Determines whether sphere contains a triangle.
        ContainmentType contains(const Vec3 &vertex1, const Vec3 &vertex2, const Vec3 &vertex3) const;
        // Determines whether sphere contains a box.
        ContainmentType contains(const BoundingBox &box) const;
        // Determines whether sphere contains a sphere.
        ContainmentType contains(const BoundingSphere &sphere) const;

        // Gets the box which contains whole sphere.
        BoundingBox getBoundingBox() const;
        // Gets the box which contains whole sphere.
        void getBoundingBox(BoundingBox &result) const;

    public:
        // Constructs a BoundingSphere that fully contains the given points
        static void fromPoints(const FVec3 *points, i32 pointsCount, BoundingSphere &result);
        // Constructs a BoundingSphere that fully contains the given points
        static void fromPoints(const DVec3 *points, i32 pointsCount, BoundingSphere &result);
        // Constructs a Bounding Sphere from a given box.
        static void fromBox(const BoundingBox &box, BoundingSphere &result);

        // Constructs a BoundingSphere that is the as large as the total combined area of the two specified spheres
        static void merge(const BoundingSphere &value1, const BoundingSphere &value2, BoundingSphere &result);
        // Constructs a BoundingSphere that is the as large as the total combined area of the specified sphere and the point.
        static void merge(const BoundingSphere &value1, const Vec3 &value2, BoundingSphere &result);

        // Transforms the bounding sphere using the specified matrix.
        static void transform(const BoundingSphere &sphere, const Mat &matrix, BoundingSphere &result);

    public:
        // The center of the sphere in three dimensional space.
        Vec3 center;
        // The radius of the sphere.
        real radius;
    };

    template <>
    struct TIsPODType<BoundingSphere>
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
    inline psize formatCustom<>(FormatBuffer &buf, const toolkit::BoundingSphere &t)
    {
        const psize beginUsed = buf.getUsed();
        Formatter::format(buf, "Center:{} Radius:%g", t.center, t.radius);
        return buf.getUsed() - beginUsed - 1;
    }
}
