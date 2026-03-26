/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/math/Vector3.hpp>
#include <toolkit/math/geometry/CollisionsHelper.hpp>

namespace toolkit {

    // Represents a three-dimensional triangle.
    struct TK_API Triangle
    {
        // The first vertex.
        Vec3 v0;
        // The second vertex.
        Vec3 v1;
        // The third vertex.
        Vec3 v2;

        Triangle() = default;
        Triangle(const Vec3& v0, const Vec3& v1, const Vec3& v2)
            : v0(v0)
            , v1(v1)
            , v2(v2) {}

        Vec3 getNormal() const
        {
            return Vec3::normalize((v1 - v0) ^ (v2 - v0));
        }

        // Determines if there is an intersection between triangle and a ray.
        bool intersects(const Ray& ray) const
        {
            real distance;
            return TkCollision::rayIntersectsTriangle(ray, v0, v1, v2, distance);
        }

        // Determines if there is an intersection between triangle and a ray. Returns distance to the intersection.
        bool intersects(const Ray& ray, real& distance) const
        {
            return TkCollision::rayIntersectsTriangle(ray, v0, v1, v2, distance);
        }

        // Determines if there is an intersection between triangle and a ray. Returns distance to the intersection and surface normal vector.
        bool intersects(const Ray& ray, real& distance, Vec3& normal) const
        {
            return TkCollision::rayIntersectsTriangle(ray, v0, v1, v2, distance, normal);
        }

        // Determines if there is an intersection between triangle and a ray. Returns point of intersection.
        bool intersects(const Ray& ray, Vec3& point) const
        {
            return TkCollision::rayIntersectsTriangle(ray, v0, v1, v2, point);
        }
    };


    template<>
    struct TIsPODType<Triangle>
    {
        enum { Value = true };
    };

}