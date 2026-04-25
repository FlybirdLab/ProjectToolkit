/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "ReflectRuntimeScaffold.hpp"

using namespace reflect_test;

TEST(ReflectMethodLookupTest, NameLookupReturnsOnlyUniqueMethods)
{
    const RkClass& klass = RuntimeActor::staticClass();
    EXPECT_NE(klass.findMethod("getHp"), nullptr);
    EXPECT_EQ(klass.findMethod("setValue"), nullptr);
    EXPECT_EQ(klass.findMethod("missing"), nullptr);
}

TEST(ReflectMethodLookupTest, TypeHandleLookupIsOnlyShortcutForUnambiguousCases)
{
    const RkClass& klass = RuntimeActor::staticClass();
    TypeHandle damageTypes[] = { typeOf<i32>() };
    TypeHandle setValueI32[] = { typeOf<i32>() };

    EXPECT_NE(klass.findMethod("applyDamage", TkSpan<const TypeHandle>(damageTypes, 1)), nullptr);
    EXPECT_EQ(klass.findMethod("setValue", TkSpan<const TypeHandle>(setValueI32, 1)), nullptr);
}

TEST(ReflectMethodLookupTest, RkParamLookupDistinguishesPassingKinds)
{
    const RkClass& klass = RuntimeActor::staticClass();
    EXPECT_NE(klass.findMethod("setValue", TkSpan<const RkParam>(kI32ValueParams, 1)), nullptr);
    EXPECT_NE(klass.findMethod("setValue", TkSpan<const RkParam>(kF32ValueParams, 1)), nullptr);
    EXPECT_NE(klass.findMethod("setValue", TkSpan<const RkParam>(kI32ConstRefParams, 1)), nullptr);
    EXPECT_NE(klass.findMethod("setValue", TkSpan<const RkParam>(kI32PtrParams, 1)), nullptr);
    EXPECT_NE(klass.findMethod("setValue", TkSpan<const RkParam>(kI32MoveParams, 1)), nullptr);
}

TEST(ReflectMethodLookupTest, SignatureHashIncludesSignatureShape)
{
    const RkMethod* valueMethod = RuntimeActor::staticClass().findMethod("setValue", TkSpan<const RkParam>(kI32ValueParams, 1));
    const RkMethod* crefMethod = RuntimeActor::staticClass().findMethod("setValue", TkSpan<const RkParam>(kI32ConstRefParams, 1));
    ASSERT_NE(valueMethod, nullptr);
    ASSERT_NE(crefMethod, nullptr);

    const u64 same = makeMethodSignatureHash(
        "reflect.test.RuntimeActor",
        "setValue",
        typeOf<void>(),
        ReturnKind::kVoid,
        TkSpan<const RkParam>(kI32ValueParams, 1));
    const u64 differentReturn = makeMethodSignatureHash(
        "reflect.test.RuntimeActor",
        "setValue",
        typeOf<i32>(),
        ReturnKind::kValue,
        TkSpan<const RkParam>(kI32ValueParams, 1));

    EXPECT_NE(valueMethod->signatureHash, 0u);
    EXPECT_EQ(valueMethod->signatureHash, same);
    EXPECT_NE(valueMethod->signatureHash, crefMethod->signatureHash);
    EXPECT_NE(valueMethod->signatureHash, differentReturn);
}
