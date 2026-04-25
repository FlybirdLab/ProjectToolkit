/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "ReflectRuntimeScaffold.hpp"

#include <toolkit/reflect/RkValue.hpp>

using namespace reflect_test;

TEST(RkValueTest, StoresTypeAndDataAsBoundaryValue)
{
    i32 value = 17;
    RkValue reflected;
    reflected.type = typeOf<i32>();
    reflected.data = &value;
    reflected.owns = false;

    EXPECT_EQ(reflected.type, typeOf<i32>());
    EXPECT_EQ(reflected.as<i32>(), 17);
}

TEST(RkValueTest, MethodInvokeCoreDoesNotNeedRkValue)
{
    const ObjectHandle handle = TypeRegistry::newObjectHandle<RuntimeActor>();
    ASSERT_TRUE(handle.isValid());

    MethodCallResult result = RuntimeActor::staticClass().findMethod("getHp")->call(handle).invoke();
    ASSERT_TRUE(result.ok);
    EXPECT_EQ(result.as<i32>(), 100);

    TypeRegistry::destroyObject(handle);
}
