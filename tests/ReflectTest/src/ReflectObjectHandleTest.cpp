/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "ReflectRuntimeScaffold.hpp"

using namespace reflect_test;

TEST(ReflectObjectHandleTest, CreateResolveDestroyInvalidatesHandle)
{
    const ObjectHandle handle = TypeRegistry::newObjectHandle<RuntimeActor>();
    ASSERT_TRUE(handle.isValid());
    EXPECT_TRUE(TypeRegistry::isObjectAlive(handle));

    RkObject* object = TypeRegistry::resolveObject(handle);
    ASSERT_NE(object, nullptr);
    EXPECT_TRUE(object->is<RuntimeActor>());
    EXPECT_NE(object->cast<RuntimeActor>(), nullptr);
    EXPECT_NE(object->cast<BaseActor>(), nullptr);

    TypeRegistry::destroyObject(handle);
    EXPECT_FALSE(TypeRegistry::isObjectAlive(handle));
    EXPECT_EQ(TypeRegistry::resolveObject(handle), nullptr);

    const ObjectHandle baseHandle = TypeRegistry::newObjectHandle<BaseActor>();
    ASSERT_TRUE(baseHandle.isValid());
    RkObject* baseObject = TypeRegistry::resolveObject(baseHandle);
    ASSERT_NE(baseObject, nullptr);
    EXPECT_EQ(baseObject->cast<RuntimeActor>(), nullptr);
    TypeRegistry::destroyObject(baseHandle);
}

TEST(ReflectObjectHandleTest, SlotReuseKeepsOldHandleInvalid)
{
    const ObjectHandle first = TypeRegistry::newObjectHandle<RuntimeActor>();
    ASSERT_TRUE(first.isValid());
    TypeRegistry::destroyObject(first);

    const ObjectHandle second = TypeRegistry::newObjectHandle<RuntimeActor>();
    ASSERT_TRUE(second.isValid());
    EXPECT_EQ(second.index, first.index);
    EXPECT_NE(second.generation, first.generation);
    EXPECT_FALSE(TypeRegistry::isObjectAlive(first));
    EXPECT_EQ(TypeRegistry::resolveObject(first), nullptr);

    TypeRegistry::destroyObject(second);
}

TEST(ReflectObjectHandleTest, InvalidAndRepeatedDestroyAreSafe)
{
    ObjectHandle invalid;
    EXPECT_EQ(TypeRegistry::resolveObject(invalid), nullptr);
    EXPECT_FALSE(TypeRegistry::isObjectAlive(invalid));
    TypeRegistry::destroyObject(invalid);

    const ObjectHandle handle = TypeRegistry::newObjectHandle<RuntimeActor>();
    TypeRegistry::destroyObject(handle);
    TypeRegistry::destroyObject(handle);
}

TEST(ReflectObjectHandleTest, NewObjectAlsoRegistersHandle)
{
    auto* actor = static_cast<RuntimeActor*>(TypeRegistry::newObject(RuntimeActor::staticTypeHandle()));
    ASSERT_NE(actor, nullptr);
    const ObjectHandle handle = actor->getObjectHandle();
    EXPECT_TRUE(handle.isValid());
    EXPECT_EQ(TypeRegistry::resolveObject(handle), actor);
    TypeRegistry::destroyObject(actor);
    EXPECT_FALSE(TypeRegistry::isObjectAlive(handle));
}
