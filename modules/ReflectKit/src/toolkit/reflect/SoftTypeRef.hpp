/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/reflect/RkManager.hpp>
#include <toolkit/reflect/RkObject.hpp>

namespace toolkit
{
    template <typename T = RkObject>
    struct SoftTypeRef
    {
        TkString m_typeName;

        SoftTypeRef() = default;
        SoftTypeRef(const TkStringView& typeName)
            : m_typeName(typeName)
        {
        }

        bool operator==(const SoftTypeRef& other) const
        {
            return m_typeName == other.m_typeName;
        }

        bool operator!=(const SoftTypeRef& other) const
        {
            return !(*this == other);
        }

        explicit operator bool() const
        {
            return m_typeName.hasChars();
        }

        TkStringView getTypeName() const
        {
            return m_typeName;
        }

        TypeHandle getType() const
        {
            return TypeRegistry::findType(m_typeName);
        }

        T* newObject() const
        {
            RkObject* object = TypeRegistry::newObject(getType());
            return object != nullptr && object->is<T>() ? static_cast<T*>(object) : nullptr;
        }
    };

    template <typename T>
    struct HashFunc<SoftTypeRef<T>>
    {
        static u32 get(const SoftTypeRef<T>& key)
        {
            return HashFunc<TkString>::get(key.m_typeName);
        }
    };
}
