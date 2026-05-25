/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */
#include "Test.hpp"
#include "toolkit/debug/Logger.hpp"

#include <cstddef>
#include <cstdio>

#include <toolkit/reflect/RkManager.hpp>
#include <toolkit/reflect/RkModule.hpp>
#include <toolkit/reflect/RkObject.hpp>
#include <toolkit/reflect/RkType.hpp>
#include <toolkit/reflect/invoking/ArgValue.hpp>
#include <toolkit/reflect/type/TypeResolver.hpp>

void rkgen_register_file_Test(::toolkit::RkModule& module);

namespace
{
    int g_failedCount = 0;

    void expect(bool condition, const char* message)
    {
        if (condition)
            return;

        ++g_failedCount;
        std::printf("[RkGenTest] failed: %s\n", message);
    }

    const toolkit::EnumItem* findEnumItem(const toolkit::RkEnum& enumType, const char* name)
    {
        for (toolkit::i32 i = 0; i < enumType.items.length(); ++i)
        {
            const toolkit::EnumItem& item = enumType.items[i];
            if (item.name == name)
                return &item;
        }
        return nullptr;
    }

    void testVec3Reflection(toolkit::RkModule& module)
    {
        const toolkit::TypeHandle vec3Handle = module.findType("test::Vec3");
        expect(vec3Handle.isValid(), "test::Vec3 should be registered");
        if (!vec3Handle.isValid())
            return;

        const toolkit::RkType& vec3Type = vec3Handle.getType();
        expect(vec3Type.kind == toolkit::TypeKind::kStruct, "test::Vec3 should be a reflected struct");
        expect(vec3Type.name == "Vec3", "test::Vec3 short name should be Vec3");
        expect(vec3Type.fullName == "test::Vec3", "test::Vec3 full name should match C++ qualified name");
        expect(vec3Type.size == sizeof(test::Vec3), "test::Vec3 reflected size should match sizeof");
        expect(vec3Type.align == alignof(test::Vec3), "test::Vec3 reflected align should match alignof");

        const toolkit::RkStruct* vec3Struct = vec3Type.asStruct();
        expect(vec3Struct != nullptr, "test::Vec3 should expose RkStruct data");
        if (vec3Struct == nullptr)
            return;

        expect(vec3Struct->fields.length() == 3, "test::Vec3 should expose 3 fields");

        const toolkit::RkField* xField = vec3Struct->findField("x");
        const toolkit::RkField* yField = vec3Struct->findField("y");
        const toolkit::RkField* zField = vec3Struct->findField("z");
        expect(xField != nullptr, "test::Vec3.x field should exist");
        expect(yField != nullptr, "test::Vec3.y field should exist");
        expect(zField != nullptr, "test::Vec3.z field should exist");
        if (xField == nullptr || yField == nullptr || zField == nullptr)
            return;

        const toolkit::TypeHandle floatType = toolkit::typeOf<float>();
        expect(xField->type == floatType, "test::Vec3.x type should be float");
        expect(yField->type == floatType, "test::Vec3.y type should be float");
        expect(zField->type == floatType, "test::Vec3.z type should be float");

        expect(xField->offset == static_cast<toolkit::u32>(offsetof(test::Vec3, x)), "test::Vec3.x offset should match offsetof");
        expect(yField->offset == static_cast<toolkit::u32>(offsetof(test::Vec3, y)), "test::Vec3.y offset should match offsetof");
        expect(zField->offset == static_cast<toolkit::u32>(offsetof(test::Vec3, z)), "test::Vec3.z offset should match offsetof");

        test::Vec3 value {};
        xField->get<float>(&value) = 1.25f;
        yField->get<float>(&value) = 2.5f;
        zField->get<float>(&value) = 3.75f;

        expect(value.x == 1.25f, "test::Vec3.x should be writable through RkField");
        expect(value.y == 2.5f, "test::Vec3.y should be writable through RkField");
        expect(value.z == 3.75f, "test::Vec3.z should be writable through RkField");
        expect(xField->get<float>(&value) == 1.25f, "test::Vec3.x should be readable through RkField");
        expect(yField->get<float>(&value) == 2.5f, "test::Vec3.y should be readable through RkField");
        expect(zField->get<float>(&value) == 3.75f, "test::Vec3.z should be readable through RkField");
    }

    void testColorReflection(toolkit::RkModule& module)
    {
        const toolkit::TypeHandle colorHandle = module.findType("test::Color");
        expect(colorHandle.isValid(), "test::Color should be registered");
        if (!colorHandle.isValid())
            return;

        const toolkit::RkType& colorType = colorHandle.getType();
        expect(colorType.kind == toolkit::TypeKind::kEnum, "test::Color should be a reflected enum");
        expect(colorType.name == "Color", "test::Color short name should be Color");
        expect(colorType.fullName == "test::Color", "test::Color full name should match C++ qualified name");
        expect(colorType.size == sizeof(test::Color), "test::Color reflected size should match sizeof");
        expect(colorType.align == alignof(test::Color), "test::Color reflected align should match alignof");

        const toolkit::RkEnum* colorEnum = colorType.asEnum();
        expect(colorEnum != nullptr, "test::Color should expose RkEnum data");
        if (colorEnum == nullptr)
            return;

        expect(colorEnum->items.length() == 3, "test::Color should expose 3 enum items");

        const toolkit::EnumItem* red = findEnumItem(*colorEnum, "Red");
        const toolkit::EnumItem* green = findEnumItem(*colorEnum, "Green");
        const toolkit::EnumItem* blue = findEnumItem(*colorEnum, "Blue");
        expect(red != nullptr, "test::Color::Red should exist");
        expect(green != nullptr, "test::Color::Green should exist");
        expect(blue != nullptr, "test::Color::Blue should exist");
        if (red == nullptr || green == nullptr || blue == nullptr)
            return;

        expect(red->value == 0, "test::Color::Red value should be 0");
        expect(green->value == 2, "test::Color::Green value should be 2");
        expect(blue->value == 3, "test::Color::Blue value should be 3");
    }

    void testActorReflection(toolkit::RkModule& module)
    {
        const toolkit::TypeHandle actorHandle = module.findType("test::Actor");
        expect(actorHandle.isValid(), "test::Actor should be registered");
        if (!actorHandle.isValid())
            return;

        const toolkit::RkType& actorType = actorHandle.getType();
        expect(actorType.kind == toolkit::TypeKind::kClass, "test::Actor should be a reflected class");
        expect(actorType.name == "Actor", "test::Actor short name should be Actor");
        expect(actorType.fullName == "test::Actor", "test::Actor full name should match C++ qualified name");
        expect(actorType.size == sizeof(test::Actor), "test::Actor reflected size should match sizeof");
        expect(actorType.align == alignof(test::Actor), "test::Actor reflected align should match alignof");

        const toolkit::RkClass* actorClass = actorType.asClass();
        expect(actorClass != nullptr, "test::Actor should expose RkClass data");
        if (actorClass == nullptr)
            return;

        expect(actorClass->spawn != nullptr, "test::Actor should expose generated spawn thunk");
        expect(actorClass->destroy != nullptr, "test::Actor should expose generated destroy thunk");
        expect(actorClass->fields.length() == 2, "test::Actor should expose 2 fields");
        expect(actorClass->methods.length() == 4, "test::Actor should expose 4 methods");

        const toolkit::RkField* healthField = actorClass->findField("health");
        const toolkit::RkField* energyField = actorClass->findField("energy");
        expect(healthField != nullptr, "test::Actor.health field should exist");
        expect(energyField != nullptr, "test::Actor.energy field should exist");
        if (healthField == nullptr || energyField == nullptr)
            return;

        expect(healthField->type == toolkit::typeOf<toolkit::i32>(), "test::Actor.health type should be i32");
        expect(energyField->type == toolkit::typeOf<toolkit::f32>(), "test::Actor.energy type should be f32");
        expect(toolkit::hasAnyFlags(healthField->flags, toolkit::FieldFlags::kOwnerClass), "test::Actor.health should use class field ownership flag");
        expect(toolkit::hasAnyFlags(energyField->flags, toolkit::FieldFlags::kOwnerClass), "test::Actor.energy should use class field ownership flag");
        expect(healthField->accessor != nullptr, "test::Actor.health should use generated accessor");
        expect(energyField->accessor != nullptr, "test::Actor.energy should use generated accessor");

        test::Actor actor;
        healthField->get<toolkit::i32>(&actor) = 12;
        energyField->get<toolkit::f32>(&actor) = 2.0f;
        expect(actor.health == 12, "test::Actor.health should be writable through generated accessor");
        expect(actor.energy == 2.0f, "test::Actor.energy should be writable through generated accessor");

        const toolkit::RkMethod* setHealth = actorClass->findMethod("SetHealth");
        const toolkit::RkMethod* getHealth = actorClass->findMethod("GetHealth");
        const toolkit::RkMethod* scaleEnergy = actorClass->findMethod("ScaleEnergy");
        const toolkit::RkMethod* sum = actorClass->findMethod("Sum");
        expect(setHealth != nullptr, "test::Actor.SetHealth method should exist");
        expect(getHealth != nullptr, "test::Actor.GetHealth method should exist");
        expect(scaleEnergy != nullptr, "test::Actor.ScaleEnergy method should exist");
        expect(sum != nullptr, "test::Actor.Sum method should exist");
        if (setHealth == nullptr || getHealth == nullptr || scaleEnergy == nullptr || sum == nullptr)
            return;
        expect(setHealth->invoke != nullptr, "test::Actor.SetHealth should have generated invoke thunk");
        expect(getHealth->invoke != nullptr, "test::Actor.GetHealth should have generated invoke thunk");
        expect(scaleEnergy->invoke != nullptr, "test::Actor.ScaleEnergy should have generated invoke thunk");
        expect(sum->invoke != nullptr, "test::Actor.Sum should have generated invoke thunk");
        if (setHealth->invoke == nullptr || getHealth->invoke == nullptr || scaleEnergy->invoke == nullptr || sum->invoke == nullptr)
            return;

        expect(setHealth->returnKind == toolkit::ReturnKind::kVoid, "test::Actor.SetHealth return kind should be void");
        expect(setHealth->params.length() == 1, "test::Actor.SetHealth should have 1 parameter");
        expect(setHealth->params[0].name == "value", "test::Actor.SetHealth parameter name should be value");
        expect(setHealth->params[0].type == toolkit::typeOf<toolkit::i32>(), "test::Actor.SetHealth parameter type should be i32");
        expect(setHealth->params[0].passing == toolkit::ArgPassing::kValue, "test::Actor.SetHealth parameter passing should be value");

        expect(getHealth->returnType == toolkit::typeOf<toolkit::i32>(), "test::Actor.GetHealth return type should be i32");
        expect(getHealth->returnKind == toolkit::ReturnKind::kValue, "test::Actor.GetHealth return kind should be value");
        expect(toolkit::hasAnyFlags(getHealth->flags, toolkit::MethodFlags::kConst), "test::Actor.GetHealth should be marked const");

        expect(scaleEnergy->returnType == toolkit::typeOf<toolkit::f32>(), "test::Actor.ScaleEnergy return type should be f32");
        expect(scaleEnergy->params.length() == 1, "test::Actor.ScaleEnergy should have 1 parameter");
        expect(scaleEnergy->params[0].type == toolkit::typeOf<toolkit::f32>(), "test::Actor.ScaleEnergy parameter type should be f32");

        expect(toolkit::hasAnyFlags(sum->flags, toolkit::MethodFlags::kStatic), "test::Actor.Sum should be marked static");
        expect(sum->params.length() == 2, "test::Actor.Sum should have 2 parameters");

        toolkit::i32 newHealth = 42;
        toolkit::RkManager::invoke(setHealth, &actor, newHealth);
        expect(actor.health == 42, "test::Actor.SetHealth invoke should write health");

        toolkit::i32 healthResult = toolkit::RkManager::invoke<toolkit::i32>(getHealth, &actor);
        expect(healthResult == 42, "test::Actor.GetHealth invoke should return health");

        toolkit::f32 factor = 3.0f;
        toolkit::f32 scaleResult = toolkit::RkManager::invoke<toolkit::f32>(scaleEnergy, &actor, factor);
        expect(scaleResult == 6.0f, "test::Actor.ScaleEnergy invoke should return scaled energy");
        expect(actor.energy == 6.0f, "test::Actor.ScaleEnergy invoke should mutate energy");

        toolkit::i32 lhs = 5;
        toolkit::i32 rhs = 7;
        toolkit::i32 sumResult = toolkit::RkManager::invoke<toolkit::i32>(sum, nullptr, lhs, rhs);
        expect(sumResult == 12, "test::Actor.Sum invoke should return lhs + rhs");

        toolkit::RkObject* spawnedObject = toolkit::RkManager::newObject(actorHandle);
        expect(spawnedObject != nullptr, "test::Actor should be creatable through RkManager");
        if (spawnedObject != nullptr)
        {
            expect(spawnedObject->getTypeHandle() == actorHandle, "spawned test::Actor should keep reflected type handle");
            auto* spawnedActor = static_cast<test::Actor*>(spawnedObject);
            expect(spawnedActor->health == 0, "spawned test::Actor should be default initialized");
            spawnedActor->SetHealth(77);
            expect(spawnedActor->GetHealth() == 77, "spawned test::Actor should behave as concrete Actor");
            toolkit::RkManager::destroyObject(spawnedObject);
        }

        toolkit::i32 ctorHealth = 91;
        toolkit::f32 ctorEnergy = 4.5f;
        toolkit::ArgValue ctorArgs[] = {
            { toolkit::typeOf<toolkit::i32>(), &ctorHealth, toolkit::ArgPassing::kValue, false },
            { toolkit::typeOf<toolkit::f32>(), &ctorEnergy, toolkit::ArgPassing::kValue, false },
        };
        toolkit::RkObject* spawnedWithArgs = toolkit::RkManager::newObject(actorHandle, ctorArgs, 2);
        expect(spawnedWithArgs != nullptr, "test::Actor should be creatable through reflected cctor arguments");
        if (spawnedWithArgs != nullptr)
        {
            auto* spawnedActor = static_cast<test::Actor*>(spawnedWithArgs);
            expect(spawnedActor->health == 91, "reflected cctor should initialize Actor.health");
            expect(spawnedActor->energy == 4.5f, "reflected cctor should initialize Actor.energy");
            toolkit::RkManager::destroyObject(spawnedWithArgs);
        }
    }

    void testNestedStructReflection(toolkit::RkModule& module)
    {
        const toolkit::TypeHandle boundsHandle = module.findType("test::nested::Bounds");
        expect(boundsHandle.isValid(), "test::nested::Bounds should be registered");
        if (!boundsHandle.isValid())
            return;

        const toolkit::RkType& boundsType = boundsHandle.getType();
        expect(boundsType.kind == toolkit::TypeKind::kStruct, "test::nested::Bounds should be a reflected struct");
        expect(boundsType.name == "Bounds", "test::nested::Bounds short name should be Bounds");
        expect(boundsType.fullName == "test::nested::Bounds", "test::nested::Bounds full name should match C++ qualified name");

        const toolkit::RkStruct* boundsStruct = boundsType.asStruct();
        expect(boundsStruct != nullptr, "test::nested::Bounds should expose RkStruct data");
        if (boundsStruct == nullptr)
            return;

        expect(boundsStruct->fields.length() == 4, "test::nested::Bounds should expose 4 fields");
        expect(boundsStruct->findField("minX") != nullptr, "test::nested::Bounds.minX field should exist");
        expect(boundsStruct->findField("minY") != nullptr, "test::nested::Bounds.minY field should exist");
        expect(boundsStruct->findField("maxX") != nullptr, "test::nested::Bounds.maxX field should exist");
        expect(boundsStruct->findField("maxY") != nullptr, "test::nested::Bounds.maxY field should exist");
    }
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    toolkit::RkModule module("RkGenTest");
    rkgen_register_file_Test(module);

    expect(module.getTypeCount() == 4, "RkGenTest module should register 4 reflected types");
    testVec3Reflection(module);
    testColorReflection(module);
    testActorReflection(module);
    testNestedStructReflection(module);
    expect(!module.findType("test::Missing").isValid(), "missing reflected type lookup should fail");

    if (g_failedCount != 0)
    {
        std::printf("[RkGenTest] %d failure(s).\n", g_failedCount);
        return 1;
    }

    std::printf("[RkGenTest] passed.\n");

    auto type = module.findType("test::Actor");
    toolkit::RkObject* instance = toolkit::RkManager::newObject(type);
    test::Actor* actor = instance->cast<test::Actor>();

    const toolkit::RkMethod* mtd = actor->getClass().findMethod("SetHealth");
    toolkit::RkManager::invoke(mtd, actor, toolkit::i32(100));
    
    LogInfo("Test health: %i", actor->GetHealth());
    toolkit::RkManager::destroyObject(instance);
    
    return 0;
}
