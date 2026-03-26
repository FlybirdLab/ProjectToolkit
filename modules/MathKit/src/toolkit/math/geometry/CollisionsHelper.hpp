/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/math/common.hpp>

namespace toolkit
{

    // Describes how one bounding volume contains another.
    enum class ContainmentType
    {
        // The two bounding volumes don't intersect at all.
        kDisjoint,
        // One bounding volume completely contains another.
        kContains,
        // The two bounding volumes overlap.
        kIntersects
    };

    // Describes the result of an intersection with a plane in three dimensions
    enum class PlaneIntersectionType
    {
        // The object is behind the plane.
        kBack,
        // The object is in front of the plane.
        kFront,
        // The object is intersecting the plane.
        kIntersecting
    };

    class TkCollision
    {
    public:
        // Determines the closest point between a point and a line.
        static void closestPointPointLine(const FVec2 &point, const FVec2 &p0, const FVec2 &p1, FVec2 &result);
        static FVec2 closestPointPointLine(const FVec2 &point, const FVec2 &p0, const FVec2 &p1);
        static void closestPointPointLine(const FVec3 &point, const FVec3 &p0, const FVec3 &p1, FVec3 &result);
        static FVec3 closestPointPointLine(const FVec3 &point, const FVec3 &p0, const FVec3 &p1);

    public:
        // Determines the closest point between a point and a triangle.
        static void closestPointPointTriangle(const FVec3 &point, const FVec3 &vertex1, const FVec3 &vertex2, const FVec3 &vertex3, FVec3 &result);
        static FVec3 closestPointPointTriangle(const FVec3 &point, const FVec3 &vertex1, const FVec3 &vertex2, const FVec3 &vertex3);

        // Determines the closest point between a and a point.
        static void closestPointPlanePoint(const Plane &plane, const FVec3 &point, FVec3 &result);
        static FVec3 closestPointPlanePoint(const Plane &plane, const FVec3 &point);
        static void closestPointBoxPoint(const BoundingBox &box, const FVec3 &point, FVec3 &result);
        static FVec3 closestPointBoxPoint(const BoundingBox &box, const FVec3 &point);
        static void closestPointRectanglePoint(const Rectangle &rect, const FVec2 &point, FVec2 &result);
        static FVec2 closestPointRectanglePoint(const Rectangle &rect, const FVec2 &point);
        static void closestPointSpherePoint(const BoundingSphere &sphere, const FVec3 &point, FVec3 &result);
        static FVec3 closestPointSpherePoint(const BoundingSphere &sphere, const FVec3 &point);

    public:
        // Determines the closest point between a and a
        static void closestPointSphereSphere(const BoundingSphere &sphere1, const BoundingSphere &sphere2, FVec3 &result);
        static FVec3 closestPointSphereSphere(const BoundingSphere &sphere1, const BoundingSphere &sphere2);
        static real distancePlanePoint(const Plane &plane, const FVec3 &point);
        static real distanceBoxPoint(const BoundingBox &box, const FVec3 &point);
        static real distanceBoxBox(const BoundingBox &box1, const BoundingBox &box2);
        static real distanceSpherePoint(const BoundingSphere &sphere, const FVec3 &point);
        static real distanceSphereSphere(const BoundingSphere &sphere1, const BoundingSphere &sphere2);

        static bool rayIntersectsPoint(const Ray &ray, const FVec3 &point);
        static bool rayIntersectsRay(const Ray &ray1, const Ray &ray2, FVec3 &point);

    public:
        static bool rayIntersectsPlane(const Ray &ray, const Plane &plane, real &distance);
        static bool rayIntersectsPlane(const Ray &ray, const Plane &plane, FVec3 &point);

        static bool rayIntersectsTriangle(const Ray &ray, const FVec3 &vertex1, const FVec3 &vertex2, const FVec3 &vertex3, real &distance);
        static bool rayIntersectsTriangle(const Ray &ray, const FVec3 &vertex1, const FVec3 &vertex2, const FVec3 &vertex3, real &distance, FVec3 &normal);
        static bool rayIntersectsTriangle(const Ray &ray, const FVec3 &vertex1, const FVec3 &vertex2, const FVec3 &vertex3, FVec3 &point);

        static bool rayIntersectsBox(const Ray &ray, const BoundingBox &box, real &distance);
        static bool rayIntersectsBox(const Ray &ray, const BoundingBox &box, real &distance, FVec3 &normal);
        static bool rayIntersectsBox(const Ray &ray, const BoundingBox &box, FVec3 &point);

        static bool rayIntersectsSphere(const Ray &ray, const BoundingSphere &sphere, real &distance);
        static bool rayIntersectsSphere(const Ray &ray, const BoundingSphere &sphere, real &distance, FVec3 &normal);
        static bool rayIntersectsSphere(const Ray &ray, const BoundingSphere &sphere, FVec3 &point);

        static PlaneIntersectionType planeIntersectsPoint(const Plane &plane, const FVec3 &point);

    public:
        static bool planeIntersectsPlane(const Plane &plane1, const Plane &plane2);
        static bool planeIntersectsPlane(const Plane &plane1, const Plane &plane2, Ray &line);

        static PlaneIntersectionType planeIntersectsTriangle(const Plane &plane, const FVec3 &vertex1, const FVec3 &vertex2, const FVec3 &vertex3);
        static PlaneIntersectionType planeIntersectsBox(const Plane &plane, const BoundingBox &box);
        static PlaneIntersectionType planeIntersectsSphere(const Plane &plane, const BoundingSphere &sphere);

        static bool boxIntersectsBox(const BoundingBox &box1, const BoundingBox &box2);
        static bool boxIntersectsSphere(const BoundingBox &box, const BoundingSphere &sphere);

        static bool sphereIntersectsTriangle(const BoundingSphere &sphere, const FVec3 &vertex1, const FVec3 &vertex2, const FVec3 &vertex3);
        static bool sphereIntersectsSphere(const BoundingSphere &sphere1, const BoundingSphere &sphere2);

        static ContainmentType boxContainsPoint(const BoundingBox &box, const FVec3 &point);
        static ContainmentType boxContainsBox(const BoundingBox &box1, const BoundingBox &box2);
        static ContainmentType boxContainsSphere(const BoundingBox &box, const BoundingSphere &sphere);

        static ContainmentType sphereContainsPoint(const BoundingSphere &sphere, const FVec3 &point);
        static ContainmentType sphereContainsTriangle(const BoundingSphere &sphere, const FVec3 &vertex1, const FVec3 &vertex2, const FVec3 &vertex3);
        static ContainmentType sphereContainsBox(const BoundingSphere &sphere, const BoundingBox &box);
        static ContainmentType sphereContainsSphere(const BoundingSphere &sphere1, const BoundingSphere &sphere2);

        static bool frustumIntersectsBox(const BoundingFrustum &frustum, const BoundingBox &box);
        static ContainmentType frustumContainsBox(const BoundingFrustum &frustum, const BoundingBox &box);

        static bool lineIntersectsLine(const FVec2 &l1p1, const FVec2 &l1p2, const FVec2 &l2p1, const FVec2 &l2p2);
        static bool lineIntersectsRect(const FVec2 &p1, const FVec2 &p2, const Rectangle &rect);

        static Vec2 lineHitsBox(const FVec3 &lineStart, const FVec3 &lineEnd, const FVec3 &boxMin, const FVec3 &boxMax);
        static bool isPointInTriangle(const FVec2 &point, const FVec2 &a, const FVec2 &b, const FVec2 &c);
    };
}