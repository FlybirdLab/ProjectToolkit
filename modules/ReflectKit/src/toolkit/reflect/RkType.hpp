/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/reflect/common.hpp>
#include <toolkit/reflect/object/ObjectHandle.hpp>
#include <toolkit/reflect/type/TypeHandle.hpp>
#include <toolkit/base/Assert.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/types/Span.hpp>
#include <toolkit/types/StringView.hpp>
#include <toolkit/types/UUID.hpp>

namespace toolkit
{
    struct TypeRef
    {
        const char *fullName = nullptr;
        mutable TypeHandle resolved;

        TypeHandle resolve() const;
        bool isResolved() const
        {
            return resolved.isValid();
        }
    };

    struct Attribute
    {
        TkStringView name;
        TkStringView value;
    };

    using AttributeSpan = TkSpan<const Attribute>;

    struct FieldSerializationMeta
    {
        FieldSerializationFlags flags = FieldSerializationFlags::kNone;
        TkStringView defaultValue;
        TkStringView oldName;
        TkStringView referencePolicy;

        bool shouldSerialize() const
        {
            return !hasAnyFlags(flags, FieldSerializationFlags::kNoSerialize);
        }
    };

    struct ObjectSpawnParams
    {
        TkUuid id;
        TypeHandle type;
        RkObject *outer = nullptr;
        TkStringView name;
        ArgValue *args = nullptr;
        u32 argCount = 0;
        u32 flags = 0;

        ObjectSpawnParams()
            : id(TkUuid::create())
        {
        }

        explicit ObjectSpawnParams(const TypeHandle &inType)
            : id(TkUuid::create()), type(inType)
        {
        }

        ObjectSpawnParams(const TkUuid &inId, const TypeHandle &inType)
            : id(inId), type(inType)
        {
        }

        ObjectSpawnParams(const TypeHandle &inType, ArgValue *inArgs, const u32 inArgCount)
            : id(TkUuid::create()), type(inType), args(inArgs), argCount(inArgCount)
        {
        }

        ObjectSpawnParams(const TkUuid &inId, const TypeHandle &inType, RkObject *inOuter, const TkStringView inName, const u32 inFlags, ArgValue *inArgs = nullptr, const u32 inArgCount = 0)
            : id(inId.isValid() ? inId : TkUuid::create()), type(inType), outer(inOuter), name(inName), args(inArgs), argCount(inArgCount), flags(inFlags)
        {
        }
    };

    using ObjectSpawnFn = RkObject *(*)(const ObjectSpawnParams &params);
    using ObjectDestroyFn = void (*)(RkObject *object);
    using StructCtorFn = void (*)(void *instance);
    using StructDtorFn = void (*)(void *instance);
    using StructCopyFn = void (*)(void *dst, const void *src);
    using FieldAccessorFn = void *(*)(void *instance);

    template <class T>
    TypeHandle typeOf();

    struct RkField
    {
        TkStringView name;
        TypeHandle type;
        u32 offset = 0;
        FieldFlags flags = FieldFlags::kNone;
        AttributeSpan attributes;
        FieldAccessorFn accessor = nullptr;
        FieldSerializationMeta serialization;

        void *getPtr(void *instance) const
        {
            if (instance == nullptr)
                return nullptr;
            if (accessor != nullptr)
                return accessor(instance);
            if (hasAnyFlags(flags, FieldFlags::kOwnerClass))
            {
                TK_ASSERT(false && "RkClass fields must use generated accessors; offset fallback is only for plain data.");
                return nullptr;
            }
            return reinterpret_cast<byte *>(instance) + offset;
        }

        const void *getPtr(const void *instance) const
        {
            // Accessors are address-only callbacks. They must not mutate the object when reached through const access.
            // TODO: Add ConstFieldAccessorFn if generated code needs truly const-correct member access callbacks.
            return const_cast<RkField *>(this)->getPtr(const_cast<void *>(instance));
        }

        template <class T>
        T &get(void *instance) const
        {
            return *static_cast<T *>(getPtr(instance));
        }

        template <class T>
        const T &get(const void *instance) const
        {
            return *static_cast<const T *>(getPtr(instance));
        }
    };

    struct RkParam
    {
        TkStringView name;
        TypeHandle type;
        ArgPassing passing = ArgPassing::kValue;
    };

    using MethodInvokeFn = bool (*)(InvokeContext &context);

    TK_API u64 makeMethodSignatureHash(
        const TkStringView &ownerFullName,
        const TkStringView &methodName,
        TypeHandle returnType,
        ReturnKind returnKind,
        TkSpan<const RkParam> params);

    struct RkMethod
    {
        TkStringView name;
        u64 signatureHash = 0;
        TypeHandle returnType;
        ReturnKind returnKind = ReturnKind::kVoid;
        TkSpan<const RkParam> params;
        u32 paramCount = 0;
        MethodFlags flags = MethodFlags::kNone;
        MethodInvokeFn invoke = nullptr;
        AttributeSpan attributes;

        bool validateArgs(const InvokeContext &context) const;
        bool invokeRaw(InvokeContext &context) const;

        template <class Ret = void, class Obj = void *, class... Args>
        Ret invokeTyped(Obj &&object, Args &&...args) const;

        MethodCallBuilder call(RkObject *instance) const;
        MethodCallBuilder callStatic() const;
        MethodCallBuilder call(ObjectHandle handle) const;
    };

    struct EnumItem
    {
        TkStringView name;
        i64 value = 0;
    };

    struct RkClassDesc
    {
        TkStringView name;
        TkStringView fullName;
        u32 size = 0;
        u32 align = 0;
        TypeHandle baseType;
        ObjectSpawnFn spawn = nullptr;
        ObjectDestroyFn destroy = nullptr;
        TkSpan<const TypeHandle> interfaces;
        TkSpan<const RkField> fields;
        TkSpan<const RkMethod> methods;
        AttributeSpan attributes;
        bool isAbstract = false;
    };

    struct RkStructDesc
    {
        TkStringView name;
        TkStringView fullName;
        u32 size = 0;
        u32 align = 0;
        StructCtorFn ctor = nullptr;
        StructDtorFn dtor = nullptr;
        StructCopyFn copy = nullptr;
        TkSpan<const RkField> fields;
        TkSpan<const RkMethod> methods;
        AttributeSpan attributes;
    };

    struct RkEnumDesc
    {
        TkStringView name;
        TkStringView fullName;
        u32 size = 0;
        u32 align = 0;
        TkSpan<const EnumItem> items;
        AttributeSpan attributes;
    };

    struct RkInterfaceDesc
    {
        TkStringView name;
        TkStringView fullName;
        u32 size = 0;
        u32 align = 0;
        TkSpan<const RkMethod> methods;
        AttributeSpan attributes;
    };

    class TK_API RkType
    {
    public:
        TypeKind kind = TypeKind::kClass;
        TypeHandle handle;
        TkStringView name;
        TkStringView fullName;
        u32 size = 0;
        u32 align = 0;
        AttributeSpan attributes;

        RkType() = default;
        RkType(
            const TypeKind inKind,
            const TkStringView inName,
            const TkStringView inFullName,
            const u32 inSize,
            const u32 inAlign,
            const AttributeSpan inAttributes)
            : kind(inKind), name(inName), fullName(inFullName), size(inSize), align(inAlign), attributes(inAttributes)
        {
        }

        virtual ~RkType() = default;

        virtual const RkClass *asClass() const
        {
            return nullptr;
        }

        virtual const RkStruct *asStruct() const
        {
            return nullptr;
        }

        virtual const RkEnum *asEnum() const
        {
            return nullptr;
        }

        virtual const RkInterface *asInterface() const
        {
            return nullptr;
        }

        virtual const RkField *findField(const TkStringView &fieldName) const
        {
            (void)fieldName;
            return nullptr;
        }

        virtual const RkMethod *findMethod(const TkStringView &methodName) const
        {
            (void)methodName;
            return nullptr;
        }

        virtual const RkMethod *findMethod(const TkStringView &methodName, u64 signatureHash) const
        {
            (void)methodName;
            (void)signatureHash;
            return nullptr;
        }

        virtual const RkMethod *findMethod(const TkStringView &methodName, TkSpan<const TypeHandle> paramTypes) const
        {
            (void)methodName;
            (void)paramTypes;
            return nullptr;
        }

        virtual const RkMethod *findMethod(const TkStringView &methodName, TkSpan<const RkParam> params) const
        {
            (void)methodName;
            (void)params;
            return nullptr;
        }
    };

    class TK_API RkClass final : public RkType
    {
    public:
        TypeHandle baseType;
        ObjectSpawnFn spawn = nullptr;
        ObjectDestroyFn destroy = nullptr;
        TkArray<TypeHandle> interfaceStorage;
        TkArray<RkField> fieldStorage;
        TkArray<RkMethod> methodStorage;
        TkSpan<const TypeHandle> interfaces;
        TkSpan<const RkField> fields;
        TkSpan<const RkMethod> methods;
        bool isAbstract = false;

        RkClass() = default;
        explicit RkClass(const RkClassDesc &desc)
            : RkType(TypeKind::kClass, desc.name, desc.fullName, desc.size, desc.align, desc.attributes), baseType(desc.baseType), spawn(desc.spawn), destroy(desc.destroy), isAbstract(desc.isAbstract)
        {
            interfaceStorage.reserve(desc.interfaces.length());
            for (const TypeHandle &interfaceType : desc.interfaces)
                interfaceStorage.add(interfaceType);
            interfaces = TkSpan<const TypeHandle>(interfaceStorage.get(), interfaceStorage.count());

            fieldStorage.reserve(desc.fields.length());
            for (const RkField &field : desc.fields)
            {
                RkField local = field;
                local.flags = local.flags | FieldFlags::kOwnerClass;
                fieldStorage.add(local);
            }
            fields = TkSpan<const RkField>(fieldStorage.get(), fieldStorage.count());

            methodStorage.reserve(desc.methods.length());
            for (const RkMethod &method : desc.methods)
            {
                RkMethod local = method;
                local.paramCount = static_cast<u32>(local.params.length());
                if (local.signatureHash == 0)
                    local.signatureHash = makeMethodSignatureHash(desc.fullName, local.name, local.returnType, local.returnKind, local.params);
                methodStorage.add(local);
            }
            methods = TkSpan<const RkMethod>(methodStorage.get(), methodStorage.count());
        }

        const RkClass *asClass() const override
        {
            return this;
        }

        const RkField *findField(const TkStringView &fieldName) const override;
        // Name-only lookup returns a method only when that name is unique; overloaded names return nullptr.
        // Use the RkParam overload to match both type and passing. The TypeHandle overload is a convenience
        // for non-overloaded value-style calls and returns nullptr when passing is ambiguous.
        const RkMethod *findMethod(const TkStringView &methodName) const override;
        const RkMethod *findMethod(const TkStringView &methodName, u64 signatureHash) const override;
        const RkMethod *findMethod(const TkStringView &methodName, TkSpan<const TypeHandle> paramTypes) const override;
        const RkMethod *findMethod(const TkStringView &methodName, TkSpan<const RkParam> params) const override;
        bool implements(TypeHandle interfaceType) const;
    };

    class TK_API RkStruct final : public RkType
    {
    public:
        StructCtorFn ctor = nullptr;
        StructDtorFn dtor = nullptr;
        StructCopyFn copy = nullptr;
        TkArray<RkField> fieldStorage;
        TkArray<RkMethod> methodStorage;
        TkSpan<const RkField> fields;
        TkSpan<const RkMethod> methods;

        RkStruct() = default;
        explicit RkStruct(const RkStructDesc &desc)
            : RkType(TypeKind::kStruct, desc.name, desc.fullName, desc.size, desc.align, desc.attributes), ctor(desc.ctor), dtor(desc.dtor), copy(desc.copy)
        {
            fieldStorage.reserve(desc.fields.length());
            for (const RkField &field : desc.fields)
                fieldStorage.add(field);
            fields = TkSpan<const RkField>(fieldStorage.get(), fieldStorage.count());

            methodStorage.reserve(desc.methods.length());
            for (const RkMethod &method : desc.methods)
            {
                RkMethod local = method;
                local.paramCount = static_cast<u32>(local.params.length());
                if (local.signatureHash == 0)
                    local.signatureHash = makeMethodSignatureHash(desc.fullName, local.name, local.returnType, local.returnKind, local.params);
                methodStorage.add(local);
            }
            methods = TkSpan<const RkMethod>(methodStorage.get(), methodStorage.count());
        }

        const RkStruct *asStruct() const override
        {
            return this;
        }

        const RkField *findField(const TkStringView &fieldName) const override;
        const RkMethod *findMethod(const TkStringView &methodName) const override;
        const RkMethod *findMethod(const TkStringView &methodName, u64 signatureHash) const override;
        const RkMethod *findMethod(const TkStringView &methodName, TkSpan<const TypeHandle> paramTypes) const override;
        const RkMethod *findMethod(const TkStringView &methodName, TkSpan<const RkParam> params) const override;
    };

    class TK_API RkEnum final : public RkType
    {
    public:
        TkArray<EnumItem> itemStorage;
        TkSpan<const EnumItem> items;

        RkEnum() = default;
        explicit RkEnum(const RkEnumDesc &desc)
            : RkType(TypeKind::kEnum, desc.name, desc.fullName, desc.size, desc.align, desc.attributes)
        {
            itemStorage.reserve(desc.items.length());
            for (const EnumItem &item : desc.items)
                itemStorage.add(item);
            items = TkSpan<const EnumItem>(itemStorage.get(), itemStorage.count());
        }

        const RkEnum *asEnum() const override
        {
            return this;
        }
    };

    class TK_API RkInterface final : public RkType
    {
    public:
        TkArray<RkMethod> methodStorage;
        TkSpan<const RkMethod> methods;

        RkInterface() = default;
        explicit RkInterface(const RkInterfaceDesc &desc)
            : RkType(TypeKind::kInterface, desc.name, desc.fullName, desc.size, desc.align, desc.attributes)
        {
            methodStorage.reserve(desc.methods.length());
            for (const RkMethod &method : desc.methods)
            {
                RkMethod local = method;
                local.paramCount = static_cast<u32>(local.params.length());
                if (local.signatureHash == 0)
                    local.signatureHash = makeMethodSignatureHash(desc.fullName, local.name, local.returnType, local.returnKind, local.params);
                methodStorage.add(local);
            }
            methods = TkSpan<const RkMethod>(methodStorage.get(), methodStorage.count());
        }

        const RkInterface *asInterface() const override
        {
            return this;
        }

        const RkMethod *findMethod(const TkStringView &methodName) const override;
        const RkMethod *findMethod(const TkStringView &methodName, u64 signatureHash) const override;
        const RkMethod *findMethod(const TkStringView &methodName, TkSpan<const TypeHandle> paramTypes) const override;
        const RkMethod *findMethod(const TkStringView &methodName, TkSpan<const RkParam> params) const override;
    };

    class TK_API TypeInitializer
    {
    public:
        TypeInitializer() = default;
        TypeInitializer(RkModule &module, const RkClassDesc &desc);
        TypeInitializer(RkModule &module, const RkStructDesc &desc);
        TypeInitializer(RkModule &module, const RkEnumDesc &desc);
        TypeInitializer(RkModule &module, const RkInterfaceDesc &desc);

        const TypeHandle &getHandle() const
        {
            return m_handle;
        }

        operator const TypeHandle &() const
        {
            return m_handle;
        }

        const RkType &getType() const;

    private:
        TypeHandle m_handle;
    };

    using RkTypeHandle = TypeHandle;
    using RkModuleType = RkModule;
    using RkTypeInitializer = TypeInitializer;
    using RkObjectSpawnParams = ObjectSpawnParams;
}
