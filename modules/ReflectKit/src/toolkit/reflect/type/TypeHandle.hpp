/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/reflect/typedef.hpp>
#include <toolkit/base/Hash.hpp>
#include <toolkit/types/String.hpp>

namespace toolkit
{
    struct TK_API TypeHandle
    {
        // TODO: Dynamic-library unload requires replacing RkModule* with a stable moduleId + typeIndex.
        // The current handle is valid only while modules live for the whole program/static-plugin lifetime.
        RkModule* module = nullptr;
        i32 typeIndex = kInvalidIndex;

        TypeHandle() = default;
        TypeHandle(RkModule* inModule, i32 inTypeIndex)
            : module(inModule), typeIndex(inTypeIndex)
        {
        }

        explicit TypeHandle(const TypeInitializer& initializer);

        bool isValid() const
        {
            return module != nullptr && typeIndex >= 0;
        }

        explicit operator bool() const
        {
            return isValid();
        }

        const RkType& getType() const;
        TkString toString(bool withModuleName = false) const;
        bool isSubclassOf(TypeHandle other) const;
        bool isAssignableFrom(TypeHandle other) const;

        bool operator==(const TypeHandle& other) const
        {
            return module == other.module && typeIndex == other.typeIndex;
        }

        bool operator!=(const TypeHandle& other) const
        {
            return !(*this == other);
        }
    };

    template <>
    struct HashFunc<TypeHandle>
    {
        static u32 get(const TypeHandle& key)
        {
            u32 hash = HashFunc<void*>::get(key.module);
            combineHash(hash, HashFunc<i32>::get(key.typeIndex));
            return hash;
        }
    };
}
