/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/reflect/typedef.hpp>
#include <toolkit/reflect/object/ObjectHandle.hpp>

namespace toolkit
{
    class ObjectRegistry
    {
        NON_COPYABLE(ObjectRegistry);

    public:
        ObjectRegistry() = default;

        // Handles are index + generation. remove() clears the slot and bumps generation, so old handles fail resolve/isAlive.
        ObjectHandle add(RkObject* object);
        void remove(ObjectHandle handle);
        RkObject* resolve(ObjectHandle handle);
        const RkObject* resolve(ObjectHandle handle) const;
        bool isAlive(ObjectHandle handle) const;

    private:
        struct Slot
        {
            RkObject* object = nullptr;
            u32 generation = 1;
        };

    private:
        TkArray<Slot> m_slots;
        TkArray<u32> m_freeList;
    };

    TK_API ObjectRegistry& getObjectRegistry();
}
