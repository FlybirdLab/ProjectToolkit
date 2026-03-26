/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Ray.hpp"

#include "Viewport.hpp"
#include "toolkit/types/String.hpp"

namespace toolkit
{

    Ray Ray::Identity(Vec3(0, 0, 0), Vec3(0, 0, 1.0f));

    TkString Ray::toString() const
    {
        return TkString::format("%s", *this);
    }

    Vec3 Ray::getPoint(real distance) const
    {
        return position + direction * distance;
    }

    Ray Ray::getPickRay(float x, float y, const Viewport &viewport, const Mat &vp)
    {
        Vec3 nearPoint(x, y, 0.0f);
        Vec3 farPoint(x, y, 1.0f);

        nearPoint = Vec3::unproject(nearPoint, viewport.x, viewport.y, viewport.w, viewport.h, viewport.minDepth, viewport.maxDepth, vp);
        farPoint = Vec3::unproject(farPoint, viewport.x, viewport.y, viewport.w, viewport.h, viewport.minDepth, viewport.maxDepth, vp);

        Vec3 direction = farPoint - nearPoint;
        direction.normalize();

        return Ray(nearPoint, direction);
    }

}