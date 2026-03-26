/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/math/common.hpp>

#include <toolkit/base/Math.hpp>
#include <toolkit/base/Format.hpp>
#include <toolkit/base/TypeTraits.hpp>

namespace toolkit {

    // Represents a four dimensional mathematical vector with 32-bit precision (per-component).
    template<typename T>
    struct Vector4Base
    {
        typedef T Real;

        union
        {
            struct
            {
                T x;
                T y;
                T z;
                T w;
            };
            T raw[4];
        };

        // Vector with all components equal 0
        static TK_API const Vector4Base<T> Zero;
        // Vector with all components equal 1
        static TK_API const Vector4Base<T> One;
        // Vector with all components equal 0.5
        static TK_API const Vector4Base<T> Half;
        // Vector X=1, Y=0, Z=0, W=0
        static TK_API const Vector4Base<T> UnitX;
        // Vector X=0, Y=1, Z=0, W=0
        static TK_API const Vector4Base<T> UnitY;
        // Vector X=0, Y=0, Z=1, W=0
        static TK_API const Vector4Base<T> UnitZ;
        // Vector X=0, Y=0, Z=0, W=1
        static TK_API const Vector4Base<T> UnitW;
        // Vector with all components equal maximum value.
        static TK_API const Vector4Base<T> Minimum;
        // Vector with all components equal minimum value.
        static TK_API const Vector4Base<T> Maximum;

        Vector4Base() = default;
        TK_FORCE_INLINE Vector4Base(T xyzw)
            : x(xyzw)
            , y(xyzw)
            , z(xyzw)
            , w(xyzw) {
        }
        TK_FORCE_INLINE explicit Vector4Base(const T* xyzw)
            : x(xyzw[0])
            , y(xyzw[1])
            , z(xyzw[2])
            , w(xyzw[3]) {
        }
        TK_FORCE_INLINE Vector4Base(T x, T y, T z, T w)
            : x(x)
            , y(y)
            , z(z)
            , w(w) {
        }
        template<typename U = T, typename TEnableIf<TNot<TIsTheSame<T, U>>::Value>::Type...>
        TK_FORCE_INLINE Vector4Base(const Vector4Base<U>& xyzw)
            : x(static_cast<T>(xyzw.x))
            , y(static_cast<T>(xyzw.y))
            , z(static_cast<T>(xyzw.z))
            , w(static_cast<T>(xyzw.w)) {
        }
        TK_API explicit Vector4Base(const FVec2& xy, T z = 0, T w = 0);
        TK_API explicit Vector4Base(const FVec2& xy, const FVec2& zw);
        TK_API explicit Vector4Base(const FVec3& xyz, T w = 0);
        TK_API explicit Vector4Base(const IVec2& xy, T z = 0, T w = 0);
        TK_API explicit Vector4Base(const IVec3& xyz, T w = 0);
        TK_API explicit Vector4Base(const DVec2& xy, T z = 0, T w = 0);
        TK_API explicit Vector4Base(const DVec2& xy, const DVec2& zw);
        TK_API explicit Vector4Base(const DVec3& xyz, T w = 0);
        TK_API explicit Vector4Base(const Color& color);
        TK_API explicit Vector4Base(const Rectangle& rect);

        TK_API TkString toString() const;

        // Gets a value indicting whether this vector is zero.
        bool isZero() const
        {
            return TkMath::isZero(x) && TkMath::isZero(y) && TkMath::isZero(z) && TkMath::isZero(w);
        }

        // Gets a value indicting whether any vector component is zero.
        bool isAnyZero() const
        {
            return TkMath::isZero(x) || TkMath::isZero(y) || TkMath::isZero(z) || TkMath::isZero(w);
        }

        // Gets a value indicting whether this vector is one.
        bool isOne() const
        {
            return TkMath::isOne(x) && TkMath::isOne(y) && TkMath::isOne(z) && TkMath::isOne(w);
        }

        // Returns the average arithmetic of all the components.
        T averageArithmetic() const
        {
            return (x + y + z + w) * 0.25f;
        }

        // Gets the sum of all vector components values.
        T sumValues() const
        {
            return x + y + z + w;
        }

        // Returns the minimum value of all the components.
        T minValue() const
        {
            return TkMath::min(x, y, z, w);
        }

        // Returns the maximum value of all the components.
        T maxValue() const
        {
            return TkMath::max(x, y, z, w);
        }

        // Returns true if vector has one or more components is not a number (NaN).
        bool isNaN() const
        {
            return isnan(x) || isnan(y) || isnan(z) || isnan(w);
        }

        // Returns true if vector has one or more components equal to +/- infinity.
        bool isInfinity() const
        {
            return isinf(x) || isinf(y) || isinf(z) || isinf(w);
        }

        // Returns true if vector has one or more components equal to +/- infinity or NaN.
        bool isNanOrInfinity() const
        {
            return isInfinity() || isNaN();
        }

        // Calculates a vector with values being absolute values of that vector.
        Vector4Base getAbsolute() const
        {
            return {TkMath::abs(x), TkMath::abs(y), TkMath::abs(z), TkMath::abs(w)};
        }

        // Calculates a vector with values being opposite to values of that vector.
        Vector4Base getNegative() const
        {
            return {-x, -y, -z, -w};
        }

    public:
        Vector4Base operator+(const Vector4Base& b) const
        {
            return {x + b.x, y + b.y, z + b.z, w + b.w};
        }

        Vector4Base operator-(const Vector4Base& b) const
        {
            return {x - b.x, y - b.y, z - b.z, w - b.w};
        }

        Vector4Base operator*(const Vector4Base& b) const
        {
            return {x * b.x, y * b.y, z * b.z, w * b.w};
        }

        Vector4Base operator/(const Vector4Base& b) const
        {
            return {x / b.x, y / b.y, z / b.z, w / b.w};
        }

        Vector4Base operator-() const
        {
            return {-x, -y, -z, -w};
        }

        Vector4Base operator+(T b) const
        {
            return {x + b, y + b, z + b, w + b};
        }

        Vector4Base operator-(T b) const
        {
            return {x - b, y - b, z - b, w - b};
        }

        Vector4Base operator*(T b) const
        {
            return {x * b, y * b, z * b, w * b};
        }

        Vector4Base operator/(T b) const
        {
            return {x / b, y / b, z / b, w / b};
        }

        Vector4Base operator+(typename TOtherFloat<T>::Type a) const
        {
            T b = static_cast<T>(a);
            return {x + b, y + b, z + b, w + b};
        }

        Vector4Base operator-(typename TOtherFloat<T>::Type a) const
        {
            T b = static_cast<T>(a);
            return {x - b, y - b, z - b, w - b};
        }

        Vector4Base operator*(typename TOtherFloat<T>::Type a) const
        {
            T b = static_cast<T>(a);
            return {x * b, y * b, z * b, w * b};
        }

        Vector4Base operator/(typename TOtherFloat<T>::Type a) const
        {
            T b = static_cast<T>(a);
            return {x / b, y / b, z / b, w / b};
        }

        Vector4Base& operator+=(const Vector4Base& b)
        {
            x += b.x;
            y += b.y;
            z += b.z;
            w += b.w;
            return *this;
        }

        Vector4Base& operator-=(const Vector4Base& b)
        {
            x -= b.x;
            y -= b.y;
            z -= b.z;
            w -= b.w;
            return *this;
        }

        Vector4Base& operator*=(const Vector4Base& b)
        {
            x *= b.x;
            y *= b.y;
            z *= b.z;
            w *= b.w;
            return *this;
        }

        Vector4Base& operator/=(const Vector4Base& b)
        {
            x /= b.x;
            y /= b.y;
            z /= b.z;
            w /= b.w;
            return *this;
        }

        Vector4Base& operator+=(T b)
        {
            x += b;
            y += b;
            z += b;
            w += b;
            return *this;
        }

        Vector4Base& operator-=(T b)
        {
            x -= b;
            y -= b;
            z -= b;
            w -= b;
            return *this;
        }

        Vector4Base& operator*=(T b)
        {
            x *= b;
            y *= b;
            z *= b;
            w *= b;
            return *this;
        }

        Vector4Base& operator/=(T b)
        {
            x /= b;
            y /= b;
            z /= b;
            w /= b;
            return *this;
        }

        bool operator==(const Vector4Base& b) const
        {
            return x == b.x && y == b.y && z == b.z && w == b.w;
        }

        bool operator!=(const Vector4Base& b) const
        {
            return x != b.x || y != b.y || z != b.z || w != b.w;
        }

        bool operator>(const Vector4Base& b) const
        {
            return x > b.x && y > b.y && z > b.z && w > b.w;
        }

        bool operator>=(const Vector4Base& b) const
        {
            return x >= b.x && y >= b.y && z >= b.z && w >= b.w;
        }

        bool operator<(const Vector4Base& b) const
        {
            return x < b.x && y < b.y && z < b.z && w < b.w;
        }

        bool operator<=(const Vector4Base& b) const
        {
            return x <= b.x && y <= b.y && z <= b.z && w <= b.w;
        }

        static bool nearEqual(const Vector4Base& a, const Vector4Base& b)
        {
            return TkMath::nearEqual(a.x, b.x) && TkMath::nearEqual(a.y, b.y) && TkMath::nearEqual(a.z, b.z) && TkMath::nearEqual(a.w, b.w);
        }

        static bool nearEqual(const Vector4Base& a, const Vector4Base& b, T epsilon)
        {
            return TkMath::nearEqual(a.x, b.x, epsilon) && TkMath::nearEqual(a.y, b.y, epsilon) && TkMath::nearEqual(a.z, b.z, epsilon) && TkMath::nearEqual(a.w, b.w, epsilon);
        }

        static void add(const Vector4Base& a, const Vector4Base& b, Vector4Base& result)
        {
            result = {a.x + b.x, a.y + b.y, a.z + b.z,a.w+ b.w};
        }

        static void subtract(const Vector4Base& a, const Vector4Base& b, Vector4Base& result)
        {
            result = {a.x - b.x, a.y - b.y, a.z - b.z,a.w- b.w};
        }

        static void multiply(const Vector4Base& a, const Vector4Base& b, Vector4Base& result)
        {
            result = {a.x * b, a.y * b, a.z * b,a.w* b};
        }

        static void divide(const Vector4Base& a, const Vector4Base& b, Vector4Base& result)
        {
            result = {a.x / b.x, a.y / b.y, a.z / b.z,a.w/ b.w};
        }

        static Vector4Base mod(const Vector4Base& a, const Vector4Base& b)
        {
            return {TkMath::mod(a.x, b.x), TkMath::mod(a.y, b.y), TkMath::mod(a.z, b.z), TkMath::mod(a.w, b.w)};
        }

        static Vector4Base floor(const Vector4Base& v)
        {
            return {TkMath::floor(v.x), TkMath::floor(v.y), TkMath::floor(v.z), TkMath::floor(v.w)};
        }

        static Vector4Base frac(const Vector4Base& v)
        {
            return {v.x - static_cast<i32>(v.x), v.y - static_cast<i32>(v.y), v.z - static_cast<i32>(v.z), v.w - static_cast<i32>(v.w)};
        }

        static Vector4Base round(const Vector4Base& v)
        {
            return {TkMath::round(v.x), TkMath::round(v.y), TkMath::round(v.z), TkMath::round(v.w)};
        }

        static Vector4Base ceil(const Vector4Base& v)
        {
            return {TkMath::ceil(v.x), TkMath::ceil(v.y), TkMath::ceil(v.z), TkMath::ceil(v.w)};
        }

        static Vector4Base abs(const Vector4Base& v)
        {
            return {TkMath::abs(v.x), TkMath::abs(v.y), TkMath::abs(v.z), TkMath::abs(v.w)};
        }

        // Restricts a value to be within a specified range (inclusive min/max).
        static Vector4Base clamp(const Vector4Base& v, const Vector4Base& min, const Vector4Base& max)
        {
            Vector4Base result;
            clamp(v, min, max, result);
            return result;
        }

        // Restricts a value to be within a specified range (inclusive min/max).
        static void clamp(const Vector4Base& v, const Vector4Base& min, const Vector4Base& max, Vector4Base& result)
        {
            result = {TkMath::clamp(v.x, min.x, max.x), TkMath::clamp(v.y, min.y, max.y), TkMath::clamp(v.z, min.z, max.z), TkMath::clamp(v.w, min.w, max.w)};
        }

        // Performs a linear interpolation between two vectors.
        static void lerp(const Vector4Base& start, const Vector4Base& end, T amount, Vector4Base& result)
        {
            result.x = TkMath::lerp(start.x, end.x, amount);
            result.y = TkMath::lerp(start.y, end.y, amount);
            result.z = TkMath::lerp(start.z, end.z, amount);
            result.w = TkMath::lerp(start.w, end.w, amount);
        }

        // Performs a linear interpolation between two vectors.
        static Vector4Base lerp(const Vector4Base& start, const Vector4Base& end, T amount)
        {
            Vector4Base result;
            lerp(start, end, amount, result);
            return result;
        }

        TK_API static Vector4Base transform(const Vector4Base& v, const Mat& m);
    };

    template<typename T>
    inline Vector4Base<T> operator+(T a, const Vector4Base<T>& b)
    {
        return b + a;
    }

    template<typename T>
    inline Vector4Base<T> operator-(T a, const Vector4Base<T>& b)
    {
        return Vector4Base<T>(a) - b;
    }

    template<typename T>
    inline Vector4Base<T> operator*(T a, const Vector4Base<T>& b)
    {
        return b * a;
    }

    template<typename T>
    inline Vector4Base<T> operator/(T a, const Vector4Base<T>& b)
    {
        return Vector4Base<T>(a) / b;
    }

    template<typename T>
    inline Vector4Base<T> operator+(typename TOtherFloat<T>::Type a, const Vector4Base<T>& b)
    {
        return b + a;
    }

    template<typename T>
    inline Vector4Base<T> operator-(typename TOtherFloat<T>::Type a, const Vector4Base<T>& b)
    {
        return Vector4Base<T>(a) - b;
    }

    template<typename T>
    inline Vector4Base<T> operator*(typename TOtherFloat<T>::Type a, const Vector4Base<T>& b)
    {
        return b * a;
    }

    template<typename T>
    inline Vector4Base<T> operator/(typename TOtherFloat<T>::Type a, const Vector4Base<T>& b)
    {
        return Vector4Base<T>(a) / b;
    }

    template<typename T>
    inline u32 getHash(const Vector4Base<T>& key)
    {
        return ((*static_cast<u32 *>(&key.x) * 397 ^ *static_cast<u32 *>(&key.y)) * 397 ^ *static_cast<u32 *>(&key.z)) * 397 ^ *static_cast<u32 *>(&key.w);
    }

    namespace math
    {
        template<typename T>
        TK_FORCE_INLINE static bool nearEqual(const Vector4Base<T>& a, const Vector4Base<T>& b)
        {
            return Vector4Base<T>::nearEqual(a, b);
        }
    }

    template<>
    struct TIsPODType<FVec4>
    {
        enum { Value = true };
    };

    template<>
    struct TIsPODType<DVec4>
    {
        enum { Value = true };
    };

    template<>
    struct TIsPODType<IVec4>
    {
        enum { Value = true };
    };

#if !defined(_MSC_VER) || defined(__clang__)
    // Forward specializations for Clang
    template<> TK_API const FVec4 FVec4::Zero;
    template<> TK_API const FVec4 FVec4::One;
    template<> TK_API const FVec4 FVec4::UnitX;
    template<> TK_API const FVec4 FVec4::UnitY;
    template<> TK_API const FVec4 FVec4::UnitZ;
    template<> TK_API const FVec4 FVec4::UnitW;
    template<> TK_API const FVec4 FVec4::Minimum;
    template<> TK_API const FVec4 FVec4::Maximum;

    template<> TK_API const DVec4 DVec4::Zero;
    template<> TK_API const DVec4 DVec4::One;
    template<> TK_API const DVec4 DVec4::UnitX;
    template<> TK_API const DVec4 DVec4::UnitY;
    template<> TK_API const DVec4 DVec4::UnitZ;
    template<> TK_API const DVec4 DVec4::UnitW;
    template<> TK_API const DVec4 DVec4::Minimum;
    template<> TK_API const DVec4 DVec4::Maximum;

    template<> TK_API const IVec4 IVec4::Zero;
    template<> TK_API const IVec4 IVec4::One;
    template<> TK_API const IVec4 IVec4::UnitX;
    template<> TK_API const IVec4 IVec4::UnitY;
    template<> TK_API const IVec4 IVec4::UnitZ;
    template<> TK_API const IVec4 IVec4::UnitW;
    template<> TK_API const IVec4 IVec4::Minimum;
    template<> TK_API const IVec4 IVec4::Maximum;
#endif

}

FORMAT_CUSTOM(toolkit::FVec4, "X:%g Y:%g Z:%g W:%g", t.x, t.y, t.z, t.w);
FORMAT_CUSTOM(toolkit::DVec4, "X:%g Y:%g Z:%g W:%g", t.x, t.y, t.z, t.w);
FORMAT_CUSTOM(toolkit::IVec4, "X:%d Y:%d Z:%d W:%d", t.x, t.y, t.z, t.w);
