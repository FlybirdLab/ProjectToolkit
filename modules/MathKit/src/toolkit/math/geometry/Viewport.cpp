/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Viewport.hpp"

#include "Rect.hpp"
#include "toolkit/math/Vector3.hpp"
#include "toolkit/math/Matrix.hpp"
#include "toolkit/types/String.hpp"

namespace toolkit
{

    Viewport::Viewport(const Rectangle &bounds)
        : x(bounds.location.x), y(bounds.location.y), w(bounds.size.x), h(bounds.size.y), minDepth(0.0f), maxDepth(1.0f)
    {
    }

    TkString Viewport::toString() const
    {
        return TkString::format("%s", *this);
    }

    Rectangle Viewport::getBounds() const
    {
        return Rectangle(x, y, w, h);
    }

    void Viewport::setBounds(const Rectangle &bounds)
    {
        x = bounds.location.x;
        y = bounds.location.y;
        w = bounds.size.x;
        h = bounds.size.y;
    }

    void Viewport::project(const Vec3 &source, const Mat &vp, Vec3 &result) const
    {
        Vec3::transform(source, vp, result);
        const real a = source.x * vp.m14 + source.y * vp.m24 + source.z * vp.m34 + vp.m44;

        if (!TkMath::isOne(a))
        {
            result /= a;
        }

        result.x = (result.x + 1.0f) * 0.5f * w + x;
        result.y = (-result.y + 1.0f) * 0.5f * h + y;
        result.z = result.z * (maxDepth - minDepth) + minDepth;
    }

    void Viewport::unproject(const Vec3 &source, const Mat &ivp, Vec3 &result) const
    {
        result.x = (source.x - x) / w * 2.0f - 1.0f;
        result.y = -((source.y - y) / h * 2.0f - 1.0f);
        result.z = (source.z - minDepth) / (maxDepth - minDepth);

        const real a = result.x * ivp.m14 + result.y * ivp.m24 + result.z * ivp.m34 + ivp.m44;
        Vec3::transform(result, ivp, result);

        if (!TkMath::isOne(a))
        {
            result /= a;
        }
    }
}