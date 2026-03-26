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

    // Represents a three dimensional mathematical vector.
    template<typename T>
    struct Vector3Base
    {
        typedef T Real;

        union
        {
            struct
            {
                T x;
                T y;
                T z;
            };
            T raw[3];
        };

        // Vector with all components equal zero (0, 0, 0).
        static TK_API const Vector3Base Zero;
        // Vector with all components equal one (1, 1, 1).
        static TK_API const Vector3Base One;
        // Vector with all components equal half (0.5, 0.5, 0.5).
        static TK_API const Vector3Base Half;
        // The x unit vector (1, 0, 0).
        static TK_API const Vector3Base UnitX;
        // The y unit vector (0, 1, 0).
        static TK_API const Vector3Base UnitY;
        // The z unit vector (0, 0, 1).
        static TK_API const Vector3Base UnitZ;
        // A unit vector designating up (0, 1, 0).
        static TK_API const Vector3Base Up;
        // A unit vector designating down (0, -1, 0).
        static TK_API const Vector3Base Down;
        // A unit vector designating a (-1, 0, 0).
        static TK_API const Vector3Base Left;
        // A unit vector designating b (1, 0, 0).
        static TK_API const Vector3Base Right;
        // A unit vector designating forward in a a-handed coordinate system (0, 0, 1).
        static TK_API const Vector3Base Forward;
        // A unit vector designating backward in a a-handed coordinate system (0, 0, -1).
        static TK_API const Vector3Base Backward;
        // Vector with all components equal maximum value.
        static TK_API const Vector3Base Minimum;
        // Vector with all components equal minimum value.
        static TK_API const Vector3Base Maximum;

        // Empty constructor.
        Vector3Base() = default;

        TK_FORCE_INLINE Vector3Base(T xyz)
            : x {xyz}
            , y {xyz}
            , z {xyz}
        {
        }

        TK_FORCE_INLINE explicit Vector3Base(const T* xyz)
            : x(xyz[0])
            , y(xyz[1])
            , z(xyz[2])
        {
        }

        TK_FORCE_INLINE Vector3Base(T x, T y, T z)
            : x(x)
            , y(y)
            , z(z)
        {
        }

        template<typename U = T>
            requires TNotSameAs<T, U>
        TK_FORCE_INLINE Vector3Base(const Vector3Base<U>& xyz)
            : x((T)xyz.x)
            , y((T)xyz.y)
            , z((T)xyz.z)
        {
        }

        TK_API Vector3Base(const Vec2& xy, T z = 0);
        TK_API Vector3Base(const DVec2& xy, T z = 0);
        TK_API Vector3Base(const IVec2& xy, T z = 0);
        TK_API Vector3Base(const IVec3& xyz);
        TK_API explicit Vector3Base(const Vec4& xyz);
        TK_API explicit Vector3Base(const DVec4& xyz);
        TK_API explicit Vector3Base(const IVec4& xyz);
        TK_API explicit Vector3Base(const Color& color);

        TK_API TkString toString() const;

        bool isNormalized() const
        {
            return TkMath::abs((x * x + y * y + z * z) - 1.0f) < 1e-4f;
        }
        bool isZero() const
        {
            return TkMath::isZero(x) && TkMath::isZero(y) && TkMath::isZero(z);
        }
        bool isAnyZero() const
        {
            return TkMath::isZero(x) || TkMath::isZero(y) || TkMath::isZero(z);
        }
        bool isOne() const
        {
            return TkMath::isOne(x) && TkMath::isOne(y) && TkMath::isOne(z);
        }
        T length() const
        {
            return TkMath::sqrt(x * x + y * y + z * z);
        }
        T lengthSquared() const
        {
            return x * x + y * y + z * z;
        }
        T invertedLength() const
        {
            return 1.0f / length();
        }

        // Returns the average arithmetic of all the components.
        T averageArithmetic() const
        {
            return (x + y + z) * 0.333333334f;
        }

        // Gets the sum of all vector components values.
        T sumValues() const
        {
            return x + y + z;
        }

        // Returns the minimum value of all the components.
        T minValue() const
        {
            return TkMath::min(x, y, z);
        }

        // Returns the maximum value of all the components.
        T maxValue() const
        {
            return TkMath::max(x, y, z);
        }

        // Returns true if vector has one or more components is not a number (NaN).
        bool isNaN() const
        {
            return isnan(x) || isnan(y) || isnan(z);
        }

        // Returns true if vector has one or more components equal to +/- infinity.
        bool isInfinity() const
        {
            return isinf(x) || isinf(y) || isinf(z);
        }

        // Returns true if vector has one or more components equal to +/- infinity or NaN.
        bool isNanOrInfinity() const
        {
            return isInfinity() || isNaN();
        }

        // Calculates a vector with values being absolute values of that vector.
        Vector3Base getAbsolute() const
        {
            return {TkMath::abs(x), TkMath::abs(y), TkMath::abs(z)};
        }

        // Calculates a vector with values being opposite to values of that vector.
        Vector3Base getNegative() const
        {
            return {-x, -y, -z};
        }

        // Calculates a normalized vector that has length equal to 1.
        Vector3Base getNormalized() const
        {
            Vector3Base result(x, y, z);
            result.normalize();
            return result;
        }

        // Performs vector normalization (scales vector up to unit length).
        void normalize()
        {
            const T length = TkMath::sqrt(x * x + y * y + z * z);
            if (length >= TkMath::ZeroTolerance)
            {
                const T inv = (T)1.0f / length;
                x *= inv;
                y *= inv;
                z *= inv;
            }
        }

        // Performs fast vector normalization (scales vector up to unit length).
        void normalizeFast()
        {
            const T inv = 1.0f / TkMath::sqrt(x * x + y * y + z * z);
            x *= inv;
            y *= inv;
            z *= inv;
        }

        Vector3Base operator+(const Vector3Base& b) const
        {
            return {x + b.x, y + b.y, z + b.z};
        }

        Vector3Base operator-(const Vector3Base& b) const
        {
            return {x - b.x, y - b.y, z - b.z};
        }

        Vector3Base operator*(const Vector3Base& b) const
        {
            return {x * b.x, y * b.y, z * b.z};
        }

        Vector3Base operator/(const Vector3Base& b) const
        {
            return {x / b.x, y / b.y, z / b.z};
        }

        Vector3Base operator-() const
        {
            return {-x, -y, -z};
        }

        Vector3Base operator+(T b) const
        {
            return {x + b, y + b, z + b};
        }

        Vector3Base operator-(T b) const
        {
            return {x - b, y - b, z - b};
        }

        Vector3Base operator*(T b) const
        {
            return {x * b, y * b, z * b};
        }

        Vector3Base operator/(T b) const
        {
            return {x / b, y / b, z / b};
        }

        Vector3Base operator+(typename TOtherFloat<T>::Type a) const
        {
            T b = (T)a;
            return {x + b, y + b, z + b};
        }

        Vector3Base operator-(typename TOtherFloat<T>::Type a) const
        {
            T b = (T)a;
            return {x - b, y - b, z - b};
        }

        Vector3Base operator*(typename TOtherFloat<T>::Type a) const
        {
            T b = (T)a;
            return {x * (T)b, y * b, z * b};
        }

        Vector3Base operator/(typename TOtherFloat<T>::Type a) const
        {
            T b = (T)a;
            return {x / b, y / b, z / b};
        }

        Vector3Base operator^(const Vector3Base& b) const
        {
            return cross(*this, b);
        }

        T operator|(const Vector3Base& b) const
        {
            return dot(*this, b);
        }

        Vector3Base& operator+=(const Vector3Base& b)
        {
            x += b.x;
            y += b.y;
            z += b.z;
            return *this;
        }

        Vector3Base& operator-=(const Vector3Base& b)
        {
            x -= b.x;
            y -= b.y;
            z -= b.z;
            return *this;
        }

        Vector3Base& operator*=(const Vector3Base& b)
        {
            x *= b.x;
            y *= b.y;
            z *= b.z;
            return *this;
        }

        Vector3Base& operator/=(const Vector3Base& b)
        {
            x /= b.x;
            y /= b.y;
            z /= b.z;
            return *this;
        }

        Vector3Base& operator+=(T b)
        {
            x += b;
            y += b;
            z += b;
            return *this;
        }

        Vector3Base& operator-=(T b)
        {
            x -= b;
            y -= b;
            z -= b;
            return *this;
        }

        Vector3Base& operator*=(T b)
        {
            x *= b;
            y *= b;
            z *= b;
            return *this;
        }

        Vector3Base& operator/=(T b)
        {
            x /= b;
            y /= b;
            z /= b;
            return *this;
        }

        bool operator==(const Vector3Base& b) const
        {
            return x == b.x && y == b.y && z == b.z;
        }

        bool operator!=(const Vector3Base& b) const
        {
            return x != b.x || y != b.y || z != b.z;
        }

        bool operator>(const Vector3Base& b) const
        {
            return x > b.x && y > b.y && z > b.z;
        }

        bool operator>=(const Vector3Base& b) const
        {
            return x >= b.x && y >= b.y && z >= b.z;
        }

        bool operator<(const Vector3Base& b) const
        {
            return x < b.x && y < b.y && z < b.z;
        }

        bool operator<=(const Vector3Base& b) const
        {
            return x <= b.x && y <= b.y && z <= b.z;
        }

        static bool nearEqual(const Vector3Base& a, const Vector3Base& b)
        {
            return TkMath::nearEqual(a.x, b.x) && TkMath::nearEqual(a.y, b.y) && TkMath::nearEqual(a.z, b.z);
        }

        static bool nearEqual(const Vector3Base& a, const Vector3Base& b, T epsilon)
        {
            return TkMath::nearEqual(a.x, b.x, epsilon) && TkMath::nearEqual(a.y, b.y, epsilon) && TkMath::nearEqual(a.z, b.z, epsilon);
        }

        static void add(const Vector3Base& a, const Vector3Base& b, Vector3Base& result)
        {
            result = {a.x + b.x, a.y + b.y, a.z + b.z};
        }

        static void subtract(const Vector3Base& a, const Vector3Base& b, Vector3Base& result)
        {
            result = {a.x - b.x, a.y - b.y, a.z - b.z};
        }

        static void multiply(const Vector3Base& a, const Vector3Base& b, Vector3Base& result)
        {
            result = {a.x * b.x, a.y * b.y, a.z * b.z};
        }

        static void divide(const Vector3Base& a, const Vector3Base& b, Vector3Base& result)
        {
            result = {a.x / b.x, a.y / b.y, a.z / b.z};
        }

        static void min(const Vector3Base& a, const Vector3Base& b, Vector3Base& result)
        {
            result = {a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y, a.z < b.z ? a.z : b.z};
        }

        static void max(const Vector3Base& a, const Vector3Base& b, Vector3Base& result)
        {
            result = {a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y, a.z > b.z ? a.z : b.z};
        }

        static Vector3Base min(const Vector3Base& a, const Vector3Base& b)
        {
            return {a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y, a.z < b.z ? a.z : b.z};
        }

        static Vector3Base max(const Vector3Base& a, const Vector3Base& b)
        {
            return {a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y, a.z > b.z ? a.z : b.z};
        }

        static Vector3Base mod(const Vector3Base& a, const Vector3Base& b)
        {
            return {TkMath::mod(a.x, b.x), TkMath::mod(a.y, b.y), TkMath::mod(a.z, b.z)};
        }

        static Vector3Base floor(const Vector3Base& v)
        {
            return {TkMath::floor(v.x), TkMath::floor(v.y), TkMath::floor(v.z)};
        }

        static Vector3Base frac(const Vector3Base& v)
        {
            return {v.x - (i32)v.x, v.y - (i32)v.y, v.z - (i32)v.z};
        }

        static Vector3Base round(const Vector3Base& v)
        {
            return {TkMath::round(v.x), TkMath::round(v.y), TkMath::round(v.z)};
        }

        static Vector3Base ceil(const Vector3Base& v)
        {
            return {TkMath::ceil(v.x), TkMath::ceil(v.y), TkMath::ceil(v.z)};
        }

        static Vector3Base abs(const Vector3Base& v)
        {
            return {TkMath::abs(v.x), TkMath::abs(v.y), TkMath::abs(v.z)};
        }

        // Restricts a value to be within a specified range (inclusive min/max).
        static Vector3Base clamp(const Vector3Base& v, const Vector3Base& min, const Vector3Base& max)
        {
            Vector3Base result;
            clamp(v, min, max, result);
            return result;
        }

        // Restricts a value to be within a specified range (inclusive min/max).
        static void clamp(const Vector3Base& v, const Vector3Base& min, const Vector3Base& max, Vector3Base& result)
        {
            result = {TkMath::clamp(v.x, min.x, max.x), TkMath::clamp(v.y, min.y, max.y), TkMath::clamp(v.z, min.z, max.z)};
        }

        // Makes sure that Length of the output vector is always below max and above 0.
        static Vector3Base clampLength(const Vector3Base& v, float max)
        {
            return clampLength(v, 0, max);
        }

        // Makes sure that Length of the output vector is always below max and above min.
        static Vector3Base clampLength(const Vector3Base& v, float min, float max)
        {
            Vector3Base result;
            clampLength(v, min, max, result);
            return result;
        }

        // Makes sure that Length of the output vector is always below max and above min.
        static void clampLength(const Vector3Base& v, float min, float max, Vector3Base& result)
        {
            result = v;
            T lenSq = result.lengthSquared();
            if (lenSq > max * max)
            {
                T scaleFactor = max / (T)TkMath::sqrt(lenSq);
                result.x *= scaleFactor;
                result.y *= scaleFactor;
                result.z *= scaleFactor;
            }
            if (lenSq < min * min)
            {
                T scaleFactor = min / (T)TkMath::sqrt(lenSq);
                result.x *= scaleFactor;
                result.y *= scaleFactor;
                result.z *= scaleFactor;
            }
        }

        // Calculates the distance between two vectors.
        static T distance(const Vector3Base& a, const Vector3Base& b)
        {
            const T x = a.x - b.x;
            const T y = a.y - b.y;
            const T z = a.z - b.z;
            return TkMath::sqrt(x * x + y * y + z * z);
        }

        // Calculates the squared distance between two vectors.
        static T distanceSquared(const Vector3Base& a, const Vector3Base& b)
        {
            const T x = a.x - b.x;
            const T y = a.y - b.y;
            const T z = a.z - b.z;
            return x * x + y * y + z * z;
        }
        // Performs vector normalization (scales vector up to unit length).
        static Vector3Base normalize(const Vector3Base& v)
        {
            Vector3Base r = v;
            const T length = TkMath::sqrt(r.x * r.x + r.y * r.y + r.z * r.z);
            if (length >= TkMath::ZeroTolerance)
            {
                const T inv = (T)1.0f / length;
                r.x *= inv;
                r.y *= inv;
                r.z *= inv;
            }
            return r;
        }
        // Performs vector normalization (scales vector up to unit length). This is a faster version that does not perform check for length equal 0 (it assumes that input vector is not empty).
        static Vector3Base normalizeFast(const Vector3Base& v)
        {
            const T inv = 1.0f / v.length();
            return {v.x * inv, v.y * inv, v.z * inv};
        }
        // Performs vector normalization (scales vector up to unit length).
        static TK_FORCE_INLINE void normalize(const Vector3Base& input, Vector3Base& result)
        {
            result = normalize(input);
        }
        // Calculates the dot product of two vectors.
        TK_FORCE_INLINE static T dot(const Vector3Base& a, const Vector3Base& b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }
        // Calculates the cross product of two vectors.
        static void cross(const Vector3Base& a, const Vector3Base& b, Vector3Base& result)
        {
            result = {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
        }
        // Calculates the cross product of two vectors.
        static Vector3Base cross(const Vector3Base& a, const Vector3Base& b)
        {
            return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
        }
        // Performs a linear interpolation between two vectors.
        static void lerp(const Vector3Base& start, const Vector3Base& end, T amount, Vector3Base& result)
        {
            result.x = TkMath::lerp(start.x, end.x, amount);
            result.y = TkMath::lerp(start.y, end.y, amount);
            result.z = TkMath::lerp(start.z, end.z, amount);
        }
        // Performs a linear interpolation between two vectors.
        static Vector3Base lerp(const Vector3Base& start, const Vector3Base& end, T amount)
        {
            Vector3Base result;
            lerp(start, end, amount, result);
            return result;
        }
        // Performs a cubic interpolation between two vectors.
        static void smoothStep(const Vector3Base& start, const Vector3Base& end, T amount, Vector3Base& result)
        {
            amount = TkMath::smoothStep(amount);
            lerp(start, end, amount, result);
        }
        // Moves a value current towards target.
        static Vec3 moveTowards(Vec3 current, Vec3 target, float maxDistanceDelta)
        {
            const Vector3Base to = target - current;
            const T distanceSq = to.lengthSquared();
            if (distanceSq == 0 || (maxDistanceDelta >= 0 && distanceSq <= maxDistanceDelta * maxDistanceDelta))
                return target;
            const T scale = maxDistanceDelta / TkMath::sqrt(distanceSq);
            return {current.x + to.x * scale, current.y + to.y * scale, current.z + to.z * scale};
        }
        // Performs a Hermite spline interpolation.
        static TK_API void hermite(const Vector3Base& value1, const Vector3Base& tangent1, const Vector3Base& value2, const Vector3Base& tangent2, T amount, Vector3Base& result);
        // Returns the reflection of a vector off a surface that has the specified normal.
        static TK_API void reflect(const Vector3Base& vector, const Vector3Base& normal, Vector3Base& result);
        // Transforms a 3D vector by the given Quaternion rotation.
        static TK_API void transform(const Vector3Base& vector, const Quat& rotation, Vector3Base& result);
        // Transforms a 3D vector by the given Quaternion rotation.
        static TK_API Vector3Base transform(const Vector3Base& vector, const Quat& rotation);
        // Transforms a 3D vector by the given matrix.
        static TK_API void transform(const Vector3Base& vector, const Mat& transform, Vector3Base& result);
        // Transforms a 3D vector by the given matrix.
        static TK_API void transform(const Vector3Base& vector, const Mat3x3& transform, Vector3Base& result);
        // Transforms a 3D vector by the given transformation.
        static TK_API void transform(const Vector3Base& vector, const Transform& transform, Vector3Base& result);
        // Transforms a 3D vector by the given matrix.
        static TK_API Vector3Base transform(const Vector3Base& vector, const Mat& transform);
        // Transforms a 3D vector by the given transformation.
        static TK_API Vector3Base transform(const Vector3Base& vector, const Transform& transform);
        // Transforms a 3D vector by the given matrix.
        static TK_API void transform(const Vector3Base& vector, const Mat& transform, Vector4Base<T>& result);
        // Performs a coordinate transformation using the given matrix.
        static TK_API void transformCoordinate(const Vector3Base& coordinate, const Mat& transform, Vector3Base& result);
        // Performs a normal transformation using the given matrix.
        static TK_API void transformNormal(const Vector3Base& normal, const Mat& transform, Vector3Base& result);
        // Projects a vector onto another vector.
        static TK_API Vector3Base project(const Vector3Base& vector, const Vector3Base& onNormal);
        // Projects a vector onto a plane defined by a normal orthogonal to the plane.
        static Vector3Base projectOnPlane(const Vector3Base& vector, const Vector3Base& planeNormal)
        {
            return vector - project(vector, planeNormal);
        }

        // Projects a 3D vector from object space into screen space using the provided viewport and transformation matrix.
        static TK_API void project(const Vector3Base& vector, float x, float y, float width, float height, float minZ, float maxZ, const Mat& worldViewProjection, Vector3Base& result);
        // Projects a 3D vector from object space into screen space using the provided viewport and transformation matrix.
        static Vector3Base project(const Vector3Base& vector, float x, float y, float width, float height, float minZ, float maxZ, const Mat& worldViewProjection)
        {
            Vector3Base result;
            project(vector, x, y, width, height, minZ, maxZ, worldViewProjection, result);
            return result;
        }

        // Projects a 3D vector from screen space into object space using the provided viewport and transformation matrix.
        static TK_API void unproject(const Vector3Base& vector, float x, float y, float width, float height, float minZ, float maxZ, const Mat& worldViewProjection, Vector3Base& result);
        // Projects a 3D vector from screen space into object space using the provided viewport and transformation matrix.
        static Vector3Base unproject(const Vector3Base& vector, float x, float y, float width, float height, float minZ, float maxZ, const Mat& worldViewProjection)
        {
            Vector3Base result;
            unproject(vector, x, y, width, height, minZ, maxZ, worldViewProjection, result);
            return result;
        }

        // Creates an orthonormal basis from a basis with at least two orthogonal vectors.
        static TK_API void createOrthonormalBasis(Vector3Base& xAxis, Vector3Base& yAxis, Vector3Base& zAxis);
        // Finds the best arbitrary axis vectors to represent U and V axes of a plane, by using this vector as the normal of the plane.
        TK_API void findBestAxisVectors(Vector3Base& firstAxis, Vector3Base& secondAxis) const;
        // Calculates the area of the triangle.
        static TK_API T triangleArea(const Vector3Base& v0, const Vector3Base& v1, const Vector3Base& v2);
        // Calculates the angle (in radians) between from and to. This is always the smallest value.
        static TK_API T angle(const Vector3Base& from, const Vector3Base& to);
        // Snaps the input position onto the grid.
        static TK_API Vector3Base snapToGrid(const Vector3Base& pos, const Vector3Base& gridSize);
        static TK_API Vector3Base snapToGrid(const Vector3Base& point, const Vector3Base& gridSize, const Quat& gridOrientation, const Vector3Base& gridOrigin = Zero, const Vector3Base& offset = Zero);
    };

    template<typename T>
    inline Vector3Base<T> operator+(T a, const Vector3Base<T>& b)
    {
        return b + a;
    }

    template<typename T>
    inline Vector3Base<T> operator-(T a, const Vector3Base<T>& b)
    {
        return Vector3Base<T>(a) - b;
    }

    template<typename T>
    inline Vector3Base<T> operator*(T a, const Vector3Base<T>& b)
    {
        return b * a;
    }

    template<typename T>
    inline Vector3Base<T> operator/(T a, const Vector3Base<T>& b)
    {
        return Vector3Base<T>(a) / b;
    }

    template<typename T>
    inline Vector3Base<T> operator+(typename TOtherFloat<T>::Type a, const Vector3Base<T>& b)
    {
        return b + a;
    }

    template<typename T>
    inline Vector3Base<T> operator-(typename TOtherFloat<T>::Type a, const Vector3Base<T>& b)
    {
        return Vector3Base<T>(a) - b;
    }

    template<typename T>
    inline Vector3Base<T> operator*(typename TOtherFloat<T>::Type a, const Vector3Base<T>& b)
    {
        return b * a;
    }

    template<typename T>
    inline Vector3Base<T> operator/(typename TOtherFloat<T>::Type a, const Vector3Base<T>& b)
    {
        return Vector3Base<T>(a) / b;
    }

    template<typename T>
    inline u32 getHash(const Vector3Base<T>& key)
    {
        return (((*(u32*)&key.x * 397) ^ *(u32*)&key.y) * 397) ^ *(u32*)&key.z;
    }

    namespace math
    {
        template<typename T>
        TK_FORCE_INLINE static bool nearEqual(const Vector3Base<T>& a, const Vector3Base<T>& b)
        {
            return Vector3Base<T>::nearEqual(a, b);
        }

        template<typename T>
        TK_FORCE_INLINE static Vector3Base<T> UnwindDegrees(const Vector3Base<T>& v)
        {
            return Vector3Base<T>{UnwindDegrees(v.x), UnwindDegrees(v.y), UnwindDegrees(v.z)};
        }
    }

    template<>
    struct TIsPODType<FVec3>
    {
        enum { Value = true };
    };

    template<>
    struct TIsPODType<DVec3>
    {
        enum { Value = true };
    };

    template<>
    struct TIsPODType<IVec3>
    {
        enum { Value = true };
    };

#if !defined(_MSC_VER) || defined(__clang__)
    // Forward specializations for Clang
    template<> TK_API const FVec3 FVec3::Zero;
    template<> TK_API const FVec3 FVec3::One;
    template<> TK_API const FVec3 FVec3::Half;
    template<> TK_API const FVec3 FVec3::UnitX;
    template<> TK_API const FVec3 FVec3::UnitY;
    template<> TK_API const FVec3 FVec3::UnitZ;
    template<> TK_API const FVec3 FVec3::Up;
    template<> TK_API const FVec3 FVec3::Down;
    template<> TK_API const FVec3 FVec3::Left;
    template<> TK_API const FVec3 FVec3::Right;
    template<> TK_API const FVec3 FVec3::Forward;
    template<> TK_API const FVec3 FVec3::Backward;
    template<> TK_API const FVec3 FVec3::Minimum;
    template<> TK_API const FVec3 FVec3::Maximum;

    template<> TK_API const DVec3 DVec3::Zero;
    template<> TK_API const DVec3 DVec3::One;
    template<> TK_API const DVec3 DVec3::Half;
    template<> TK_API const DVec3 DVec3::UnitX;
    template<> TK_API const DVec3 DVec3::UnitY;
    template<> TK_API const DVec3 DVec3::UnitZ;
    template<> TK_API const DVec3 DVec3::Up;
    template<> TK_API const DVec3 DVec3::Down;
    template<> TK_API const DVec3 DVec3::Left;
    template<> TK_API const DVec3 DVec3::Right;
    template<> TK_API const DVec3 DVec3::Forward;
    template<> TK_API const DVec3 DVec3::Backward;
    template<> TK_API const DVec3 DVec3::Minimum;
    template<> TK_API const DVec3 DVec3::Maximum;

    template<> TK_API const IVec3 IVec3::Zero;
    template<> TK_API const IVec3 IVec3::One;
    template<> TK_API const IVec3 IVec3::Half;
    template<> TK_API const IVec3 IVec3::UnitX;
    template<> TK_API const IVec3 IVec3::UnitY;
    template<> TK_API const IVec3 IVec3::UnitZ;
    template<> TK_API const IVec3 IVec3::Up;
    template<> TK_API const IVec3 IVec3::Down;
    template<> TK_API const IVec3 IVec3::Left;
    template<> TK_API const IVec3 IVec3::Right;
    template<> TK_API const IVec3 IVec3::Forward;
    template<> TK_API const IVec3 IVec3::Backward;
    template<> TK_API const IVec3 IVec3::Minimum;
    template<> TK_API const IVec3 IVec3::Maximum;
#endif
}


FORMAT_CUSTOM(toolkit::FVec3, "X:%g Y:%g Z:%g", t.x, t.y, t.z);
FORMAT_CUSTOM(toolkit::DVec3, "X:%g Y:%g Z:%g", t.x, t.y, t.z);
FORMAT_CUSTOM(toolkit::IVec3, "X:%d Y:%d Z:%d", t.x, t.y, t.z);
