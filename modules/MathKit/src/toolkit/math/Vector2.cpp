/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "Vector2.hpp"

#include "Vector3.hpp"
#include "Vector4.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/utils/Color.hpp"

namespace toolkit
{

    // Float
    static_assert(sizeof(FVec2) == 8, "Invalid FVec2 type size.");

    template <>
    const FVec2 FVec2::Zero(0.0f);
    template <>
    const FVec2 FVec2::One(1.0f);
    template <>
    const FVec2 FVec2::Half(0.5f);
    template <>
    const FVec2 FVec2::UnitX(1.0f, 0.0f);
    template <>
    const FVec2 FVec2::UnitY(0.0f, 1.0f);
    template <>
    const FVec2 FVec2::Minimum(kMinF32);
    template <>
    const FVec2 FVec2::Maximum(kMaxF32);

    template <>
    FVec2::Vector2Base(const IVec3 &xy)
        : x((float)xy.x), y((float)xy.y)
    {
    }

    template <>
    FVec2::Vector2Base(const IVec4 &xy)
        : x((float)xy.x), y((float)xy.y)
    {
    }

    template <>
    FVec2::Vector2Base(const FVec3 &xy)
        : x(xy.x), y(xy.y)
    {
    }

    template <>
    FVec2::Vector2Base(const FVec4 &xy)
        : x(xy.x), y(xy.y)
    {
    }

    template <>
    FVec2::Vector2Base(const DVec3 &xy)
        : x((float)xy.x), y((float)xy.y)
    {
    }

    template <>
    FVec2::Vector2Base(const DVec4 &xy)
        : x((float)xy.x), y((float)xy.y)
    {
    }

    template <>
    FVec2::Vector2Base(const Color &color)
        : x(color.r), y(color.g)
    {
    }

    template <>
    TkString FVec2::toString() const
    {
        return TkString::format("%s", *this);
    }

    template <>
    float FVec2::triangleArea(const FVec2 &v0, const FVec2 &v1, const FVec2 &v2)
    {
        return TkMath::abs((v0.x * (v1.y - v2.y) + v1.x * (v2.y - v0.y) + v2.x * (v0.y - v1.y)) / 2);
    }

    template <>
    float FVec2::angle(const FVec2 &from, const FVec2 &to)
    {
        const float dot_ = TkMath::clamp(dot(normalize(from), normalize(to)), -1.0f, 1.0f);
        if (TkMath::abs(dot_) > 1.0f - TkMath::ZeroTolerance)
            return dot_ > 0.0f ? 0.0f : TkMath::PI;
        return TkMath::acos(dot_);
    }

    // Double
    static_assert(sizeof(DVec2) == 16, "Invalid DVec2 type size.");

    template <>
    const DVec2 DVec2::Zero(0.0);
    template <>
    const DVec2 DVec2::One(1.0);
    template <>
    const DVec2 DVec2::UnitX(1.0, 0.0);
    template <>
    const DVec2 DVec2::UnitY(0.0, 1.0);
    template <>
    const DVec2 DVec2::Minimum(kMinF64);
    template <>
    const DVec2 DVec2::Maximum(kMaxF64);

    template <>
    DVec2::Vector2Base(const IVec3 &xy)
        : x((double)xy.x), y((double)xy.y)
    {
    }

    template <>
    DVec2::Vector2Base(const IVec4 &xy)
        : x((double)xy.x), y((double)xy.y)
    {
    }

    template <>
    DVec2::Vector2Base(const FVec3 &xy)
        : x((double)xy.x), y((double)xy.y)
    {
    }

    template <>
    DVec2::Vector2Base(const FVec4 &xy)
        : x((i32)xy.x), y((double)xy.y)
    {
    }

    template <>
    DVec2::Vector2Base(const DVec3 &xy)
        : x(xy.x), y(xy.y)
    {
    }

    template <>
    DVec2::Vector2Base(const DVec4 &xy)
        : x(xy.x), y(xy.y)
    {
    }

    template <>
    DVec2::Vector2Base(const Color &color)
        : x((double)color.r), y((double)color.g)
    {
    }

    template <>
    TkString DVec2::toString() const
    {
        return TkString::format("%s", *this);
    }

    template <>
    double DVec2::triangleArea(const DVec2 &v0, const DVec2 &v1, const DVec2 &v2)
    {
        return TkMathD::abs((v0.x * (v1.y - v2.y) + v1.x * (v2.y - v0.y) + v2.x * (v0.y - v1.y)) / 2);
    }

    template <>
    double DVec2::angle(const DVec2 &from, const DVec2 &to)
    {
        const double dot_ = TkMath::clamp(dot(normalize(from), normalize(to)), -1.0, 1.0);
        if (TkMathD::abs(dot_) > 1.0 - TkMath::ZeroTolerance)
            return dot_ > 0.0 ? 0.0 : TkMath::PI;
        return TkMath::acos(dot_);
    }

    // Int
    static_assert(sizeof(IVec2) == 8, "Invalid IVec2 type size.");

    template <>
    const IVec2 IVec2::Zero(0);
    template <>
    const IVec2 IVec2::One(1);
    template <>
    const IVec2 IVec2::UnitX(1, 0);
    template <>
    const IVec2 IVec2::UnitY(0, 1);
    template <>
    const IVec2 IVec2::Minimum(kMinI32);
    template <>
    const IVec2 IVec2::Maximum(kMaxI32);

    template <>
    IVec2::Vector2Base(const IVec3 &xy)
        : x(xy.x), y(xy.y)
    {
    }

    template <>
    IVec2::Vector2Base(const IVec4 &xy)
        : x(xy.x), y(xy.y)
    {
    }

    template <>
    IVec2::Vector2Base(const FVec3 &xy)
        : x((i32)xy.x), y((i32)xy.y)
    {
    }

    template <>
    IVec2::Vector2Base(const FVec4 &xy)
        : x((i32)xy.x), y((i32)xy.y)
    {
    }

    template <>
    IVec2::Vector2Base(const DVec3 &xy)
        : x((i32)xy.x), y((i32)xy.y)
    {
    }

    template <>
    IVec2::Vector2Base(const DVec4 &xy)
        : x((i32)xy.x), y((i32)xy.y)
    {
    }

    template <>
    IVec2::Vector2Base(const Color &color)
        : x((i32)color.r), y((i32)color.g)
    {
    }

    template <>
    TkString IVec2::toString() const
    {
        return TkString::format("%s", *this);
    }

    template <>
    i32 IVec2::triangleArea(const IVec2 &v0, const IVec2 &v1, const IVec2 &v2)
    {
        return TkMath::abs((v0.x * (v1.y - v2.y) + v1.x * (v2.y - v0.y) + v2.x * (v0.y - v1.y)) / 2);
    }

    template <>
    i32 IVec2::angle(const IVec2 &from, const IVec2 &to)
    {
        return 0;
    }
}