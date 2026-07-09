/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "ReflectRuntimeScaffold.hpp"

using namespace reflect_test;

TEST(ReflectReturnValueTest, ReadsValuesRefsAndPointersWithDeclaredModel)
{
    i32 value = 12;
    i32* pointer = &value;

    ReturnValue byValue;
    byValue.type = typeOf<i32>();
    byValue.data = &value;
    byValue.kind = ReturnKind::kValue;
    EXPECT_EQ(byValue.as<i32>(), 12);
    EXPECT_EQ(byValue.cref<i32>(), 12);

    ReturnValue ref;
    ref.type = typeOf<i32>();
    ref.data = &value;
    ref.kind = ReturnKind::kLValueRef;
    ref.ref<i32>() = 14;
    EXPECT_EQ(value, 14);
    EXPECT_EQ(ref.as<i32>(), 14);

    ReturnValue cref;
    cref.type = typeOf<i32>();
    cref.data = &value;
    cref.kind = ReturnKind::kConstLValueRef;
    EXPECT_EQ(cref.cref<i32>(), 14);
    EXPECT_EQ(cref.as<i32>(), 14);

    ReturnValue ptr;
    ptr.type = typeOf<i32>();
    ptr.data = &pointer;
    ptr.kind = ReturnKind::kPointer;
    EXPECT_EQ(ptr.ptr<i32>(), &value);
}

TEST(ReflectReturnValueTest, InvokeTypedReturnsOnlyMatchingReferenceAndPointerKinds)
{
    const ObjectHandle handle = TypeRegistry::newObjectHandle<RuntimeActor>();
    ASSERT_TRUE(handle.isValid());

    const RkClass& klass = RuntimeActor::staticClass();
    EXPECT_EQ(klass.findMethod("getHp")->invokeTyped<i32>(handle), 100);

    i32& ref = klass.findMethod("hpRef")->invokeTyped<i32&>(handle);
    ref = 77;
    EXPECT_EQ(klass.findMethod("hpConstRef")->invokeTyped<const i32&>(handle), 77);
    EXPECT_EQ(klass.findMethod("hpPointer")->invokeTyped<i32*>(handle), &ref);

    MethodCallResult result = klass.findMethod("getHp")->call(handle).invoke();
    ASSERT_TRUE(result.ok);
    EXPECT_EQ(result.as<i32>(), 77);

    TypeRegistry::destroyObject(handle);
}
