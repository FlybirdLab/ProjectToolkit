/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/base/Assert.hpp>
#include <toolkit/reflect/common.hpp>
#include <toolkit/reflect/type/TypeHandle.hpp>

namespace toolkit
{
    // ReturnValue does not own returned storage. kValue data is owned by the active CallFrame;
    // ref data points at the referenced T; pointer data points at a T* variable and type is the pointee type.
    struct ReturnValue
    {
        TypeHandle type;
        void* data = nullptr;
        ReturnKind kind = ReturnKind::kVoid;

        ReturnValue() = default;
        ReturnValue(const ReturnValue&) = delete;
        ReturnValue& operator=(const ReturnValue&) = delete;

        ReturnValue(ReturnValue&& other) noexcept
            : type(other.type)
            , data(other.data)
            , kind(other.kind)
        {
            other.data = nullptr;
            other.kind = ReturnKind::kVoid;
        }

        ReturnValue& operator=(ReturnValue&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                type = other.type;
                data = other.data;
                kind = other.kind;
                other.data = nullptr;
                other.kind = ReturnKind::kVoid;
            }
            return *this;
        }

        ~ReturnValue()
        {
            reset();
        }

        void reset()
        {
            type = TypeHandle();
            data = nullptr;
            kind = ReturnKind::kVoid;
        }

        template <class T>
        T as() const
        {
            // `as<T>()` always returns by value; reference-based returns are copied out.
            TK_ASSERT(data != nullptr);
            TK_ASSERT(kind == ReturnKind::kValue || kind == ReturnKind::kLValueRef || kind == ReturnKind::kConstLValueRef);
            return *reinterpret_cast<const T*>(data);
        }

        template <class T>
        T& ref() const
        {
            TK_ASSERT(data != nullptr);
            TK_ASSERT(kind == ReturnKind::kLValueRef);
            return *reinterpret_cast<T*>(data);
        }

        template <class T>
        const T& cref() const
        {
            TK_ASSERT(data != nullptr);
            TK_ASSERT(kind == ReturnKind::kConstLValueRef || kind == ReturnKind::kValue || kind == ReturnKind::kLValueRef);
            return *reinterpret_cast<const T*>(data);
        }

        template <class T>
        T* ptr() const
        {
            TK_ASSERT(data != nullptr);
            TK_ASSERT(kind == ReturnKind::kPointer);
            return *reinterpret_cast<T* const*>(data);
        }
    };
}
