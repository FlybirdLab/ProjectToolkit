/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */


#pragma once

#include "toolkit/reflect/common.hpp"
#include "toolkit/reflect/RkObject.hpp"
#include "toolkit/reflect/RkObjectRef.hpp"
#include "toolkit/types/DateTime.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/types/StringView.hpp"
#include "toolkit/types/TimeSpan.hpp"
#include "toolkit/types/UUID.hpp"
#include "toolkit/types/Version.hpp"
#include "toolkit/math/Quat.hpp"
#include "toolkit/math/Vector3.hpp"

namespace test
{
    using toolkit::Quat;
    using toolkit::FVec3;
    using toolkit::RkObjectRef;
    using toolkit::TkDateTime;
    using toolkit::TkString;
    using toolkit::TkStringView;
    using toolkit::TkTimeSpan;
    using toolkit::TkUuid;
    using toolkit::TkVersion;

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

    rkattr(LoadOrder=10, Plugin=Core)
    rkstruct RuntimePayload
    {
        rkattr(NoSerialize, OldName=legacyName, Default=42)
        rkfield() toolkit::i32 migratedValue = 42;

        rkattr(EditorOnly, ReadOnly)
        rkfield() TkString label;

        rkfield() TkStringView labelView;
        rkfield() TkUuid uuid;
        rkfield() TkDateTime dateTime;
        rkfield() TkTimeSpan timeSpan;
        rkfield() TkVersion version;
        rkfield() FVec3 position;
        rkfield() Quat rotation;

        rkattr(SoftObjectReference, ReferencePolicy=Weak)
        rkfield() RkObjectRef<test::Actor> owner;

        rkattr(RuntimeCallable)
        rkfunc TkString Describe(TkStringView prefix, FVec3 offset) const
        {
            (void)prefix;
            (void)offset;
            return label;
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
