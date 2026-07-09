/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

namespace toolkit
{
    class RkModule;
    class RkType;
    class RkClass;
    class RkStruct;
    class RkEnum;
    class RkInterface;
    class RkObject;
    class TypeRegistry;
    class TypeInitializer;
    class ObjectRegistry;
    class CallFrame;
    class MethodCallBuilder;
    class MethodCallResult;

    struct TypeHandle;
    struct TypeRef;
    struct ObjectSpawnParams;
    struct ObjectHandle;
    struct Attribute;
    struct RkField;
    struct RkParam;
    struct RkMethod;
    struct EnumItem;
    struct ArgValue;
    struct ReturnValue;
    struct InvokeContext;
    struct RkValue;
    using RkValue = RkValue;

    enum class TypeKind : u8;
    enum class FieldFlags : u32;
    enum class MethodFlags : u32;
    enum class ArgPassing : u8;
    enum class ReturnKind : u8;
    enum class BuiltinTypeId : u8;

    TK_API RkObject* resolveObjectHandle(ObjectHandle handle);
}
