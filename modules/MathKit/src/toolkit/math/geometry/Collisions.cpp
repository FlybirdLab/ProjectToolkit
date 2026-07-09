/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * References:
 *     Real-Time Collision Detection by Christer Ericson
 *     Jorgy343
 *     RayIntersectsSphere
 *     Real-Time Rendering, Third Edition
 *     http://www.cs.virginia.edu/~gfx/Courses/2003/ImageSynthesis/papers/Acceleration/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
 */

#include "Collisions.hpp"

#include "toolkit/math/Vector2.hpp"
#include "toolkit/math/Vector3.hpp"
#include "Ray.hpp"
#include "Plane.hpp"
#include "BoundingBox.hpp"
#include "BoundingSphere.hpp"
#include "BoundingFrustum.hpp"
#include "Rect.hpp"

namespace toolkit
{
    namespace
    {
        struct Vec2Lite
        {
            real x;
            real y;
        };

        static Vec2Lite projectToPlane2D(const Vec3 &v, int axis)
        {
            switch (axis)
            {
            case 0:
                return {v.y, v.z};
            case 1:
                return {v.x, v.z};
            default:
                return {v.x, v.y};
            }
        }

        static real cross2d(const Vec2Lite &a, const Vec2Lite &b)
        {
            return a.x * b.y - a.y * b.x;
        }

        static bool pointInTriangle2D(const Vec2Lite &p, const Vec2Lite &a, const Vec2Lite &b, const Vec2Lite &c)
        {
            const Vec2Lite ap{a.x - p.x, a.y - p.y};
            const Vec2Lite bp{b.x - p.x, b.y - p.y};
            const Vec2Lite cp{c.x - p.x, c.y - p.y};
            const bool orientation = cross2d(ap, bp) > 0;
            if ((cross2d(bp, cp) > 0) != orientation)
                return false;
            return (cross2d(cp, ap) > 0) == orientation;
        }

        static bool coplanarRayIntersectsTriangle(const Ray &ray, const Vec3 &vertex1, const Vec3 &vertex2, const Vec3 &vertex3, real &distance)
        {
            Vec3 edge1, edge2, planeNormal;
            Vec3::subtract(vertex2, vertex1, edge1);
            Vec3::subtract(vertex3, vertex1, edge2);
            Vec3::cross(edge1, edge2, planeNormal);

            const Vec3 absNormal = planeNormal.getAbsolute();
            int axis = 2;
            if (absNormal.x >= absNormal.y && absNormal.x >= absNormal.z)
                axis = 0;
            else if (absNormal.y >= absNormal.z)
                axis = 1;

            const Vec2Lite p = projectToPlane2D(ray.position, axis);
            const Vec2Lite d = projectToPlane2D(ray.direction, axis);
            const Vec2Lite a = projectToPlane2D(vertex1, axis);
            const Vec2Lite b = projectToPlane2D(vertex2, axis);
            const Vec2Lite c = projectToPlane2D(vertex3, axis);

            if (pointInTriangle2D(p, a, b, c))
            {
                distance = 0.0f;
                return true;
            }

            const Vec2Lite edgesStart[3] = {a, b, c};
            const Vec2Lite edgesEnd[3] = {b, c, a};
            real bestDistance = kMaxReal;
            bool hit = false;

            for (int i = 0; i < 3; ++i)
            {
                const Vec2Lite s = edgesStart[i];
                const Vec2Lite e = edgesEnd[i];
                const Vec2Lite edge{e.x - s.x, e.y - s.y};
                const Vec2Lite delta{s.x - p.x, s.y - p.y};
                const real denom = cross2d(d, edge);

                if (TkMath::isZero(denom))
                {
                    if (!TkMath::isZero(cross2d(delta, d)))
                        continue;

                    const real dd = d.x * d.x + d.y * d.y;
                    if (TkMath::isZero(dd))
                        continue;

                    const Vec2Lite candidates[2] = {s, e};
                    for (const Vec2Lite &candidate : candidates)
                    {
                        const Vec2Lite rel{candidate.x - p.x, candidate.y - p.y};
                        const real t = (rel.x * d.x + rel.y * d.y) / dd;
                        if (t >= 0.0f && t < bestDistance)
                        {
                            bestDistance = t;
                            hit = true;
                        }
                    }
                    continue;
                }

                const real t = cross2d(delta, edge) / denom;
                const real u = cross2d(delta, d) / denom;
                if (t >= 0.0f && u >= 0.0f && u <= 1.0f && t < bestDistance)
                {
                    bestDistance = t;
                    hit = true;
                }
            }

            distance = hit ? bestDistance : 0.0f;
            return hit;
        }
    }


    void TkCollision::closestPointPointLine(const FVec2 &point, const FVec2 &p0, const FVec2 &p1, FVec2 &result)
    {
        const FVec2 p = point - p0;
        FVec2 n = p1 - p0;

        const float length = n.length();
        if (length < 1e-10f)
        {
            // Both points are the same, just give any
            result = p0;
            return;
        }
        n /= length;

        const float dot = FVec2::dot(n, p);
        if (dot <= 0.0f)
        {
            // Before first point
            result = p0;
        }
        else if (dot >= length)
        {
            // After first point
            result = p1;
        }
        else
        {
            // Inside
            result = p0 + n * dot;
        }
    }

    FVec2 TkCollision::closestPointPointLine(const FVec2 &point, const FVec2 &p0, const FVec2 &p1)
    {
        FVec2 result;
        closestPointPointLine(point, p0, p1, result);
        return result;
    }

    void TkCollision::closestPointPointLine(const Vec3 &point, const Vec3 &p0, const Vec3 &p1, Vec3 &result)
    {
        const Vec3 p = point - p0;
        Vec3 n = p1 - p0;
        const real length = n.length();
        if (length < 1e-10f)
        {
            result = p0;
            return;
        }
        n /= length;
        const real dot = Vec3::dot(n, p);
        if (dot <= 0.0f)
        {
            result = p0;
            return;
        }
        if (dot >= length)
        {
            result = p1;
            return;
        }
        result = p0 + n * dot;
    }

    Vec3 TkCollision::closestPointPointLine(const Vec3 &point, const Vec3 &p0, const Vec3 &p1)
    {
        Vec3 result;
        closestPointPointLine(point, p0, p1, result);
        return result;
    }

    void TkCollision::closestPointPointTriangle(const Vec3 &point, const Vec3 &vertex1, const Vec3 &vertex2, const Vec3 &vertex3, Vec3 &result)
    {
        // Check if P in vertex region outside A
        const Vec3 ab = vertex2 - vertex1;
        const Vec3 ac = vertex3 - vertex1;
        const Vec3 ap = point - vertex1;

        const real d1 = Vec3::dot(ab, ap);
        const real d2 = Vec3::dot(ac, ap);
        if (d1 <= 0.0f && d2 <= 0.0f)
        {
            result = vertex1; // Barycentric coordinates (1,0,0)
            return;
        }

        // Check if P in vertex region outside B
        const Vec3 bp = point - vertex2;
        const real d3 = Vec3::dot(ab, bp);
        const real d4 = Vec3::dot(ac, bp);
        if (d3 >= 0.0f && d4 <= d3)
        {
            result = vertex2; // Barycentric coordinates (0,1,0)
            return;
        }

        // Check if P in edge region of AB, if so return projection of P onto AB
        const real vc = d1 * d4 - d3 * d2;
        if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
        {
            const real v = d1 / (d1 - d3);
            result = vertex1 + v * ab; // Barycentric coordinates (1-v,v,0)
            return;
        }

        // Check if P in vertex region outside C
        const Vec3 cp = point - vertex3;
        const real d5 = Vec3::dot(ab, cp);
        const real d6 = Vec3::dot(ac, cp);
        if (d6 >= 0.0f && d5 <= d6)
        {
            result = vertex3; // Barycentric coordinates (0,0,1)
            return;
        }

        // Check if P in edge region of AC, if so return projection of P onto AC
        const real vb = d5 * d2 - d1 * d6;
        if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
        {
            const real w = d2 / (d2 - d6);
            result = vertex1 + w * ac; // Barycentric coordinates (1-w,0,w)
            return;
        }

        // Check if P in edge region of BC, if so return projection of P onto BC
        const real va = d3 * d6 - d5 * d4;
        if (va <= 0.0f && d4 - d3 >= 0.0f && d5 - d6 >= 0.0f)
        {
            const real w = (d4 - d3) / (d4 - d3 + (d5 - d6));
            result = vertex2 + w * (vertex3 - vertex2); // Barycentric coordinates (0,1-w,w)
            return;
        }

        // P inside face region. Compute Q through its Barycentric coordinates (u,v,w)
        const real denom = 1.0f / (va + vb + vc);
        const real v2 = vb * denom;
        const real w2 = vc * denom;
        result = vertex1 + ab * v2 + ac * w2; //= u*vertex1 + v*vertex2 + w*vertex3, u = va * denom = 1.0f - v - w
    }

    Vec3 TkCollision::closestPointPointTriangle(const Vec3 &point, const Vec3 &vertex1, const Vec3 &vertex2, const Vec3 &vertex3)
    {
        Vec3 result;
        closestPointPointTriangle(point, vertex1, vertex2, vertex3, result);
        return result;
    }

    void TkCollision::closestPointPlanePoint(const Plane &plane, const Vec3 &point, Vec3 &result)
    {
        const real dot = Vec3::dot(plane.normal, point) + plane.d;
        const real t = dot;
        result = point - t * plane.normal;
    }

    Vec3 TkCollision::closestPointPlanePoint(const Plane &plane, const Vec3 &point)
    {
        Vec3 result;
        closestPointPlanePoint(plane, point, result);
        return result;
    }

    void TkCollision::closestPointBoxPoint(const BoundingBox &box, const Vec3 &point, Vec3 &result)
    {
        Vec3 temp;
        Vec3::max(point, box.minimum, temp);
        Vec3::min(temp, box.maximum, result);
    }

    Vec3 TkCollision::closestPointBoxPoint(const BoundingBox &box, const Vec3 &point)
    {
        Vec3 result;
        closestPointBoxPoint(box, point, result);
        return result;
    }

    void TkCollision::closestPointRectanglePoint(const Rectangle &rect, const FVec2 &point, FVec2 &result)
    {
        FVec2 temp, end;
        FVec2::add(rect.location, rect.size, end);
        FVec2::max(point, rect.location, temp);
        FVec2::min(temp, end, result);
    }

    FVec2 TkCollision::closestPointRectanglePoint(const Rectangle &rect, const FVec2 &point)
    {
        FVec2 result;
        closestPointRectanglePoint(rect, point, result);
        return result;
    }

    void TkCollision::closestPointSpherePoint(const BoundingSphere &sphere, const Vec3 &point, Vec3 &result)
    {
        // Get the unit direction from the sphere's center to the point.
        Vec3::subtract(point, sphere.center, result);
        result.normalize();

        // Multiply the unit direction by the sphere's radius to get a vector
        // the length of the sphere.
        result *= sphere.radius;

        // Add the sphere's center to the direction to get a point on the sphere.
        result += sphere.center;
    }

    Vec3 TkCollision::closestPointSpherePoint(const BoundingSphere &sphere, const Vec3 &point)
    {
        Vec3 result;
        closestPointSpherePoint(sphere, point, result);
        return result;
    }

    void TkCollision::closestPointSphereSphere(const BoundingSphere &sphere1, const BoundingSphere &sphere2, Vec3 &result)
    {
        // Get the unit direction from the first sphere's center to the second sphere's center.
        Vec3::subtract(sphere2.center, sphere1.center, result);
        result.normalize();

        // Multiply the unit direction by the first sphere's radius to get a vector
        // the length of the first sphere.
        result *= sphere1.radius;

        // Add the first sphere's center to the direction to get a point on the first sphere.
        result += sphere1.center;
    }

    Vec3 TkCollision::closestPointSphereSphere(const BoundingSphere &sphere1, const BoundingSphere &sphere2)
    {
        Vec3 result;
        closestPointSphereSphere(sphere1, sphere2, result);
        return result;
    }

    real TkCollision::distancePlanePoint(const Plane &plane, const Vec3 &point)
    {
        const real dot = Vec3::dot(plane.normal, point);
        return dot + plane.d;
    }

    real TkCollision::distanceBoxPoint(const BoundingBox &box, const Vec3 &point)
    {
        real distance = 0.0f;

        if (point.x < box.minimum.x)
            distance += (box.minimum.x - point.x) * (box.minimum.x - point.x);
        if (point.x > box.maximum.x)
            distance += (point.x - box.maximum.x) * (point.x - box.maximum.x);

        if (point.y < box.minimum.y)
            distance += (box.minimum.y - point.y) * (box.minimum.y - point.y);
        if (point.y > box.maximum.y)
            distance += (point.y - box.maximum.y) * (point.y - box.maximum.y);

        if (point.z < box.minimum.z)
            distance += (box.minimum.z - point.z) * (box.minimum.z - point.z);
        if (point.z > box.maximum.z)
            distance += (point.z - box.maximum.z) * (point.z - box.maximum.z);

        return TkMath::sqrt(distance);
    }

    real TkCollision::distanceBoxBox(const BoundingBox &box1, const BoundingBox &box2)
    {
        real distance = 0.0f;

        // Distance for X
        if (box1.minimum.x > box2.maximum.x)
        {
            const real delta = box2.maximum.x - box1.minimum.x;
            distance += delta * delta;
        }
        else if (box2.minimum.x > box1.maximum.x)
        {
            const real delta = box1.maximum.x - box2.minimum.x;
            distance += delta * delta;
        }

        // Distance for Y
        if (box1.minimum.y > box2.maximum.y)
        {
            const real delta = box2.maximum.y - box1.minimum.y;
            distance += delta * delta;
        }
        else if (box2.minimum.y > box1.maximum.y)
        {
            const real delta = box1.maximum.y - box2.minimum.y;
            distance += delta * delta;
        }

        // Distance for Z
        if (box1.minimum.z > box2.maximum.z)
        {
            const real delta = box2.maximum.z - box1.minimum.z;
            distance += delta * delta;
        }
        else if (box2.minimum.z > box1.maximum.z)
        {
            const real delta = box1.maximum.z - box2.minimum.z;
            distance += delta * delta;
        }

        return TkMath::sqrt(distance);
    }

    real TkCollision::distanceSpherePoint(const BoundingSphere &sphere, const Vec3 &point)
    {
        real distance = Vec3::distance(sphere.center, point);
        distance -= sphere.radius;
        return TkMath::max<real>(distance, 0.0f);
    }

    real TkCollision::distanceSphereSphere(const BoundingSphere &sphere1, const BoundingSphere &sphere2)
    {
        real distance = Vec3::distance(sphere1.center, sphere2.center);
        distance -= sphere1.radius + sphere2.radius;
        return TkMath::max<real>(distance, 0.0f);
    }

    bool TkCollision::rayIntersectsPoint(const Ray &ray, const Vec3 &point)
    {
        Vec3 m;
        Vec3::subtract(ray.position, point, m);

        // Same thing as RayIntersectsSphere except that the radius of the sphere (point)
        // is the epsilon for zero.
        const real b = Vec3::dot(m, ray.direction);
        const real c = Vec3::dot(m, m) - TkMath::ZeroTolerance;

        if (c > 0.0f && b > 0.0f)
            return false;

        const real discriminant = b * b - c;

        if (discriminant < 0.0f)
            return false;

        return true;
    }

    bool TkCollision::rayIntersectsRay(const Ray &ray1, const Ray &ray2, Vec3 &point)
    {
        Vec3 cross;

        Vec3::cross(ray1.direction, ray2.direction, cross);
        real denominator = cross.length();

        // Lines are parallel
        if (TkMath::isZero(denominator))
        {
            // Lines are parallel and on top of each other
            if (TkMath::nearEqual(ray2.position.x, ray1.position.x) &&
                TkMath::nearEqual(ray2.position.y, ray1.position.y) &&
                TkMath::nearEqual(ray2.position.z, ray1.position.z))
            {
                point = Vec3::kZero;
                return true;
            }
        }

        denominator = denominator * denominator;

        // 3x3 matrix for the first ray
        const real m11 = ray2.position.x - ray1.position.x;
        const real m12 = ray2.position.y - ray1.position.y;
        const real m13 = ray2.position.z - ray1.position.z;
        real m21 = ray2.direction.x;
        real m22 = ray2.direction.y;
        real m23 = ray2.direction.z;
        const real m31 = cross.x;
        const real m32 = cross.y;
        const real m33 = cross.z;

        // Determinant of first matrix
        const real dets =
            m11 * m22 * m33 +
            m12 * m23 * m31 +
            m13 * m21 * m32 -
            m11 * m23 * m32 -
            m12 * m21 * m33 -
            m13 * m22 * m31;

        // 3x3 matrix for the second ray
        m21 = ray1.direction.x;
        m22 = ray1.direction.y;
        m23 = ray1.direction.z;

        // Determinant of the second matrix
        const real dett =
            m11 * m22 * m33 +
            m12 * m23 * m31 +
            m13 * m21 * m32 -
            m11 * m23 * m32 -
            m12 * m21 * m33 -
            m13 * m22 * m31;

        // t values of the point of intersection
        const real s = dets / denominator;
        const real t = dett / denominator;

        // The points of intersection.
        const Vec3 point1 = ray1.position + s * ray1.direction;
        const Vec3 point2 = ray2.position + t * ray2.direction;

        // If the points are not equal, no intersection has occurred
        if (!TkMath::nearEqual(point2.x, point1.x) ||
            !TkMath::nearEqual(point2.y, point1.y) ||
            !TkMath::nearEqual(point2.z, point1.z))
        {
            point = Vec3::kZero;
            return false;
        }

        point = point1;
        return true;
    }

    bool TkCollision::rayIntersectsPlane(const Ray &ray, const Plane &plane, real &distance)
    {
        const real direction = Vec3::dot(plane.normal, ray.direction);

        if (TkMath::isZero(direction))
        {
            distance = 0.0f;
            return false;
        }

        const real position = Vec3::dot(plane.normal, ray.position);
        distance = (-plane.d - position) / direction;

        if (distance < Plane::kDistanceEpsilon)
        {
            distance = 0.0f;
            return false;
        }

        return true;
    }

    bool TkCollision::rayIntersectsPlane(const Ray &ray, const Plane &plane, Vec3 &point)
    {
        real distance;
        if (!rayIntersectsPlane(ray, plane, distance))
        {
            point = Vec3::kZero;
            return false;
        }

        point = ray.position + ray.direction * distance;
        return true;
    }

    bool TkCollision::rayIntersectsTriangle(const Ray &ray, const Vec3 &vertex1, const Vec3 &vertex2, const Vec3 &vertex3, real &distance)
    {
        distance = 0.0f;

        // Compute vectors along two edges of the triangle
        Vec3 edge1, edge2;

        // Edge 1
        edge1.x = vertex2.x - vertex1.x;
        edge1.y = vertex2.y - vertex1.y;
        edge1.z = vertex2.z - vertex1.z;

        // Edge2
        edge2.x = vertex3.x - vertex1.x;
        edge2.y = vertex3.y - vertex1.y;
        edge2.z = vertex3.z - vertex1.z;

        // Cross product of ray direction and edge2 - first part of determinant
        Vec3 directionCrossEdge2;
        directionCrossEdge2.x = ray.direction.y * edge2.z - ray.direction.z * edge2.y;
        directionCrossEdge2.y = ray.direction.z * edge2.x - ray.direction.x * edge2.z;
        directionCrossEdge2.z = ray.direction.x * edge2.y - ray.direction.y * edge2.x;

        // Compute the determinant (dot product of edge1 and the first part of determinant)
        const real determinant = edge1.x * directionCrossEdge2.x + edge1.y * directionCrossEdge2.y + edge1.z * directionCrossEdge2.z;

        // If the ray is parallel to the triangle plane, there is no collision
        // This also means that we are not culling, the ray may hit both the
        // back and the front of the triangle.
        if (TkMath::isZero(determinant))
        {
            const real planeDistance = Vec3::dot(Vec3::cross(edge1, edge2), ray.position - vertex1);
            if (!TkMath::isZero(planeDistance))
                return false;
            return coplanarRayIntersectsTriangle(ray, vertex1, vertex2, vertex3, distance);
        }

        const real inverseDeterminant = 1.0f / determinant;

        // Calculate the U parameter of the intersection point
        Vec3 distanceVector;
        distanceVector.x = ray.position.x - vertex1.x;
        distanceVector.y = ray.position.y - vertex1.y;
        distanceVector.z = ray.position.z - vertex1.z;

        real triangleU = distanceVector.x * directionCrossEdge2.x + distanceVector.y * directionCrossEdge2.y + distanceVector.z * directionCrossEdge2.z;
        triangleU *= inverseDeterminant;

        // Make sure it is inside the triangle
        if (triangleU < 0.0f || triangleU > 1.0f)
        {
            return false;
        }

        // Calculate the V parameter of the intersection point
        Vec3 distanceCrossEdge1;
        distanceCrossEdge1.x = distanceVector.y * edge1.z - distanceVector.z * edge1.y;
        distanceCrossEdge1.y = distanceVector.z * edge1.x - distanceVector.x * edge1.z;
        distanceCrossEdge1.z = distanceVector.x * edge1.y - distanceVector.y * edge1.x;

        real triangleV = ray.direction.x * distanceCrossEdge1.x + ray.direction.y * distanceCrossEdge1.y + ray.direction.z * distanceCrossEdge1.z;
        triangleV *= inverseDeterminant;

        // Make sure it is inside the triangle
        if (triangleV < 0.0f || triangleU + triangleV > 1.0f)
        {
            return false;
        }

        // Compute the distance along the ray to the triangle
        real rayDistance = edge2.x * distanceCrossEdge1.x + edge2.y * distanceCrossEdge1.y + edge2.z * distanceCrossEdge1.z;
        rayDistance *= inverseDeterminant;

        // Check if the triangle is behind the ray origin
        if (rayDistance < 0.0f)
        {
            return false;
        }

        distance = rayDistance;
        return true;
    }

    bool TkCollision::rayIntersectsTriangle(const Ray &ray, const Vec3 &vertex1, const Vec3 &vertex2, const Vec3 &vertex3, real &distance, Vec3 &normal)
    {
        distance = 0.0f;
        normal = Vec3::kUp;

        // Compute vectors along two edges of the triangle
        Vec3 edge1, edge2;

        // Edge 1
        edge1.x = vertex2.x - vertex1.x;
        edge1.y = vertex2.y - vertex1.y;
        edge1.z = vertex2.z - vertex1.z;

        // Edge2
        edge2.x = vertex3.x - vertex1.x;
        edge2.y = vertex3.y - vertex1.y;
        edge2.z = vertex3.z - vertex1.z;

        // Cross product of ray direction and edge2 - first part of determinant
        Vec3 directionCrossEdge2;
        directionCrossEdge2.x = ray.direction.y * edge2.z - ray.direction.z * edge2.y;
        directionCrossEdge2.y = ray.direction.z * edge2.x - ray.direction.x * edge2.z;
        directionCrossEdge2.z = ray.direction.x * edge2.y - ray.direction.y * edge2.x;

        // Compute the determinant (dot product of edge1 and the first part of determinant)
        const real determinant = edge1.x * directionCrossEdge2.x + edge1.y * directionCrossEdge2.y + edge1.z * directionCrossEdge2.z;

        // If the ray is parallel to the triangle plane, there is no collision
        // This also means that we are not culling, the ray may hit both the
        // back and the front of the triangle.
        if (TkMath::isZero(determinant))
        {
            const real planeDistance = Vec3::dot(Vec3::cross(edge1, edge2), ray.position - vertex1);
            if (!TkMath::isZero(planeDistance))
                return false;

            if (!coplanarRayIntersectsTriangle(ray, vertex1, vertex2, vertex3, distance))
                return false;

            Vec3::cross(edge1, edge2, normal);
            normal.normalize();
            return true;
        }

        const real inverseDeterminant = 1.0f / determinant;

        // Calculate the U parameter of the intersection point
        Vec3 distanceVector;
        distanceVector.x = ray.position.x - vertex1.x;
        distanceVector.y = ray.position.y - vertex1.y;
        distanceVector.z = ray.position.z - vertex1.z;

        real triangleU = distanceVector.x * directionCrossEdge2.x + distanceVector.y * directionCrossEdge2.y + distanceVector.z * directionCrossEdge2.z;
        triangleU *= inverseDeterminant;

        // Make sure it is inside the triangle
        if (triangleU < 0.0f || triangleU > 1.0f)
        {
            return false;
        }

        // Calculate the V parameter of the intersection point
        Vec3 distanceCrossEdge1;
        distanceCrossEdge1.x = distanceVector.y * edge1.z - distanceVector.z * edge1.y;
        distanceCrossEdge1.y = distanceVector.z * edge1.x - distanceVector.x * edge1.z;
        distanceCrossEdge1.z = distanceVector.x * edge1.y - distanceVector.y * edge1.x;

        real triangleV = ray.direction.x * distanceCrossEdge1.x + ray.direction.y * distanceCrossEdge1.y + ray.direction.z * distanceCrossEdge1.z;
        triangleV *= inverseDeterminant;

        // Make sure it is inside the triangle
        if (triangleV < 0.0f || triangleU + triangleV > 1.0f)
        {
            return false;
        }

        // Compute the distance along the ray to the triangle
        real rayDistance = edge2.x * distanceCrossEdge1.x + edge2.y * distanceCrossEdge1.y + edge2.z * distanceCrossEdge1.z;
        rayDistance *= inverseDeterminant;

        // Check if the triangle is behind the ray origin
        if (rayDistance < 0.0f)
        {
            return false;
        }

        Vec3::cross(edge1, edge2, normal);
        normal.normalize();

        distance = rayDistance;

        return true;
    }

    bool TkCollision::rayIntersectsTriangle(const Ray &ray, const Vec3 &vertex1, const Vec3 &vertex2, const Vec3 &vertex3, Vec3 &point)
    {
        real distance;
        if (!rayIntersectsTriangle(ray, vertex1, vertex2, vertex3, distance))
        {
            point = Vec3::kZero;
            return false;
        }
        point = ray.position + ray.direction * distance;
        return true;
    }

    bool TkCollision::rayIntersectsBox(const Ray &ray, const BoundingBox &box, real &distance)
    {
        distance = 0.0f;
        real tmax = kMaxReal;

        if (TkMath::isZero(ray.direction.x))
        {
            if (ray.position.x < box.minimum.x || ray.position.x > box.maximum.x)
            {
                distance = 0.0f;
                return false;
            }
        }
        else
        {
            const real inverse = 1.0f / ray.direction.x;
            real t1 = (box.minimum.x - ray.position.x) * inverse;
            real t2 = (box.maximum.x - ray.position.x) * inverse;

            if (t1 > t2)
            {
                const real temp = t1;
                t1 = t2;
                t2 = temp;
            }

            distance = TkMath::max(t1, distance);
            tmax = TkMath::min(t2, tmax);

            if (distance > tmax)
            {
                distance = 0.0f;
                return false;
            }
        }

        if (TkMath::isZero(ray.direction.y))
        {
            if (ray.position.y < box.minimum.y || ray.position.y > box.maximum.y)
            {
                distance = 0.0f;
                return false;
            }
        }
        else
        {
            const real inverse = 1.0f / ray.direction.y;
            real t1 = (box.minimum.y - ray.position.y) * inverse;
            real t2 = (box.maximum.y - ray.position.y) * inverse;

            if (t1 > t2)
            {
                const real temp = t1;
                t1 = t2;
                t2 = temp;
            }

            distance = TkMath::max(t1, distance);
            tmax = TkMath::min(t2, tmax);

            if (distance > tmax)
            {
                distance = 0.0f;
                return false;
            }
        }

        if (TkMath::isZero(ray.direction.z))
        {
            if (ray.position.z < box.minimum.z || ray.position.z > box.maximum.z)
            {
                distance = 0.0f;
                return false;
            }
        }
        else
        {
            const real inverse = 1.0f / ray.direction.z;
            real t1 = (box.minimum.z - ray.position.z) * inverse;
            real t2 = (box.maximum.z - ray.position.z) * inverse;

            if (t1 > t2)
            {
                const real temp = t1;
                t1 = t2;
                t2 = temp;
            }

            distance = TkMath::max(t1, distance);
            tmax = TkMath::min(t2, tmax);

            if (distance > tmax)
            {
                distance = 0.0f;
                return false;
            }
        }

        return true;
    }

    bool TkCollision::rayIntersectsBox(const Ray &ray, const BoundingBox &box, real &distance, Vec3 &normal)
    {
        if (!rayIntersectsBox(ray, box, distance))
        {
            normal = Vec3::kUp;
            return false;
        }

        // TODO: optimize this

        const Vec3 point = ray.position + ray.direction * distance;
        Vec3 size;
        Vec3::subtract(box.maximum, box.minimum, size);
        const Vec3 center = box.minimum + size * 0.5f;
        const Vec3 localPoint = point - center;

        real dMin = kMaxReal;

        real d = TkMath::abs(size.x - TkMath::abs(localPoint.x));
        if (d < dMin)
        {
            dMin = d;
            normal = Vec3(TkMath::sign(localPoint.x), 0, 0);
        }

        d = TkMath::abs(size.y - TkMath::abs(localPoint.y));
        if (d < dMin)
        {
            dMin = d;
            normal = Vec3(0, TkMath::sign(localPoint.y), 0);
        }

        d = TkMath::abs(size.z - TkMath::abs(localPoint.z));
        if (d < dMin)
        {
            normal = Vec3(0, 0, TkMath::sign(localPoint.z));
        }

        return true;
    }

    bool TkCollision::rayIntersectsBox(const Ray &ray, const BoundingBox &box, Vec3 &point)
    {
        real distance;
        if (!rayIntersectsBox(ray, box, distance))
        {
            point = Vec3::kZero;
            return false;
        }

        point = ray.position + ray.direction * distance;
        return true;
    }

    bool TkCollision::rayIntersectsSphere(const Ray &ray, const BoundingSphere &sphere, real &distance)
    {
        Vec3 m;
        Vec3::subtract(ray.position, sphere.center, m);

        const real b = Vec3::dot(m, ray.direction);
        const real c = Vec3::dot(m, m) - sphere.radius * sphere.radius;

        if (c > 0.0f && b > 0.0f)
        {
            distance = 0.0f;
            return false;
        }

        const real discriminant = b * b - c;

        if (discriminant < 0.0f)
        {
            distance = 0.0f;
            return false;
        }

        distance = -b - TkMath::sqrt(discriminant);

        if (distance < 0.0f)
            distance = 0.0f;

        return true;
    }

    bool TkCollision::rayIntersectsSphere(const Ray &ray, const BoundingSphere &sphere, real &distance, Vec3 &normal)
    {
        if (!rayIntersectsSphere(ray, sphere, distance))
        {
            normal = Vec3::kUp;
            return false;
        }
        const Vec3 point = ray.position + ray.direction * distance;
        normal = Vec3::normalize(point - sphere.center);
        return true;
    }

    bool TkCollision::rayIntersectsSphere(const Ray &ray, const BoundingSphere &sphere, Vec3 &point)
    {
        real distance;
        if (!rayIntersectsSphere(ray, sphere, distance))
        {
            point = Vec3::kZero;
            return false;
        }
        point = ray.position + ray.direction * distance;
        return true;
    }

    PlaneIntersectionType TkCollision::planeIntersectsPoint(const Plane &plane, const Vec3 &point)
    {
        const real distance = Vec3::dot(plane.normal, point) + plane.d;
        if (distance > Plane::kDistanceEpsilon)
            return PlaneIntersectionType::kFront;
        if (distance < -Plane::kDistanceEpsilon)
            return PlaneIntersectionType::kBack;
        return PlaneIntersectionType::kIntersecting;
    }

    bool TkCollision::planeIntersectsPlane(const Plane &plane1, const Plane &plane2)
    {
        Vec3 direction;
        Vec3::cross(plane1.normal, plane2.normal, direction);

        // If direction is the zero vector, the planes are parallel and possibly
        // coincident. It is not an intersection. The dot product will tell us.
        const real denominator = Vec3::dot(direction, direction);

        return !TkMath::isZero(denominator);
    }

    bool TkCollision::planeIntersectsPlane(const Plane &plane1, const Plane &plane2, Ray &line)
    {
        Vec3 direction;
        Vec3::cross(plane1.normal, plane2.normal, direction);

        // If direction is the zero vector, the planes are parallel and possibly coincident. It is not an intersection. The dot product will tell us.
        const real denominator = Vec3::dot(direction, direction);

        // We assume the planes are normalized, theconstore the denominator
        // only serves as a parallel and coincident check. Otherwise we need
        // to divide the point by the denominator.
        if (TkMath::isZero(denominator))
        {
            return false;
        }

        Vec3 point;
        const Vec3 temp = plane1.d * plane2.normal - plane2.d * plane1.normal;
        Vec3::cross(temp, direction, point);

        line.position = point;
        line.direction = direction;
        line.direction.normalize();

        return true;
    }

    PlaneIntersectionType TkCollision::planeIntersectsTriangle(const Plane &plane, const Vec3 &vertex1, const Vec3 &vertex2, const Vec3 &vertex3)
    {
        const PlaneIntersectionType test1 = planeIntersectsPoint(plane, vertex1);
        const PlaneIntersectionType test2 = planeIntersectsPoint(plane, vertex2);
        const PlaneIntersectionType test3 = planeIntersectsPoint(plane, vertex3);

        if (test1 == PlaneIntersectionType::kFront && test2 == PlaneIntersectionType::kFront && test3 == PlaneIntersectionType::kFront)
            return PlaneIntersectionType::kFront;

        if (test1 == PlaneIntersectionType::kBack && test2 == PlaneIntersectionType::kBack && test3 == PlaneIntersectionType::kBack)
            return PlaneIntersectionType::kBack;

        return PlaneIntersectionType::kIntersecting;
    }

    PlaneIntersectionType TkCollision::planeIntersectsBox(const Plane &plane, const BoundingBox &box)
    {
        Vec3 min;
        Vec3 max;

        max.x = plane.normal.x >= 0.0f ? box.minimum.x : box.maximum.x;
        max.y = plane.normal.y >= 0.0f ? box.minimum.y : box.maximum.y;
        max.z = plane.normal.z >= 0.0f ? box.minimum.z : box.maximum.z;
        min.x = plane.normal.x >= 0.0f ? box.maximum.x : box.minimum.x;
        min.y = plane.normal.y >= 0.0f ? box.maximum.y : box.minimum.y;
        min.z = plane.normal.z >= 0.0f ? box.maximum.z : box.minimum.z;

        real distance = Vec3::dot(plane.normal, max);
        if (distance + plane.d > Plane::kDistanceEpsilon)
            return PlaneIntersectionType::kFront;
        distance = Vec3::dot(plane.normal, min);
        if (distance + plane.d < Plane::kDistanceEpsilon)
            return PlaneIntersectionType::kBack;
        return PlaneIntersectionType::kIntersecting;
    }

    PlaneIntersectionType TkCollision::planeIntersectsSphere(const Plane &plane, const BoundingSphere &sphere)
    {
        real distance = Vec3::dot(plane.normal, sphere.center);
        distance += plane.d;

        if (distance > sphere.radius)
            return PlaneIntersectionType::kFront;
        if (distance < -sphere.radius)
            return PlaneIntersectionType::kBack;
        return PlaneIntersectionType::kIntersecting;
    }

    bool TkCollision::boxIntersectsBox(const BoundingBox &box1, const BoundingBox &box2)
    {
        if (box1.minimum.x > box2.maximum.x || box2.minimum.x > box1.maximum.x)
            return false;
        if (box1.minimum.y > box2.maximum.y || box2.minimum.y > box1.maximum.y)
            return false;
        if (box1.minimum.z > box2.maximum.z || box2.minimum.z > box1.maximum.z)
            return false;
        return true;
    }

    bool TkCollision::boxIntersectsSphere(const BoundingBox &box, const BoundingSphere &sphere)
    {
        Vec3 vector;
        Vec3::clamp(sphere.center, box.minimum, box.maximum, vector);
        const real distance = Vec3::distanceSquared(sphere.center, vector);
        return distance <= sphere.radius * sphere.radius;
    }

    bool TkCollision::sphereIntersectsTriangle(const BoundingSphere &sphere, const Vec3 &vertex1, const Vec3 &vertex2, const Vec3 &vertex3)
    {
        Vec3 point;
        closestPointPointTriangle(sphere.center, vertex1, vertex2, vertex3, point);
        const Vec3 v = point - sphere.center;

        const real dot = Vec3::dot(v, v);

        return dot <= sphere.radius * sphere.radius;
    }

    bool TkCollision::sphereIntersectsSphere(const BoundingSphere &sphere1, const BoundingSphere &sphere2)
    {
        const real radiisum = sphere1.radius + sphere2.radius;
        return Vec3::distanceSquared(sphere1.center, sphere2.center) <= radiisum * radiisum;
    }

    ContainmentType TkCollision::boxContainsPoint(const BoundingBox &box, const Vec3 &point)
    {
        if (box.minimum.x <= point.x && box.maximum.x >= point.x &&
            box.minimum.y <= point.y && box.maximum.y >= point.y &&
            box.minimum.z <= point.z && box.maximum.z >= point.z)
        {
            return ContainmentType::kContains;
        }

        return ContainmentType::kDisjoint;
    }

    ContainmentType TkCollision::boxContainsBox(const BoundingBox &box1, const BoundingBox &box2)
    {
        if (box1.maximum.x < box2.minimum.x || box1.minimum.x > box2.maximum.x)
            return ContainmentType::kDisjoint;

        if (box1.maximum.y < box2.minimum.y || box1.minimum.y > box2.maximum.y)
            return ContainmentType::kDisjoint;

        if (box1.maximum.z < box2.minimum.z || box1.minimum.z > box2.maximum.z)
            return ContainmentType::kDisjoint;

        if (box1.minimum.x <= box2.minimum.x && (box2.maximum.x <= box1.maximum.x && box1.minimum.y <= box2.minimum.y && box2.maximum.y <= box1.maximum.y) &&
            box1.minimum.z <= box2.minimum.z && box2.maximum.z <= box1.maximum.z)
        {
            return ContainmentType::kContains;
        }

        return ContainmentType::kIntersects;
    }

    ContainmentType TkCollision::boxContainsSphere(const BoundingBox &box, const BoundingSphere &sphere)
    {
        Vec3 vector;
        Vec3::clamp(sphere.center, box.minimum, box.maximum, vector);
        const real distance = Vec3::distanceSquared(sphere.center, vector);

        if (distance > sphere.radius * sphere.radius)
            return ContainmentType::kDisjoint;

        if (box.minimum.x + sphere.radius <= sphere.center.x && sphere.center.x <= box.maximum.x - sphere.radius && (box.maximum.x - box.minimum.x > sphere.radius && box.minimum.y + sphere.radius <= sphere.center.y) && (sphere.center.y <= box.maximum.y - sphere.radius && box.maximum.y - box.minimum.y > sphere.radius && (box.minimum.z + sphere.radius <= sphere.center.z && sphere.center.z <= box.maximum.z - sphere.radius && box.maximum.z - box.minimum.z > sphere.radius)))
        {
            return ContainmentType::kContains;
        }

        return ContainmentType::kIntersects;
    }

    ContainmentType TkCollision::sphereContainsPoint(const BoundingSphere &sphere, const Vec3 &point)
    {
        if (Vec3::distanceSquared(point, sphere.center) <= sphere.radius * sphere.radius)
            return ContainmentType::kContains;
        return ContainmentType::kDisjoint;
    }

    ContainmentType TkCollision::sphereContainsTriangle(const BoundingSphere &sphere, const Vec3 &vertex1, const Vec3 &vertex2, const Vec3 &vertex3)
    {
        const ContainmentType test1 = sphereContainsPoint(sphere, vertex1);
        const ContainmentType test2 = sphereContainsPoint(sphere, vertex2);
        const ContainmentType test3 = sphereContainsPoint(sphere, vertex3);

        if (test1 == ContainmentType::kContains && test2 == ContainmentType::kContains && test3 == ContainmentType::kContains)
            return ContainmentType::kContains;

        if (sphereIntersectsTriangle(sphere, vertex1, vertex2, vertex3))
            return ContainmentType::kIntersects;

        return ContainmentType::kDisjoint;
    }

    ContainmentType TkCollision::sphereContainsBox(const BoundingSphere &sphere, const BoundingBox &box)
    {
        Vec3 vector;

        if (!boxIntersectsSphere(box, sphere))
            return ContainmentType::kDisjoint;

        const real radiusSquared = sphere.radius * sphere.radius;

        vector.x = sphere.center.x - box.minimum.x;
        vector.y = sphere.center.y - box.maximum.y;
        vector.z = sphere.center.z - box.maximum.z;
        if (vector.lengthSquared() > radiusSquared)
            return ContainmentType::kIntersects;

        vector.x = sphere.center.x - box.maximum.x;
        vector.y = sphere.center.y - box.maximum.y;
        vector.z = sphere.center.z - box.maximum.z;
        if (vector.lengthSquared() > radiusSquared)
            return ContainmentType::kIntersects;

        vector.x = sphere.center.x - box.maximum.x;
        vector.y = sphere.center.y - box.minimum.y;
        vector.z = sphere.center.z - box.maximum.z;
        if (vector.lengthSquared() > radiusSquared)
            return ContainmentType::kIntersects;

        vector.x = sphere.center.x - box.minimum.x;
        vector.y = sphere.center.y - box.minimum.y;
        vector.z = sphere.center.z - box.maximum.z;
        if (vector.lengthSquared() > radiusSquared)
            return ContainmentType::kIntersects;

        vector.x = sphere.center.x - box.minimum.x;
        vector.y = sphere.center.y - box.maximum.y;
        vector.z = sphere.center.z - box.minimum.z;
        if (vector.lengthSquared() > radiusSquared)
            return ContainmentType::kIntersects;

        vector.x = sphere.center.x - box.maximum.x;
        vector.y = sphere.center.y - box.maximum.y;
        vector.z = sphere.center.z - box.minimum.z;
        if (vector.lengthSquared() > radiusSquared)
            return ContainmentType::kIntersects;

        vector.x = sphere.center.x - box.maximum.x;
        vector.y = sphere.center.y - box.minimum.y;
        vector.z = sphere.center.z - box.minimum.z;
        if (vector.lengthSquared() > radiusSquared)
            return ContainmentType::kIntersects;

        vector.x = sphere.center.x - box.minimum.x;
        vector.y = sphere.center.y - box.minimum.y;
        vector.z = sphere.center.z - box.minimum.z;
        if (vector.lengthSquared() > radiusSquared)
            return ContainmentType::kIntersects;

        return ContainmentType::kContains;
    }

    ContainmentType TkCollision::sphereContainsSphere(const BoundingSphere &sphere1, const BoundingSphere &sphere2)
    {
        const real distance = Vec3::distance(sphere1.center, sphere2.center);
        if (sphere1.radius + sphere2.radius < distance)
            return ContainmentType::kDisjoint;
        if (sphere1.radius - sphere2.radius < distance)
            return ContainmentType::kIntersects;
        return ContainmentType::kContains;
    }

    bool TkCollision::frustumIntersectsBox(const BoundingFrustum &frustum, const BoundingBox &box)
    {
        return frustumContainsBox(frustum, box) != ContainmentType::kDisjoint;
    }

    ContainmentType TkCollision::frustumContainsBox(const BoundingFrustum &frustum, const BoundingBox &box)
    {
        auto result = ContainmentType::kContains;
        for (i32 i = 0; i < 6; i++)
        {
            Plane plane = frustum.planes[i];

            Vec3 p = box.minimum;
            if (plane.normal.x >= 0)
                p.x = box.maximum.x;
            if (plane.normal.y >= 0)
                p.y = box.maximum.y;
            if (plane.normal.z >= 0)
                p.z = box.maximum.z;
            if (Vec3::dot(plane.normal, p) + plane.d < Plane::kDistanceEpsilon)
                return ContainmentType::kDisjoint;

            p = box.maximum;
            if (plane.normal.x >= 0)
                p.x = box.minimum.x;
            if (plane.normal.y >= 0)
                p.y = box.minimum.y;
            if (plane.normal.z >= 0)
                p.z = box.minimum.z;
            if (Vec3::dot(plane.normal, p) + plane.d < Plane::kDistanceEpsilon)
                result = ContainmentType::kIntersects;
        }
        return result;
    }

    bool TkCollision::lineIntersectsLine(const FVec2 &l1p1, const FVec2 &l1p2, const FVec2 &l2p1, const FVec2 &l2p2)
    {
        float q = (l1p1.y - l2p1.y) * (l2p2.x - l2p1.x) - (l1p1.x - l2p1.x) * (l2p2.y - l2p1.y);
        const float d = (l1p2.x - l1p1.x) * (l2p2.y - l2p1.y) - (l1p2.y - l1p1.y) * (l2p2.x - l2p1.x);

        if (TkMath::isZero(d))
            return false;

        const float r = q / d;
        q = (l1p1.y - l2p1.y) * (l1p2.x - l1p1.x) - (l1p1.x - l2p1.x) * (l1p2.y - l1p1.y);
        const float s = q / d;

        return !(r < 0 || r > 1 || s < 0 || s > 1);
    }

    bool TkCollision::lineIntersectsRect(const FVec2 &p1, const FVec2 &p2, const Rectangle &rect)
    {
        const float left = rect.getLeft();
        const float right = rect.getRight();
        const float top = rect.getTop();
        const float bottom = rect.getBottom();

        if (rect.contains(p1) || rect.contains(p2))
            return true;

        const float minX = TkMath::min(p1.x, p2.x);
        const float maxX = TkMath::max(p1.x, p2.x);
        const float minY = TkMath::min(p1.y, p2.y);
        const float maxY = TkMath::max(p1.y, p2.y);
        if (maxX < left || minX > right || maxY < top || minY > bottom)
            return false;

        const float dx = p2.x - p1.x;
        const float dy = p2.y - p1.y;
        float t0 = 0.0f;
        float t1 = 1.0f;

        auto clip = [&](float p, float q) -> bool
        {
            if (TkMath::isZero(p))
                return q >= 0.0f;

            const float r = q / p;
            if (p < 0.0f)
            {
                if (r > t1)
                    return false;
                if (r > t0)
                    t0 = r;
            }
            else
            {
                if (r < t0)
                    return false;
                if (r < t1)
                    t1 = r;
            }
            return true;
        };

        return clip(-dx, p1.x - left) &&
               clip(dx, right - p1.x) &&
               clip(-dy, p1.y - top) &&
               clip(dy, bottom - p1.y);
    }

    Vec2 TkCollision::lineHitsBox(const Vec3 &lineStart, const Vec3 &lineEnd, const Vec3 &boxMin, const Vec3 &boxMax)
    {
        const Vec3 invDirection = 1.0f / (lineEnd - lineStart);
        const Vec3 enterIntersection = (boxMin - lineStart) * invDirection;
        const Vec3 exitIntersection = (boxMax - lineStart) * invDirection;
        const Vec3 minIntersections = Vec3::min(enterIntersection, exitIntersection);
        const Vec3 maxIntersections = Vec3::max(enterIntersection, exitIntersection);
        return Vec2(TkMath::saturate(minIntersections.maxValue()), TkMath::saturate(maxIntersections.minValue()));
    }

    bool TkCollision::isPointInTriangle(const FVec2 &point, const FVec2 &a, const FVec2 &b, const FVec2 &c)
    {
        const FVec2 an = a - point;
        const FVec2 bn = b - point;
        const FVec2 cn = c - point;

        const bool orientation = FVec2::cross(an, bn) > 0;

        if (FVec2::cross(bn, cn) > 0 != orientation)
            return false;
        return FVec2::cross(cn, an) > 0 == orientation;
    }
}
