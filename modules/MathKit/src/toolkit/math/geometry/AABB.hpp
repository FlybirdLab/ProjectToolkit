#pragma once

#include <toolkit/math/Vector3.hpp>

namespace toolkit {

    // Integer axis aligned bounding box
    struct TK_API AABB
    {
        i32 minX;
        i32 maxX;

        i32 minY;
        i32 maxY;

        i32 minZ;
        i32 maxZ;

    public:
        AABB()
        {
            minX = kMaxI32;
            maxX = kMinI32;
            minY = kMaxI32;
            maxY = kMinI32;
            minZ = kMaxI32;
            maxZ = kMinI32;
        }

        AABB(float mix, float miy, float miz, float max, float may, float maz)
        {
            minX = (i32)mix;
            maxX = (i32)maz;
            minY = (i32)miy;
            maxY = (i32)may;
            minZ = (i32)miz;
            maxZ = (i32)maz;
        }

        AABB(const AABB& other)
        {
            set(other);
        }

        i32 width() const
        {
            return maxX - minX;
        }

        i32 height() const
        {
            return maxY - minY;
        }

        i32 depth() const
        {
            return maxZ - minZ;
        }

        i32 x() const
        {
            return (maxX + minX) / 2;
        }

        i32 y() const
        {
            return (maxY + minY) / 2;
        }

        i32 z() const
        {
            return (maxZ + minZ) / 2;
        }

        bool isEmpty() const
        {
            return minX >= maxX || minY >= maxY || minZ >= maxZ;
        }
        
        void clear()
        {
            minX = kMaxI32;
            maxX = kMinI32;
            minY = kMaxI32;
            maxY = kMinI32;
            minZ = kMaxI32;
            maxZ = kMinI32;
        }

        void add(const Vec3& inCoordinate)
        {
            add(inCoordinate.x, inCoordinate.y, inCoordinate.z);
        }

        void add(real inX, real inY, real inZ)
        {
            TK_ASSERT(!isinf(inX) && !isnan(inX));
            TK_ASSERT(!isinf(inY) && !isnan(inY));
            TK_ASSERT(!isinf(inZ) && !isnan(inZ));

            minX = TkMath::min(minX, (i32)TkMath::floor(inX));
            minY = TkMath::min(minY, (i32)TkMath::floor(inY));
            minZ = TkMath::min(minZ, (i32)TkMath::floor(inZ));

            maxX = TkMath::max(maxX, (i32)TkMath::ceil(inX));
            maxY = TkMath::max(maxY, (i32)TkMath::ceil(inY));
            maxZ = TkMath::max(maxZ, (i32)TkMath::ceil(inZ));
        }


        void add(const AABB& bounds)
        {
            minX = TkMath::min(minX, bounds.minX);
            minY = TkMath::min(minY, bounds.minY);
            minZ = TkMath::min(minZ, bounds.minZ);

            maxX = TkMath::max(maxX, bounds.maxX);
            maxY = TkMath::max(maxY, bounds.maxY);
            maxZ = TkMath::max(maxZ, bounds.maxZ);
        }


        void set(const AABB& bounds)
        {
            minX = bounds.minX;
            minY = bounds.minY;
            minZ = bounds.minZ;

            maxX = bounds.maxX;
            maxY = bounds.maxY;
            maxZ = bounds.maxZ;
        }


        void translate(i32 X, i32 Y, i32 Z)
        {
            minX += X;
            minY += Y;
            minZ += Z;

            maxX += X;
            maxY += Y;
            maxZ += Z;
        }


        void translate(const Vec3& translation)
        {
            minX = (i32)TkMath::floor(minX + translation.x);
            minY = (i32)TkMath::floor(minY + translation.y);
            minZ = (i32)TkMath::floor(minZ + translation.z);

            maxX = (i32)TkMath::ceil(maxX + translation.x);
            maxY = (i32)TkMath::ceil(maxY + translation.y);
            maxZ = (i32)TkMath::ceil(maxZ + translation.z);
        }


        AABB translated(const Vec3& translation) const
        {
            AABB result;
            result.translate(translation);
            return result;
        }


        void set(const AABB& other, const Vec3& translation)
        {
            minX = (i32)TkMath::floor(other.minX + translation.x);
            minY = (i32)TkMath::floor(other.minY + translation.y);
            minZ = (i32)TkMath::floor(other.minZ + translation.z);

            maxX = (i32)TkMath::ceil(other.maxX + translation.x);
            maxY = (i32)TkMath::ceil(other.maxY + translation.y);
            maxZ = (i32)TkMath::ceil(other.maxZ + translation.z);
        }

    public:
        bool isOutside(const AABB& other) const
        {
            return maxX - other.minX < 0 || minX - other.maxX > 0 ||
                maxY - other.minY < 0 || minY - other.maxY > 0 ||
                maxZ - other.minZ < 0 || minZ - other.maxZ > 0;
        }


        static bool isOutside(const AABB& left, const AABB& right)
        {
            return left.maxX - right.minX < 0 || left.minX - right.maxX > 0 ||
                left.maxY - right.minY < 0 || left.minY - right.maxY > 0 ||
                left.maxZ - right.minZ < 0 || left.minZ - right.maxZ > 0;
        }
    };

}