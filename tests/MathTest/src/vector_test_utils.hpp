#pragma once

#include "common.hpp"

#include <type_traits>

#include <toolkit/math/Matrix.hpp>
#include <toolkit/math/Quat.hpp>
#include <toolkit/math/Transform.hpp>
#include <toolkit/math/Vector2.hpp>
#include <toolkit/math/Vector3.hpp>
#include <toolkit/math/Vector4.hpp>
#include <toolkit/math/geometry/AABB.hpp>
#include <toolkit/math/geometry/BoundingBox.hpp>
#include <toolkit/math/geometry/BoundingFrustum.hpp>
#include <toolkit/math/geometry/BoundingSphere.hpp>
#include <toolkit/math/geometry/OrientedBoundingBox.hpp>
#include <toolkit/math/geometry/Plane.hpp>
#include <toolkit/math/geometry/Ray.hpp>
#include <toolkit/math/geometry/Rect.hpp>
#include <toolkit/math/geometry/Triangle.hpp>
#include <toolkit/math/geometry/Viewport.hpp>
#include <toolkit/utils/Color32.hpp>
#include <toolkit/utils/Color.hpp>
#include <toolkit/utils/Half.hpp>

namespace math_test
{
    template <typename T>
    inline void expectScalarNear(T actual, T expected, T epsilon = static_cast<T>(1e-5))
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            EXPECT_NEAR(actual, expected, epsilon);
        }
        else
        {
            EXPECT_EQ(actual, expected);
        }
    }

    template <typename T>
    inline void expectVec2Near(const toolkit::Vector2<T> &actual, const toolkit::Vector2<T> &expected, T epsilon = static_cast<T>(1e-5))
    {
        expectScalarNear(actual.x, expected.x, epsilon);
        expectScalarNear(actual.y, expected.y, epsilon);
    }

    template <typename T>
    inline void expectVec3Near(const toolkit::Vector3<T> &actual, const toolkit::Vector3<T> &expected, T epsilon = static_cast<T>(1e-5))
    {
        expectScalarNear(actual.x, expected.x, epsilon);
        expectScalarNear(actual.y, expected.y, epsilon);
        expectScalarNear(actual.z, expected.z, epsilon);
    }

    template <typename T>
    inline void expectVec4Near(const toolkit::Vector4<T> &actual, const toolkit::Vector4<T> &expected, T epsilon = static_cast<T>(1e-5))
    {
        expectScalarNear(actual.x, expected.x, epsilon);
        expectScalarNear(actual.y, expected.y, epsilon);
        expectScalarNear(actual.z, expected.z, epsilon);
        expectScalarNear(actual.w, expected.w, epsilon);
    }

    inline void expectQuatNear(const toolkit::Quat &actual, const toolkit::Quat &expected, float epsilon = 1e-5f)
    {
        const float dot = toolkit::Quat::dot(actual, expected);
        EXPECT_NEAR(toolkit::TkMath::abs(dot), 1.0f, epsilon);
    }

    inline void expectMatNear(const toolkit::Mat &actual, const toolkit::Mat &expected, float epsilon = 1e-5f)
    {
        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                EXPECT_NEAR(actual.values[row][col], expected.values[row][col], epsilon);
            }
        }
    }

    inline void expectMat3Near(const toolkit::Mat3x3 &actual, const toolkit::Mat3x3 &expected, float epsilon = 1e-5f)
    {
        for (int row = 0; row < 3; ++row)
        {
            for (int col = 0; col < 3; ++col)
            {
                EXPECT_NEAR(actual.values[row][col], expected.values[row][col], epsilon);
            }
        }
    }

    inline void expectTransformNear(const toolkit::Transform &actual, const toolkit::Transform &expected, float epsilon = 1e-5f)
    {
        expectVec3Near(actual.translation, expected.translation, epsilon);
        expectQuatNear(actual.orientation, expected.orientation, epsilon);
        expectVec3Near(actual.scale, expected.scale, epsilon);
    }

    inline void expectAabbEq(const toolkit::AABB &actual, const toolkit::AABB &expected)
    {
        EXPECT_EQ(actual.minX, expected.minX);
        EXPECT_EQ(actual.maxX, expected.maxX);
        EXPECT_EQ(actual.minY, expected.minY);
        EXPECT_EQ(actual.maxY, expected.maxY);
        EXPECT_EQ(actual.minZ, expected.minZ);
        EXPECT_EQ(actual.maxZ, expected.maxZ);
    }

    inline void expectBoundingBoxNear(const toolkit::BoundingBox &actual, const toolkit::BoundingBox &expected, float epsilon = 1e-5f)
    {
        expectVec3Near(actual.minimum, expected.minimum, epsilon);
        expectVec3Near(actual.maximum, expected.maximum, epsilon);
    }

    inline void expectBoundingSphereNear(const toolkit::BoundingSphere &actual, const toolkit::BoundingSphere &expected, float epsilon = 1e-5f)
    {
        expectVec3Near(actual.center, expected.center, epsilon);
        EXPECT_NEAR(actual.radius, expected.radius, epsilon);
    }

    inline void expectPlaneNear(const toolkit::Plane &actual, const toolkit::Plane &expected, float epsilon = 1e-5f)
    {
        expectVec3Near(actual.normal, expected.normal, epsilon);
        EXPECT_NEAR(actual.d, expected.d, epsilon);
    }

    inline void expectRayNear(const toolkit::Ray &actual, const toolkit::Ray &expected, float epsilon = 1e-5f)
    {
        expectVec3Near(actual.position, expected.position, epsilon);
        expectVec3Near(actual.direction, expected.direction, epsilon);
    }

    inline void expectRectNear(const toolkit::Rectangle &actual, const toolkit::Rectangle &expected, float epsilon = 1e-5f)
    {
        expectVec2Near(actual.location, expected.location, epsilon);
        expectVec2Near(actual.size, expected.size, epsilon);
    }

    inline void expectColorNear(const toolkit::Color &actual, const toolkit::Color &expected, float epsilon = 1e-5f)
    {
        EXPECT_NEAR(actual.r, expected.r, epsilon);
        EXPECT_NEAR(actual.g, expected.g, epsilon);
        EXPECT_NEAR(actual.b, expected.b, epsilon);
        EXPECT_NEAR(actual.a, expected.a, epsilon);
    }

    inline void expectColor32Eq(const toolkit::Color32 &actual, const toolkit::Color32 &expected)
    {
        EXPECT_EQ(actual.r, expected.r);
        EXPECT_EQ(actual.g, expected.g);
        EXPECT_EQ(actual.b, expected.b);
        EXPECT_EQ(actual.a, expected.a);
    }
}
