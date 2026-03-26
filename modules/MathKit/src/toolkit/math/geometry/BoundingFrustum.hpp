/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/math/geometry/Plane.hpp>
#include <toolkit/math/Matrix.hpp>
#include <toolkit/math/geometry/CollisionsHelper.hpp>

namespace toolkit {

    // Defines a frustum which can be used in frustum culling, zoom to Extents (zoom to fit) operations, (matrix, frustum, camera) interchange, and many kind of intersection testing.
    struct TK_API BoundingFrustum
    {
        Mat matrix;

        union
        {
            struct
            {
                Plane pNear;
                Plane pFar;
                Plane pLeft;
                Plane pRight;
                Plane pTop;
                Plane pBottom;
            };
            Plane planes[6];
        };

        BoundingFrustum() = default;
        BoundingFrustum(const Mat& matrix)
        {
            setMatrix(matrix);
        }

        TkString toString() const;

        // Gets the matrix that describes this bounding frustum.
        TK_FORCE_INLINE const Mat& getMatrix() const
        {
            return matrix;
        }

        // Gets the matrix that describes this bounding frustum.
        TK_FORCE_INLINE Mat getMatrix()
        {
            return matrix;
        }

        // Gets the inverted matrix to that describes this bounding frustum.
        TK_FORCE_INLINE void getInvMatrix(Mat& result) const
        {
            Mat::invert(matrix, result);
        }

        // Sets the matrix (made from view and projection matrices) that describes this bounding frustum.
        void setMatrix(const Mat& view, const Mat& projection)
        {
            Mat viewProjection;
            Mat::multiply(view, projection, viewProjection);
            setMatrix(viewProjection);
        }

        // Sets the matrix that describes this bounding frustum.
        void setMatrix(const Mat& matrix);

        // Gets the near.
        TK_FORCE_INLINE Plane getNear() const
        {
            return pNear;
        }

        // Gets the far plane of the BoundingFrustum.
        TK_FORCE_INLINE Plane getFar() const
        {
            return pFar;
        }

        // Gets the left plane of the BoundingFrustum.
        TK_FORCE_INLINE Plane getLeft() const
        {
            return pLeft;
        }

        // Gets the right plane of the BoundingFrustum.
        TK_FORCE_INLINE Plane getRight() const
        {
            return pRight;
        }

        // Gets the top plane of the BoundingFrustum.
        TK_FORCE_INLINE Plane getTop() const
        {
            return pTop;
        }

        // Gets the bottom plane of the BoundingFrustum.
        TK_FORCE_INLINE Plane getBottom() const
        {
            return pBottom;
        }

        // Gets the one of the 6 planes related to this frustum.
        Plane getPlane(i32 index) const;

        // Gets the the 8 corners of the frustum: Near1 (near right down corner), Near2 (near right top corner), Near3 (near Left top corner), Near4 (near Left down corner), Far1 (far right down corner), Far2 (far right top corner), Far3 (far left top corner), Far4 (far left down corner).
        void getCorners(FVec3 corners[8]) const;

        // Gets the the 8 corners of the frustum: Near1 (near right down corner), Near2 (near right top corner), Near3 (near Left top corner), Near4 (near Left down corner), Far1 (far right down corner), Far2 (far right top corner), Far3 (far left top corner), Far4 (far left down corner).
        void getCorners(DVec3 corners[8]) const;

        // Gets bounding box that contains whole frustum.
        void getBox(BoundingBox& result) const;

        // Gets bounding sphere that contains whole frustum.
        void getSphere(BoundingSphere& result) const;

        // Determines whether this frustum is orthographic.
        TK_FORCE_INLINE bool isOrthographic() const
        {
            return pLeft.normal == -pRight.normal && pTop.normal == -pBottom.normal;
        }

        // Gets the width of the frustum at specified depth.
        float getWidthAtDepth(float depth) const;

        // Gets the height of the frustum at specified depth.
        float getHeightAtDepth(float depth) const;

        TK_FORCE_INLINE bool operator==(const BoundingFrustum& other) const
        {
            return matrix == other.matrix;
        }

        TK_FORCE_INLINE bool operator!=(const BoundingFrustum& other) const
        {
            return matrix != other.matrix;
        }

        // Checks whether a point lays inside, intersects or lays outside the frustum.
        ContainmentType contains(const Vec3& point) const;

        // Determines the intersection relationship between the frustum and a bounding box.
        TK_FORCE_INLINE ContainmentType contains(const BoundingBox& box) const
        {
            return TkCollision::frustumContainsBox(*this, box);
        }

        // Determines the intersection relationship between the frustum and a bounding sphere.
        ContainmentType contains(const BoundingSphere& sphere) const;

        // Checks whether the current BoundingFrustum intersects a BoundingSphere.
        bool intersects(const BoundingSphere& sphere) const;

        // Checks whether the current BoundingFrustum intersects a BoundingBox.
        TK_FORCE_INLINE bool intersects(const BoundingBox& box) const
        {
            return TkCollision::frustumContainsBox(*this, box) != ContainmentType::kDisjoint;
        }
    };

    template<>
    struct TIsPODType<BoundingFrustum>
    {
        enum { Value = true };
    };

}

namespace toolkit
{
    template <>
    inline psize formatCustom<>(FormatBuffer &buf, const toolkit::BoundingFrustum &t)
    {
        const psize beginUsed = buf.getUsed();
        Formatter::format(buf, "{}", t.getMatrix());
        return buf.getUsed() - beginUsed - 1;
    }
}
