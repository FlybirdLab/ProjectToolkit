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
    // Represents a three dimensional mathematical vector.
    template <typename T>
    class Vector3
    {
    public:
        // Vector with all components equal zero (0, 0, 0).
        static TK_API const Vector3 kZero;
        // Vector with all components equal one (1, 1, 1).
        static TK_API const Vector3 kOne;
        // Vector with all components equal half (0.5, 0.5, 0.5).
        static TK_API const Vector3 kHalf;
        // The x unit vector (1, 0, 0).
        static TK_API const Vector3 kUnitX;
        // The y unit vector (0, 1, 0).
        static TK_API const Vector3 kUnitY;
        // The z unit vector (0, 0, 1).
        static TK_API const Vector3 kUnitZ;
        // A unit vector designating up (0, 1, 0).
        static TK_API const Vector3 kUp;
        // A unit vector designating down (0, -1, 0).
        static TK_API const Vector3 kDown;
        // A unit vector designating a (-1, 0, 0).
        static TK_API const Vector3 kLeft;
        // A unit vector designating b (1, 0, 0).
        static TK_API const Vector3 kRight;
        // A unit vector designating forward in a a-handed coordinate system (0, 0, 1).
        static TK_API const Vector3 kForward;
        // A unit vector designating backward in a a-handed coordinate system (0, 0, -1).
        static TK_API const Vector3 kBackward;
        // Vector with all components equal maximum value.
        static TK_API const Vector3 kMinimum;
        // Vector with all components equal minimum value.
        static TK_API const Vector3 kMaximum;

    public:
        Vector3() = default;

        TK_FORCE_INLINE Vector3(T xyz)
            : x{xyz}, y{xyz}, z{xyz}
        {
        }

        TK_FORCE_INLINE explicit Vector3(const T *xyz)
            : x(xyz[0]), y(xyz[1]), z(xyz[2])
        {
        }

        TK_FORCE_INLINE Vector3(T x, T y, T z)
            : x(x), y(y), z(z)
        {
        }

        template <typename U = T>
            requires TNotSameAs<T, U>
        TK_FORCE_INLINE Vector3(const Vector3<U> &xyz)
            : x((T)xyz.x), y((T)xyz.y), z((T)xyz.z)
        {
        }

        TK_API Vector3(const Vec2 &xy, T z = 0);
        TK_API Vector3(const DVec2 &xy, T z = 0);
        TK_API Vector3(const IVec2 &xy, T z = 0);
        TK_API Vector3(const IVec3 &xyz);
        TK_API explicit Vector3(const Vec4 &xyz);
        TK_API explicit Vector3(const DVec4 &xyz);
        TK_API explicit Vector3(const IVec4 &xyz);
        TK_API explicit Vector3(const Color &color);

    public:
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
        Vector3 getAbsolute() const
        {
            return {TkMath::abs(x), TkMath::abs(y), TkMath::abs(z)};
        }

        // Calculates a vector with values being opposite to values of that vector.
        Vector3 getNegative() const
        {
            return {-x, -y, -z};
        }

        // Calculates a normalized vector that has length equal to 1.
        Vector3 getNormalized() const
        {
            Vector3 result(x, y, z);
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

    public:
        Vector3 operator+(const Vector3 &b) const
        {
            return {x + b.x, y + b.y, z + b.z};
        }

        Vector3 operator-(const Vector3 &b) const
        {
            return {x - b.x, y - b.y, z - b.z};
        }

        Vector3 operator*(const Vector3 &b) const
        {
            return {x * b.x, y * b.y, z * b.z};
        }

        Vector3 operator/(const Vector3 &b) const
        {
            return {x / b.x, y / b.y, z / b.z};
        }

        Vector3 operator-() const
        {
            return {-x, -y, -z};
        }

        Vector3 operator+(T b) const
        {
            return {x + b, y + b, z + b};
        }

        Vector3 operator-(T b) const
        {
            return {x - b, y - b, z - b};
        }

        Vector3 operator*(T b) const
        {
            return {x * b, y * b, z * b};
        }

        Vector3 operator/(T b) const
        {
            return {x / b, y / b, z / b};
        }

        Vector3 operator+(typename TOtherFloat<T>::Type a) const
        {
            T b = (T)a;
            return {x + b, y + b, z + b};
        }

        Vector3 operator-(typename TOtherFloat<T>::Type a) const
        {
            T b = (T)a;
            return {x - b, y - b, z - b};
        }

        Vector3 operator*(typename TOtherFloat<T>::Type a) const
        {
            T b = (T)a;
            return {x * (T)b, y * b, z * b};
        }

        Vector3 operator/(typename TOtherFloat<T>::Type a) const
        {
            T b = (T)a;
            return {x / b, y / b, z / b};
        }

        Vector3 operator^(const Vector3 &b) const
        {
            return cross(*this, b);
        }

        T operator|(const Vector3 &b) const
        {
            return dot(*this, b);
        }

        Vector3 &operator+=(const Vector3 &b)
        {
            x += b.x;
            y += b.y;
            z += b.z;
            return *this;
        }

        Vector3 &operator-=(const Vector3 &b)
        {
            x -= b.x;
            y -= b.y;
            z -= b.z;
            return *this;
        }

        Vector3 &operator*=(const Vector3 &b)
        {
            x *= b.x;
            y *= b.y;
            z *= b.z;
            return *this;
        }

        Vector3 &operator/=(const Vector3 &b)
        {
            x /= b.x;
            y /= b.y;
            z /= b.z;
            return *this;
        }

        Vector3 &operator+=(T b)
        {
            x += b;
            y += b;
            z += b;
            return *this;
        }

        Vector3 &operator-=(T b)
        {
            x -= b;
            y -= b;
            z -= b;
            return *this;
        }

        Vector3 &operator*=(T b)
        {
            x *= b;
            y *= b;
            z *= b;
            return *this;
        }

        Vector3 &operator/=(T b)
        {
            x /= b;
            y /= b;
            z /= b;
            return *this;
        }

        bool operator==(const Vector3 &b) const
        {
            return x == b.x && y == b.y && z == b.z;
        }

        bool operator!=(const Vector3 &b) const
        {
            return x != b.x || y != b.y || z != b.z;
        }

        bool operator>(const Vector3 &b) const
        {
            return x > b.x && y > b.y && z > b.z;
        }

        bool operator>=(const Vector3 &b) const
        {
            return x >= b.x && y >= b.y && z >= b.z;
        }

        bool operator<(const Vector3 &b) const
        {
            return x < b.x && y < b.y && z < b.z;
        }

        bool operator<=(const Vector3 &b) const
        {
            return x <= b.x && y <= b.y && z <= b.z;
        }

    public:
        static bool nearEqual(const Vector3 &a, const Vector3 &b)
        {
            return TkMath::nearEqual(a.x, b.x) && TkMath::nearEqual(a.y, b.y) && TkMath::nearEqual(a.z, b.z);
        }

        static bool nearEqual(const Vector3 &a, const Vector3 &b, T epsilon)
        {
            return TkMath::nearEqual(a.x, b.x, epsilon) && TkMath::nearEqual(a.y, b.y, epsilon) && TkMath::nearEqual(a.z, b.z, epsilon);
        }

        static void add(const Vector3 &a, const Vector3 &b, Vector3 &result)
        {
            result = {a.x + b.x, a.y + b.y, a.z + b.z};
        }

        static void subtract(const Vector3 &a, const Vector3 &b, Vector3 &result)
        {
            result = {a.x - b.x, a.y - b.y, a.z - b.z};
        }

        static void multiply(const Vector3 &a, const Vector3 &b, Vector3 &result)
        {
            result = {a.x * b.x, a.y * b.y, a.z * b.z};
        }

        static void divide(const Vector3 &a, const Vector3 &b, Vector3 &result)
        {
            result = {a.x / b.x, a.y / b.y, a.z / b.z};
        }

        static void min(const Vector3 &a, const Vector3 &b, Vector3 &result)
        {
            result = {a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y, a.z < b.z ? a.z : b.z};
        }

        static void max(const Vector3 &a, const Vector3 &b, Vector3 &result)
        {
            result = {a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y, a.z > b.z ? a.z : b.z};
        }

        static Vector3 min(const Vector3 &a, const Vector3 &b)
        {
            return {a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y, a.z < b.z ? a.z : b.z};
        }

        static Vector3 max(const Vector3 &a, const Vector3 &b)
        {
            return {a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y, a.z > b.z ? a.z : b.z};
        }

        static Vector3 mod(const Vector3 &a, const Vector3 &b)
        {
            return {TkMath::mod(a.x, b.x), TkMath::mod(a.y, b.y), TkMath::mod(a.z, b.z)};
        }

        static Vector3 floor(const Vector3 &v)
        {
            return {TkMath::floor(v.x), TkMath::floor(v.y), TkMath::floor(v.z)};
        }

        static Vector3 frac(const Vector3 &v)
        {
            return {v.x - (i32)v.x, v.y - (i32)v.y, v.z - (i32)v.z};
        }

        static Vector3 round(const Vector3 &v)
        {
            return {TkMath::round(v.x), TkMath::round(v.y), TkMath::round(v.z)};
        }

        static Vector3 ceil(const Vector3 &v)
        {
            return {TkMath::ceil(v.x), TkMath::ceil(v.y), TkMath::ceil(v.z)};
        }

        static Vector3 abs(const Vector3 &v)
        {
            return {TkMath::abs(v.x), TkMath::abs(v.y), TkMath::abs(v.z)};
        }

        // Restricts a value to be within a specified range (inclusive min/max).
        static Vector3 clamp(const Vector3 &v, const Vector3 &min, const Vector3 &max)
        {
            Vector3 result;
            clamp(v, min, max, result);
            return result;
        }

        // Restricts a value to be within a specified range (inclusive min/max).
        static void clamp(const Vector3 &v, const Vector3 &min, const Vector3 &max, Vector3 &result)
        {
            result = {TkMath::clamp(v.x, min.x, max.x), TkMath::clamp(v.y, min.y, max.y), TkMath::clamp(v.z, min.z, max.z)};
        }

        // Makes sure that Length of the output vector is always below max and above 0.
        static Vector3 clampLength(const Vector3 &v, float max)
        {
            return clampLength(v, 0, max);
        }

        // Makes sure that Length of the output vector is always below max and above min.
        static Vector3 clampLength(const Vector3 &v, float min, float max)
        {
            Vector3 result;
            clampLength(v, min, max, result);
            return result;
        }

        // Makes sure that Length of the output vector is always below max and above min.
        static void clampLength(const Vector3 &v, float min, float max, Vector3 &result)
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
        static T distance(const Vector3 &a, const Vector3 &b)
        {
            const T x = a.x - b.x;
            const T y = a.y - b.y;
            const T z = a.z - b.z;
            return TkMath::sqrt(x * x + y * y + z * z);
        }

        // Calculates the squared distance between two vectors.
        static T distanceSquared(const Vector3 &a, const Vector3 &b)
        {
            const T x = a.x - b.x;
            const T y = a.y - b.y;
            const T z = a.z - b.z;
            return x * x + y * y + z * z;
        }
        // Performs vector normalization (scales vector up to unit length).
        static Vector3 normalize(const Vector3 &v)
        {
            Vector3 r = v;
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
        static Vector3 normalizeFast(const Vector3 &v)
        {
            const T inv = 1.0f / v.length();
            return {v.x * inv, v.y * inv, v.z * inv};
        }
        // Performs vector normalization (scales vector up to unit length).
        static TK_FORCE_INLINE void normalize(const Vector3 &input, Vector3 &result)
        {
            result = normalize(input);
        }
        // Calculates the dot product of two vectors.
        TK_FORCE_INLINE static T dot(const Vector3 &a, const Vector3 &b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }
        // Calculates the cross product of two vectors.
        static void cross(const Vector3 &a, const Vector3 &b, Vector3 &result)
        {
            result = {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
        }
        // Calculates the cross product of two vectors.
        static Vector3 cross(const Vector3 &a, const Vector3 &b)
        {
            return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
        }
        // Performs a linear interpolation between two vectors.
        static void lerp(const Vector3 &start, const Vector3 &end, T amount, Vector3 &result)
        {
            result.x = TkMath::lerp(start.x, end.x, amount);
            result.y = TkMath::lerp(start.y, end.y, amount);
            result.z = TkMath::lerp(start.z, end.z, amount);
        }
        // Performs a linear interpolation between two vectors.
        static Vector3 lerp(const Vector3 &start, const Vector3 &end, T amount)
        {
            Vector3 result;
            lerp(start, end, amount, result);
            return result;
        }
        // Performs a cubic interpolation between two vectors.
        static void smoothStep(const Vector3 &start, const Vector3 &end, T amount, Vector3 &result)
        {
            amount = TkMath::smoothStep(amount);
            lerp(start, end, amount, result);
        }
        // Moves a value current towards target.
        static Vec3 moveTowards(Vec3 current, Vec3 target, float maxDistanceDelta)
        {
            const Vector3 to = target - current;
            const T distanceSq = to.lengthSquared();
            if (distanceSq == 0 || (maxDistanceDelta >= 0 && distanceSq <= maxDistanceDelta * maxDistanceDelta))
                return target;
            const T scale = maxDistanceDelta / TkMath::sqrt(distanceSq);
            return {current.x + to.x * scale, current.y + to.y * scale, current.z + to.z * scale};
        }
        // Performs a Hermite spline interpolation.
        static TK_API void hermite(const Vector3 &value1, const Vector3 &tangent1, const Vector3 &value2, const Vector3 &tangent2, T amount, Vector3 &result);
        // Returns the reflection of a vector off a surface that has the specified normal.
        static TK_API void reflect(const Vector3 &vector, const Vector3 &normal, Vector3 &result);
        // Transforms a 3D vector by the given Quaternion rotation.
        static TK_API void transform(const Vector3 &vector, const Quat &rotation, Vector3 &result);
        // Transforms a 3D vector by the given Quaternion rotation.
        static TK_API Vector3 transform(const Vector3 &vector, const Quat &rotation);
        // Transforms a 3D vector by the given matrix.
        static TK_API void transform(const Vector3 &vector, const Mat &transform, Vector3 &result);
        // Transforms a 3D vector by the given matrix.
        static TK_API void transform(const Vector3 &vector, const Mat3x3 &transform, Vector3 &result);
        // Transforms a 3D vector by the given transformation.
        static TK_API void transform(const Vector3 &vector, const Transform &transform, Vector3 &result);
        // Transforms a 3D vector by the given matrix.
        static TK_API Vector3 transform(const Vector3 &vector, const Mat &transform);
        // Transforms a 3D vector by the given transformation.
        static TK_API Vector3 transform(const Vector3 &vector, const Transform &transform);
        // Transforms a 3D vector by the given matrix.
        static TK_API void transform(const Vector3 &vector, const Mat &transform, Vector4<T> &result);
        // Performs a coordinate transformation using the given matrix.
        static TK_API void transformCoordinate(const Vector3 &coordinate, const Mat &transform, Vector3 &result);
        // Performs a normal transformation using the given matrix.
        static TK_API void transformNormal(const Vector3 &normal, const Mat &transform, Vector3 &result);
        // Projects a vector onto another vector.
        static TK_API Vector3 project(const Vector3 &vector, const Vector3 &onNormal);
        // Projects a vector onto a plane defined by a normal orthogonal to the plane.
        static Vector3 projectOnPlane(const Vector3 &vector, const Vector3 &planeNormal)
        {
            return vector - project(vector, planeNormal);
        }

        // Projects a 3D vector from object space into screen space using the provided viewport and transformation matrix.
        static TK_API void project(const Vector3 &vector, float x, float y, float width, float height, float minZ, float maxZ, const Mat &worldViewProjection, Vector3 &result);
        // Projects a 3D vector from object space into screen space using the provided viewport and transformation matrix.
        static Vector3 project(const Vector3 &vector, float x, float y, float width, float height, float minZ, float maxZ, const Mat &worldViewProjection)
        {
            Vector3 result;
            project(vector, x, y, width, height, minZ, maxZ, worldViewProjection, result);
            return result;
        }

        // Projects a 3D vector from screen space into object space using the provided viewport and transformation matrix.
        static TK_API void unproject(const Vector3 &vector, float x, float y, float width, float height, float minZ, float maxZ, const Mat &worldViewProjection, Vector3 &result);
        // Projects a 3D vector from screen space into object space using the provided viewport and transformation matrix.
        static Vector3 unproject(const Vector3 &vector, float x, float y, float width, float height, float minZ, float maxZ, const Mat &worldViewProjection)
        {
            Vector3 result;
            unproject(vector, x, y, width, height, minZ, maxZ, worldViewProjection, result);
            return result;
        }

        // Creates an orthonormal basis from a basis with at least two orthogonal vectors.
        static TK_API void createOrthonormalBasis(Vector3 &xAxis, Vector3 &yAxis, Vector3 &zAxis);
        // Finds the best arbitrary axis vectors to represent U and V axes of a plane, by using this vector as the normal of the plane.
        TK_API void findBestAxisVectors(Vector3 &firstAxis, Vector3 &secondAxis) const;
        // Calculates the area of the triangle.
        static TK_API T triangleArea(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2);
        // Calculates the angle (in radians) between from and to. This is always the smallest value.
        static TK_API T angle(const Vector3 &from, const Vector3 &to);
        // Snaps the input position onto the grid.
        static TK_API Vector3 snapToGrid(const Vector3 &pos, const Vector3 &gridSize);
        static TK_API Vector3 snapToGrid(const Vector3 &point, const Vector3 &gridSize, const Quat &gridOrientation, const Vector3 &gridOrigin = kZero, const Vector3 &offset = kZero);

    public:
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
    };

    template <typename T>
    inline Vector3<T> operator+(T a, const Vector3<T> &b)
    {
        return b + a;
    }

    template <typename T>
    inline Vector3<T> operator-(T a, const Vector3<T> &b)
    {
        return Vector3<T>(a) - b;
    }

    template <typename T>
    inline Vector3<T> operator*(T a, const Vector3<T> &b)
    {
        return b * a;
    }

    template <typename T>
    inline Vector3<T> operator/(T a, const Vector3<T> &b)
    {
        return Vector3<T>(a) / b;
    }

    template <typename T>
    inline Vector3<T> operator+(typename TOtherFloat<T>::Type a, const Vector3<T> &b)
    {
        return b + a;
    }

    template <typename T>
    inline Vector3<T> operator-(typename TOtherFloat<T>::Type a, const Vector3<T> &b)
    {
        return Vector3<T>(a) - b;
    }

    template <typename T>
    inline Vector3<T> operator*(typename TOtherFloat<T>::Type a, const Vector3<T> &b)
    {
        return b * a;
    }

    template <typename T>
    inline Vector3<T> operator/(typename TOtherFloat<T>::Type a, const Vector3<T> &b)
    {
        return Vector3<T>(a) / b;
    }

    template <typename T>
    inline u32 getHash(const Vector3<T> &key)
    {
        return (((*(u32 *)&key.x * 397) ^ *(u32 *)&key.y) * 397) ^ *(u32 *)&key.z;
    }

    namespace math
    {
        template <typename T>
        TK_FORCE_INLINE static bool nearEqual(const Vector3<T> &a, const Vector3<T> &b)
        {
            return Vector3<T>::nearEqual(a, b);
        }

        template <typename T>
        TK_FORCE_INLINE static Vector3<T> UnwindDegrees(const Vector3<T> &v)
        {
            return Vector3<T>{UnwindDegrees(v.x), UnwindDegrees(v.y), UnwindDegrees(v.z)};
        }
    }

    template <>
    struct TIsPODType<FVec3>
    {
        enum
        {
            Value = true
        };
    };

    template <>
    struct TIsPODType<DVec3>
    {
        enum
        {
            Value = true
        };
    };

    template <>
    struct TIsPODType<IVec3>
    {
        enum
        {
            Value = true
        };
    };

#if !defined(_MSC_VER) || defined(__clang__)
    // Forward specializations for Clang
    template <>
    TK_API const FVec3 FVec3::kZero;
    template <>
    TK_API const FVec3 FVec3::kOne;
    template <>
    TK_API const FVec3 FVec3::kHalf;
    template <>
    TK_API const FVec3 FVec3::kUnitX;
    template <>
    TK_API const FVec3 FVec3::kUnitY;
    template <>
    TK_API const FVec3 FVec3::kUnitZ;
    template <>
    TK_API const FVec3 FVec3::kUp;
    template <>
    TK_API const FVec3 FVec3::kDown;
    template <>
    TK_API const FVec3 FVec3::kLeft;
    template <>
    TK_API const FVec3 FVec3::kRight;
    template <>
    TK_API const FVec3 FVec3::kForward;
    template <>
    TK_API const FVec3 FVec3::kBackward;
    template <>
    TK_API const FVec3 FVec3::kMinimum;
    template <>
    TK_API const FVec3 FVec3::kMaximum;

    template <>
    TK_API const DVec3 DVec3::kZero;
    template <>
    TK_API const DVec3 DVec3::kOne;
    template <>
    TK_API const DVec3 DVec3::kHalf;
    template <>
    TK_API const DVec3 DVec3::kUnitX;
    template <>
    TK_API const DVec3 DVec3::kUnitY;
    template <>
    TK_API const DVec3 DVec3::kUnitZ;
    template <>
    TK_API const DVec3 DVec3::kUp;
    template <>
    TK_API const DVec3 DVec3::kDown;
    template <>
    TK_API const DVec3 DVec3::kLeft;
    template <>
    TK_API const DVec3 DVec3::kRight;
    template <>
    TK_API const DVec3 DVec3::kForward;
    template <>
    TK_API const DVec3 DVec3::kBackward;
    template <>
    TK_API const DVec3 DVec3::kMinimum;
    template <>
    TK_API const DVec3 DVec3::kMaximum;

    template <>
    TK_API const IVec3 IVec3::kZero;
    template <>
    TK_API const IVec3 IVec3::kOne;
    template <>
    TK_API const IVec3 IVec3::kHalf;
    template <>
    TK_API const IVec3 IVec3::kUnitX;
    template <>
    TK_API const IVec3 IVec3::kUnitY;
    template <>
    TK_API const IVec3 IVec3::kUnitZ;
    template <>
    TK_API const IVec3 IVec3::kUp;
    template <>
    TK_API const IVec3 IVec3::kDown;
    template <>
    TK_API const IVec3 IVec3::kLeft;
    template <>
    TK_API const IVec3 IVec3::kRight;
    template <>
    TK_API const IVec3 IVec3::kForward;
    template <>
    TK_API const IVec3 IVec3::kBackward;
    template <>
    TK_API const IVec3 IVec3::kMinimum;
    template <>
    TK_API const IVec3 IVec3::kMaximum;
#endif
}

FORMAT_CUSTOM(toolkit::FVec3, "X:%g Y:%g Z:%g", t.x, t.y, t.z);
FORMAT_CUSTOM(toolkit::DVec3, "X:%g Y:%g Z:%g", t.x, t.y, t.z);
FORMAT_CUSTOM(toolkit::IVec3, "X:%d Y:%d Z:%d", t.x, t.y, t.z);
