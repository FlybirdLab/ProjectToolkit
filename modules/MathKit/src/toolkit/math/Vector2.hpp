/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/math/common.hpp>

#include <toolkit/base/Math.hpp>
#include <toolkit/base/TypeTraits.hpp>
#include <toolkit/base/Format.hpp>

namespace toolkit
{
    // Represents a two dimensional mathematical vector.
    template <typename T>
    class Vector2
    {
    public:
        // Vector with all components equal 0
        static TK_API const Vector2<T> kZero;
        // Vector with all components equal 1
        static TK_API const Vector2<T> kOne;
        // Vector with all components equal 0.5
        static TK_API const Vector2<T> kHalf;
        // Vector X=1, Y=0
        static TK_API const Vector2<T> kUnitX;
        // Vector X=0, Y=1
        static TK_API const Vector2<T> kUnitY;
        // Vector with all components equal maximum value.
        static TK_API const Vector2<T> kMinimum;
        // Vector with all components equal minimum value.
        static TK_API const Vector2<T> kMaximum;

    public:
        Vector2() = default;
        TK_FORCE_INLINE Vector2(T xy)
            : x(xy), y(xy)
        {
        }
        TK_FORCE_INLINE explicit Vector2(const T *xy)
            : x(xy[0]), y(xy[1])
        {
        }
        TK_FORCE_INLINE Vector2(T x, T y)
            : x(x), y(y)
        {
        }
        template <typename U = T, typename TEnableIf<TNot<TIsTheSame<T, U>>::Value>::Type...>
        TK_FORCE_INLINE Vector2(const Vector2<U> &xy)
            : x((T)xy.X), y((T)xy.y)
        {
        }
        TK_API explicit Vector2(const IVec3 &xy);
        TK_API explicit Vector2(const IVec4 &xy);
        TK_API explicit Vector2(const FVec3 &xy);
        TK_API explicit Vector2(const FVec4 &xy);
        TK_API explicit Vector2(const DVec3 &xy);
        TK_API explicit Vector2(const DVec4 &xy);
        TK_API explicit Vector2(const Color &color);

    public:
        TK_API TkString toString() const;

    public:
        // Gets a value indicting whether this instance is normalized.
        bool isNormalized() const
        {
            return TkMath::abs((x * x + y * y) - 1.0f) < 1e-4f;
        }

        // Gets a value indicting whether this vector is zero.
        bool isZero() const
        {
            return TkMath::isZero(x) && TkMath::isZero(y);
        }

        // Gets a value indicting whether any vector component is zero.
        bool isAnyZero() const
        {
            return TkMath::isZero(x) || TkMath::isZero(y);
        }

        // Gets a value indicting whether this vector is zero.
        bool isOne() const
        {
            return TkMath::isOne(x) && TkMath::isOne(y);
        }

        // Calculates the length of the vector.
        T length() const
        {
            return TkMath::sqrt(x * x + y * y);
        }

        // Calculates the squared length of the vector.
        T lengthSquared() const
        {
            return x * x + y * y;
        }

        // Calculates inverted length of the vector (1 / length).
        T invLength() const
        {
            return 1.0f / length();
        }

        // Returns the average arithmetic of all the components.
        T averageArithmetic() const
        {
            return (x + y) * 0.5f;
        }

        // Gets the sum of all vector components values.
        T sumValues() const
        {
            return x + y;
        }

        // Gets the multiplication result of all vector components values.
        T mulValues() const
        {
            return x * y;
        }

        // Returns the minimum value of all the components.
        T minValue() const
        {
            return TkMath::min(x, y);
        }

        // Returns the maximum value of all the components.
        T maxValue() const
        {
            return TkMath::max(x, y);
        }

        // Returns true if vector has one or more components is not a number (NaN).
        bool isNaN() const
        {
            return isnan(x) || isnan(y);
        }

        // Returns true if vector has one or more components equal to +/- infinity.
        bool isInfinity() const
        {
            return isinf(x) || isinf(y);
        }

        // Returns true if vector has one or more components equal to +/- infinity or NaN.
        bool isNanOrInfinity() const
        {
            return isInfinity() || isNaN();
        }

        // Calculates a vector with values being absolute values of that vector.
        Vector2 getAbsolute() const
        {
            return Vector2(TkMath::abs(x), TkMath::abs(y));
        }

        // Calculates a vector with values being opposite to values of that vector.
        Vector2 getNegative() const
        {
            return Vector2(-x, -y);
        }

        // Calculates a normalized vector that has length equal to 1.
        Vector2 getNormalized() const
        {
            Vector2 result(x, y);
            result.normalize();
            return result;
        }

    public:
        // Performs vector normalization (scales vector up to unit length).
        void normalize()
        {
            const T length = TkMath::sqrt(x * x + y * y);
            if (length >= TkMath::ZeroTolerance)
            {
                const T invLength = (T)1.0f / length;
                x *= invLength;
                y *= invLength;
            }
        }

    public:
        Vector2 operator+(const Vector2 &b) const
        {
            return Vector2(x + b.x, y + b.y);
        }

        Vector2 operator-(const Vector2 &b) const
        {
            return Vector2(x - b.x, y - b.y);
        }

        Vector2 operator*(const Vector2 &b) const
        {
            return Vector2(x * b.x, y * b.y);
        }

        Vector2 operator/(const Vector2 &b) const
        {
            return Vector2(x / b.x, y / b.y);
        }

        Vector2 operator-() const
        {
            return Vector2(-x, -y);
        }

        Vector2 operator+(T b) const
        {
            return Vector2(x + b, y + b);
        }

        Vector2 operator-(T b) const
        {
            return Vector2(x - b, y - b);
        }

        Vector2 operator*(T b) const
        {
            return Vector2(x * b, y * b);
        }

        Vector2 operator/(T b) const
        {
            return Vector2(x / b, y / b);
        }

        Vector2 operator+(typename TOtherFloat<T>::Type a) const
        {
            T b = (T)a;
            return Vector2(x + b, y + b);
        }

        Vector2 operator-(typename TOtherFloat<T>::Type a) const
        {
            T b = (T)a;
            return Vector2(x - b, y - b);
        }

        Vector2 operator*(typename TOtherFloat<T>::Type a) const
        {
            T b = (T)a;
            return Vector2(x * b, y * b);
        }

        Vector2 operator/(typename TOtherFloat<T>::Type a) const
        {
            T b = (T)a;
            return Vector2(x / b, y / b);
        }

        Vector2 &operator+=(const Vector2 &b)
        {
            x += b.x;
            y += b.y;
            return *this;
        }

        Vector2 &operator-=(const Vector2 &b)
        {
            x -= b.x;
            y -= b.y;
            return *this;
        }

        Vector2 &operator*=(const Vector2 &b)
        {
            x *= b.x;
            y *= b.y;
            return *this;
        }

        Vector2 &operator/=(const Vector2 &b)
        {
            x /= b.x;
            y /= b.y;
            return *this;
        }

        Vector2 &operator+=(T b)
        {
            x += b;
            y += b;
            return *this;
        }

        Vector2 &operator-=(T b)
        {
            x -= b;
            y -= b;
            return *this;
        }

        Vector2 &operator*=(T b)
        {
            x *= b;
            y *= b;
            return *this;
        }

        Vector2 &operator/=(T b)
        {
            x /= b;
            y /= b;
            return *this;
        }

        bool operator==(const Vector2 &b) const
        {
            return x == b.x && y == b.y;
        }

        bool operator!=(const Vector2 &b) const
        {
            return x != b.x || y != b.y;
        }

        bool operator>(const Vector2 &b) const
        {
            return x > b.x && y > b.y;
        }

        bool operator>=(const Vector2 &b) const
        {
            return x >= b.x && y >= b.y;
        }

        bool operator<(const Vector2 &b) const
        {
            return x < b.x && y < b.y;
        }

        bool operator<=(const Vector2 &b) const
        {
            return x <= b.x && y <= b.y;
        }

    public:
        static bool nearEqual(const Vector2 &a, const Vector2 &b)
        {
            return TkMath::nearEqual(a.x, b.x) && TkMath::nearEqual(a.y, b.y);
        }

        static bool nearEqual(const Vector2 &a, const Vector2 &b, T epsilon)
        {
            return TkMath::nearEqual(a.x, b.x, epsilon) && TkMath::nearEqual(a.y, b.y, epsilon);
        }

    public:
        static T dot(const Vector2 &a, const Vector2 &b)
        {
            return a.x * b.x + a.y * b.y;
        }

        static T cross(const Vector2 &a, const Vector2 &b)
        {
            return a.x * b.y - a.y * b.x;
        }

        static void add(const Vector2 &a, const Vector2 &b, Vector2 &result)
        {
            result = Vector2(a.x + b.x, a.y + b.y);
        }

        static void subtract(const Vector2 &a, const Vector2 &b, Vector2 &result)
        {
            result = Vector2(a.x - b.x, a.y - b.y);
        }

        static void multiply(const Vector2 &a, const Vector2 &b, Vector2 &result)
        {
            result = Vector2(a.x * b.x, a.y * b.y);
        }

        static void divide(const Vector2 &a, const Vector2 &b, Vector2 &result)
        {
            result = Vector2(a.x / b.x, a.y / b.y);
        }

        static void min(const Vector2 &a, const Vector2 &b, Vector2 &result)
        {
            result = Vector2(a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y);
        }

        static void max(const Vector2 &a, const Vector2 &b, Vector2 &result)
        {
            result = Vector2(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y);
        }

    public:
        static Vector2 min(const Vector2 &a, const Vector2 &b)
        {
            return Vector2(a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y);
        }

        static Vector2 max(const Vector2 &a, const Vector2 &b)
        {
            return Vector2(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y);
        }

        static Vector2 mod(const Vector2 &a, const Vector2 &b)
        {
            return Vector2(TkMath::mod(a.x, b.x), TkMath::mod(a.y, b.y));
        }

        static Vector2 floor(const Vector2 &v)
        {
            return Vector2(TkMath::floor(v.x), TkMath::floor(v.y));
        }

        static Vector2 frac(const Vector2 &v)
        {
            return Vector2(v.x - (i32)v.x, v.y - (i32)v.y);
        }

        static Vector2 round(const Vector2 &v)
        {
            return Vector2(TkMath::round(v.x), TkMath::round(v.y));
        }

        static Vector2 ceil(const Vector2 &v)
        {
            return Vector2(TkMath::ceil(v.x), TkMath::ceil(v.y));
        }

        static Vector2 abs(const Vector2 &v)
        {
            return Vector2(TkMath::abs(v.x), TkMath::abs(v.y));
        }

        // Restricts a value to be within a specified range (inclusive min/max).
        static Vector2 clamp(const Vector2 &v, const Vector2 &min, const Vector2 &max)
        {
            Vector2 result;
            clamp(v, min, max, result);
            return result;
        }

        // Restricts a value to be within a specified range (inclusive min/max).
        static void clamp(const Vector2 &v, const Vector2 &min, const Vector2 &max, Vector2 &result)
        {
            result = Vector2(TkMath::clamp(v.x, min.x, max.x), TkMath::clamp(v.y, min.y, max.y));
        }

        // Calculates distance between two points in 2D.
        static T distance(const Vector2 &a, const Vector2 &b)
        {
            const T x = a.x - b.x;
            const T y = a.y - b.y;
            return TkMath::sqrt(x * x + y * y);
        }

        // Calculates the squared distance between two points in 2D.
        static T distanceSquared(const Vector2 &a, const Vector2 &b)
        {
            const T x = a.x - b.x;
            const T y = a.y - b.y;
            return x * x + y * y;
        }

        // Performs vector normalization (scales vector up to unit length).
        static Vector2 normalize(const Vector2 &v)
        {
            Vector2 r = v;
            const T length = TkMath::sqrt(r.x * r.x + r.y * r.y);
            if (length >= TkMath::ZeroTolerance)
            {
                const T inv = (T)1.0f / length;
                r.x *= inv;
                r.y *= inv;
            }
            return r;
        }

        // Performs a linear interpolation between two vectors.
        static void lerp(const Vector2 &start, const Vector2 &end, T amount, Vector2 &result)
        {
            result.x = TkMath::lerp(start.x, end.x, amount);
            result.y = TkMath::lerp(start.y, end.y, amount);
        }

        // Performs a linear interpolation between two vectors.
        static Vector2 lerp(const Vector2 &start, const Vector2 &end, T amount)
        {
            Vector2 result;
            lerp(start, end, amount, result);
            return result;
        }

        // Calculates the area of the triangle.
        TK_API static T triangleArea(const Vector2 &v0, const Vector2 &v1, const Vector2 &v2);
        // Calculates the angle (in radians) between from and to. This is always the smallest value.
        TK_API static T angle(const Vector2 &from, const Vector2 &to);

    public:
        union
        {
            struct
            {
                T x;
                T y;
            };
            T raw[2];
        };
    };

    template <typename T>
    inline Vector2<T> operator+(T a, const Vector2<T> &b)
    {
        return b + a;
    }

    template <typename T>
    inline Vector2<T> operator-(T a, const Vector2<T> &b)
    {
        return Vector2<T>(a) - b;
    }

    template <typename T>
    inline Vector2<T> operator*(T a, const Vector2<T> &b)
    {
        return b * a;
    }

    template <typename T>
    inline Vector2<T> operator/(T a, const Vector2<T> &b)
    {
        return Vector2<T>(a) / b;
    }

    template <typename T>
    inline Vector2<T> operator+(typename TOtherFloat<T>::Type a, const Vector2<T> &b)
    {
        return b + a;
    }

    template <typename T>
    inline Vector2<T> operator-(typename TOtherFloat<T>::Type a, const Vector2<T> &b)
    {
        return Vector2<T>(a) - b;
    }

    template <typename T>
    inline Vector2<T> operator*(typename TOtherFloat<T>::Type a, const Vector2<T> &b)
    {
        return b * a;
    }

    template <typename T>
    inline Vector2<T> operator/(typename TOtherFloat<T>::Type a, const Vector2<T> &b)
    {
        return Vector2<T>(a) / b;
    }

    template <typename T>
    inline u32 getHash(const Vector2<T> &key)
    {
        return (*(u32 *)&key.x * 397) ^ *(u32 *)&key.y;
    }

    namespace math
    {
        template <typename T>
        TK_FORCE_INLINE static bool nearEqual(const Vector2<T> &a, const Vector2<T> &b)
        {
            return Vector2<T>::nearEqual(a, b);
        }
    }

    template <>
    struct TIsPODType<FVec2>
    {
        enum
        {
            Value = true
        };
    };

    template <>
    struct TIsPODType<DVec2>
    {
        enum
        {
            Value = true
        };
    };

    template <>
    struct TIsPODType<IVec2>
    {
        enum
        {
            Value = true
        };
    };

#if !defined(_MSC_VER) || defined(__clang__)
    // Forward specializations for Clang
    template <>
    TK_API const FVec2 FVec2::kZero;
    template <>
    TK_API const FVec2 FVec2::kOne;
    template <>
    TK_API const FVec2 FVec2::kUnitX;
    template <>
    TK_API const FVec2 FVec2::kUnitY;
    template <>
    TK_API const FVec2 FVec2::kMinimum;
    template <>
    TK_API const FVec2 FVec2::kMaximum;

    template <>
    TK_API const DVec2 DVec2::kZero;
    template <>
    TK_API const DVec2 DVec2::kOne;
    template <>
    TK_API const DVec2 DVec2::kUnitX;
    template <>
    TK_API const DVec2 DVec2::kUnitY;
    template <>
    TK_API const DVec2 DVec2::kMinimum;
    template <>
    TK_API const DVec2 DVec2::kMaximum;

    template <>
    TK_API const IVec2 IVec2::kZero;
    template <>
    TK_API const IVec2 IVec2::kOne;
    template <>
    TK_API const IVec2 IVec2::kUnitX;
    template <>
    TK_API const IVec2 IVec2::kUnitY;
    template <>
    TK_API const IVec2 IVec2::kMinimum;
    template <>
    TK_API const IVec2 IVec2::kMaximum;
#endif

}

FORMAT_CUSTOM(toolkit::FVec2, "X:%g Y:%g", t.x, t.y);
FORMAT_CUSTOM(toolkit::DVec2, "X:%g Y:%g", t.x, t.y);
FORMAT_CUSTOM(toolkit::IVec2, "X:%d Y:%d", t.x, t.y);
