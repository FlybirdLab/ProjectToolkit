/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/math/Vector2.hpp>

namespace toolkit {

    struct Mat;
    struct Rectangle;

    // Describes the viewport dimensions.
    struct TK_API Viewport
    {
        union
        {
            struct
            {
                float x;
                float y;
            };

            FVec2 loc;
        };

        union
        {
            struct
            {
                float w;
                float h;
            };

            FVec2 s;
        };

        float minDepth;
        float maxDepth;

        Viewport() = default;
        Viewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f)
            : x(x)
            , y(y)
            , w(width)
            , h(height)
            , minDepth(minDepth)
            , maxDepth(maxDepth) {
        }
        explicit Viewport(const FVec2& size)
            : x(0)
            , y(0)
            , w(size.x)
            , h(size.y)
            , minDepth(0.0f)
            , maxDepth(1.0f) {
        }
        Viewport(const Rectangle& bounds);

        TkString toString() const;

        // Gets the aspect ratio used by the viewport.
        float getAspectRatio() const
        {
            return h != 0.0f ? w / h : 0.0f;
        }

        // Gets the size of the viewport.
        Rectangle getBounds() const;
        // Sets the size of the viewport.
        void setBounds(const Rectangle& bounds);

        bool operator==(const Viewport& other) const
        {
            return x == other.x && y == other.y && w == other.w && h == other.h && minDepth == other.minDepth && maxDepth == other.maxDepth;
        }

        bool operator!=(const Viewport& other) const
        {
            return x != other.x || y != other.y || w != other.w || h != other.h || minDepth != other.minDepth || maxDepth != other.maxDepth;
        }

        // Projects a 3D vector from object space into screen space.
        void project(const Vec3& source, const Mat& vp, Vec3& result) const;
        // Converts a screen space point into a corresponding point in world space.
        void unproject(const Vec3& source, const Mat& ivp, Vec3& result) const;
    };


    template<>
    struct TIsPODType<Viewport>
    {
        enum { Value = true };
    };
}

FORMAT_CUSTOM(toolkit::Viewport, "X:%g Y:%g W:%g H:%g MinDepth:%g MaxDepth:%g", t.x, t.y, t.w, t.h, t.minDepth, t.maxDepth);
