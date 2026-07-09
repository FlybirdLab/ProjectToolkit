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

    // Represents a three-dimensional triangle.
    class TK_API Triangle
    {
    public:
        Triangle() = default;
        Triangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2)
            : v0(v0), v1(v1), v2(v2) {}

    public:
        Vec3 getNormal() const;
        
        // Determines if there is an intersection between triangle and a ray.
        bool intersects(const Ray &ray) const;
        // Determines if there is an intersection between triangle and a ray. Returns distance to the intersection.
        bool intersects(const Ray &ray, real &distance) const;
        // Determines if there is an intersection between triangle and a ray. Returns distance to the intersection and surface normal vector.
        bool intersects(const Ray &ray, real &distance, Vec3 &normal) const;
        // Determines if there is an intersection between triangle and a ray. Returns point of intersection.
        bool intersects(const Ray &ray, Vec3 &point) const;

    public:
        // The first vertex.
        Vec3 v0;
        // The second vertex.
        Vec3 v1;
        // The third vertex.
        Vec3 v2;
    };

    template <>
    struct TIsPODType<Triangle>
    {
        enum
        {
            Value = true
        };
    };

}