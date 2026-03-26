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

namespace toolkit {

    // Represents a two dimensional mathematical vector.
    template<typename T>
    struct Vector2Base
    {
    public:
        typedef T Real;
        
        union
        {
            struct { T x; T y; };
            T raw[2];
        };

    public:
        // Vector with all components equal 0
        static TK_API const Vector2Base<T> Zero;
        // Vector with all components equal 1
        static TK_API const Vector2Base<T> One;
        // Vector with all components equal 0.5
        static TK_API const Vector2Base<T> Half;
        // Vector X=1, Y=0
        static TK_API const Vector2Base<T> UnitX;
        // Vector X=0, Y=1
        static TK_API const Vector2Base<T> UnitY;
        // Vector with all components equal maximum value.
        static TK_API const Vector2Base<T> Minimum;
        // Vector with all components equal minimum value.
        static TK_API const Vector2Base<T> Maximum;

    public:
        Vector2Base() = default;
        TK_FORCE_INLINE Vector2Base(T xy)
            : x(xy)
            , y(xy) {
        }
        TK_FORCE_INLINE explicit Vector2Base(const T* xy)
            : x(xy[0])
            , y(xy[1]) {
        }
        TK_FORCE_INLINE Vector2Base(T x, T y)
            : x(x)
            , y(y) {
        }
        template<typename U = T, typename TEnableIf<TNot<TIsTheSame<T, U>>::Value>::Type...>
        TK_FORCE_INLINE Vector2Base(const Vector2Base<U>& xy)
            : x((T)xy.X)
            , y((T)xy.y) {
        }
        TK_API explicit Vector2Base(const IVec3& xy);
        TK_API explicit Vector2Base(const IVec4& xy);
        TK_API explicit Vector2Base(const FVec3& xy);
        TK_API explicit Vector2Base(const FVec4& xy);
        TK_API explicit Vector2Base(const DVec3& xy);
        TK_API explicit Vector2Base(const DVec4& xy);
        TK_API explicit Vector2Base(const Color& color);

    public:
        TK_API TkString toString() const;

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
        Vector2Base getAbsolute() const
        {
            return Vector2Base(TkMath::abs(x), TkMath::abs(y));
        }

        // Calculates a vector with values being opposite to values of that vector.
        Vector2Base getNegative() const
        {
            return Vector2Base(-x, -y);
        }

        // Calculates a normalized vector that has length equal to 1.
        Vector2Base getNormalized() const
        {
            Vector2Base result(x, y);
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
        Vector2Base operator+(const Vector2Base& b) const
        {
            return Vector2Base(x + b.x, y + b.y);
        }

        Vector2Base operator-(const Vector2Base& b) const
        {
            return Vector2Base(x - b.x, y - b.y);
        }

        Vector2Base operator*(const Vector2Base& b) const
        {
            return Vector2Base(x * b.x, y * b.y);
        }

        Vector2Base operator/(const Vector2Base& b) const
        {
            return Vector2Base(x / b.x, y / b.y);
        }

        Vector2Base operator-() const
        {
            return Vector2Base(-x, -y);
        }

        Vector2Base operator+(T b) const
        {
            return Vector2Base(x + b, y + b);
        }

        Vector2Base operator-(T b) const
        {
            return Vector2Base(x - b, y - b);
        }

        Vector2Base operator*(T b) const
        {
            return Vector2Base(x * b, y * b);
        }

        Vector2Base operator/(T b) const
        {
            return Vector2Base(x / b, y / b);
        }

        Vector2Base operator+(typename TOtherFloat<T>::Type a) const
        {
            T b = (T)a;
            return Vector2Base(x + b, y + b);
        }

        Vector2Base operator-(typename TOtherFloat<T>::Type a) const
        {
            T b = (T)a;
            return Vector2Base(x - b, y - b);
        }

        Vector2Base operator*(typename TOtherFloat<T>::Type a) const
        {
            T b = (T)a;
            return Vector2Base(x * b, y * b);
        }

        Vector2Base operator/(typename TOtherFloat<T>::Type a) const
        {
            T b = (T)a;
            return Vector2Base(x / b, y / b);
        }

        Vector2Base& operator+=(const Vector2Base& b)
        {
            x += b.x;
            y += b.y;
            return *this;
        }

        Vector2Base& operator-=(const Vector2Base& b)
        {
            x -= b.x;
            y -= b.y;
            return *this;
        }

        Vector2Base& operator*=(const Vector2Base& b)
        {
            x *= b.x;
            y *= b.y;
            return *this;
        }

        Vector2Base& operator/=(const Vector2Base& b)
        {
            x /= b.x;
            y /= b.y;
            return *this;
        }

        Vector2Base& operator+=(T b)
        {
            x += b;
            y += b;
            return *this;
        }

        Vector2Base& operator-=(T b)
        {
            x -= b;
            y -= b;
            return *this;
        }

        Vector2Base& operator*=(T b)
        {
            x *= b;
            y *= b;
            return *this;
        }

        Vector2Base& operator/=(T b)
        {
            x /= b;
            y /= b;
            return *this;
        }

        bool operator==(const Vector2Base& b) const
        {
            return x == b.x && y == b.y;
        }

        bool operator!=(const Vector2Base& b) const
        {
            return x != b.x || y != b.y;
        }

        bool operator>(const Vector2Base& b) const
        {
            return x > b.x && y > b.y;
        }

        bool operator>=(const Vector2Base& b) const
        {
            return x >= b.x && y >= b.y;
        }

        bool operator<(const Vector2Base& b) const
        {
            return x < b.x && y < b.y;
        }

        bool operator<=(const Vector2Base& b) const
        {
            return x <= b.x && y <= b.y;
        }

    public:
        static bool nearEqual(const Vector2Base& a, const Vector2Base& b)
        {
            return TkMath::nearEqual(a.x, b.x) && TkMath::nearEqual(a.y, b.y);
        }

        static bool nearEqual(const Vector2Base& a, const Vector2Base& b, T epsilon)
        {
            return TkMath::nearEqual(a.x, b.x, epsilon) && TkMath::nearEqual(a.y, b.y, epsilon);
        }

    public:
        static T dot(const Vector2Base& a, const Vector2Base& b)
        {
            return a.x * b.x + a.y * b.y;
        }

        static T cross(const Vector2Base& a, const Vector2Base& b)
        {
            return a.x * b.y - a.y * b.x;
        }

        static void add(const Vector2Base& a, const Vector2Base& b, Vector2Base& result)
        {
            result = Vector2Base(a.x + b.x, a.y + b.y);
        }

        static void subtract(const Vector2Base& a, const Vector2Base& b, Vector2Base& result)
        {
            result = Vector2Base(a.x - b.x, a.y - b.y);
        }

        static void multiply(const Vector2Base& a, const Vector2Base& b, Vector2Base& result)
        {
            result = Vector2Base(a.x * b.x, a.y * b.y);
        }

        static void divide(const Vector2Base& a, const Vector2Base& b, Vector2Base& result)
        {
            result = Vector2Base(a.x / b.x, a.y / b.y);
        }

        static void min(const Vector2Base& a, const Vector2Base& b, Vector2Base& result)
        {
            result = Vector2Base(a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y);
        }

        static void max(const Vector2Base& a, const Vector2Base& b, Vector2Base& result)
        {
            result = Vector2Base(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y);
        }

    public:
        static Vector2Base min(const Vector2Base& a, const Vector2Base& b)
        {
            return Vector2Base(a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y);
        }

        static Vector2Base max(const Vector2Base& a, const Vector2Base& b)
        {
            return Vector2Base(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y);
        }

        static Vector2Base mod(const Vector2Base& a, const Vector2Base& b)
        {
            return Vector2Base(TkMath::mod(a.x, b.x), TkMath::mod(a.y, b.y));
        }

        static Vector2Base floor(const Vector2Base& v)
        {
            return Vector2Base(TkMath::floor(v.x), TkMath::floor(v.y));
        }

        static Vector2Base frac(const Vector2Base& v)
        {
            return Vector2Base(v.x - (i32)v.x, v.y - (i32)v.y);
        }

        static Vector2Base round(const Vector2Base& v)
        {
            return Vector2Base(TkMath::round(v.x), TkMath::round(v.y));
        }

        static Vector2Base ceil(const Vector2Base& v)
        {
            return Vector2Base(TkMath::ceil(v.x), TkMath::ceil(v.y));
        }

        static Vector2Base abs(const Vector2Base& v)
        {
            return Vector2Base(TkMath::abs(v.x), TkMath::abs(v.y));
        }

        // Restricts a value to be within a specified range (inclusive min/max).
        static Vector2Base clamp(const Vector2Base& v, const Vector2Base& min, const Vector2Base& max)
        {
            Vector2Base result;
            clamp(v, min, max, result);
            return result;
        }

        // Restricts a value to be within a specified range (inclusive min/max).
        static void clamp(const Vector2Base& v, const Vector2Base& min, const Vector2Base& max, Vector2Base& result)
        {
            result = Vector2Base(TkMath::clamp(v.x, min.x, max.x), TkMath::clamp(v.y, min.y, max.y));
        }

        // Calculates distance between two points in 2D.
        static T distance(const Vector2Base& a, const Vector2Base& b)
        {
            const T x = a.x - b.x;
            const T y = a.y - b.y;
            return TkMath::sqrt(x * x + y * y);
        }

        // Calculates the squared distance between two points in 2D.
        static T distanceSquared(const Vector2Base& a, const Vector2Base& b)
        {
            const T x = a.x - b.x;
            const T y = a.y - b.y;
            return x * x + y * y;
        }

        // Performs vector normalization (scales vector up to unit length).
        static Vector2Base normalize(const Vector2Base& v)
        {
            Vector2Base r = v;
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
        static void lerp(const Vector2Base& start, const Vector2Base& end, T amount, Vector2Base& result)
        {
            result.x = TkMath::lerp(start.x, end.x, amount);
            result.y = TkMath::lerp(start.y, end.y, amount);
        }

        // Performs a linear interpolation between two vectors.
        static Vector2Base lerp(const Vector2Base& start, const Vector2Base& end, T amount)
        {
            Vector2Base result;
            lerp(start, end, amount, result);
            return result;
        }

        // Calculates the area of the triangle.
        TK_API static T triangleArea(const Vector2Base& v0, const Vector2Base& v1, const Vector2Base& v2);
        // Calculates the angle (in radians) between from and to. This is always the smallest value.
        TK_API static T angle(const Vector2Base& from, const Vector2Base& to);
    };


    template<typename T>
    inline Vector2Base<T> operator+(T a, const Vector2Base<T>& b)
    {
        return b + a;
    }

    template<typename T>
    inline Vector2Base<T> operator-(T a, const Vector2Base<T>& b)
    {
        return Vector2Base<T>(a) - b;
    }

    template<typename T>
    inline Vector2Base<T> operator*(T a, const Vector2Base<T>& b)
    {
        return b * a;
    }

    template<typename T>
    inline Vector2Base<T> operator/(T a, const Vector2Base<T>& b)
    {
        return Vector2Base<T>(a) / b;
    }

    template<typename T>
    inline Vector2Base<T> operator+(typename TOtherFloat<T>::Type a, const Vector2Base<T>& b)
    {
        return b + a;
    }

    template<typename T>
    inline Vector2Base<T> operator-(typename TOtherFloat<T>::Type a, const Vector2Base<T>& b)
    {
        return Vector2Base<T>(a) - b;
    }

    template<typename T>
    inline Vector2Base<T> operator*(typename TOtherFloat<T>::Type a, const Vector2Base<T>& b)
    {
        return b * a;
    }

    template<typename T>
    inline Vector2Base<T> operator/(typename TOtherFloat<T>::Type a, const Vector2Base<T>& b)
    {
        return Vector2Base<T>(a) / b;
    }

    template<typename T>
    inline u32 getHash(const Vector2Base<T>& key)
    {
        return (*(u32*)&key.x * 397) ^ *(u32*)&key.Y;
    }

    namespace math
    {
        template<typename T>
        TK_FORCE_INLINE static bool nearEqual(const Vector2Base<T>& a, const Vector2Base<T>& b)
        {
            return Vector2Base<T>::nearEqual(a, b);
        }
    }

    template<>
    struct TIsPODType<FVec2>
    {
        enum { Value = true };
    };

    template<>
    struct TIsPODType<DVec2>
    {
        enum { Value = true };
    };

    template<>
    struct TIsPODType<IVec2>
    {
        enum { Value = true };
    };

#if !defined(_MSC_VER) || defined(__clang__)
    // Forward specializations for Clang
    template<> TK_API const FVec2 FVec2::Zero;
    template<> TK_API const FVec2 FVec2::One;
    template<> TK_API const FVec2 FVec2::UnitX;
    template<> TK_API const FVec2 FVec2::UnitY;
    template<> TK_API const FVec2 FVec2::Minimum;
    template<> TK_API const FVec2 FVec2::Maximum;

    template<> TK_API const DVec2 DVec2::Zero;
    template<> TK_API const DVec2 DVec2::One;
    template<> TK_API const DVec2 DVec2::UnitX;
    template<> TK_API const DVec2 DVec2::UnitY;
    template<> TK_API const DVec2 DVec2::Minimum;
    template<> TK_API const DVec2 DVec2::Maximum;

    template<> TK_API const IVec2 IVec2::Zero;
    template<> TK_API const IVec2 IVec2::One;
    template<> TK_API const IVec2 IVec2::UnitX;
    template<> TK_API const IVec2 IVec2::UnitY;
    template<> TK_API const IVec2 IVec2::Minimum;
    template<> TK_API const IVec2 IVec2::Maximum;
#endif

}

FORMAT_CUSTOM(toolkit::FVec2, "X:%g Y:%g", t.x, t.y);
FORMAT_CUSTOM(toolkit::DVec2, "X:%g Y:%g", t.x, t.y);
FORMAT_CUSTOM(toolkit::IVec2, "X:%d Y:%d", t.x, t.y);
