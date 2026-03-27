/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * References:
 *    http://www.chadvernon.com/blog/resources/directx9/frustum-culling/
 */

#include "BoundingFrustum.hpp"

#include "BoundingBox.hpp"
#include "BoundingSphere.hpp"
#include "toolkit/types/String.hpp"

namespace toolkit
{

    TkString BoundingFrustum::toString() const
    {
        return TkString::format("%s", *this);
    }

    void BoundingFrustum::setMatrix(const Mat &matrix)
    {
        // Set matrix
        this->matrix = matrix;

        // Left plane
        this->pLeft.normal.x = matrix.m14 + matrix.m11;
        this->pLeft.normal.y = matrix.m24 + matrix.m21;
        this->pLeft.normal.z = matrix.m34 + matrix.m31;
        this->pLeft.d = matrix.m44 + matrix.m41;
        this->pLeft.normalize();

        // Right plane
        this->pRight.normal.x = matrix.m14 - matrix.m11;
        this->pRight.normal.y = matrix.m24 - matrix.m21;
        this->pRight.normal.z = matrix.m34 - matrix.m31;
        this->pRight.d = matrix.m44 - matrix.m41;
        this->pRight.normalize();

        // Top plane
        this->pTop.normal.x = matrix.m14 - matrix.m12;
        this->pTop.normal.y = matrix.m24 - matrix.m22;
        this->pTop.normal.z = matrix.m34 - matrix.m32;
        this->pTop.d = matrix.m44 - matrix.m42;
        this->pTop.normalize();

        // Bottom plane
        this->pBottom.normal.x = matrix.m14 + matrix.m12;
        this->pBottom.normal.y = matrix.m24 + matrix.m22;
        this->pBottom.normal.z = matrix.m34 + matrix.m32;
        this->pBottom.d = matrix.m44 + matrix.m42;
        this->pBottom.normalize();

        // Near plane
        this->pNear.normal.x = matrix.m13;
        this->pNear.normal.y = matrix.m23;
        this->pNear.normal.z = matrix.m33;
        this->pNear.d = matrix.m43;
        this->pNear.normalize();

        // Far plane
        this->pFar.normal.x = matrix.m14 - matrix.m13;
        this->pFar.normal.y = matrix.m24 - matrix.m23;
        this->pFar.normal.z = matrix.m34 - matrix.m33;
        this->pFar.d = matrix.m44 - matrix.m43;
        this->pFar.normalize();
    }

    Plane BoundingFrustum::getPlane(i32 index) const
    {
        if (index > 5)
            return Plane();
        return this->planes[index];
    }

    static Vec3 Get3PlanesInterPoint(const Plane &p1, const Plane &p2, const Plane &p3)
    {
        const Vec3 n2Xn3 = Vec3::cross(p2.normal, p3.normal);
        const Vec3 n3Xn1 = Vec3::cross(p3.normal, p1.normal);
        const Vec3 n1Xn2 = Vec3::cross(p1.normal, p2.normal);
        const real div1 = Vec3::dot(p1.normal, n2Xn3);
        const real div2 = Vec3::dot(p2.normal, n3Xn1);
        const real div3 = Vec3::dot(p3.normal, n1Xn2);
        if (TkMath::isZero(div1 * div2 * div3))
            return Vec3::kZero;
        return n2Xn3 * (-p1.d / div1) - n3Xn1 * (p2.d / div2) - n1Xn2 * (p3.d / div3);
    }

    void BoundingFrustum::getCorners(FVec3 corners[8]) const
    {
        corners[0] = Get3PlanesInterPoint(this->pNear, this->pBottom, this->pRight);
        corners[1] = Get3PlanesInterPoint(this->pNear, this->pTop, this->pRight);
        corners[2] = Get3PlanesInterPoint(this->pNear, this->pTop, this->pLeft);
        corners[3] = Get3PlanesInterPoint(this->pNear, this->pBottom, this->pLeft);
        corners[4] = Get3PlanesInterPoint(this->pFar, this->pBottom, this->pRight);
        corners[5] = Get3PlanesInterPoint(this->pFar, this->pTop, this->pRight);
        corners[6] = Get3PlanesInterPoint(this->pFar, this->pTop, this->pLeft);
        corners[7] = Get3PlanesInterPoint(this->pFar, this->pBottom, this->pLeft);
    }

    void BoundingFrustum::getCorners(DVec3 corners[8]) const
    {
        corners[0] = Get3PlanesInterPoint(this->pNear, this->pBottom, this->pRight);
        corners[1] = Get3PlanesInterPoint(this->pNear, this->pTop, this->pRight);
        corners[2] = Get3PlanesInterPoint(this->pNear, this->pTop, this->pLeft);
        corners[3] = Get3PlanesInterPoint(this->pNear, this->pBottom, this->pLeft);
        corners[4] = Get3PlanesInterPoint(this->pFar, this->pBottom, this->pRight);
        corners[5] = Get3PlanesInterPoint(this->pFar, this->pTop, this->pRight);
        corners[6] = Get3PlanesInterPoint(this->pFar, this->pTop, this->pLeft);
        corners[7] = Get3PlanesInterPoint(this->pFar, this->pBottom, this->pLeft);
    }

    void BoundingFrustum::getBox(BoundingBox &result) const
    {
        Vec3 corners[8];
        getCorners(corners);
        BoundingBox::fromPoints(corners, 8, result);
    }

    void BoundingFrustum::getSphere(BoundingSphere &result) const
    {
        Vec3 corners[8];
        getCorners(corners);
        BoundingSphere::fromPoints(corners, 8, result);
    }

    float BoundingFrustum::getWidthAtDepth(float depth) const
    {
        const float hAngle = TkMath::PI / 2.0f - TkMath::acos((float)Vec3::dot(this->pNear.normal, this->pLeft.normal));
        return TkMath::tan(hAngle) * depth * 2.0f;
    }

    float BoundingFrustum::getHeightAtDepth(float depth) const
    {
        const float vAngle = TkMath::PI / 2.0f - TkMath::acos((float)Vec3::dot(this->pNear.normal, this->pTop.normal));
        return TkMath::tan(vAngle) * depth * 2.0f;
    }

    ContainmentType BoundingFrustum::contains(const Vec3 &point) const
    {
        PlaneIntersectionType result = PlaneIntersectionType::kFront;
        for (i32 i = 0; i < 6; i++)
        {
            const PlaneIntersectionType planeResult = this->planes[i].intersects(point);
            switch (planeResult)
            {
            case PlaneIntersectionType::kBack:
                return ContainmentType::kDisjoint;
            case PlaneIntersectionType::kIntersecting:
                result = PlaneIntersectionType::kIntersecting;
                break;
            default:
                break;
            }
        }
        switch (result)
        {
        case PlaneIntersectionType::kIntersecting:
            return ContainmentType::kIntersects;
        default:
            return ContainmentType::kContains;
        }
    }

    ContainmentType BoundingFrustum::contains(const BoundingSphere &sphere) const
    {
        auto result = PlaneIntersectionType::kFront;
        for (i32 i = 0; i < 6; i++)
        {
            const PlaneIntersectionType planeResult = this->planes[i].intersects(sphere);
            switch (planeResult)
            {
            case PlaneIntersectionType::kBack:
                return ContainmentType::kDisjoint;
            case PlaneIntersectionType::kIntersecting:
                result = PlaneIntersectionType::kIntersecting;
                break;
            default:
                break;
            }
        }
        switch (result)
        {
        case PlaneIntersectionType::kIntersecting:
            return ContainmentType::kIntersects;
        default:
            return ContainmentType::kContains;
        }
    }

    bool BoundingFrustum::intersects(const BoundingSphere &sphere) const
    {
        for (i32 i = 0; i < 6; i++)
        {
            const real distance = Vec3::dot(this->planes[i].normal, sphere.center) + this->planes[i].d;
            if (distance < -sphere.radius)
                return false;
        }
        return true;
    }
}