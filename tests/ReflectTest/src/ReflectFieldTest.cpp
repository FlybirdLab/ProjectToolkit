/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "ReflectRuntimeScaffold.hpp"

using namespace reflect_test;

TEST(ReflectFieldTest, AccessorFieldsReadWriteAndKeepAttributes)
{
    RuntimeActor actor{ObjectSpawnParams(RuntimeActor::staticTypeHandle())};
    actor.hp = 100;

    const RkField* field = RuntimeActor::staticClass().findField("hp");
    ASSERT_NE(field, nullptr);
    EXPECT_EQ(field->type, typeOf<i32>());
    ASSERT_EQ(field->attributes.length(), 1);
    EXPECT_EQ(field->attributes[0].name, TkStringView("category"));

    EXPECT_EQ(field->get<i32>(&actor), 100);
    field->get<i32>(&actor) = 72;
    EXPECT_EQ(actor.hp, 72);

    const RuntimeActor& constActor = actor;
    EXPECT_EQ(field->get<i32>(&constActor), 72);
}

TEST(ReflectFieldTest, PointerFieldAccessorCanReadAndWritePointerCell)
{
    RuntimeActor actor{ObjectSpawnParams(RuntimeActor::staticTypeHandle())};
    i32 value = 12;

    const RkField* field = RuntimeActor::staticClass().findField("hpPtr");
    ASSERT_NE(field, nullptr);
    auto& pointerCell = field->get<i32*>(&actor);
    pointerCell = &value;
    EXPECT_EQ(actor.hpPtr, &value);
    EXPECT_EQ(*field->get<i32*>(&actor), 12);
}

TEST(ReflectFieldTest, MissingFieldReturnsNull)
{
    EXPECT_EQ(RuntimeActor::staticClass().findField("missing"), nullptr);
}

TEST(ReflectFieldTest, ClassFieldWithoutAccessorReturnsNullInRelease)
{
    RuntimeActor actor{ObjectSpawnParams(RuntimeActor::staticTypeHandle())};
    RkField field;
    field.name = "bad";
    field.type = typeOf<i32>();
    field.flags = FieldFlags::kOwnerClass;

#if !TK_ASSERT_ENABLED
    EXPECT_EQ(field.getPtr(&actor), nullptr);
#else
    SUCCEED();
#endif
}
