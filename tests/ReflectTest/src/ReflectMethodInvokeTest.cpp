/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "ReflectRuntimeScaffold.hpp"

using namespace reflect_test;

TEST(ReflectMethodInvokeTest, InvokesBasicValueRefPointerMoveAndStaticMethods)
{
    const ObjectHandle handle = TypeRegistry::newObjectHandle<RuntimeActor>();
    ASSERT_TRUE(handle.isValid());
    auto* actor = static_cast<RuntimeActor*>(TypeRegistry::resolveObject(handle));
    ASSERT_NE(actor, nullptr);

    const RkClass& klass = RuntimeActor::staticClass();
    klass.findMethod("applyDamage")->invokeTyped<void>(handle, 25);
    EXPECT_EQ(actor->hp, 75);
    EXPECT_EQ(klass.findMethod("getHp")->invokeTyped<i32>(handle), 75);
    EXPECT_FLOAT_EQ(klass.findMethod("getRatio")->invokeTyped<f32>(handle), 0.5f);

    const i32 bonus = 5;
    klass.findMethod("addConstRef")->invokeTyped<void>(handle, bonus);
    EXPECT_EQ(actor->hp, 80);

    i32 refValue = 10;
    ASSERT_TRUE(klass.findMethod("mutateRef")->call(handle).ref(refValue).invoke().ok);
    EXPECT_EQ(refValue, 13);

    i32 pointerValue = 44;
    ASSERT_TRUE(klass.findMethod("setPointer")->call(handle).ptr(&pointerValue).invoke().ok);
    EXPECT_EQ(actor->hpPtr, &pointerValue);

    i32 moveValue = 31;
    ASSERT_TRUE(klass.findMethod("setMove")->call(handle).move(moveValue).invoke().ok);
    EXPECT_EQ(actor->lastMoveI32, 31);

    MethodCallResult staticResult = klass.findMethod("staticAdd")->callStatic().arg(7).arg(8).invoke();
    ASSERT_TRUE(staticResult.ok);
    EXPECT_EQ(staticResult.as<i32>(), 15);

    TypeRegistry::destroyObject(handle);
}

TEST(ReflectMethodInvokeTest, InvokeRawRejectsInvalidArguments)
{
    const ObjectHandle handle = TypeRegistry::newObjectHandle<RuntimeActor>();
    ASSERT_TRUE(handle.isValid());
    RuntimeActor* actor = static_cast<RuntimeActor*>(TypeRegistry::resolveObject(handle));
    ASSERT_NE(actor, nullptr);

    const RkMethod* mutateRef = RuntimeActor::staticClass().findMethod("mutateRef");
    const RkMethod* addConstRef = RuntimeActor::staticClass().findMethod("addConstRef");
    const RkMethod* setPointer = RuntimeActor::staticClass().findMethod("setPointer");
    const RkMethod* setMove = RuntimeActor::staticClass().findMethod("setMove");
    ASSERT_NE(mutateRef, nullptr);
    ASSERT_NE(addConstRef, nullptr);
    ASSERT_NE(setPointer, nullptr);
    ASSERT_NE(setMove, nullptr);

    CallFrame frame;
    i32 value = 5;
    ArgValue valueArg = ArgBuilder::make(frame, 5);
    ArgValue refArg = ArgBuilder::makeRef(value);
    ArgValue crefArg = ArgBuilder::makeConstRef(value);
    ArgValue ptrArg = ArgBuilder::makePointer(frame, &value);
    ArgValue moveArg = ArgBuilder::makeMove(frame, value);
    ArgValue f32Arg = ArgBuilder::make(frame, 1.0f);

    InvokeContext ctx;
    ctx.instance = actor;
    ctx.args = &valueArg;
    ctx.argCount = 0;
    EXPECT_FALSE(mutateRef->invokeRaw(ctx));
    ctx.argCount = 2;
    EXPECT_FALSE(mutateRef->invokeRaw(ctx));

    ctx.argCount = 1;
    ctx.args = &f32Arg;
    EXPECT_FALSE(mutateRef->invokeRaw(ctx));
    ctx.args = &valueArg;
    EXPECT_FALSE(mutateRef->invokeRaw(ctx));
    ctx.args = &crefArg;
    EXPECT_FALSE(mutateRef->invokeRaw(ctx));

    ctx.args = &valueArg;
    EXPECT_TRUE(addConstRef->validateArgs(ctx));
    ctx.args = &refArg;
    EXPECT_TRUE(addConstRef->validateArgs(ctx));
    ctx.args = &crefArg;
    EXPECT_TRUE(addConstRef->validateArgs(ctx));

    ctx.args = &ptrArg;
    EXPECT_TRUE(setPointer->validateArgs(ctx));
    ctx.args = &refArg;
    EXPECT_FALSE(setPointer->validateArgs(ctx));

    ctx.args = &moveArg;
    EXPECT_TRUE(setMove->validateArgs(ctx));
    ctx.args = &valueArg;
    EXPECT_TRUE(setMove->validateArgs(ctx));
    ctx.args = &crefArg;
    EXPECT_FALSE(setMove->validateArgs(ctx));

    TypeRegistry::destroyObject(handle);
}

TEST(ReflectMethodInvokeTest, StaticAndNonStaticInstanceRulesAreEnforced)
{
    const RkClass& klass = RuntimeActor::staticClass();
    EXPECT_TRUE(hasAnyFlags(klass.findMethod("staticAdd")->flags, MethodFlags::kStatic));
    EXPECT_TRUE(hasAnyFlags(klass.findMethod("getHp")->flags, MethodFlags::kConst));

    MethodCallResult staticResult = klass.findMethod("staticAdd")->callStatic().arg(1).arg(2).invoke();
    EXPECT_TRUE(staticResult.ok);
    EXPECT_EQ(staticResult.as<i32>(), 3);

    MethodCallResult nonStaticResult = klass.findMethod("getHp")->callStatic().invoke();
    EXPECT_FALSE(nonStaticResult.ok);
}
