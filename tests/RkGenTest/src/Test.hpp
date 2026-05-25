/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */


#pragma once

#include "toolkit/reflect/common.hpp"
#include "toolkit/reflect/RkObject.hpp"

namespace test
{

    rkenum class Color : toolkit::u8
    {
        Red,
        Green = 2,
        Blue,
    };

    rkstruct Vec3
    {
        float x;
        float y;
        float z = 0.0f;
    };

    rkclass Actor : public toolkit::RkObject
    {
    public:
        rkctor Actor() = default;

        rkctor Actor(toolkit::i32 initialHealth, toolkit::f32 initialEnergy)
            : health(initialHealth)
            , energy(initialEnergy)
        {
        }

        rkfield() toolkit::i32 health = 0;
        rkfield(GetEnergy, SetEnergy) toolkit::f32 energy = 1.0f;

        rkfunc void SetHealth(toolkit::i32 value)
        {
            health = value;
        }

        rkfunc toolkit::i32 GetHealth() const noexcept
        {
            return health;
        }

        rkfunc toolkit::f32 ScaleEnergy(toolkit::f32 factor)
        {
            energy *= factor;
            return energy;
        }

        toolkit::f32& GetEnergy()
        {
            return energy;
        }

        void SetEnergy(toolkit::f32 value)
        {
            energy = value;
        }

        rkfunc static toolkit::i32 Sum(toolkit::i32 lhs, toolkit::i32 rhs)
        {
            return lhs + rhs;
        }
    };

    namespace nested
    {
        rkstruct Bounds
        {
            float minX;
            float minY;
            float maxX;
            float maxY;
        };
    }

}
