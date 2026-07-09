/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Triangle.hpp"

namespace toolkit
{
    Vec3 Triangle::getNormal() const
    {
        Vec3 normal;
        Vec3::cross(v1 - v0, v2 - v0, normal);
        normal.normalize();
        return normal;
    }

    bool Triangle::intersects(const Ray &ray) const
    {
        real distance;
        return TkCollision::rayIntersectsTriangle(ray, v0, v1, v2, distance);
    }

    bool Triangle::intersects(const Ray &ray, real &distance) const
    {
        return TkCollision::rayIntersectsTriangle(ray, v0, v1, v2, distance);
    }

    bool Triangle::intersects(const Ray &ray, real &distance, Vec3 &normal) const
    {
        return TkCollision::rayIntersectsTriangle(ray, v0, v1, v2, distance, normal);
    }

    bool Triangle::intersects(const Ray &ray, Vec3 &point) const
    {
        return TkCollision::rayIntersectsTriangle(ray, v0, v1, v2, point);
    }

} // namespace toolkit
