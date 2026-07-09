/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/base/TypeTraits.hpp>
#include <toolkit/reflect/invoking/ArgValue.hpp>
#include <toolkit/reflect/invoking/CallFrame.hpp>

namespace toolkit
{
    template <class T>
    struct MoveArg
    {
        T* value = nullptr;
    };

    template <class T>
    MoveArg<TRemoveCVT<TRemoveReferenceT<T>>> moveArg(T& value)
    {
        using ValueType = TRemoveCVT<TRemoveReferenceT<T>>;
        return MoveArg<ValueType>{ &value };
    }

    template <class T>
    MoveArg<TRemoveCVT<TRemoveReferenceT<T>>> rkMoveArg(T& value)
    {
        return moveArg(value);
    }

    struct ArgBuilder
    {
        template <class T>
        static ArgValue make(CallFrame& frame, T&& value)
        {
            using RawType = TRemoveCVT<TRemoveReferenceT<T>>;
            using PointeeType = TRemovePointerT<RawType>;

            ArgValue arg;

            if constexpr (TIsPointer<RawType>::Value)
            {
                // Pointer args store the pointee type; data points at a CallFrame-owned T* cell.
                arg.type = typeOf<PointeeType>();
                RawType* storage = frame.emplace<RawType>(value);
                arg.data = storage;
                arg.passing = ArgPassing::kPointer;
                arg.owns = true;
            }
            else if constexpr (TIsLValueReference<T>::Value && !TAreTypesEqual<TRemoveReferenceT<T>, TRemoveConstT<TRemoveReferenceT<T>>>::Value)
            {
                arg.type = typeOf<RawType>();
                arg.data = const_cast<RawType*>(&value);
                arg.passing = ArgPassing::kConstLValueRef;
                arg.owns = false;
            }
            else if constexpr (TIsLValueReference<T>::Value)
            {
                arg.type = typeOf<RawType>();
                arg.data = &value;
                arg.passing = ArgPassing::kLValueRef;
                arg.owns = false;
            }
            else
            {
                arg.type = typeOf<RawType>();
                RawType* storage = frame.emplace<RawType>(toolkit::forward<T>(value));
                arg.data = storage;
                arg.passing = ArgPassing::kValue;
                arg.owns = true;
            }

            return arg;
        }

        template <class T>
        static ArgValue makeRef(T& value)
        {
            ArgValue arg;
            arg.type = typeOf<TRemoveCVT<TRemoveReferenceT<T>>>();
            arg.data = &value;
            arg.passing = ArgPassing::kLValueRef;
            return arg;
        }

        template <class T>
        static ArgValue makeConstRef(const T& value)
        {
            ArgValue arg;
            arg.type = typeOf<TRemoveCVT<TRemoveReferenceT<T>>>();
            arg.data = const_cast<T*>(&value);
            arg.passing = ArgPassing::kConstLValueRef;
            return arg;
        }

        template <class T>
        static ArgValue makePointer(CallFrame& frame, T* value)
        {
            ArgValue arg;
            arg.type = typeOf<T>();
            T** storage = frame.emplace<T*>(value);
            arg.data = storage;
            arg.passing = ArgPassing::kPointer;
            arg.owns = true;
            return arg;
        }

        template <class T>
        static ArgValue makeMove(CallFrame& frame, T& value)
        {
            using ValueType = TRemoveCVT<TRemoveReferenceT<T>>;

            ArgValue arg;
            arg.type = typeOf<ValueType>();
            ValueType* storage = frame.emplace<ValueType>(toolkit::moveTemp(value));
            arg.data = storage;
            arg.passing = ArgPassing::kRValueRef;
            arg.owns = true;
            return arg;
        }

        template <class T>
        static ArgValue make(CallFrame& frame, MoveArg<T> value)
        {
            TK_ASSERT(value.value != nullptr);
            return makeMove(frame, *value.value);
        }
    };
}
