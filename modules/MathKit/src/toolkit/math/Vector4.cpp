/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Vector4.hpp"

#include "Vector2.hpp"
#include "Vector3.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/utils/Color.hpp"
#include "geometry/Rect.hpp"
#include "Matrix.hpp"

namespace toolkit
{
    // Float

    static_assert(sizeof(FVec4) == 16, "Invalid FVec4 type size.");

    template <>
    const FVec4 FVec4::Zero(0.0f);
    template <>
    const FVec4 FVec4::One(1.0f);
    template <>
    const FVec4 FVec4::Half(0.5f);
    template <>
    const FVec4 FVec4::UnitX(1.0f, 0.0f, 0.0f, 0.0f);
    template <>
    const FVec4 FVec4::UnitY(0.0f, 1.0f, 0.0f, 0.0f);
    template <>
    const FVec4 FVec4::UnitZ(0.0f, 0.0f, 1.0f, 0.0f);
    template <>
    const FVec4 FVec4::UnitW(0.0f, 0.0f, 0.0f, 1.0f);
    template <>
    const FVec4 FVec4::Minimum(kMinF32);
    template <>
    const FVec4 FVec4::Maximum(kMaxF32);

    template <>
    FVec4::Vector4Base(const FVec2 &xy, float z, float w)
        : x(xy.x), y(xy.y), z(z), w(w)
    {
    }

    template <>
    FVec4::Vector4Base(const FVec2 &xy, const FVec2 &zw)
        : x(xy.x), y(xy.y), z(zw.x), w(zw.y)
    {
    }

    template <>
    FVec4::Vector4Base(const FVec3 &xyz, float w)
        : x(xyz.x), y(xyz.y), z(xyz.z), w(w)
    {
    }

    template <>
    FVec4::Vector4Base(const IVec2 &xy, float z, float w)
        : x((float)xy.x), y((float)xy.y), z(z), w(w)
    {
    }

    template <>
    FVec4::Vector4Base(const IVec3 &xyz, float w)
        : x((float)xyz.x), y((float)xyz.y), z((float)xyz.z), w(w)
    {
    }

    template <>
    FVec4::Vector4Base(const DVec2 &xy, float z, float w)
        : x(static_cast<float>(xy.x)), y(static_cast<float>(xy.y)), z(z), w(w)
    {
    }

    template <>
    FVec4::Vector4Base(const DVec2 &xy, const DVec2 &zw)
        : x(static_cast<float>(xy.x)), y(static_cast<float>(xy.y)), z(static_cast<float>(zw.x)), w(static_cast<float>(zw.y))
    {
    }

    template <>
    FVec4::Vector4Base(const DVec3 &xyz, float w)
        : x(static_cast<float>(xyz.x)), y(static_cast<float>(xyz.y)), z(static_cast<float>(xyz.z)), w(w)
    {
    }

    template <>
    FVec4::Vector4Base(const Color &color)
        : x(color.r), y(color.g), z(color.b), w(color.a)
    {
    }

    template <>
    FVec4::Vector4Base(const Rectangle &rect)
        : x(rect.location.x), y(rect.location.y), z(rect.size.x), w(rect.size.y)
    {
    }

    template <>
    TkString FVec4::toString() const
    {
        return TkString::format("%s", *this);
    }

    template <>
    FVec4 FVec4::transform(const FVec4 &v, const Mat &m)
    {
        return {
            m.values[0][0] * v.raw[0] + m.values[1][0] * v.raw[1] + m.values[2][0] * v.raw[2] + m.values[3][0] * v.raw[3],
            m.values[0][1] * v.raw[0] + m.values[1][1] * v.raw[1] + m.values[2][1] * v.raw[2] + m.values[3][1] * v.raw[3],
            m.values[0][2] * v.raw[0] + m.values[1][2] * v.raw[1] + m.values[2][2] * v.raw[2] + m.values[3][2] * v.raw[3],
            m.values[0][3] * v.raw[0] + m.values[1][3] * v.raw[1] + m.values[2][3] * v.raw[2] + m.values[3][3] * v.raw[3]};
    }

    // Double
    static_assert(sizeof(DVec4) == 32, "Invalid DVec4 type size.");

    template <>
    const DVec4 DVec4::Zero(0.0);
    template <>
    const DVec4 DVec4::One(1.0);
    template <>
    const DVec4 DVec4::UnitX(1.0, 0.0, 0.0, 0.0);
    template <>
    const DVec4 DVec4::UnitY(0.0, 1.0, 0.0, 0.0);
    template <>
    const DVec4 DVec4::UnitZ(0.0, 0.0, 1.0, 0.0);
    template <>
    const DVec4 DVec4::UnitW(0.0, 0.0, 0.0, 1.0);
    template <>
    const DVec4 DVec4::Minimum(kMinF64);
    template <>
    const DVec4 DVec4::Maximum(kMaxF64);

    template <>
    DVec4::Vector4Base(const FVec2 &xy, double z, double w)
        : x((double)xy.x), y((double)xy.y), z(z), w(w)
    {
    }

    template <>
    DVec4::Vector4Base(const FVec2 &xy, const FVec2 &zw)
        : x((double)xy.x), y((double)xy.y), z((double)zw.x), w((double)zw.y)
    {
    }

    template <>
    DVec4::Vector4Base(const FVec3 &xyz, double w)
        : x((double)xyz.x), y((double)xyz.y), z((double)xyz.z), w(w)
    {
    }

    template <>
    DVec4::Vector4Base(const IVec2 &xy, double z, double w)
        : x((double)xy.x), y((double)xy.y), z(z), w(w)
    {
    }

    template <>
    DVec4::Vector4Base(const IVec3 &xyz, double w)
        : x((double)xyz.x), y((double)xyz.y), z((double)xyz.z), w(w)
    {
    }

    template <>
    DVec4::Vector4Base(const DVec2 &xy, double z, double w)
        : x(xy.x), y(xy.y), z(z), w(w)
    {
    }

    template <>
    DVec4::Vector4Base(const DVec2 &xy, const DVec2 &zw)
        : x(xy.x), y(xy.y), z(zw.x), w(zw.y)
    {
    }

    template <>
    DVec4::Vector4Base(const DVec3 &xyz, double w)
        : x((double)xyz.x), y((double)xyz.y), z((double)xyz.z), w(w)
    {
    }

    template <>
    DVec4::Vector4Base(const Color &color)
        : x((double)color.r), y((double)color.g), z((double)color.b), w((double)color.a)
    {
    }

    template <>
    DVec4::Vector4Base(const Rectangle &rect)
        : x((double)rect.location.x), y((double)rect.location.y), z((double)rect.size.x), w((double)rect.size.y)
    {
    }

    template <>
    TkString DVec4::toString() const
    {
        return TkString::format("%s", *this);
    }

    template <>
    DVec4 DVec4::transform(const DVec4 &v, const Mat &m)
    {
        return {
            m.values[0][0] * v.raw[0] + m.values[1][0] * v.raw[1] + m.values[2][0] * v.raw[2] + m.values[3][0] * v.raw[3],
            m.values[0][1] * v.raw[0] + m.values[1][1] * v.raw[1] + m.values[2][1] * v.raw[2] + m.values[3][1] * v.raw[3],
            m.values[0][2] * v.raw[0] + m.values[1][2] * v.raw[1] + m.values[2][2] * v.raw[2] + m.values[3][2] * v.raw[3],
            m.values[0][3] * v.raw[0] + m.values[1][3] * v.raw[1] + m.values[2][3] * v.raw[2] + m.values[3][3] * v.raw[3]};
    }

    // Int
    static_assert(sizeof(IVec4) == 16, "Invalid IVec4 type size.");

    template <>
    const IVec4 IVec4::Zero(0);
    template <>
    const IVec4 IVec4::One(1);
    template <>
    const IVec4 IVec4::UnitX(1, 0, 0, 0);
    template <>
    const IVec4 IVec4::UnitY(0, 1, 0, 0);
    template <>
    const IVec4 IVec4::UnitZ(0, 0, 1, 0);
    template <>
    const IVec4 IVec4::UnitW(0, 0, 0, 1);
    template <>
    const IVec4 IVec4::Minimum(kMinI32);
    template <>
    const IVec4 IVec4::Maximum(kMaxI32);

    template <>
    IVec4::Vector4Base(const FVec2 &xy, i32 z, i32 w)
        : x((i32)xy.x), y((i32)xy.y), z(z), w(w)
    {
    }

    template <>
    IVec4::Vector4Base(const FVec2 &xy, const FVec2 &zw)
        : x((i32)xy.x), y((i32)xy.y), z((i32)zw.x), w((i32)zw.y)
    {
    }

    template <>
    IVec4::Vector4Base(const FVec3 &xyz, i32 w)
        : x((i32)xyz.x), y((i32)xyz.y), z((i32)xyz.z), w(w)
    {
    }

    template <>
    IVec4::Vector4Base(const IVec2 &xy, i32 z, i32 w)
        : x(xy.x), y(xy.y), z(z), w(w)
    {
    }

    template <>
    IVec4::Vector4Base(const IVec3 &xyz, i32 w)
        : x(xyz.x), y(xyz.y), z(xyz.z), w(w)
    {
    }

    template <>
    IVec4::Vector4Base(const DVec2 &xy, i32 z, i32 w)
        : x((i32)xy.x), y((i32)xy.y), z(z), w(w)
    {
    }

    template <>
    IVec4::Vector4Base(const DVec2 &xy, const DVec2 &zw)
        : x((i32)xy.x), y((i32)xy.y), z((i32)zw.x), w((i32)zw.y)
    {
    }

    template <>
    IVec4::Vector4Base(const DVec3 &xyz, i32 w)
        : x((i32)xyz.x), y((i32)xyz.y), z((i32)xyz.z), w(w)
    {
    }

    template <>
    IVec4::Vector4Base(const Color &color)
        : x((i32)color.r), y((i32)color.g), z((i32)color.b), w((i32)color.a)
    {
    }

    template <>
    IVec4::Vector4Base(const Rectangle &rect)
        : x((i32)rect.location.x), y((i32)rect.location.y), z((i32)rect.size.x), w((i32)rect.size.y)
    {
    }

    template <>
    TkString IVec4::toString() const
    {
        return TkString::format("%s", *this);
    }

    template <>
    IVec4 IVec4::transform(const IVec4 &v, const Mat &m)
    {
        return v;
    }
}
