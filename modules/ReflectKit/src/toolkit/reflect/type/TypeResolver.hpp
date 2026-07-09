/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/base/TypeTraits.hpp>
#include <toolkit/reflect/type/TypeHandle.hpp>
#include <toolkit/reflect/common.hpp>

#include <type_traits>
#include <typeinfo>

namespace toolkit
{
    TK_API TypeHandle getBuiltinTypeHandle(BuiltinTypeId id);
    TK_API TypeHandle getOpaqueRuntimeTypeHandle(const TkStringView &fullName, u32 size, u32 align);

    namespace detail
    {
        template <class T, class = void>
        struct THasStaticTypeHandle : TBoolConstant<false>
        {
        };

        template <class T>
        struct THasStaticTypeHandle<T, std::void_t<decltype(T::staticTypeHandle())>> : TBoolConstant<true>
        {
        };
    }

    template <class T, class Enable = void>
    struct TypeResolver
    {
        static TypeHandle get()
        {
            if constexpr (detail::THasStaticTypeHandle<T>::Value)
                return T::staticTypeHandle();
            else if constexpr (TIsCompleteType<T>::Value)
                return getOpaqueRuntimeTypeHandle(TkStringView(typeid(T).name()), static_cast<u32>(sizeof(T)), static_cast<u32>(alignof(T)));
            else
                return TypeHandle();
        }
    };

    template <class T>
    struct TypeResolver<T*, void>
    {
        static TypeHandle get()
        {
            // Pointer types are not registered as independent reflected types.
            // ArgPassing::kPointer and ReturnKind::kPointer carry pointer semantics while TypeHandle stores the pointee.
            if constexpr (detail::THasStaticTypeHandle<T>::Value || TIsCompleteType<T>::Value)
                return TypeResolver<T>::get();
            else
                return TypeHandle();
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
