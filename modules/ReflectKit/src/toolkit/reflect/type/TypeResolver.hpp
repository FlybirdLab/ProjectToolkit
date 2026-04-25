/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/base/TypeTraits.hpp>
#include <toolkit/reflect/type/TypeHandle.hpp>
#include <toolkit/reflect/common.hpp>

namespace toolkit
{
    TK_API TypeHandle getBuiltinTypeHandle(BuiltinTypeId id);

    template <class T, class Enable = void>
    struct TypeResolver
    {
        static TypeHandle get()
        {
            return T::staticTypeHandle();
        }
    };

    template <class T>
    struct TypeResolver<T*, void>
    {
        static TypeHandle get()
        {
            // Pointer types are not registered as independent reflected types.
            // ArgPassing::kPointer and ReturnKind::kPointer carry pointer semantics while TypeHandle stores the pointee.
            return TypeResolver<T>::get();
        }
    };

    template <class T>
    struct TypeResolver<const T, void> : TypeResolver<T>
    {
    };

    template <class T>
    struct TypeResolver<T&, void> : TypeResolver<T>
    {
    };

    template <class T>
    struct TypeResolver<T&&, void> : TypeResolver<T>
    {
    };

    template <>
    struct TypeResolver<void, void>
    {
        static TypeHandle get()
        {
            return getBuiltinTypeHandle(BuiltinTypeId::kVoid);
        }
    };

    template <>
    struct TypeResolver<bool, void>
    {
        static TypeHandle get()
        {
            return getBuiltinTypeHandle(BuiltinTypeId::kBool);
        }
    };

    template <>
    struct TypeResolver<char, void>
    {
        static TypeHandle get()
        {
            return getBuiltinTypeHandle(BuiltinTypeId::kChar);
        }
    };

    template <>
    struct TypeResolver<u8, void>
    {
        static TypeHandle get()
        {
            return getBuiltinTypeHandle(BuiltinTypeId::kU8);
        }
    };

    template <>
    struct TypeResolver<u16, void>
    {
        static TypeHandle get()
        {
            return getBuiltinTypeHandle(BuiltinTypeId::kU16);
        }
    };

    template <>
    struct TypeResolver<u32, void>
    {
        static TypeHandle get()
        {
            return getBuiltinTypeHandle(BuiltinTypeId::kU32);
        }
    };

    template <>
    struct TypeResolver<u64, void>
    {
        static TypeHandle get()
        {
            return getBuiltinTypeHandle(BuiltinTypeId::kU64);
        }
    };

    template <>
    struct TypeResolver<i8, void>
    {
        static TypeHandle get()
        {
            return getBuiltinTypeHandle(BuiltinTypeId::kI8);
        }
    };

    template <>
    struct TypeResolver<i16, void>
    {
        static TypeHandle get()
        {
            return getBuiltinTypeHandle(BuiltinTypeId::kI16);
        }
    };

    template <>
    struct TypeResolver<i32, void>
    {
        static TypeHandle get()
        {
            return getBuiltinTypeHandle(BuiltinTypeId::kI32);
        }
    };

    template <>
    struct TypeResolver<i64, void>
    {
        static TypeHandle get()
        {
            return getBuiltinTypeHandle(BuiltinTypeId::kI64);
        }
    };

    template <>
    struct TypeResolver<f32, void>
    {
        static TypeHandle get()
        {
            return getBuiltinTypeHandle(BuiltinTypeId::kF32);
        }
    };

    template <>
    struct TypeResolver<f64, void>
    {
        static TypeHandle get()
        {
            return getBuiltinTypeHandle(BuiltinTypeId::kF64);
        }
    };

    template <class T>
    TypeHandle typeOf()
    {
        return TypeResolver<TRemoveCVT<TRemoveReferenceT<T>>>::get();
    }
}
