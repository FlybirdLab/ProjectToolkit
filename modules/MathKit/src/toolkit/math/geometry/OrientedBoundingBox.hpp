/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/math/Transform.hpp>

namespace toolkit {

    enum class ContainmentType;
    
    // Oriented Bounding Box (OBB) is a rectangular block, much like an AABB (Bounding Box) but with an arbitrary orientation in 3D space.
    struct TK_API OrientedBoundingBox
    {
        // Half lengths of the box along each axis.
        Vec3 extents;
        // The transformation which aligns and scales the box, and its translation vector represents the center of the box.
        Transform transformation;

        OrientedBoundingBox() = default;
        OrientedBoundingBox(const Vec3& extents, const Transform& transformation)
            : extents {extents}
            , transformation {transformation}
        {
        }
        OrientedBoundingBox(const BoundingBox& bb);
        OrientedBoundingBox(const Vec3& extents, const Mat& transformation);
        OrientedBoundingBox(const Vec3& extents, const Mat3x3& rotationScale, const Vec3& translation);
        OrientedBoundingBox(const Vec3& minimum, const Vec3& maximum);
        OrientedBoundingBox(Vec3 points[], i32 pointCount);

        TkString toString() const;

        // Gets the eight corners of the bounding box.
        void getCorners(FVec3 corners[8]) const;
        void getCorners(DVec3 corners[8]) const;

        // The size of the OBB if no scaling is applied to the transformation matrix.
        Vec3 getSizeUnscaled() const
        {
            return extents * 2.0f;
        }

        // Returns the size of the OBB taking into consideration the scaling applied to the transformation matrix.
        Vec3 getSize() const;

        // Returns the square size of the OBB taking into consideration the scaling applied to the transformation matrix.
        Vec3 getSizeSquared() const;

        // Gets the center of the OBB.
        TK_FORCE_INLINE Vec3 getCenter() const
        {
            return transformation.Translation;
        }

        // Gets the AABB which contains all OBB corners.
        BoundingBox getBoundingBox() const;
        // Gets the AABB which contains all OBB corners.
        void getBoundingBox(BoundingBox& result) const;

        // Transforms this box using a transformation matrix.
        void transform(const Mat& matrix);
        void transform(const Transform& transform);

        // Scales the OBB by scaling its extents without affecting the transformation matrix.
        // By keeping transformation matrix scaling-free, the collision detection methods will be more accurate.
        void scale(const Vec3& scaling)
        {
            extents *= scaling;
        }

        // Scales the OBB by scaling its extents without affecting the transformation matrix.
        // By keeping transformation matrix scaling-free, the collision detection methods will be more accurate.
        void scale(real scaling)
        {
            extents *= scaling;
        }

        // Translates the OBB to a new position using a translation vector.
        void translate(const Vec3& translation)
        {
            transformation.Translation += translation;
        }

        TK_FORCE_INLINE bool operator==(const OrientedBoundingBox& other) const
        {
            return extents == other.extents && transformation == other.transformation;
        }

        TK_FORCE_INLINE bool operator!=(const OrientedBoundingBox& other) const
        {
            return extents != other.extents || transformation != other.transformation;
        }

        TK_FORCE_INLINE OrientedBoundingBox operator*(const Mat& matrix) const
        {
            OrientedBoundingBox result = *this;
            result.transform(matrix);
            return result;
        }

        TK_FORCE_INLINE OrientedBoundingBox operator*(const Transform& matrix) const
        {
            OrientedBoundingBox result = *this;
            result.transform(matrix);
            return result;
        }

        // Creates the centered box (axis aligned).
        static void createCentered(const Vec3& center, const Vec3& size, OrientedBoundingBox& result)
        {
            result.extents = size * 0.5f;
            result.transformation = Transform(center);
        }

        // Creates the centered box (axis-aligned).
        static OrientedBoundingBox createCentered(const Vec3& center, const Vec3& size)
        {
            OrientedBoundingBox result;
            createCentered(center, size, result);
            return result;
        }

        // Determines whether a OBB contains a point.
        ContainmentType contains(const Vec3& point, real* distance = nullptr) const;

        // Determines whether a OBB contains a sphere.
        ContainmentType contains(const BoundingSphere& sphere, bool ignoreScale = false) const;

        // Determines whether there is an intersection between oriented box and a ray. Returns point of the intersection.
        bool intersects(const Ray& ray, Vec3& point) const;

        // Determines if there is an intersection between oriented box and a ray. Returns distance to the intersection.
        bool intersects(const Ray& ray, real& distance) const;

        // Determines if there is an intersection between oriented box and a ray. Returns distance to the intersection and surface normal.
        bool intersects(const Ray& ray, real& distance, Vec3& normal) const;

        // Determines whether there is an intersection between a Ray and a OBB.
        bool intersects(const Ray& ray) const
        {
            Vec3 point;
            return intersects(ray, point);
        }
    };


    template<>
    struct TIsPODType<OrientedBoundingBox>
    {
        enum { Value = true };
    };

}

namespace toolkit
{
    template <>
    inline psize formatCustom<>(FormatBuffer &buf, const toolkit::OrientedBoundingBox &t)
    {
        const psize beginUsed = buf.getUsed();
        Formatter::format(buf, "Center:%s Size:%s", t.getCenter(), t.getSize());
        return buf.getUsed() - beginUsed - 1;
    }
}
