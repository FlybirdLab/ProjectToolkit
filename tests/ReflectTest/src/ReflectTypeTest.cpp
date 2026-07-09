/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "ReflectRuntimeScaffold.hpp"

using namespace reflect_test;

TEST(ReflectTypeTest, BuiltinTypeHandlesAreStableAndNormalizeCvRefPointer)
{
    EXPECT_TRUE(typeOf<void>().isValid());
    EXPECT_TRUE(typeOf<i32>().isValid());
    EXPECT_TRUE(typeOf<f32>().isValid());
    EXPECT_EQ(typeOf<i32>(), typeOf<i32>());
    EXPECT_EQ(typeOf<const i32>(), typeOf<i32>());
    EXPECT_EQ(typeOf<i32&>(), typeOf<i32>());
    EXPECT_EQ(typeOf<i32*>(), typeOf<i32>());
}

TEST(ReflectTypeTest, ClassRegistrationIsQueryable)
{
    const TypeHandle found = RkManager::findType("reflect.test.RuntimeActor");
    ASSERT_TRUE(found.isValid());
    EXPECT_EQ(found, RuntimeActor::staticTypeHandle());
    EXPECT_EQ(RkManager::findType("reflect.test.RuntimeActor"), found);
    EXPECT_FALSE(RkManager::findType("reflect.test.DoesNotExist").isValid());

    const RkClass& klass = RuntimeActor::staticClass();
    EXPECT_EQ(klass.fullName, TkStringView("reflect.test.RuntimeActor"));
    EXPECT_EQ(klass.size, static_cast<u32>(sizeof(RuntimeActor)));
    EXPECT_EQ(klass.align, static_cast<u32>(alignof(RuntimeActor)));
    EXPECT_EQ(klass.baseType, BaseActor::staticTypeHandle());
}

TEST(ReflectTypeTest, FinalizeDirtyStateIsRepeatable)
{
    TypeRegistry::markDirty();
    EXPECT_TRUE(TypeRegistry::isDirty());
    TypeRegistry::finalize();
    EXPECT_FALSE(TypeRegistry::isDirty());
    TypeRegistry::finalize();
    EXPECT_FALSE(TypeRegistry::isDirty());

    static RkModule lateModule("ReflectTest.LateTypes");
    static bool registered = false;
    if (!registered)
    {
        RkStructDesc desc;
        desc.name = "LateStruct";
        desc.fullName = "reflect.test.LateStruct";
        desc.size = sizeof(i32);
        desc.align = alignof(i32);
        lateModule.registerType(desc);
        registered = true;
    }

    EXPECT_TRUE(TypeRegistry::isDirty());
    EXPECT_TRUE(RkManager::findType("reflect.test.LateStruct").isValid());
    EXPECT_FALSE(TypeRegistry::isDirty());
}
