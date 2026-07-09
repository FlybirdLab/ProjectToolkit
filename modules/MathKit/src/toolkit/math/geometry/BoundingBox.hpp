/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/math/Vector3.hpp>
#include <toolkit/math/geometry/Collisions.hpp>

namespace toolkit
{

    // Represents an axis-aligned bounding box in three dimensional space.
    class TK_API BoundingBox
    {
    public:
        // A which represents an empty space.
        static const BoundingBox kEmpty;
        // A located at zero point with zero size.
        static const BoundingBox kZero;

    public:
        BoundingBox() = default;
        BoundingBox(const Vec3 &point)
            : minimum(point), maximum(point)
        {
        }
        BoundingBox(const Vec3 &minimum, const Vec3 &maximum)
            : minimum(minimum), maximum(maximum)
        {
        }

    public:
        TkString toString() const;

        // Gets the eight corners of the bounding box.
        void getCorners(FVec3 corners[8]) const;
        // Gets the eight corners of the bounding box.
        void getCorners(DVec3 corners[8]) const;

        // Calculates volume of the box.
        real getVolume() const;
        // Calculates the size of the box.
        Vec3 getSize() const;
        // Calculates the size of the box.
        void getSize(Vec3 &result) const
        {
            Vec3::subtract(maximum, minimum, result);
        }
        // Sets the size of the box.
        void setSize(const Vec3 &value);

        // Gets the center point location.
        Vec3 getCenter() const
        {
            return minimum + (maximum - minimum) * 0.5f;
        }

        // Gets the center point location.
        void getCenter(Vec3 &result) const
        {
            result = minimum + (maximum - minimum) * 0.5f;
        }
        // Sets the center point location.
        void setCenter(const Vec3 &value);

    public:
        static bool nearEqual(const BoundingBox &a, const BoundingBox &b)
        {
            return Vec3::nearEqual(a.minimum, b.minimum) && Vec3::nearEqual(a.maximum, b.maximum);
        }

        static bool nearEqual(const BoundingBox &a, const BoundingBox &b, real epsilon)
        {
            return Vec3::nearEqual(a.minimum, b.minimum, epsilon) && Vec3::nearEqual(a.maximum, b.maximum, epsilon);
        }

    public:
        // Merges the box with a point.
        void merge(const Vec3 &point);
        // Merges the box with the other box.
        void merge(const BoundingBox &box);

        // Creates the bounding box that is offseted by the given vector. Adds the offset value to minimum and maximum points.
        BoundingBox makeOffsetted(const Vec3 &offset) const;

    public:
        TK_FORCE_INLINE bool operator==(const BoundingBox &other) const
        {
            return minimum == other.minimum && maximum == other.maximum;
        }

        TK_FORCE_INLINE bool operator!=(const BoundingBox &other) const
        {
            return minimum != other.minimum || maximum != other.maximum;
        }

        TK_FORCE_INLINE BoundingBox operator*(const Mat &matrix) const
        {
            BoundingBox result;
            transform(*this, matrix, result);
            return result;
        }

    public:
        // Constructs a Bounding Box that fully contains the given pair of points.
        static void fromPoints(const Vec3 &pointA, const Vec3 &pointB, BoundingBox &result)
        {
            Vec3::min(pointA, pointB, result.minimum);
            Vec3::max(pointA, pointB, result.maximum);
        }

        // Constructs a Bounding Box that fully contains the given points.
        static void fromPoints(const FVec3 *points, i32 pointsCount, BoundingBox &result);

        // Constructs a Bounding Box that fully contains the given points.
        static void fromPoints(const DVec3 *points, i32 pointsCount, BoundingBox &result);

        // Constructs a Bounding Box from a given sphere.
        static void fromSphere(const BoundingSphere &sphere, BoundingBox &result);

        // Constructs a Bounding Box from a given sphere.
        static BoundingBox fromSphere(const BoundingSphere &sphere);

        // Constructs a Bounding Box that is as large as the total combined area of the two specified boxes.
        static void merge(const BoundingBox &value1, const BoundingBox &value2, BoundingBox &result)
        {
            Vec3::min(value1.minimum, value2.minimum, result.minimum);
            Vec3::max(value1.maximum, value2.maximum, result.maximum);
        }

        // Transforms the bounding box using the specified matrix.
        static BoundingBox transform(const BoundingBox &box, const Mat &matrix);

        // Creates the bounding box that is offseted by the given vector. Adds the offset value to minimum and maximum points.
        static BoundingBox makeOffsetted(const BoundingBox &box, const Vec3 &offset);
        // Creates the bounding box that is scaled by the given factor. Applies scale to the size of the bounds.
        static BoundingBox makeScaled(const BoundingBox &box, real scale);

        // Transforms the bounding box using the specified matrix.
        static void transform(const BoundingBox &box, const Mat &matrix, BoundingBox &result);
        // Transforms the bounding box using the specified transformation.
        static void transform(const BoundingBox &box, const Transform &transform, BoundingBox &result);

    public:
        // Determines if there is an intersection between box and a ray.
        TK_FORCE_INLINE bool intersects(const Ray &ray) const
        {
            real distance;
            return TkCollision::rayIntersectsBox(ray, *this, distance);
        }

        // Determines if there is an intersection between box and a ray.
        TK_FORCE_INLINE bool intersects(const Ray &ray, real &distance) const
        {
            return TkCollision::rayIntersectsBox(ray, *this, distance);
        }

        // Determines if there is an intersection between box and a ray.
        TK_FORCE_INLINE bool intersects(const Ray &ray, real &distance, Vec3 &normal) const
        {
            return TkCollision::rayIntersectsBox(ray, *this, distance, normal);
        }

        // Determines if there is an intersection between box and a ray.
        TK_FORCE_INLINE bool intersects(const Ray &ray, Vec3 &point) const
        {
            return TkCollision::rayIntersectsBox(ray, *this, point);
        }

        // Determines if there is an intersection between box and a plane.
        TK_FORCE_INLINE PlaneIntersectionType intersects(const Plane &plane) const
        {
            return TkCollision::planeIntersectsBox(plane, *this);
        }

        // Determines if there is an intersection between two boxes.
        TK_FORCE_INLINE bool intersects(const BoundingBox &box) const
        {
            return TkCollision::boxIntersectsBox(*this, box);
        }

        // Determines if there is an intersection between box and a sphere.
        TK_FORCE_INLINE bool intersects(const BoundingSphere &sphere) const
        {
            return TkCollision::boxIntersectsSphere(*this, sphere);
        }

        // Determines whether box contains a point.
        TK_FORCE_INLINE ContainmentType contains(const Vec3 &point) const
        {
            return TkCollision::boxContainsPoint(*this, point);
        }

        // Determines whether box contains a Bounding Box.
        TK_FORCE_INLINE ContainmentType contains(const BoundingBox &box) const
        {
            return TkCollision::boxContainsBox(*this, box);
        }

        // Determines whether box contains a Bounding Sphere.
        TK_FORCE_INLINE ContainmentType contains(const BoundingSphere &sphere) const
        {
            return TkCollision::boxContainsSphere(*this, sphere);
        }

        // Determines the distance between box and a point.
        TK_FORCE_INLINE real distance(const Vec3 &point) const
        {
            return TkCollision::distanceBoxPoint(*this, point);
        }

        // Determines the distance between two boxes.
        TK_FORCE_INLINE real distance(const BoundingBox &box) const
        {
            return TkCollision::distanceBoxBox(*this, box);
        }

    public:
        // The minimum point of the box.
        Vec3 minimum;
        // The maximum point of the box.
        Vec3 maximum;
    };

    template <>
    struct TIsPODType<BoundingBox>
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
    inline psize formatCustom<>(FormatBuffer &buf, const toolkit::BoundingBox &t)
    {
        const psize beginUsed = buf.getUsed();
        Formatter::format(buf, "Minimum:%s Maximum:%s", t.minimum, t.maximum);
        return buf.getUsed() - beginUsed - 1;
    }
}
