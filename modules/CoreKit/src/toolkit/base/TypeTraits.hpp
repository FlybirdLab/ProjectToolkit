/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>

#include <concepts>
#include <type_traits>

namespace toolkit
{
    template <bool TValue>
    struct TBoolConstant
    {
        using ValueType = bool;
        using Type = TBoolConstant<TValue>;

        static constexpr bool Value = TValue;
        static constexpr bool value = TValue;

        constexpr operator bool() const noexcept
        {
            return Value;
        }

        constexpr bool operator()() const noexcept
        {
            return Value;
        }
    };

    template <typename T>
    struct TTypeIdentity : std::type_identity<T>
    {
        using Type = typename std::type_identity<T>::type;
    };

    ////////////////////////////////////////////////////////////////////////////////////

    template <typename... Types>
    struct TAnd : TBoolConstant<std::conjunction_v<Types...>>
    {
    };

    template <typename... Types>
    struct TOr : TBoolConstant<std::disjunction_v<Types...>>
    {
    };

    template <typename Type>
    struct TNot : TBoolConstant<std::negation_v<Type>>
    {
    };

    ////////////////////////////////////////////////////////////////////////////////////

    template <typename A, typename B>
    struct TIsTheSame : TBoolConstant<std::is_same_v<A, B>>
    {
    };

    template <typename X, typename Y>
    struct TAreTypesEqual : TBoolConstant<std::is_same_v<X, Y>>
    {
    };

    ////////////////////////////////////////////////////////////////////////////////////

    template <typename T>
    struct TIsLValueReference : TBoolConstant<std::is_lvalue_reference_v<T>>
    {
    };

    template <typename T>
    struct TIsRValueReferenceType : TBoolConstant<std::is_rvalue_reference_v<T>>
    {
    };

    template <typename T>
    struct TIsReferenceType : TBoolConstant<std::is_reference_v<T>>
    {
    };

    template <typename T>
    struct TIsVoidType : TBoolConstant<std::is_void_v<std::remove_cv_t<T>>>
    {
    };

    template <typename T>
    struct TIsPointer : TBoolConstant<std::is_pointer_v<std::remove_cv_t<T>>>
    {
    };

    template <typename T>
    struct TIsEnum : TBoolConstant<std::is_enum_v<T>>
    {
    };

    template <typename T>
    struct TIsUnsigned : TBoolConstant<std::is_unsigned_v<T>>
    {
    };

    template <class Base, class Derived>
    struct TIsBaseOf : TBoolConstant<std::is_base_of_v<Base, Derived>>
    {
    };

    template <typename T>
    struct TIsFunction : TBoolConstant<std::is_function_v<T>>
    {
    };

    template <typename T>
    struct TIsCopyConstructible : TBoolConstant<std::is_copy_constructible_v<T>>
    {
    };

    template <typename T>
    struct TIsMoveConstructible : TBoolConstant<std::is_move_constructible_v<T>>
    {
    };

    template <typename T, typename = void>
    struct TIsCompleteType : TBoolConstant<false>
    {
    };

    template <typename T>
    struct TIsCompleteType<T, std::void_t<decltype(sizeof(T))>> : TBoolConstant<true>
    {
    };

    ////////////////////////////////////////////////////////////////////////////////////

    template <typename T>
    struct TRemoveCV
    {
        using Type = std::remove_cv_t<T>;
    };

    template <typename T>
    struct TRemoveReference
    {
        using Type = std::remove_reference_t<T>;
    };

    template <typename T>
    struct TRemoveConst
    {
        using Type = std::remove_const_t<T>;
    };

    template <typename T>
    struct TRemoveVolatile
    {
        using Type = std::remove_volatile_t<T>;
    };

    template <typename T>
    struct TRemoveCVRef
    {
        using Type = std::remove_cvref_t<T>;
    };

    template <typename T>
    struct TAddCV
    {
        using Type = std::add_cv_t<T>;
    };

    template <typename T>
    struct TAddConst
    {
        using Type = std::add_const_t<T>;
    };

    template <typename T>
    struct TAddLValueReference
    {
        using Type = std::add_lvalue_reference_t<T>;
    };

    template <typename T>
    struct TAddRValueReference
    {
        using Type = std::add_rvalue_reference_t<T>;
    };

    template <typename T>
    struct TRemovePointer
    {
        using Type = T;
    };

    template <typename T>
    struct TRemovePointer<T *>
    {
        using Type = typename TRemovePointer<T>::Type;
    };

    template <typename T>
    struct TDecay
    {
        using Type = std::decay_t<T>;
    };

    template <typename T>
    using TRemoveCVT = typename TRemoveCV<T>::Type;

    template <typename T>
    using TRemoveReferenceT = typename TRemoveReference<T>::Type;

    template <typename T>
    using TRemoveConstT = typename TRemoveConst<T>::Type;

    template <typename T>
    using TRemoveVolatileT = typename TRemoveVolatile<T>::Type;

    template <typename T>
    using TRemoveCVRefT = typename TRemoveCVRef<T>::Type;

    template <typename T>
    using TAddCVT = typename TAddCV<T>::Type;

    template <typename T>
    using TAddConstT = typename TAddConst<T>::Type;

    template <typename T>
    using TAddLValueReferenceT = typename TAddLValueReference<T>::Type;

    template <typename T>
    using TAddRValueReferenceT = typename TAddRValueReference<T>::Type;

    template <typename T>
    using TRemovePointerT = typename TRemovePointer<T>::Type;

    template <typename T>
    using TDecayT = typename TDecay<T>::Type;

    ////////////////////////////////////////////////////////////////////////////////////

    template <typename T, bool IsComplete = TIsCompleteType<TRemoveCVT<T>>::Value>
    struct TIsPODTypeImpl : TBoolConstant<
        TIsEnum<T>::Value ||
        TIsPointer<T>::Value>
    {
    };

    template <typename T>
    struct TIsPODTypeImpl<T, true> : TBoolConstant<
        std::is_trivially_copyable_v<TRemoveCVT<T>> ||
        TIsEnum<T>::Value ||
        TIsPointer<T>::Value>
    {
    };

    template <typename T>
    struct TIsPODType : TIsPODTypeImpl<T>
    {
    };

    template <typename T>
    struct TIsTriviallyCopyConstructible : TBoolConstant<
        std::is_trivially_copy_constructible_v<T> ||
        TIsPODType<T>::Value>
    {
    };

    template <typename T>
    struct TIsTriviallyConstructible : TBoolConstant<TIsPODType<T>::Value>
    {
    };

    template <typename T, bool IsComplete = TIsCompleteType<TRemoveCVT<T>>::Value>
    struct TIsTriviallyDestructibleImpl : TBoolConstant<
        TIsEnum<T>::Value ||
        TIsPointer<T>::Value>
    {
    };

    template <typename T>
    struct TIsTriviallyDestructibleImpl<T, true> : TBoolConstant<
        std::is_enum_v<T> || std::is_trivially_destructible_v<T>>
    {
    };

    template <typename T>
    struct TIsTriviallyDestructible : TIsTriviallyDestructibleImpl<T>
    {
    };

    template <typename T>
    struct TIsTriviallyCopyAssignable : TBoolConstant<
        std::is_trivially_copy_assignable_v<T> ||
        TIsPODType<T>::Value>
    {
    };

    ////////////////////////////////////////////////////////////////////////////////////

    template <bool Condition, typename TrueResult, typename FalseResult>
    struct TStaticIf
    {
        using Type = std::conditional_t<Condition, TrueResult, FalseResult>;
        using Value = Type;
    };

    template <bool Predicate, typename Result = void>
    struct TEnableIf : std::enable_if<Predicate, Result>
    {
    };

    template <bool Predicate, typename Result = void>
    using TEnableIfT = std::enable_if_t<Predicate, Result>;

    ////////////////////////////////////////////////////////////////////////////////////

    template <typename T>
    inline TRemoveReferenceT<T> &&moveTemp(T &&obj) noexcept
    {
        return static_cast<TRemoveReferenceT<T> &&>(obj);
    }

    template <typename T>
    inline void swap(T &a, T &b) noexcept
    {
        T tmp = moveTemp(a);
        a = moveTemp(b);
        b = moveTemp(tmp);
    }

    template <typename T>
    inline T &&forward(TRemoveReferenceT<T> &t) noexcept
    {
        return static_cast<T &&>(t);
    }

    template <typename T>
    inline T &&forward(TRemoveReferenceT<T> &&t) noexcept
    {
        static_assert(!TIsLValueReference<T>::Value, "Bad forward call");
        return static_cast<T &&>(t);
    }

    ////////////////////////////////////////////////////////////////////////////////////

    template <typename TFn, typename... Args>
    struct TInvokeResult : std::invoke_result<TFn, Args...>
    {
        using Type = typename std::invoke_result<TFn, Args...>::type;
    };

    template <typename TFn, typename... Args>
    using TInvokeResultT = typename TInvokeResult<TFn, Args...>::Type;

    template <typename T, typename U>
    concept TSameAs = std::same_as<T, U>;

    template <typename T, typename U>
    concept TNotSameAs = !std::same_as<T, U>;

    template <typename T>
    concept TPointerType = std::is_pointer_v<std::remove_cv_t<T>>;

    template <typename T>
    concept TEnumType = std::is_enum_v<T>;

    template <typename T>
    concept TPODType = (TIsPODType<T>::Value == true);

    template <typename T>
    concept TTriviallyConstructibleType = TPODType<T>;

    template <typename T>
    concept TTriviallyDestructibleType = (TIsTriviallyDestructible<T>::Value == true);

    template <typename T>
    concept TTriviallyCopyAssignableType =
        std::is_trivially_copy_assignable_v<T> || TPODType<T>;

    template <typename T>
    concept TMoveConstructibleType = std::move_constructible<T>;

    template <typename T>
    concept TArithmeticType = std::is_arithmetic_v<T>;

    template <typename TFn, typename... Args>
    concept TInvocable = std::invocable<TFn, Args...>;

    template <typename TFn, typename ReturnType, typename... Args>
    concept TInvocableAs =
        std::invocable<TFn, Args...> &&
        ((std::same_as<std::remove_cv_t<ReturnType>, void> &&
          std::same_as<std::remove_cv_t<std::invoke_result_t<TFn, Args...>>, void>) ||
         (!std::same_as<std::remove_cv_t<ReturnType>, void> &&
          std::convertible_to<std::invoke_result_t<TFn, Args...>, ReturnType>));

    ////////////////////////////////////////////////////////////////////////////////////

    template <typename T>
        requires std::same_as<TRemoveCVT<T>, unsigned int>
    inline T reverseBits(T bits)
    {
        bits = (bits << 16) | (bits >> 16);
        bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
        bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
        bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
        bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);
        return bits;
    }

    ////////////////////////////////////////////////////////////////////////////////////

    template <typename T, typename Arg>
    struct TIsBitwiseConstructible : TBoolConstant<false>
    {
        static_assert(!TIsReferenceType<T>::Value && !TIsReferenceType<Arg>::Value, "TIsBitwiseConstructible cannot use reference types");
        static_assert(TAreTypesEqual<T, TRemoveCVT<T>>::Value && TAreTypesEqual<Arg, TRemoveCVT<Arg>>::Value, "TIsBitwiseConstructible cannot use qualified types");
    };

    template <typename T>
    struct TIsBitwiseConstructible<T, T> : TBoolConstant<TIsTriviallyCopyConstructible<T>::Value>
    {
    };

    template <typename T, typename U>
    struct TIsBitwiseConstructible<const T, U> : TIsBitwiseConstructible<T, U>
    {
    };

    template <typename T>
    struct TIsBitwiseConstructible<const T *, T *> : TBoolConstant<true>
    {
    };

    template <>
    struct TIsBitwiseConstructible<u8, i8> : TBoolConstant<true>
    {
    };

    template <>
    struct TIsBitwiseConstructible<i8, u8> : TBoolConstant<true>
    {
    };

    template <>
    struct TIsBitwiseConstructible<u16, i16> : TBoolConstant<true>
    {
    };

    template <>
    struct TIsBitwiseConstructible<i16, u16> : TBoolConstant<true>
    {
    };

    template <>
    struct TIsBitwiseConstructible<u32, i32> : TBoolConstant<true>
    {
    };

    template <>
    struct TIsBitwiseConstructible<i32, u32> : TBoolConstant<true>
    {
    };

    template <>
    struct TIsBitwiseConstructible<u64, i64> : TBoolConstant<true>
    {
    };

    template <>
    struct TIsBitwiseConstructible<i64, u64> : TBoolConstant<true>
    {
    };

    template <typename T, typename U>
    concept TBitwiseConstructibleFrom = TIsBitwiseConstructible<T, U>::Value;

    ////////////////////////////////////////////////////////////////////////////////////

    template <typename T>
    struct TOtherFloat
    {
        using Type = float;
    };

    template <>
    struct TOtherFloat<float>
    {
        using Type = double;
    };
}
