#pragma once

#include <toolkit/math/Vector3.hpp>

namespace toolkit
{
    // Integer axis aligned bounding box
    class TK_API AABB
    {
    public:
        AABB();
        AABB(float mix, float miy, float miz, float max, float may, float maz);
        AABB(const AABB &other)
        {
            set(other);
        }

    public:
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

        void clear();

        void add(const Vec3 &inCoordinate)
        {
            add(inCoordinate.x, inCoordinate.y, inCoordinate.z);
        }
        void add(real inX, real inY, real inZ);
        void add(const AABB &bounds);

        void set(const AABB &bounds);

        void translate(i32 X, i32 Y, i32 Z);
        void translate(const Vec3 &translation);
        AABB translated(const Vec3 &translation) const;

        void set(const AABB &other, const Vec3 &translation);

    public:
        bool isOutside(const AABB &other) const;
        static bool isOutside(const AABB &left, const AABB &right);

    public:
        i32 minX;
        i32 maxX;

        i32 minY;
        i32 maxY;

        i32 minZ;
        i32 maxZ;
    };

}