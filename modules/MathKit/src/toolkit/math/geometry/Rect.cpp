/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Rect.hpp"

#include "toolkit/types/String.hpp"

namespace toolkit
{

    Rectangle Rectangle::kEmpty(0, 0, 0, 0);

    TkString Rectangle::toString() const
    {
        return TkString::format("%s", *this);
    }

    bool Rectangle::contains(const FVec2 &location) const
    {
        return location.x >= this->location.x && location.y >= this->location.y && (location.x <= this->location.x + size.x && location.y <= this->location.y + size.y);
    }

    bool Rectangle::contains(const Rectangle &value) const
    {
        return location.x <= value.location.x && value.getRight() <= getRight() && location.y <= value.location.y && value.getBottom() <= getBottom();
    }

    bool Rectangle::intersects(const Rectangle &value) const
    {
        return value.location.x <= getRight() && location.x <= value.getRight() && value.location.y <= getBottom() && location.y <= value.getBottom();
    }

    void Rectangle::offset(float x, float y)
    {
        location.x += x;
        location.y += y;
    }

    void Rectangle::offset(const FVec2 &offset)
    {
        location += offset;
    }

    Rectangle Rectangle::makeOffsetted(const FVec2 &offset) const
    {
        return Rectangle(location + offset, size);
    }

    void Rectangle::expand(float toExpand)
    {
        location -= FVec2(toExpand, toExpand);
        size += FVec2(toExpand * 2.0f, toExpand * 2.0f);
    }

    Rectangle Rectangle::makeExpanded(float toExpand) const
    {
        return Rectangle(location - FVec2(toExpand, toExpand), size + FVec2(toExpand * 2.0f, toExpand * 2.0f));
    }

    void Rectangle::scale(float scale)
    {
        const FVec2 toExpand = size * (scale - 1.0f) * 0.5f;
        location -= toExpand;
        size += toExpand * 2.0f;
    }

    Rectangle Rectangle::makeScaled(float scale) const
    {
        const FVec2 toExpand = size * (scale - 1.0f) * 0.5f;
        return Rectangle(location - toExpand, size + toExpand * 2.0f);
    }

    Rectangle Rectangle::unionRectangle(const Rectangle &a, const FVec2 &b)
    {
        const float left = TkMath::min(a.getLeft(), b.x);
        const float right = TkMath::max(a.getRight(), b.x);
        const float top = TkMath::min(a.getTop(), b.y);
        const float bottom = TkMath::max(a.getBottom(), b.y);
        return Rectangle(left, top, TkMath::max(right - left, 0.0f), TkMath::max(bottom - top, 0.0f));
    }

    Rectangle Rectangle::unionRectangle(const Rectangle &a, const Rectangle &b)
    {
        const float left = TkMath::min(a.getLeft(), b.getLeft());
        const float right = TkMath::max(a.getRight(), b.getRight());
        const float top = TkMath::min(a.getTop(), b.getTop());
        const float bottom = TkMath::max(a.getBottom(), b.getBottom());
        return Rectangle(left, top, TkMath::max(right - left, 0.0f), TkMath::max(bottom - top, 0.0f));
    }

    Rectangle Rectangle::shared(const Rectangle &a, const Rectangle &b)
    {
        const float left = TkMath::max(a.getLeft(), b.getLeft());
        const float right = TkMath::min(a.getRight(), b.getRight());
        const float top = TkMath::max(a.getTop(), b.getTop());
        const float bottom = TkMath::min(a.getBottom(), b.getBottom());
        return Rectangle(left, top, TkMath::max(right - left, 0.0f), TkMath::max(bottom - top, 0.0f));
    }

    Rectangle Rectangle::fromPoints(const FVec2 &p1, const FVec2 &p2)
    {
        const FVec2 upperLeft = FVec2::min(p1, p2);
        const FVec2 rightBottom = FVec2::max(p1, p2);
        return Rectangle(upperLeft, TkMath::max(rightBottom - upperLeft, FVec2::kZero));
    }

    Rectangle Rectangle::fromPoints(const FVec2 *points, i32 pointsCount)
    {
        TK_ASSERT(pointsCount > 0);
        FVec2 upperLeft = points[0];
        FVec2 rightBottom = points[0];
        for (i32 i = 1; i < pointsCount; i++)
        {
            upperLeft = FVec2::min(upperLeft, points[i]);
            rightBottom = FVec2::max(rightBottom, points[i]);
        }
        return Rectangle(upperLeft, TkMath::max(rightBottom - upperLeft, FVec2::kZero));
    }
}
