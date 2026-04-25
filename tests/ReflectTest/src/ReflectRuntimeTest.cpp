/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "common.hpp"

#include <toolkit/reflect/RkManager.hpp>
#include <toolkit/reflect/type/TypeResolver.hpp>
#include <toolkit/reflect/invoking/MethodCall.hpp>

namespace
{
    class TestActor : public RkObject
    {
        R_OBJECT(TestActor)

    public:
        explicit TestActor(const ObjectSpawnParams& params)
            : RkObject(params)
        {
        }

        void applyDamage(const i32 value)
        {
            hp -= value;
            totalDamage += value;
        }

        i32 getHp() const
        {
            return hp;
        }

        void addFromRef(const i32& value)
        {
            hp += value;
        }

        void setHpPointer(i32* value)
        {
            hpPtr = value;
        }

        i32* getHpPointer() const
        {
            return hpPtr;
        }

        const i32& getHpRef() const
        {
            return hp;
        }

        void setValue(const i32 value)
        {
            lastI32 = value;
        }

        void setValue(const f32 value)
        {
            lastF32 = value;
        }

        void setValueRef(const i32& value)
        {
            lastRefI32 = value;
        }

        void setMoveValue(i32&& value)
        {
            lastMoveI32 = value;
            value = 0;
        }

        i32 hp = 100;
        i32 totalDamage = 0;
        i32* hpPtr = nullptr;
        i32 lastI32 = 0;
        i32 lastRefI32 = 0;
        i32 lastMoveI32 = 0;
        f32 lastF32 = 0.0f;
    };

    RkModule& getReflectTestModule()
    {
        static RkModule module("ReflectTest.Runtime");
        return module;
    }

    RkObject* spawnTestActor(const ObjectSpawnParams& params)
    {
        return new TestActor(params);
    }

    void destroyTestActor(RkObject* object)
    {
        delete static_cast<TestActor*>(object);
    }

    void* getTestActorHp(void* object)
    {
        return &(static_cast<TestActor*>(object)->hp);
    }

    void* getTestActorHpPtr(void* object)
    {
        return &(static_cast<TestActor*>(object)->hpPtr);
    }

    bool invokeApplyDamage(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;

        auto* self = static_cast<TestActor*>(context.instance);
        const i32 value = *static_cast<const i32*>(context.args[0].data);
        self->applyDamage(value);

        if (context.returnValue != nullptr)
        {
            context.returnValue->type = typeOf<void>();
            context.returnValue->data = nullptr;
            context.returnValue->kind = ReturnKind::kVoid;
        }
        return true;
    }

    bool invokeGetHp(InvokeContext& context)
    {
        if (context.instance == nullptr || context.result == nullptr)
            return false;

        auto* self = static_cast<TestActor*>(context.instance);
        *static_cast<i32*>(context.result) = self->getHp();

        if (context.returnValue != nullptr)
        {
            context.returnValue->type = typeOf<i32>();
            context.returnValue->data = context.result;
            context.returnValue->kind = ReturnKind::kValue;
        }
        return true;
    }

    bool invokeAddFromConstRef(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;

        auto* self = static_cast<TestActor*>(context.instance);
        const i32& value = *static_cast<const i32*>(context.args[0].data);
        self->addFromRef(value);

        if (context.returnValue != nullptr)
        {
            context.returnValue->type = typeOf<void>();
            context.returnValue->data = nullptr;
            context.returnValue->kind = ReturnKind::kVoid;
        }
        return true;
    }

    bool invokeSetHpPointer(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;

        auto* self = static_cast<TestActor*>(context.instance);
        i32* value = *static_cast<i32**>(context.args[0].data);
        self->setHpPointer(value);

        if (context.returnValue != nullptr)
        {
            context.returnValue->type = typeOf<void>();
            context.returnValue->data = nullptr;
            context.returnValue->kind = ReturnKind::kVoid;
        }
        return true;
    }

    bool invokeGetHpPointer(InvokeContext& context)
    {
        if (context.instance == nullptr || context.returnValue == nullptr)
            return false;

        auto* self = static_cast<TestActor*>(context.instance);
        context.returnValue->type = typeOf<i32>();
        context.returnValue->data = &(self->hpPtr);
        context.returnValue->kind = ReturnKind::kPointer;
        return true;
    }

    bool invokeGetHpRef(InvokeContext& context)
    {
        if (context.instance == nullptr || context.returnValue == nullptr)
            return false;

        auto* self = static_cast<TestActor*>(context.instance);
        context.returnValue->type = typeOf<i32>();
        context.returnValue->data = &(self->hp);
        context.returnValue->kind = ReturnKind::kConstLValueRef;
        return true;
    }

    bool invokeSetValueI32(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;

        auto* self = static_cast<TestActor*>(context.instance);
        const i32 value = *static_cast<const i32*>(context.args[0].data);
        self->setValue(value);
        if (context.returnValue != nullptr)
        {
            context.returnValue->type = typeOf<void>();
            context.returnValue->data = nullptr;
            context.returnValue->kind = ReturnKind::kVoid;
        }
        return true;
    }

    bool invokeSetValueF32(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;

        auto* self = static_cast<TestActor*>(context.instance);
        const f32 value = *static_cast<const f32*>(context.args[0].data);
        self->setValue(value);
        if (context.returnValue != nullptr)
        {
            context.returnValue->type = typeOf<void>();
            context.returnValue->data = nullptr;
            context.returnValue->kind = ReturnKind::kVoid;
        }
        return true;
    }

    bool invokeSetValueConstRef(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;

        auto* self = static_cast<TestActor*>(context.instance);
        const i32& value = *static_cast<const i32*>(context.args[0].data);
        self->setValueRef(value);
        if (context.returnValue != nullptr)
        {
            context.returnValue->type = typeOf<void>();
            context.returnValue->data = nullptr;
            context.returnValue->kind = ReturnKind::kVoid;
        }
        return true;
    }

    bool invokeSetMoveValue(InvokeContext& context)
    {
        if (context.instance == nullptr || context.argCount != 1 || context.args == nullptr)
            return false;

        auto* self = static_cast<TestActor*>(context.instance);
        i32& value = *static_cast<i32*>(context.args[0].data);
        self->setMoveValue(static_cast<i32&&>(value));
        if (context.returnValue != nullptr)
        {
            context.returnValue->type = typeOf<void>();
            context.returnValue->data = nullptr;
            context.returnValue->kind = ReturnKind::kVoid;
        }
        return true;
    }

    const RkParam kApplyDamageParams[] =
    {
        RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kValue },
    };

    const RkParam kAddFromConstRefParams[] =
    {
        RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kConstLValueRef },
    };

    const RkParam kSetHpPointerParams[] =
    {
        RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kPointer },
    };

    const RkParam kSetValueI32Params[] =
    {
        RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kValue },
    };

    const RkParam kSetValueF32Params[] =
    {
        RkParam{ .name = "value", .type = typeOf<f32>(), .passing = ArgPassing::kValue },
    };

    const RkParam kSetValueConstRefParams[] =
    {
        RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kConstLValueRef },
    };

    const RkParam kSetMoveValueParams[] =
    {
        RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kRValueRef },
    };

    const RkField kTestActorFields[] =
    {
        RkField{
            .name = "hp",
            .type = typeOf<i32>(),
            .offset = 0,
            .flags = FieldFlags::kNone,
            .attributes = AttributeSpan(),
            .accessor = &getTestActorHp,
        },
        RkField{
            .name = "hpPtr",
            .type = typeOf<i32>(),
            .offset = 0,
            .flags = FieldFlags::kNone,
            .attributes = AttributeSpan(),
            .accessor = &getTestActorHpPtr,
        },
    };

    const RkMethod kTestActorMethods[] =
    {
        RkMethod{
            .name = "applyDamage",
            .signatureHash = 0,
            .returnType = typeOf<void>(),
            .returnKind = ReturnKind::kVoid,
            .params = TkSpan<const RkParam>(kApplyDamageParams, 1),
            .paramCount = 1,
            .flags = MethodFlags::kNone,
            .invoke = &invokeApplyDamage,
            .attributes = AttributeSpan(),
        },
        RkMethod{
            .name = "getHp",
            .signatureHash = 0,
            .returnType = typeOf<i32>(),
            .returnKind = ReturnKind::kValue,
            .params = TkSpan<const RkParam>(),
            .paramCount = 0,
            .flags = MethodFlags::kConst,
            .invoke = &invokeGetHp,
            .attributes = AttributeSpan(),
        },
        RkMethod{
            .name = "addFromRef",
            .signatureHash = 0,
            .returnType = typeOf<void>(),
            .returnKind = ReturnKind::kVoid,
            .params = TkSpan<const RkParam>(kAddFromConstRefParams, 1),
            .paramCount = 1,
            .flags = MethodFlags::kNone,
            .invoke = &invokeAddFromConstRef,
            .attributes = AttributeSpan(),
        },
        RkMethod{
            .name = "setHpPointer",
            .signatureHash = 0,
            .returnType = typeOf<void>(),
            .returnKind = ReturnKind::kVoid,
            .params = TkSpan<const RkParam>(kSetHpPointerParams, 1),
            .paramCount = 1,
            .flags = MethodFlags::kNone,
            .invoke = &invokeSetHpPointer,
            .attributes = AttributeSpan(),
        },
        RkMethod{
            .name = "getHpPointer",
            .signatureHash = 0,
            .returnType = typeOf<i32>(),
            .returnKind = ReturnKind::kPointer,
            .params = TkSpan<const RkParam>(),
            .paramCount = 0,
            .flags = MethodFlags::kConst,
            .invoke = &invokeGetHpPointer,
            .attributes = AttributeSpan(),
        },
        RkMethod{
            .name = "getHpRef",
            .signatureHash = 0,
            .returnType = typeOf<i32>(),
            .returnKind = ReturnKind::kConstLValueRef,
            .params = TkSpan<const RkParam>(),
            .paramCount = 0,
            .flags = MethodFlags::kConst,
            .invoke = &invokeGetHpRef,
            .attributes = AttributeSpan(),
        },
        RkMethod{
            .name = "setValue",
            .signatureHash = 0,
            .returnType = typeOf<void>(),
            .returnKind = ReturnKind::kVoid,
            .params = TkSpan<const RkParam>(kSetValueI32Params, 1),
            .paramCount = 1,
            .flags = MethodFlags::kNone,
            .invoke = &invokeSetValueI32,
            .attributes = AttributeSpan(),
        },
        RkMethod{
            .name = "setValue",
            .signatureHash = 0,
            .returnType = typeOf<void>(),
            .returnKind = ReturnKind::kVoid,
            .params = TkSpan<const RkParam>(kSetValueF32Params, 1),
            .paramCount = 1,
            .flags = MethodFlags::kNone,
            .invoke = &invokeSetValueF32,
            .attributes = AttributeSpan(),
        },
        RkMethod{
            .name = "setValue",
            .signatureHash = 0,
            .returnType = typeOf<void>(),
            .returnKind = ReturnKind::kVoid,
            .params = TkSpan<const RkParam>(kSetValueConstRefParams, 1),
            .paramCount = 1,
            .flags = MethodFlags::kNone,
            .invoke = &invokeSetValueConstRef,
            .attributes = AttributeSpan(),
        },
        RkMethod{
            .name = "setMoveValue",
            .signatureHash = 0,
            .returnType = typeOf<void>(),
            .returnKind = ReturnKind::kVoid,
            .params = TkSpan<const RkParam>(kSetMoveValueParams, 1),
            .paramCount = 1,
            .flags = MethodFlags::kNone,
            .invoke = &invokeSetMoveValue,
            .attributes = AttributeSpan(),
        },
    };

    TypeInitializer TestActor::TypeInitializer(
        getReflectTestModule(),
        RkClassDesc{
            .name = "TestActor",
            .fullName = "reflect.test.TestActor",
            .size = static_cast<u32>(sizeof(TestActor)),
            .align = static_cast<u32>(alignof(TestActor)),
            .baseType = TypeHandle(),
            .spawn = &spawnTestActor,
            .destroy = &destroyTestActor,
            .interfaces = TkSpan<const TypeHandle>(),
            .fields = TkSpan<const RkField>(kTestActorFields, 2),
            .methods = TkSpan<const RkMethod>(kTestActorMethods, 10),
            .attributes = AttributeSpan(),
            .isAbstract = false,
        });
}

TEST(ReflectRuntimeTest, BuiltinTypesResolve)
{
    EXPECT_TRUE(typeOf<void>().isValid());
    EXPECT_TRUE(typeOf<i32>().isValid());
    EXPECT_TRUE(typeOf<f32>().isValid());
}

TEST(ReflectRuntimeTest, ClassRegistrationIsQueryable)
{
    const TypeHandle found = RkManager::findType("reflect.test.TestActor");
    EXPECT_TRUE(found.isValid());
    EXPECT_EQ(found, TestActor::staticTypeHandle());
    EXPECT_EQ(TestActor::staticClass().fullName, TkStringView("reflect.test.TestActor"));
}

TEST(ReflectRuntimeTest, FieldAccessorReadsAndWrites)
{
    auto* actor = static_cast<TestActor*>(TypeRegistry::newObject(TestActor::staticTypeHandle()));
    ASSERT_NE(actor, nullptr);
    const ObjectHandle handle = actor->getObjectHandle();
    EXPECT_TRUE(handle.isValid());
    EXPECT_TRUE(TypeRegistry::isObjectAlive(handle));

    const RkField* field = actor->getClass().findField("hp");
    ASSERT_NE(field, nullptr);

    EXPECT_EQ(field->get<i32>(actor), 100);
    field->get<i32>(actor) = 72;
    EXPECT_EQ(actor->hp, 72);

    TypeRegistry::destroyObject(actor);
    EXPECT_FALSE(TypeRegistry::isObjectAlive(handle));
}

TEST(ReflectRuntimeTest, MethodLookupAndInvokeWorkWithHandles)
{
    const RkClass& klass = TestActor::staticClass();

    TypeHandle paramTypes[] = { typeOf<i32>() };
    const RkMethod* applyDamage = klass.findMethod("applyDamage", TkSpan<const TypeHandle>(paramTypes, 1));
    ASSERT_NE(applyDamage, nullptr);

    const ObjectHandle handle = TypeRegistry::newObjectHandle(TestActor::staticTypeHandle());
    ASSERT_TRUE(handle.isValid());

    applyDamage->invokeTyped<void>(handle, 35);

    const RkMethod* getHp = klass.findMethod("getHp");
    ASSERT_NE(getHp, nullptr);
    EXPECT_EQ(getHp->invokeTyped<i32>(handle), 65);

    MethodCallResult result = getHp->call(handle).invoke();
    ASSERT_TRUE(result.ok);
    EXPECT_EQ(result.as<i32>(), 65);

    TypeRegistry::destroyObject(handle);
}

TEST(ReflectRuntimeTest, MethodLookupSupportsPassingKinds)
{
    const RkClass& klass = TestActor::staticClass();

    const RkParam crefParams[] =
    {
        RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kConstLValueRef },
    };
    const RkParam ptrParams[] =
    {
        RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kPointer },
    };

    EXPECT_NE(klass.findMethod("addFromRef", TkSpan<const RkParam>(crefParams, 1)), nullptr);
    EXPECT_NE(klass.findMethod("setHpPointer", TkSpan<const RkParam>(ptrParams, 1)), nullptr);
}

TEST(ReflectRuntimeTest, MethodOverloadsResolveByTypeAndPassing)
{
    const RkClass& klass = TestActor::staticClass();

    const RkParam i32Params[] =
    {
        RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kValue },
    };
    const RkParam f32Params[] =
    {
        RkParam{ .name = "value", .type = typeOf<f32>(), .passing = ArgPassing::kValue },
    };
    const RkParam crefParams[] =
    {
        RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kConstLValueRef },
    };

    EXPECT_EQ(klass.findMethod("setValue"), nullptr);
    EXPECT_NE(klass.findMethod("setValue", TkSpan<const RkParam>(i32Params, 1)), nullptr);
    EXPECT_NE(klass.findMethod("setValue", TkSpan<const RkParam>(f32Params, 1)), nullptr);
    EXPECT_NE(klass.findMethod("setValue", TkSpan<const RkParam>(crefParams, 1)), nullptr);
}

TEST(ReflectRuntimeTest, SignatureHashIncludesReturnAndPassing)
{
    const RkClass& klass = TestActor::staticClass();

    const RkParam valueParams[] =
    {
        RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kValue },
    };
    const RkParam crefParams[] =
    {
        RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kConstLValueRef },
    };
    const RkParam ptrParams[] =
    {
        RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kPointer },
    };
    const RkParam moveParams[] =
    {
        RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kRValueRef },
    };

    const u64 valueHash = makeMethodSignatureHash("reflect.test.TestActor", "foo", typeOf<void>(), ReturnKind::kVoid, TkSpan<const RkParam>(valueParams, 1));
    const u64 crefHash = makeMethodSignatureHash("reflect.test.TestActor", "foo", typeOf<void>(), ReturnKind::kVoid, TkSpan<const RkParam>(crefParams, 1));
    const u64 ptrHash = makeMethodSignatureHash("reflect.test.TestActor", "foo", typeOf<void>(), ReturnKind::kVoid, TkSpan<const RkParam>(ptrParams, 1));
    const u64 moveHash = makeMethodSignatureHash("reflect.test.TestActor", "foo", typeOf<void>(), ReturnKind::kVoid, TkSpan<const RkParam>(moveParams, 1));
    const u64 valueReturnHash = makeMethodSignatureHash("reflect.test.TestActor", "foo", typeOf<i32>(), ReturnKind::kValue, TkSpan<const RkParam>(valueParams, 1));

    EXPECT_NE(valueHash, 0u);
    EXPECT_NE(valueHash, crefHash);
    EXPECT_NE(valueHash, ptrHash);
    EXPECT_NE(valueHash, moveHash);
    EXPECT_NE(valueHash, valueReturnHash);

    const RkMethod* setI32 = klass.findMethod("setValue", TkSpan<const RkParam>(valueParams, 1));
    ASSERT_NE(setI32, nullptr);
    EXPECT_NE(setI32->signatureHash, 0u);
}

TEST(ReflectRuntimeTest, InvokeRawRejectsInvalidArgs)
{
    const ObjectHandle handle = TypeRegistry::newObjectHandle<TestActor>();
    ASSERT_TRUE(handle.isValid());

    const RkClass& klass = TestActor::staticClass();
    const RkMethod* addFromRef = klass.findMethod("addFromRef");
    ASSERT_NE(addFromRef, nullptr);

    i32 value = 7;
    MethodCallResult result = addFromRef->call(handle).ptr(&value).invoke();
    EXPECT_FALSE(result.ok);

    TypeRegistry::destroyObject(handle);
}

TEST(ReflectRuntimeTest, MethodInvokeSupportsConstRefPointerAndReferenceReturns)
{
    const ObjectHandle handle = TypeRegistry::newObjectHandle<TestActor>();
    ASSERT_TRUE(handle.isValid());

    auto* actor = static_cast<TestActor*>(TypeRegistry::resolveObject(handle));
    ASSERT_NE(actor, nullptr);

    const RkClass& klass = TestActor::staticClass();
    const RkMethod* addFromRef = klass.findMethod("addFromRef");
    const RkMethod* setHpPointer = klass.findMethod("setHpPointer");
    const RkMethod* getHpPointer = klass.findMethod("getHpPointer");
    const RkMethod* getHpRef = klass.findMethod("getHpRef");
    ASSERT_NE(addFromRef, nullptr);
    ASSERT_NE(setHpPointer, nullptr);
    ASSERT_NE(getHpPointer, nullptr);
    ASSERT_NE(getHpRef, nullptr);

    const i32 bonus = 5;
    addFromRef->invokeTyped<void>(handle, bonus);
    EXPECT_EQ(actor->hp, 105);

    i32 value = 33;
    setHpPointer->call(handle).ptr(&value).invoke();
    EXPECT_EQ(getHpPointer->invokeTyped<i32*>(handle), &value);
    EXPECT_EQ(getHpRef->invokeTyped<const i32&>(handle), 105);

    TypeRegistry::destroyObject(handle);
}

TEST(ReflectRuntimeTest, OverloadedMethodsInvokeExpectedTargets)
{
    const ObjectHandle handle = TypeRegistry::newObjectHandle<TestActor>();
    ASSERT_TRUE(handle.isValid());

    auto* actor = static_cast<TestActor*>(TypeRegistry::resolveObject(handle));
    ASSERT_NE(actor, nullptr);

    const RkClass& klass = TestActor::staticClass();
    const RkParam i32Params[] =
    {
        RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kValue },
    };
    const RkParam f32Params[] =
    {
        RkParam{ .name = "value", .type = typeOf<f32>(), .passing = ArgPassing::kValue },
    };
    const RkParam crefParams[] =
    {
        RkParam{ .name = "value", .type = typeOf<i32>(), .passing = ArgPassing::kConstLValueRef },
    };

    const RkMethod* setI32 = klass.findMethod("setValue", TkSpan<const RkParam>(i32Params, 1));
    const RkMethod* setF32 = klass.findMethod("setValue", TkSpan<const RkParam>(f32Params, 1));
    const RkMethod* setCRef = klass.findMethod("setValue", TkSpan<const RkParam>(crefParams, 1));
    ASSERT_NE(setI32, nullptr);
    ASSERT_NE(setF32, nullptr);
    ASSERT_NE(setCRef, nullptr);

    setI32->invokeTyped<void>(handle, 11);
    EXPECT_EQ(actor->lastI32, 11);

    setF32->invokeTyped<void>(handle, 3.5f);
    EXPECT_FLOAT_EQ(actor->lastF32, 3.5f);

    const i32 refValue = 27;
    setCRef->invokeTyped<void>(handle, refValue);
    EXPECT_EQ(actor->lastRefI32, 27);

    const RkMethod* setMove = klass.findMethod("setMoveValue");
    ASSERT_NE(setMove, nullptr);
    i32 moveValue = 41;
    MethodCallResult moveResult = setMove->call(handle).move(moveValue).invoke();
    EXPECT_TRUE(moveResult.ok);
    EXPECT_EQ(actor->lastMoveI32, 41);

    TypeRegistry::destroyObject(handle);
}

TEST(ReflectRuntimeTest, ObjectHandleResolveAndDestroyInvalidateGeneration)
{
    const ObjectHandle handle = TypeRegistry::newObjectHandle<TestActor>();
    ASSERT_TRUE(handle.isValid());
    ASSERT_TRUE(TypeRegistry::isObjectAlive(handle));

    RkObject* object = TypeRegistry::resolveObject(handle);
    ASSERT_NE(object, nullptr);
    EXPECT_TRUE(object->is<TestActor>());

    TypeRegistry::destroyObject(handle);

    EXPECT_FALSE(TypeRegistry::isObjectAlive(handle));
    EXPECT_EQ(TypeRegistry::resolveObject(handle), nullptr);

    TypeRegistry::destroyObject(handle);

    const ObjectHandle reused = TypeRegistry::newObjectHandle<TestActor>();
    ASSERT_TRUE(reused.isValid());
    EXPECT_EQ(reused.index, handle.index);
    EXPECT_NE(reused.generation, handle.generation);
    EXPECT_FALSE(TypeRegistry::isObjectAlive(handle));
    EXPECT_EQ(TypeRegistry::resolveObject(handle), nullptr);

    TypeRegistry::destroyObject(reused);
}
