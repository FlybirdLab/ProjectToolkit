/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "ReflectRuntimeScaffold.hpp"

using namespace reflect_test;

TEST(ReflectMethodCallBuilderTest, SupportsAllArgumentKindsAndResultAccessors)
{
    const ObjectHandle handle = TypeRegistry::newObjectHandle<RuntimeActor>();
    ASSERT_TRUE(handle.isValid());
    auto* actor = static_cast<RuntimeActor*>(TypeRegistry::resolveObject(handle));
    ASSERT_NE(actor, nullptr);

    const RkClass& klass = RuntimeActor::staticClass();
    EXPECT_TRUE(klass.findMethod("applyDamage")->call(handle).arg(3).invoke().ok);
    EXPECT_EQ(actor->hp, 97);

    i32 refValue = 2;
    EXPECT_TRUE(klass.findMethod("mutateRef")->call(handle).ref(refValue).invoke().ok);
    EXPECT_EQ(refValue, 5);

    const i32 crefValue = 4;
    EXPECT_TRUE(klass.findMethod("addConstRef")->call(handle).cref(crefValue).invoke().ok);
    EXPECT_EQ(actor->hp, 101);

    i32 pointer = 6;
    EXPECT_TRUE(klass.findMethod("setPointer")->call(handle).ptr(&pointer).invoke().ok);
    EXPECT_EQ(actor->hpPtr, &pointer);

    i32 moved = 8;
    EXPECT_TRUE(klass.findMethod("setMove")->call(handle).move(moved).invoke().ok);
    EXPECT_EQ(actor->lastMoveI32, 8);

    MethodCallResult valueResult = klass.findMethod("getHp")->call(handle).invoke();
    ASSERT_TRUE(valueResult.ok);
    EXPECT_EQ(valueResult.as<i32>(), 101);

    MethodCallResult ptrResult = klass.findMethod("hpPointer")->call(handle).invoke();
    ASSERT_TRUE(ptrResult.ok);
    EXPECT_EQ(ptrResult.ptr<i32>(), &actor->hp);

    TypeRegistry::destroyObject(handle);
}

TEST(ReflectMethodCallBuilderTest, FailedInvokeReturnsNotOk)
{
    const ObjectHandle handle = TypeRegistry::newObjectHandle<RuntimeActor>();
    ASSERT_TRUE(handle.isValid());

    MethodCallResult result = RuntimeActor::staticClass().findMethod("mutateRef")->call(handle).arg(1).invoke();
    EXPECT_FALSE(result.ok);

    TypeRegistry::destroyObject(handle);
}
