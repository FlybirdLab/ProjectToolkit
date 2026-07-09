/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "ReflectRuntimeScaffold.hpp"

#include <toolkit/reflect/invoking/ArgBuilder.hpp>

using namespace reflect_test;

TEST(ReflectArgBuilderTest, BuildsValueRefConstRefPointerAndMoveArgs)
{
    CallFrame frame;
    i32 value = 5;
    const i32 constValue = 6;
    i32* pointer = &value;

    ArgValue valueArg = ArgBuilder::make(frame, 3);
    EXPECT_EQ(valueArg.type, typeOf<i32>());
    EXPECT_EQ(valueArg.passing, ArgPassing::kValue);
    EXPECT_TRUE(valueArg.owns);

    ArgValue refArg = ArgBuilder::makeRef(value);
    EXPECT_EQ(refArg.type, typeOf<i32>());
    EXPECT_EQ(refArg.passing, ArgPassing::kLValueRef);
    EXPECT_FALSE(refArg.owns);

    ArgValue crefArg = ArgBuilder::makeConstRef(constValue);
    EXPECT_EQ(crefArg.type, typeOf<i32>());
    EXPECT_EQ(crefArg.passing, ArgPassing::kConstLValueRef);
    EXPECT_FALSE(crefArg.owns);

    ArgValue ptrArg = ArgBuilder::makePointer(frame, pointer);
    EXPECT_EQ(ptrArg.type, typeOf<i32>());
    EXPECT_EQ(ptrArg.passing, ArgPassing::kPointer);
    EXPECT_TRUE(ptrArg.owns);
    EXPECT_EQ(*static_cast<i32**>(ptrArg.data), pointer);

    ArgValue moveArg = ArgBuilder::makeMove(frame, value);
    EXPECT_EQ(moveArg.type, typeOf<i32>());
    EXPECT_EQ(moveArg.passing, ArgPassing::kRValueRef);
    EXPECT_TRUE(moveArg.owns);
}

TEST(ReflectArgBuilderTest, RkMoveArgBuildsRValueRef)
{
    CallFrame frame;
    i32 value = 42;
    ArgValue arg = ArgBuilder::make(frame, rkMoveArg(value));
    EXPECT_EQ(arg.type, typeOf<i32>());
    EXPECT_EQ(arg.passing, ArgPassing::kRValueRef);
    EXPECT_TRUE(arg.owns);
}

TEST(ReflectArgBuilderTest, TypeOfPointerUsesPointeeType)
{
    EXPECT_EQ(typeOf<RuntimeActor*>(), RuntimeActor::staticTypeHandle());
}
