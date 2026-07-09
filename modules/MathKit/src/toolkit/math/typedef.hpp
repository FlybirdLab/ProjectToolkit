/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>

namespace toolkit {

    // Vector2
    template<typename T> class Vector2;
    typedef Vector2<float> FVec2;
    typedef Vector2<double> DVec2;
    typedef Vector2<i32> IVec2;
    typedef Vector2<real> Vec2;

    // Vector3
    template<typename T> class Vector3;
    typedef Vector3<float> FVec3;
    typedef Vector3<double> DVec3;
    typedef Vector3<i32> IVec3;
    typedef Vector3<real> Vec3;

    // Vector4
    template<typename T> class Vector4;
    typedef Vector4<float> FVec4;
    typedef Vector4<double> DVec4;
    typedef Vector4<i32> IVec4;
    typedef Vector4<real> Vec4;

    class BoundingBox;
    class Mat;
    struct Mat3x3;
    class Ray;
    class Plane;
    class Rectangle;
    class Quat;
    class BoundingSphere;
    class BoundingFrustum;
    class OrientedBoundingBox;
    class Transform;
    class Color;
    struct Color32;

}
