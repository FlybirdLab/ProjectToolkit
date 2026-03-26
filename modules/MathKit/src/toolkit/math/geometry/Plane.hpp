/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/math/Vector3.hpp>
#include "CollisionsHelper.hpp"

namespace toolkit {

    // Represents a plane in three dimensional space.
    struct TK_API Plane
    {
        static constexpr real DistanceEpsilon = 0.0001f;
        static constexpr real NormalEpsilon = 1.0f / 65535.0f;

        // The normal vector of the plane.
        Vec3 normal;
        // The distance of the plane along its normal from the origin.
        real d;

        Plane() = default;
        Plane(const Vec3& normal, real d)
            : normal(normal)
            , d(d) {
        }
        Plane(const Vec3& point, const Vec3& normal)
            : normal(normal)
            , d(-Vec3::dot(normal, point)) {
        }
        Plane(const Vec3& point1, const Vec3& point2, const Vec3& point3);

        TkString toString() const;

        // Changes the coefficients of the normal vector of the plane to make it of unit length.
        void normalize();

        void negate()
        {
            normal = normal.getNegative();
            d *= -1;
        }

        Vec3 pointOnPlane() const
        {
            return normal * d;
        }

        Plane negated() const
        {
            return Plane(-normal, -d);
        }

        Plane operator*(real scale) const
        {
            return Plane(normal * scale, d * scale);
        }

        bool operator==(const Plane& other) const
        {
            return normal == other.normal && d == other.d;
        }

        bool operator!=(const Plane& other) const
        {
            return normal != other.normal || d != other.d;
        }

        static bool nearEqual(const Plane& a, const Plane& b)
        {
            return Vec3::nearEqual(a.normal, b.normal) && TkMath::nearEqual(a.d, b.d);
        }

        void translate(const Vec3& translation)
        {
            const Vec3 mul = normal * translation;
            d += mul.sumValues();
        }

        static Plane translated(const Plane& plane, const Vec3& translation)
        {
            Plane result = plane;
            result.translate(translation);
            return result;
        }

        static Plane translated(const Plane& plane, float translateX, float translateY, float translateZ)
        {
            Plane result = plane;
            result.translate(Vec3(translateX, translateY, translateZ));
            return result;
        }

        static Vec3 intersection(const Plane& inPlane1, const Plane& inPlane2, const Plane& inPlane3);

        // Determines if there is an intersection between plane and a point.
        PlaneIntersectionType intersects(const Vec3& point) const
        {
            return TkCollision::planeIntersectsPoint(*this, point);
        }

        // Determines if there is an intersection between plane and a ray.
        bool intersects(const Ray& ray) const
        {
            real distance;
            return TkCollision::rayIntersectsPlane(ray, *this, distance);
        }

        // Determines if there is an intersection between plane and a ray. Returns distance to the intersection.
        bool intersects(const Ray& ray, real& distance) const
        {
            return TkCollision::rayIntersectsPlane(ray, *this, distance);
        }

        // Determines if there is an intersection between plane and a ray.
        bool intersects(const Ray& ray, Vec3& point) const
        {
            return TkCollision::rayIntersectsPlane(ray, *this, point);
        }

        // Determines if there is an intersection between two planes.
        bool intersects(const Plane& plane) const
        {
            return TkCollision::planeIntersectsPlane(*this, plane);
        }

        // Determines if there is an intersection between two planes. Returns ray that defines a line of intersection.
        bool intersects(const Plane& plane, Ray& line) const
        {
            return TkCollision::planeIntersectsPlane(*this, plane, line);
        }

        // Determines if there is an intersection between plane and a triangle.
        PlaneIntersectionType intersects(const Vec3& vertex1, const Vec3& vertex2, const Vec3& vertex3) const
        {
            return TkCollision::planeIntersectsTriangle(*this, vertex1, vertex2, vertex3);
        }

        // Determines if there is an intersection between plane and a box.
        PlaneIntersectionType intersects(const BoundingBox& box) const
        {
            return TkCollision::planeIntersectsBox(*this, box);
        }

        // Determines if there is an intersection between plane and a sphere.
        PlaneIntersectionType intersects(const BoundingSphere& sphere) const
        {
            return TkCollision::planeIntersectsSphere(*this, sphere);
        }

        // Scales the plane by the given factor.
        static void multiply(const Plane& value, real scale, Plane& result);

        // Scales the plane by the given factor.
        static Plane multiply(const Plane& value, real scale);

        // Calculates the dot product of the specified vector and plane.
        static void dot(const Plane& left, const Vec4& right, real& result);

        // Calculates the dot product of the specified vector and plane.
        static real dot(const Plane& left, const Vec4& right);

        // Calculates the dot product of a specified vector and the normal of the plane plus the distance value of the plane.
        static void dotCoordinate(const Plane& left, const Vec3& right, real& result);

        // Calculates the dot product of a specified vector and the normal of the plane plus the distance value of the plane.
        static real dotCoordinate(const Plane& left, const Vec3& right);

        // Calculates the dot product of the specified vector and the normal of the plane.
        static void dotNormal(const Plane& left, const Vec3& right, real& result);

        // Calculates the dot product of the specified vector and the normal of the plane.
        static real dotNormal(const Plane& left, const Vec3& right);

        // Changes the coefficients of the normal vector of the plane to make it of unit length.
        static void normalize(const Plane& plane, Plane& result);

        // Changes the coefficients of the normal vector of the plane to make it of unit length.
        static Plane normalize(const Plane& plane);

        // Transforms a normalized plane by a quaternion rotation.
        static void transform(const Plane& plane, const Quat& rotation, Plane& result);

        // Transforms a normalized plane by a quaternion rotation.
        static Plane transform(const Plane& plane, const Quat& rotation);

        // Transforms a normalized plane by a matrix.
        static void transform(const Plane& plane, const Mat& transformation, Plane& result);

        // Transforms a normalized plane by a matrix.
        static Plane transform(const Plane& plane, const Mat& transformation);
    };


    inline Plane operator*(real a, const Plane& b)
    {
        return b * a;
    }


    template<>
    struct TIsPODType<Plane>
    {
        enum { Value = true };
    };

}

namespace toolkit
{
    template <>
    inline psize formatCustom<>(FormatBuffer &buf, const toolkit::Plane &t)
    {
        const psize beginUsed = buf.getUsed();
        Formatter::format(buf, "Normal:{} D:%g", t.normal, t.d);
        return buf.getUsed() - beginUsed - 1;
    }
}
