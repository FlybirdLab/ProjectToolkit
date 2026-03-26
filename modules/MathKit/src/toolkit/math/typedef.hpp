/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>

namespace toolkit {

    // Vector2
    template<typename T> struct Vector2Base;
    typedef Vector2Base<float> FVec2;
    typedef Vector2Base<double> DVec2;
    typedef Vector2Base<i32> IVec2;
    typedef Vector2Base<real> Vec2;

    // Vector3
    template<typename T> struct Vector3Base;
    typedef Vector3Base<float> FVec3;
    typedef Vector3Base<double> DVec3;
    typedef Vector3Base<i32> IVec3;
    typedef Vector3Base<real> Vec3;

    // Vector4
    template<typename T> struct Vector4Base;
    typedef Vector4Base<float> FVec4;
    typedef Vector4Base<double> DVec4;
    typedef Vector4Base<i32> IVec4;
    typedef Vector4Base<real> Vec4;

    struct BoundingBox;
    struct Mat;
    struct Mat3x3;
    struct Ray;
    struct Plane;
    struct Rectangle;
    struct Quat;
    struct BoundingSphere;
    struct BoundingFrustum;
    struct OrientedBoundingBox;
    struct Transform;
    struct Color;
    struct Color32;

}