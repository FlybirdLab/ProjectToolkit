/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "ReflectRuntimeScaffold.hpp"

using namespace reflect_test;

TEST(ReflectGeneratedCodeMockTest, FieldAccessorAndWrapperReadConventionsMatchRuntime)
{
    const ObjectHandle handle = TypeRegistry::newObjectHandle<RuntimeActor>();
    ASSERT_TRUE(handle.isValid());
    auto* actor = static_cast<RuntimeActor*>(TypeRegistry::resolveObject(handle));
    ASSERT_NE(actor, nullptr);

    const RkField* hpField = RuntimeActor::staticClass().findField("hp");
    ASSERT_NE(hpField, nullptr);
    hpField->get<i32>(actor) = 64;
    EXPECT_EQ(actor->hp, 64);

    i32 value = 19;
    MethodCallResult ptrResult = RuntimeActor::staticClass().findMethod("setPointer")->call(handle).ptr(&value).invoke();
    ASSERT_TRUE(ptrResult.ok);
    EXPECT_EQ(actor->hpPtr, &value);

    MethodCallResult valueResult = RuntimeActor::staticClass().findMethod("getHp")->call(handle).invoke();
    ASSERT_TRUE(valueResult.ok);
    EXPECT_EQ(valueResult.as<i32>(), 64);

    MethodCallResult refResult = RuntimeActor::staticClass().findMethod("hpRef")->call(handle).invoke();
    ASSERT_TRUE(refResult.ok);
    refResult.ref<i32>() = 81;
    EXPECT_EQ(actor->hp, 81);

    MethodCallResult pointerResult = RuntimeActor::staticClass().findMethod("hpPointer")->call(handle).invoke();
    ASSERT_TRUE(pointerResult.ok);
    EXPECT_EQ(pointerResult.ptr<i32>(), &actor->hp);

    TypeRegistry::destroyObject(handle);
}

TEST(ReflectGeneratedCodeMockTest, ClassDescAndTypeInitializerExposeGeneratedMetadata)
{
    const RkClass& klass = RuntimeActor::staticClass();
    EXPECT_EQ(klass.fields.length(), 2);
    EXPECT_EQ(klass.methods.length(), 16);
    EXPECT_EQ(klass.findMethod("setValue"), nullptr);
    EXPECT_NE(klass.findMethod("setValue", TkSpan<const RkParam>(kI32PtrParams, 1)), nullptr);
}
