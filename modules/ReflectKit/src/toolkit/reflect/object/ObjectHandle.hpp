/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/Hash.hpp>

namespace toolkit
{
    // ObjectHandle is an index plus generation token. Registry removal increments generation,
    // making stale handles fail resolve() and isAlive() even when the slot is reused.
    struct ObjectHandle
    {
        static constexpr u32 kInvalidIndex = kMaxU32;

        u32 index = kInvalidIndex;
        u32 generation = 0;

        bool isValid() const
        {
            return index != kInvalidIndex;
        }

        explicit operator bool() const
        {
            return isValid();
        }

        bool operator==(const ObjectHandle& other) const
        {
            return index == other.index && generation == other.generation;
        }

        bool operator!=(const ObjectHandle& other) const
        {
            return !(*this == other);
        }
    };

    template <>
    struct HashFunc<ObjectHandle>
    {
        static u32 get(const ObjectHandle& value)
        {
            u32 hash = HashFunc<u32>::get(value.index);
            combineHash(hash, HashFunc<u32>::get(value.generation));
            return hash;
        }
    };
}
