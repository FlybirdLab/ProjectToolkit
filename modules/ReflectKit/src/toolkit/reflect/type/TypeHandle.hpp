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
        RkModule *module = nullptr;
        u64 moduleId = 0;
        u32 moduleGeneration = 0;
        u64 typeId = 0;
        i32 typeIndex = kInvalidIndex;

        TypeHandle() = default;
        TypeHandle(RkModule *inModule, i32 inTypeIndex);
        TypeHandle(RkModule *inModule, i32 inTypeIndex, u64 inModuleId, u32 inModuleGeneration, u64 inTypeId)
            : module(inModule), moduleId(inModuleId), moduleGeneration(inModuleGeneration), typeId(inTypeId), typeIndex(inTypeIndex)
        {
        }

        explicit TypeHandle(const TypeInitializer& initializer);

        bool isValid() const;
        bool isAlive() const;

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
            if (typeId != 0 || other.typeId != 0)
                return moduleId == other.moduleId && typeId == other.typeId && typeIndex == other.typeIndex;
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
            u32 hash = key.typeId != 0 ? HashFunc<u64>::get(key.moduleId) : HashFunc<void*>::get(key.module);
            combineHash(hash, HashFunc<i32>::get(key.typeIndex));
            combineHash(hash, HashFunc<u64>::get(key.typeId));
            return hash;
        }
    };
}
