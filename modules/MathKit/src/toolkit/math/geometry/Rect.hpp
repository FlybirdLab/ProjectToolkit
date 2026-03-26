/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/math/Vector2.hpp>

namespace toolkit
{

    // Describes rectangle in 2D space defines by location of its upper-left corner and the size.
    class TK_API Rectangle
    {
    public:
        // The empty rectangle.
        static Rectangle Empty;

    public:
        Rectangle() = default;
        Rectangle(float x, float y, float width, float height)
            : location(x, y), size(width, height)
        {
        }
        Rectangle(const FVec2 &location, const FVec2 &size)
            : location(location), size(size)
        {
        }

    public:
        TkString toString() const;

        // Gets width of the rectangle.
        float getWidth() const
        {
            return size.x;
        }

        // Gets height of the rectangle.
        float getHeight() const
        {
            return size.y;
        }

        // Gets Y coordinate of the top edge of the rectangle.
        float getY() const
        {
            return location.y;
        }

        // Gets Y coordinate of the top edge of the rectangle.
        float getTop() const
        {
            return location.y;
        }

        // Gets Y coordinate of the bottom edge of the rectangle.
        float getBottom() const
        {
            return location.y + size.y;
        }

        // Gets X coordinate of the left edge of the rectangle.
        float getX() const
        {
            return location.x;
        }

        // Gets X coordinate of the left edge of the rectangle.
        float getLeft() const
        {
            return location.x;
        }

        // Gets X coordinate of the right edge of the rectangle.
        float getRight() const
        {
            return location.x + size.x;
        }

        // Gets position of the upper left corner of the rectangle.
        FVec2 getUpperLeft() const
        {
            return location;
        }

        // Gets position of the upper right corner of the rectangle.
        FVec2 getUpperRight() const
        {
            return location + FVec2(size.x, 0);
        }

        // Gets position of the bottom right corner of the rectangle.
        FVec2 getLowerRight() const
        {
            return location + size;
        }

        // Gets position of the bottom left corner of the rectangle.
        FVec2 getLowerLeft() const
        {
            return location + FVec2(0, size.y);
        }

        // Gets position of the upper left corner of the rectangle.
        FVec2 getTopLeft() const
        {
            return location;
        }

        // Gets position of the upper right corner of the rectangle.
        FVec2 getTopRight() const
        {
            return location + FVec2(size.x, 0);
        }

        // Gets position of the bottom right corner of the rectangle.
        FVec2 getBottomRight() const
        {
            return location + size;
        }

        // Gets position of the bottom left corner of the rectangle.
        FVec2 getBottomLeft() const
        {
            return location + FVec2(0, size.y);
        }

        // Gets center position of the rectangle.
        FVec2 getCenter() const
        {
            return location + size * 0.5f;
        }

    public:
        Rectangle operator+(const FVec2 &v) const
        {
            return Rectangle(location + v, size);
        }

        Rectangle operator-(const FVec2 &v) const
        {
            return Rectangle(location - v, size);
        }

        Rectangle &operator+=(const FVec2 &b)
        {
            offset(b);
            return *this;
        }

        Rectangle &operator-=(const FVec2 &b)
        {
            offset(-b);
            return *this;
        }

        Rectangle operator*(float b) const
        {
            return makeScaled(b);
        }

        Rectangle operator/(float b) const
        {
            return makeScaled(1.0f / b);
        }

        Rectangle &operator*=(float b)
        {
            scale(b);
            return *this;
        }

        Rectangle &operator/=(float b)
        {
            scale(1.0f / b);
            return *this;
        }

        // Comparison operators
        bool operator==(const Rectangle &b) const
        {
            return location == b.location && size == b.size;
        }

        bool operator!=(const Rectangle &b) const
        {
            return location != b.location || size != b.size;
        }

        static bool nearEqual(const Rectangle &a, const Rectangle &b)
        {
            return FVec2::nearEqual(a.location, b.location) && FVec2::nearEqual(a.size, b.size);
        }

        static bool nearEqual(const Rectangle &a, const Rectangle &b, float epsilon)
        {
            return FVec2::nearEqual(a.location, b.location, epsilon) && FVec2::nearEqual(a.size, b.size, epsilon);
        }

    public:
        // Checks if rectangle contains given point.
        bool contains(const FVec2 &location) const;
        // Determines whether this rectangle entirely contains a specified rectangle.
        bool contains(const Rectangle &value) const;
        // Determines whether a specified rectangle intersects with this rectangle.
        bool intersects(const Rectangle &value) const;

        // Offsets rectangle position.
        void offset(float x, float y);
        // Offsets rectangle position.
        void offset(const FVec2 &offset);

    public:
        // Make offseted rectangle.
        Rectangle makeOffsetted(const FVec2 &offset) const;

        // Expands rectangle area in all directions by given amount.
        void expand(float toExpand);

        // Makes expanded rectangle area in all directions by given amount.
        Rectangle makeExpanded(float toExpand) const;

        // Scale rectangle area in all directions by given amount.
        void scale(float scale);

        // Makes scaled rectangle area in all directions by given amount.
        Rectangle makeScaled(float scale) const;

    public:
        // Calculates a rectangle that contains the union of rectangle and the arbitrary point.
        static Rectangle unionRectangle(const Rectangle &a, const FVec2 &b);
        // Calculates a rectangle that contains the union of a and b rectangles.
        static Rectangle unionRectangle(const Rectangle &a, const Rectangle &b);

        // Calculates a rectangle that contains the shared part of a and b rectangles.
        static Rectangle shared(const Rectangle &a, const Rectangle &b);

        // Creates rectangle from two points.
        static Rectangle fromPoints(const FVec2 &p1, const FVec2 &p2);
        // Creates rectangle from list of points.
        static Rectangle fromPoints(const FVec2 *points, i32 pointsCount);

    public:
        // Rectangle location (coordinates of the upper-left corner)
        FVec2 location;
        // Rectangle size
        FVec2 size;
    };

    template <>
    struct TIsPODType<Rectangle>
    {
        enum
        {
            Value = true
        };
    };
}

FORMAT_CUSTOM(toolkit::Rectangle, "X:%g Y:%g Width:%g Height:%g", t.location.x, t.location.y, t.size.x, t.size.y);
