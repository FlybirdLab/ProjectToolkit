/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include "common.hpp"

#include <toolkit/reflect/RkManager.hpp>
#include <toolkit/reflect/type/TypeResolver.hpp>
#include <toolkit/reflect/invoking/MethodCall.hpp>

namespace reflect_test
{
    class BaseActor : public RkObject
    {
    public:
        static ::toolkit::TypeInitializer TypeInitializer;

        static TypeHandle staticTypeHandle()
        {
            return TypeInitializer.getHandle();
        }

        static const RkClass& staticClass()
        {
            return static_cast<const RkClass&>(TypeInitializer.getType());
        }

        explicit BaseActor(const ObjectSpawnParams& params)
            : RkObject(params)
        {
        }

        const RkClass& getClass() const override
        {
            return staticClass();
        }
    };

    class RuntimeActor : public BaseActor
    {
    public:
        static ::toolkit::TypeInitializer TypeInitializer;

        static TypeHandle staticTypeHandle()
        {
            return TypeInitializer.getHandle();
        }

        static const RkClass& staticClass()
        {
            return static_cast<const RkClass&>(TypeInitializer.getType());
        }

        explicit RuntimeActor(const ObjectSpawnParams& params)
            : BaseActor(params)
        {
        }

        const RkClass& getClass() const override
        {
            return staticClass();
        }

        void applyDamage(i32 value)
        {
            hp -= value;
            lastI32 = value;
        }

        i32 getHp() const
        {
            return hp;
        }

        f32 getRatio() const
        {
            return ratio;
        }

        void addConstRef(const i32& value)
        {
            hp += value;
            lastRefI32 = value;
        }

        void mutateRef(i32& value)
        {
            value += 3;
            lastRefI32 = value;
        }

        void setPointer(i32* value)
        {
            hpPtr = value;
        }

        void setMove(i32&& value)
        {
            lastMoveI32 = value;
            value = 0;
        }

        void setValue(i32 value)
        {
            lastI32 = value;
        }

        void setValue(f32 value)
        {
            lastF32 = value;
        }

        void setValueRef(const i32& value)
        {
            lastRefI32 = value;
        }

        void setValuePtr(i32* value)
        {
            hpPtr = value;
            lastPointerValue = value != nullptr ? *value : -1;
        }

        void setValueMove(i32&& value)
        {
            lastMoveI32 = value;
            value = 0;
        }

        i32& hpRef()
        {
            return hp;
        }

        const i32& hpConstRef() const
        {
            return hp;
        }

        i32* hpPointer()
        {
            hpPtr = &hp;
            return hpPtr;
        }

        static i32 staticAdd(i32 a, i32 b)
        {
            return a + b;
        }

        i32 hp = 100;
        f32 ratio = 0.5f;
        i32* hpPtr = nullptr;
        i32 lastI32 = 0;
        i32 lastRefI32 = 0;
        i32 lastMoveI32 = 0;
        i32 lastPointerValue = 0;
        f32 lastF32 = 0.0f;
    };

    inline RkModule& module()
    {
        static RkModule value("ReflectTest.RuntimeScaffold");
        return value;
    }

    inline RkObject* spawnBaseActor(const ObjectSpawnParams& params)
    {
        return new BaseActor(params);
    }

    inline RkObject* spawnRuntimeActor(const ObjectSpawnParams& params)
    {
        return new RuntimeActor(params);
    }

    inline void destroyBaseActor(RkObject* object)
    {
        delete static_cast<BaseActor*>(object);
    }

    inline void destroyRuntimeActor(RkObject* object)
    {
        delete static_cast<RuntimeActor*>(object);
    }

    inline void* hpAccessor(void* object)
    {
        return &static_cast<RuntimeActor*>(object)->hp;
    }

    inline void* hpPtrAccessor(void* object)
    {
        return &static_cast<RuntimeActor*>(object)->hpPtr;
    }

    inline bool writeVoid(InvokeContext& context)
    {
        if (context.returnValue != nullptr)
        {
            context.returnValue->type = typeOf<void>();
            context.returnValue->data = nullptr;
            context.returnValue->kind = ReturnKind::kVoid;
        }
        return true;
    }

    inline bool invokeApplyDamage(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;
        static_cast<RuntimeActor*>(context.instance)->applyDamage(*static_cast<const i32*>(context.args[0].data));
        return writeVoid(context);
    }

    inline bool invokeGetHp(InvokeContext& context)
    {
        if (context.instance == nullptr || context.result == nullptr || context.returnValue == nullptr)
            return false;
        *static_cast<i32*>(context.result) = static_cast<RuntimeActor*>(context.instance)->getHp();
        context.returnValue->type = typeOf<i32>();
        context.returnValue->data = context.result;
        context.returnValue->kind = ReturnKind::kValue;
        return true;
    }

    inline bool invokeGetRatio(InvokeContext& context)
    {
        if (context.instance == nullptr || context.result == nullptr || context.returnValue == nullptr)
            return false;
        *static_cast<f32*>(context.result) = static_cast<RuntimeActor*>(context.instance)->getRatio();
        context.returnValue->type = typeOf<f32>();
        context.returnValue->data = context.result;
        context.returnValue->kind = ReturnKind::kValue;
        return true;
    }

    inline bool invokeAddConstRef(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;
        static_cast<RuntimeActor*>(context.instance)->addConstRef(*static_cast<const i32*>(context.args[0].data));
        return writeVoid(context);
    }

    inline bool invokeMutateRef(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;
        static_cast<RuntimeActor*>(context.instance)->mutateRef(*static_cast<i32*>(context.args[0].data));
        return writeVoid(context);
    }

    inline bool invokeSetPointer(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;
        static_cast<RuntimeActor*>(context.instance)->setPointer(*static_cast<i32**>(context.args[0].data));
        return writeVoid(context);
    }

    inline bool invokeSetMove(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;
        i32& value = *static_cast<i32*>(context.args[0].data);
        static_cast<RuntimeActor*>(context.instance)->setMove(static_cast<i32&&>(value));
        return writeVoid(context);
    }

    inline bool invokeSetValueI32(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;
        static_cast<RuntimeActor*>(context.instance)->setValue(*static_cast<const i32*>(context.args[0].data));
        return writeVoid(context);
    }

    inline bool invokeSetValueF32(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;
        static_cast<RuntimeActor*>(context.instance)->setValue(*static_cast<const f32*>(context.args[0].data));
        return writeVoid(context);
    }

    inline bool invokeSetValueConstRef(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;
        static_cast<RuntimeActor*>(context.instance)->setValueRef(*static_cast<const i32*>(context.args[0].data));
        return writeVoid(context);
    }

    inline bool invokeSetValuePtr(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;
        static_cast<RuntimeActor*>(context.instance)->setValuePtr(*static_cast<i32**>(context.args[0].data));
        return writeVoid(context);
    }

    inline bool invokeSetValueMove(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;
        i32& value = *static_cast<i32*>(context.args[0].data);
        static_cast<RuntimeActor*>(context.instance)->setValueMove(static_cast<i32&&>(value));
        return writeVoid(context);
    }

    inline bool invokeHpRef(InvokeContext& context)
    {
        if (context.instance == nullptr || context.returnValue == nullptr)
            return false;
        context.returnValue->type = typeOf<i32>();
        context.returnValue->data = &static_cast<RuntimeActor*>(context.instance)->hpRef();
        context.returnValue->kind = ReturnKind::kLValueRef;
        return true;
    }

    inline bool invokeHpConstRef(InvokeContext& context)
    {
        if (context.instance == nullptr || context.returnValue == nullptr)
            return false;
        context.returnValue->type = typeOf<i32>();
        context.returnValue->data = const_cast<i32*>(&static_cast<RuntimeActor*>(context.instance)->hpConstRef());
        context.returnValue->kind = ReturnKind::kConstLValueRef;
        return true;
    }

    inline bool invokeHpPointer(InvokeContext& context)
    {
        if (context.instance == nullptr || context.returnValue == nullptr)
            return false;
        RuntimeActor* actor = static_cast<RuntimeActor*>(context.instance);
        actor->hpPointer();
        context.returnValue->type = typeOf<i32>();
        context.returnValue->data = &actor->hpPtr;
        context.returnValue->kind = ReturnKind::kPointer;
        return true;
    }

    inline bool invokeStaticAdd(InvokeContext& context)
    {
        if (context.argCount != 2 || context.args == nullptr || context.result == nullptr || context.returnValue == nullptr)
            return false;
        const i32 lhs = *static_cast<const i32*>(context.args[0].data);
        const i32 rhs = *static_cast<const i32*>(context.args[1].data);
        *static_cast<i32*>(context.result) = RuntimeActor::staticAdd(lhs, rhs);
        context.returnValue->type = typeOf<i32>();
        context.returnValue->data = context.result;
        context.returnValue->kind = ReturnKind::kValue;
        return true;
    }

    inline const Attribute kHpAttributes[] = { Attribute{ .name = "category", .value = "stats" } };
    inline const RkParam kI32ValueParams[] = { RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kValue } };
    inline const RkParam kF32ValueParams[] = { RkParam{ .name = "value", .type = typeOf<f32>(), .passing = ArgPassing::kValue } };
    inline const RkParam kI32ConstRefParams[] = { RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kConstLValueRef } };
    inline const RkParam kI32RefParams[] = { RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kLValueRef } };
    inline const RkParam kI32PtrParams[] = { RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kPointer } };
    inline const RkParam kI32MoveParams[] = { RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kRValueRef } };
    inline const RkParam kStaticAddParams[] =
    {
        RkParam{ .name = "a", .type = typeOf<i32>(), .passing = ArgPassing::kValue },
        RkParam{ .name = "b", .type = typeOf<i32>(), .passing = ArgPassing::kValue },
    };

    inline const RkField kRuntimeActorFields[] =
    {
        RkField{ .name = "hp", .type = typeOf<i32>(), .attributes = AttributeSpan(kHpAttributes, 1), .accessor = &hpAccessor },
        RkField{ .name = "hpPtr", .type = typeOf<i32>(), .accessor = &hpPtrAccessor },
    };

    inline const RkMethod kRuntimeActorMethods[] =
    {
        RkMethod{ .name = "applyDamage", .returnType = typeOf<void>(), .returnKind = ReturnKind::kVoid, .params = TkSpan<const RkParam>(kI32ValueParams, 1), .paramCount = 1, .invoke = &invokeApplyDamage },
        RkMethod{ .name = "getHp", .returnType = typeOf<i32>(), .returnKind = ReturnKind::kValue, .flags = MethodFlags::kConst, .invoke = &invokeGetHp },
        RkMethod{ .name = "getRatio", .returnType = typeOf<f32>(), .returnKind = ReturnKind::kValue, .flags = MethodFlags::kConst, .invoke = &invokeGetRatio },
        RkMethod{ .name = "addConstRef", .returnType = typeOf<void>(), .returnKind = ReturnKind::kVoid, .params = TkSpan<const RkParam>(kI32ConstRefParams, 1), .paramCount = 1, .invoke = &invokeAddConstRef },
        RkMethod{ .name = "mutateRef", .returnType = typeOf<void>(), .returnKind = ReturnKind::kVoid, .params = TkSpan<const RkParam>(kI32RefParams, 1), .paramCount = 1, .invoke = &invokeMutateRef },
        RkMethod{ .name = "setPointer", .returnType = typeOf<void>(), .returnKind = ReturnKind::kVoid, .params = TkSpan<const RkParam>(kI32PtrParams, 1), .paramCount = 1, .invoke = &invokeSetPointer },
        RkMethod{ .name = "setMove", .returnType = typeOf<void>(), .returnKind = ReturnKind::kVoid, .params = TkSpan<const RkParam>(kI32MoveParams, 1), .paramCount = 1, .invoke = &invokeSetMove },
        RkMethod{ .name = "hpRef", .returnType = typeOf<i32>(), .returnKind = ReturnKind::kLValueRef, .invoke = &invokeHpRef },
        RkMethod{ .name = "hpConstRef", .returnType = typeOf<i32>(), .returnKind = ReturnKind::kConstLValueRef, .flags = MethodFlags::kConst, .invoke = &invokeHpConstRef },
        RkMethod{ .name = "hpPointer", .returnType = typeOf<i32>(), .returnKind = ReturnKind::kPointer, .invoke = &invokeHpPointer },
        RkMethod{ .name = "staticAdd", .returnType = typeOf<i32>(), .returnKind = ReturnKind::kValue, .params = TkSpan<const RkParam>(kStaticAddParams, 2), .paramCount = 2, .flags = MethodFlags::kStatic, .invoke = &invokeStaticAdd },
        RkMethod{ .name = "setValue", .returnType = typeOf<void>(), .returnKind = ReturnKind::kVoid, .params = TkSpan<const RkParam>(kI32ValueParams, 1), .paramCount = 1, .invoke = &invokeSetValueI32 },
        RkMethod{ .name = "setValue", .returnType = typeOf<void>(), .returnKind = ReturnKind::kVoid, .params = TkSpan<const RkParam>(kF32ValueParams, 1), .paramCount = 1, .invoke = &invokeSetValueF32 },
        RkMethod{ .name = "setValue", .returnType = typeOf<void>(), .returnKind = ReturnKind::kVoid, .params = TkSpan<const RkParam>(kI32ConstRefParams, 1), .paramCount = 1, .invoke = &invokeSetValueConstRef },
        RkMethod{ .name = "setValue", .returnType = typeOf<void>(), .returnKind = ReturnKind::kVoid, .params = TkSpan<const RkParam>(kI32PtrParams, 1), .paramCount = 1, .invoke = &invokeSetValuePtr },
        RkMethod{ .name = "setValue", .returnType = typeOf<void>(), .returnKind = ReturnKind::kVoid, .params = TkSpan<const RkParam>(kI32MoveParams, 1), .paramCount = 1, .invoke = &invokeSetValueMove },
    };

    inline ::toolkit::TypeInitializer BaseActor::TypeInitializer(
        module(),
        RkClassDesc{
            .name = "BaseActor",
            .fullName = "reflect.test.BaseActor",
            .size = static_cast<u32>(sizeof(BaseActor)),
            .align = static_cast<u32>(alignof(BaseActor)),
            .spawn = &spawnBaseActor,
            .destroy = &destroyBaseActor,
        });

    inline ::toolkit::TypeInitializer RuntimeActor::TypeInitializer(
        module(),
        RkClassDesc{
            .name = "RuntimeActor",
            .fullName = "reflect.test.RuntimeActor",
            .size = static_cast<u32>(sizeof(RuntimeActor)),
            .align = static_cast<u32>(alignof(RuntimeActor)),
            .baseType = BaseActor::staticTypeHandle(),
            .spawn = &spawnRuntimeActor,
            .destroy = &destroyRuntimeActor,
            .fields = TkSpan<const RkField>(kRuntimeActorFields, 2),
            .methods = TkSpan<const RkMethod>(kRuntimeActorMethods, 16),
        });
}
