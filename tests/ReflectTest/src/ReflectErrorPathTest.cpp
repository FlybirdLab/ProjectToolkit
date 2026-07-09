/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "ReflectRuntimeScaffold.hpp"

using namespace reflect_test;

TEST(ReflectErrorPathTest, InvalidLookupsAndHandlesAreSafe)
{
    TypeHandle invalidType;
    EXPECT_FALSE(invalidType.isValid());

    ObjectHandle invalidHandle;
    EXPECT_EQ(TypeRegistry::resolveObject(invalidHandle), nullptr);
    EXPECT_FALSE(TypeRegistry::isObjectAlive(invalidHandle));
    TypeRegistry::destroyObject(invalidHandle);

    EXPECT_FALSE(RkManager::findType("reflect.test.Nope").isValid());
    EXPECT_EQ(RuntimeActor::staticClass().findField("nope"), nullptr);
    EXPECT_EQ(RuntimeActor::staticClass().findMethod("nope"), nullptr);
}

TEST(ReflectErrorPathTest, NullInvokeFunctionReturnsFalse)
{
    RkMethod method;
    method.name = "nullInvoke";
    method.returnType = typeOf<void>();
    method.returnKind = ReturnKind::kVoid;
    method.flags = MethodFlags::kStatic;

    InvokeContext context;
    EXPECT_FALSE(method.invokeRaw(context));
}

TEST(ReflectErrorPathTest, VoidMethodAllowsNullReturnValue)
{
    const ObjectHandle handle = TypeRegistry::newObjectHandle<RuntimeActor>();
    ASSERT_TRUE(handle.isValid());

    const RkMethod* method = RuntimeActor::staticClass().findMethod("applyDamage");
    ASSERT_NE(method, nullptr);

    CallFrame frame;
    ArgValue arg = ArgBuilder::make(frame, 1);
    InvokeContext context;
    context.instance = TypeRegistry::resolveObject(handle);
    context.args = &arg;
    context.argCount = 1;
    context.returnValue = nullptr;
    EXPECT_TRUE(method->invokeRaw(context));

    TypeRegistry::destroyObject(handle);
}
