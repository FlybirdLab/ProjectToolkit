/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/reflect/typedef.hpp>

#define rkattr(...)
#define rkclass class
#define rkstruct struct
#define rkenum enum
#define rkfield(...)
#define rkfunc
#define rkctor

#define R_OBJECT(Type)                                                                                                            \
public:                                                                                                                           \
    static ::toolkit::TypeInitializer TypeInitializer;                                                                            \
    static ::toolkit::TypeHandle staticTypeHandle() { return TypeInitializer.getHandle(); }                                       \
    static const ::toolkit::RkClass &staticClass() { return static_cast<const ::toolkit::RkClass &>(TypeInitializer.getType()); } \
    static const ::toolkit::RkClass &staticType() { return staticClass(); }                                                       \
    const ::toolkit::RkClass &getClass() const override { return staticClass(); }                                                 \
                                                                                                                                  \
private:                                                                                                                          \
    friend struct Type##_ReflectAccess;                                                                                           \
                                                                                                                                  \
public:

#define R_STRUCT(Type)                                                                                                               \
public:                                                                                                                              \
    static ::toolkit::TypeInitializer TypeInitializer;                                                                               \
    static ::toolkit::TypeHandle staticTypeHandle() { return TypeInitializer.getHandle(); }                                          \
    static const ::toolkit::RkStruct &staticStruct() { return static_cast<const ::toolkit::RkStruct &>(TypeInitializer.getType()); } \
    static const ::toolkit::RkStruct &staticType() { return staticStruct(); }                                                        \
                                                                                                                                     \
private:                                                                                                                             \
    friend struct Type##_ReflectAccess;                                                                                              \
public:

#define R_OBJECT_NO_SPAWN(Type) R_OBJECT(Type)

#define R_OBJECT_SPAWN(Type)                                                                    \
    R_OBJECT_NO_SPAWN(Type)                                                                     \
public:                                                                                         \
    using SpawnParams = ::toolkit::ObjectSpawnParams;                                           \
    static Type *spawn(const ::toolkit::ObjectSpawnParams &params) { return new Type(params); } \
    explicit Type() : Type(::toolkit::ObjectSpawnParams(Type::TypeInitializer)) {}              \
    explicit Type(const ::toolkit::ObjectSpawnParams &params)

#define R_OBJECT_WITH_CONSTRUCTOR_IMPL(Type, BaseType)                                          \
    R_OBJECT_NO_SPAWN(Type)                                                                     \
public:                                                                                         \
    using SpawnParams = ::toolkit::ObjectSpawnParams;                                           \
    static Type *spawn(const ::toolkit::ObjectSpawnParams &params) { return new Type(params); } \
    explicit Type(const ::toolkit::ObjectSpawnParams &params) : BaseType(params) {}             \
    explicit Type() : BaseType(::toolkit::ObjectSpawnParams(Type::TypeInitializer)) {}

#define R_IMPLEMENT_OBJECT(Type, BaseType, Module, FullName)                                \
    static ::toolkit::RkObject *rk_spawn_##Type(const ::toolkit::ObjectSpawnParams &params) \
    {                                                                                       \
        return Type::spawn(params);                                                         \
    }                                                                                       \
    static void rk_destroy_##Type(::toolkit::RkObject *object)                              \
    {                                                                                       \
        delete static_cast<Type *>(object);                                                 \
    }                                                                                       \
    ::toolkit::TypeInitializer Type::TypeInitializer(                                       \
        Module,                                                                             \
        ::toolkit::RkClassDesc{                                                             \
            .name = #Type,                                                                  \
            .fullName = FullName,                                                           \
            .size = static_cast<::toolkit::u32>(sizeof(Type)),                              \
            .align = static_cast<::toolkit::u32>(alignof(Type)),                            \
            .baseType = BaseType::staticTypeHandle(),                                       \
            .spawn = &rk_spawn_##Type,                                                      \
            .destroy = &rk_destroy_##Type,                                                  \
        });

#define R_IMPLEMENT_OBJECT_NO_BASE(Type, Module, FullName)                                  \
    static ::toolkit::RkObject *rk_spawn_##Type(const ::toolkit::ObjectSpawnParams &params) \
    {                                                                                       \
        return Type::spawn(params);                                                         \
    }                                                                                       \
    static void rk_destroy_##Type(::toolkit::RkObject *object)                              \
    {                                                                                       \
        delete static_cast<Type *>(object);                                                 \
    }                                                                                       \
    ::toolkit::TypeInitializer Type::TypeInitializer(                                       \
        Module,                                                                             \
        ::toolkit::RkClassDesc{                                                             \
            .name = #Type,                                                                  \
            .fullName = FullName,                                                           \
            .size = static_cast<::toolkit::u32>(sizeof(Type)),                              \
            .align = static_cast<::toolkit::u32>(alignof(Type)),                            \
            .spawn = &rk_spawn_##Type,                                                      \
            .destroy = &rk_destroy_##Type,                                                  \
        });

#define R_IMPLEMENT_OBJECT_NO_SPAWN_WITH_BASE(Type, BaseType, Module, FullName) \
    ::toolkit::TypeInitializer Type::TypeInitializer(                           \
        Module,                                                                 \
        ::toolkit::RkClassDesc{                                                 \
            .name = #Type,                                                      \
            .fullName = FullName,                                               \
            .size = static_cast<::toolkit::u32>(sizeof(Type)),                  \
            .align = static_cast<::toolkit::u32>(alignof(Type)),                \
            .baseType = BaseType::staticTypeHandle(),                           \
        });

#define R_IMPLEMENT_OBJECT_NO_SPAWN(Type, Module, FullName)      \
    ::toolkit::TypeInitializer Type::TypeInitializer(            \
        Module,                                                  \
        ::toolkit::RkClassDesc{                                  \
            .name = #Type,                                       \
            .fullName = FullName,                                \
            .size = static_cast<::toolkit::u32>(sizeof(Type)),   \
            .align = static_cast<::toolkit::u32>(alignof(Type)), \
        });

#define R_IMPLEMENT_STRUCT(Type, Module, FullName)               \
    ::toolkit::TypeInitializer Type::TypeInitializer(            \
        Module,                                                  \
        ::toolkit::RkStructDesc{                                 \
            .name = #Type,                                       \
            .fullName = FullName,                                \
            .size = static_cast<::toolkit::u32>(sizeof(Type)),   \
            .align = static_cast<::toolkit::u32>(alignof(Type)), \
        });

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
        kNoSerialize = 1u << 2u,
        kEditorOnly = 1u << 3u,
        kObjectReference = 1u << 4u,
        kSoftObjectReference = 1u << 5u,
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

    enum class FieldSerializationFlags : u32
    {
        kNone = 0,
        kNoSerialize = 1u << 0u,
        kReadOnly = 1u << 1u,
        kEditorOnly = 1u << 2u,
        kObjectReference = 1u << 3u,
        kSoftObjectReference = 1u << 4u,
    };

    inline FieldFlags operator|(const FieldFlags a, const FieldFlags b)
    {
        return static_cast<FieldFlags>(static_cast<u32>(a) | static_cast<u32>(b));
    }

    inline MethodFlags operator|(const MethodFlags a, const MethodFlags b)
    {
        return static_cast<MethodFlags>(static_cast<u32>(a) | static_cast<u32>(b));
    }

    inline FieldSerializationFlags operator|(const FieldSerializationFlags a, const FieldSerializationFlags b)
    {
        return static_cast<FieldSerializationFlags>(static_cast<u32>(a) | static_cast<u32>(b));
    }

    inline bool hasAnyFlags(const FieldFlags value, const FieldFlags flags)
    {
        return (static_cast<u32>(value) & static_cast<u32>(flags)) != 0;
    }

    inline bool hasAnyFlags(const MethodFlags value, const MethodFlags flags)
    {
        return (static_cast<u32>(value) & static_cast<u32>(flags)) != 0;
    }

    inline bool hasAnyFlags(const FieldSerializationFlags value, const FieldSerializationFlags flags)
    {
        return (static_cast<u32>(value) & static_cast<u32>(flags)) != 0;
    }
}
