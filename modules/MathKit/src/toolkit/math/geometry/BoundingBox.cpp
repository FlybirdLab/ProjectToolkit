/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * References:
 *    http://dev.theomader.com/transform-bounding-boxes/
 */
#include "BoundingBox.hpp"

#include "BoundingSphere.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/math/Matrix.hpp"
#include "toolkit/math/Transform.hpp"

namespace toolkit
{

    const BoundingBox BoundingBox::kEmpty = (Vec3(kMaxF32), Vec3(kMinF32));
    const BoundingBox BoundingBox::kZero(Vec3(0.0f));

    TkString BoundingBox::toString() const
    {
        return TkString::format("{}", *this);
    }

    void BoundingBox::getCorners(FVec3 corners[8]) const
    {
        corners[0] = FVec3((float)minimum.x, (float)maximum.y, (float)maximum.z);
        corners[1] = FVec3((float)maximum.x, (float)maximum.y, (float)maximum.z);
        corners[2] = FVec3((float)maximum.x, (float)minimum.y, (float)maximum.z);
        corners[3] = FVec3((float)minimum.x, (float)minimum.y, (float)maximum.z);
        corners[4] = FVec3((float)minimum.x, (float)maximum.y, (float)minimum.z);
        corners[5] = FVec3((float)maximum.x, (float)maximum.y, (float)minimum.z);
        corners[6] = FVec3((float)maximum.x, (float)minimum.y, (float)minimum.z);
        corners[7] = FVec3((float)minimum.x, (float)minimum.y, (float)minimum.z);
    }

    void BoundingBox::getCorners(DVec3 corners[8]) const
    {
        corners[0] = DVec3(minimum.x, maximum.y, maximum.z);
        corners[1] = DVec3(maximum.x, maximum.y, maximum.z);
        corners[2] = DVec3(maximum.x, minimum.y, maximum.z);
        corners[3] = DVec3(minimum.x, minimum.y, maximum.z);
        corners[4] = DVec3(minimum.x, maximum.y, minimum.z);
        corners[5] = DVec3(maximum.x, maximum.y, minimum.z);
        corners[6] = DVec3(maximum.x, minimum.y, minimum.z);
        corners[7] = DVec3(minimum.x, minimum.y, minimum.z);
    }

    real BoundingBox::getVolume() const
    {
        Vec3 size;
        Vec3::subtract(maximum, minimum, size);
        return size.x * size.y * size.z;
    }

    Vec3 BoundingBox::getSize() const
    {
        Vec3 size;
        Vec3::subtract(maximum, minimum, size);
        return size;
    }

    void BoundingBox::setSize(const Vec3 &value)
    {
        Vec3 center;
        getCenter(center);
        const Vec3 sizeHalf = value * 0.5f;
        minimum = center - sizeHalf;
        maximum = center + sizeHalf;
    }

    void BoundingBox::setCenter(const Vec3 &value)
    {
        const Vec3 sizeHalf = getSize() * 0.5f;
        minimum = value - sizeHalf;
        maximum = value + sizeHalf;
    }

    void BoundingBox::merge(const Vec3 &point)
    {
        Vec3::min(minimum, point, minimum);
        Vec3::max(maximum, point, maximum);
    }

    void BoundingBox::merge(const BoundingBox &box)
    {
        Vec3::min(minimum, box.minimum, minimum);
        Vec3::max(maximum, box.maximum, maximum);
    }

    BoundingBox BoundingBox::makeOffsetted(const Vec3 &offset) const
    {
        BoundingBox result;
        result.minimum = minimum + offset;
        result.maximum = maximum + offset;
        return result;
    }

    void BoundingBox::fromPoints(const FVec3 *points, i32 pointsCount, BoundingBox &result)
    {
        TK_ASSERT(points && pointsCount > 0);
        FVec3 min = points[0];
        FVec3 max = points[0];
        for (i32 i = 1; i < pointsCount; i++)
        {
            FVec3::min(min, points[i], min);
            FVec3::max(max, points[i], max);
        }
        result = BoundingBox(min, max);
    }

    void BoundingBox::fromPoints(const DVec3 *points, i32 pointsCount, BoundingBox &result)
    {
        TK_ASSERT(points && pointsCount > 0);
        DVec3 min = points[0];
        DVec3 max = points[0];
        for (i32 i = 1; i < pointsCount; i++)
        {
            DVec3::min(min, points[i], min);
            DVec3::max(max, points[i], max);
        }
        result = BoundingBox((Vec3)min, (Vec3)max);
    }

    void BoundingBox::fromSphere(const BoundingSphere &sphere, BoundingBox &result)
    {
        result = BoundingBox(
            Vec3(sphere.center.x - sphere.radius, sphere.center.y - sphere.radius, sphere.center.z - sphere.radius),
            Vec3(sphere.center.x + sphere.radius, sphere.center.y + sphere.radius, sphere.center.z + sphere.radius));
    }

    BoundingBox BoundingBox::fromSphere(const BoundingSphere &sphere)
    {
        BoundingBox result;
        fromSphere(sphere, result);
        return result;
    }

    BoundingBox BoundingBox::transform(const BoundingBox &box, const Mat &matrix)
    {
        BoundingBox result;
        transform(box, matrix, result);
        return result;
    }

    BoundingBox BoundingBox::makeOffsetted(const BoundingBox &box, const Vec3 &offset)
    {
        BoundingBox result;
        result.minimum = box.minimum + offset;
        result.maximum = box.maximum + offset;
        return result;
    }

    BoundingBox BoundingBox::makeScaled(const BoundingBox &box, real scale)
    {
        Vec3 size;
        Vec3::subtract(box.maximum, box.minimum, size);
        Vec3 sizeHalf = size * 0.5f;
        const Vec3 center = box.minimum + sizeHalf;
        sizeHalf = sizeHalf * scale;
        return BoundingBox(center - sizeHalf, center + sizeHalf);
    }

    void BoundingBox::transform(const BoundingBox &box, const Mat &matrix, BoundingBox &result)
    {
        const auto right = matrix.getRight();
        const auto xa = right * box.minimum.x;
        const auto xb = right * box.maximum.x;

        const auto up = matrix.getUp();
        const auto ya = up * box.minimum.y;
        const auto yb = up * box.maximum.y;

        const auto backward = matrix.getBackward();
        const auto za = backward * box.minimum.z;
        const auto zb = backward * box.maximum.z;

        const auto translation = matrix.getTranslation();
        const auto min = Vec3::min(xa, xb) + Vec3::min(ya, yb) + Vec3::min(za, zb) + translation;
        const auto max = Vec3::max(xa, xb) + Vec3::max(ya, yb) + Vec3::max(za, zb) + translation;
        result = BoundingBox(min, max);
    }

    void BoundingBox::transform(const BoundingBox &box, const Transform &transform, BoundingBox &result)
    {
        const auto right = FVec3::transform(FVec3::kRight, transform.orientation);
        const auto xa = right * box.minimum.x;
        const auto xb = right * box.maximum.x;

        const auto up = FVec3::transform(FVec3::kUp, transform.orientation);
        const auto ya = up * box.minimum.y;
        const auto yb = up * box.maximum.y;

        const auto backward = FVec3::transform(FVec3::kBackward, transform.orientation);
        const auto za = backward * box.minimum.z;
        const auto zb = backward * box.maximum.z;

        const auto min = Vec3::min(xa, xb) + Vec3::min(ya, yb) + Vec3::min(za, zb) + transform.translation;
        const auto max = Vec3::max(xa, xb) + Vec3::max(ya, yb) + Vec3::max(za, zb) + transform.translation;
        result = BoundingBox(min, max);
    }

}