/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/reflect/typedef.hpp>

#define rattr(...)
#define rclass class
#define rstruct struct
#define renum enum
#define rfield
#define rfunc

#define R_OBJECT(Type)                                                                                                            \
public:                                                                                                                           \
    static ::toolkit::TypeInitializer TypeInitializer;                                                                            \
    static ::toolkit::TypeHandle staticTypeHandle() { return TypeInitializer.getHandle(); }                                       \
    static const ::toolkit::RkClass &staticClass() { return static_cast<const ::toolkit::RkClass &>(TypeInitializer.getType()); } \
    static const ::toolkit::RkClass &staticType() { return staticClass(); }                                                       \
    const ::toolkit::RkClass &getClass() const override { return staticClass(); }                                                 \
                                                                                                                                  \
private:                                                                                                                          \
    friend struct Type##_ReflectAccess;

namespace toolkit
{
    enum class TypeKind : u8
    {
        kClass = 0,
        kStruct = 1,
        kEnum = 2,
        kInterface = 3,
    };

    enum class FieldFlags : u32
    {
        kNone = 0,
        kStatic = 1u << 0u,
        kReadOnly = 1u << 1u,
        kOwnerClass = 1u << 30u,
    };

    enum class MethodFlags : u32
    {
        kNone = 0,
        kStatic = 1u << 0u,
        kConst = 1u << 1u,
    };

    enum class ArgPassing : u8
    {
        kValue = 0,
        kLValueRef = 1,
        kConstLValueRef = 2,
        kRValueRef = 3,
        kPointer = 4,
    };

    enum class ReturnKind : u8
    {
        kVoid = 0,
        kValue = 1,
        kLValueRef = 2,
        kConstLValueRef = 3,
        kPointer = 4,
    };

    enum class BuiltinTypeId : u8
    {
        kVoid = 0,
        kBool,
        kChar,
        kU8,
        kU16,
        kU32,
        kU64,
        kI8,
        kI16,
        kI32,
        kI64,
        kF32,
        kF64,
    };

    inline FieldFlags operator|(const FieldFlags a, const FieldFlags b)
    {
        return static_cast<FieldFlags>(static_cast<u32>(a) | static_cast<u32>(b));
    }

    inline MethodFlags operator|(const MethodFlags a, const MethodFlags b)
    {
        return static_cast<MethodFlags>(static_cast<u32>(a) | static_cast<u32>(b));
    }

    inline bool hasAnyFlags(const FieldFlags value, const FieldFlags flags)
    {
        return (static_cast<u32>(value) & static_cast<u32>(flags)) != 0;
    }

    inline bool hasAnyFlags(const MethodFlags value, const MethodFlags flags)
    {
        return (static_cast<u32>(value) & static_cast<u32>(flags)) != 0;
    }
}
