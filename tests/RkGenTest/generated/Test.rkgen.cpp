#include "E:/Projects/ToolkitProject/tests/RkGenTest/src/Test.hpp"
#include <toolkit/reflect/RkModule.hpp>
#include <toolkit/reflect/RkManager.hpp>
#include <toolkit/reflect/RkObject.hpp>
#include <toolkit/reflect/RkType.hpp>
#include <toolkit/reflect/invoking/ArgValue.hpp>
#include <toolkit/reflect/invoking/InvokeContext.hpp>
#include <toolkit/reflect/invoking/ReturnValue.hpp>
#include <toolkit/reflect/type/TypeResolver.hpp>
#include <toolkit/memory/Memory.hpp>
#include <cstddef>

namespace {
    const ::toolkit::EnumItem s_enum_items_test__Color[] = {
        { "Red", static_cast<::toolkit::i64>(0) },
        { "Green", static_cast<::toolkit::i64>(2) },
        { "Blue", static_cast<::toolkit::i64>(3) },
    };
    
    void* rk_field_accessor_test__Vec3_x(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::Vec3*>(instance)->x;
    }
    
    void* rk_field_accessor_test__Vec3_y(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::Vec3*>(instance)->y;
    }
    
    void* rk_field_accessor_test__Vec3_z(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::Vec3*>(instance)->z;
    }
    
    const ::toolkit::RkField s_fields_test__Vec3[] = {
        { "x", ::toolkit::typeOf<float>(), static_cast<::toolkit::u32>(offsetof(test::Vec3, x)), toolkit::FieldFlags::kNone, {}, nullptr },
        { "y", ::toolkit::typeOf<float>(), static_cast<::toolkit::u32>(offsetof(test::Vec3, y)), toolkit::FieldFlags::kNone, {}, nullptr },
        { "z", ::toolkit::typeOf<float>(), static_cast<::toolkit::u32>(offsetof(test::Vec3, z)), toolkit::FieldFlags::kNone, {}, nullptr },
    };
    
    const ::toolkit::RkMethod s_methods_test__Vec3[] = {
        {},
    };
    
    ::toolkit::RkObject* rk_spawn_test__Actor(const ::toolkit::ObjectSpawnParams& params)
    {
        if (params.argCount == 0)
        {
            auto* object = ::toolkit::TkMemory::newObject<test::Actor>(::toolkit::RkManager::getAllocator());
            ::toolkit::RkObjectSpawnAccess::apply(object, params);
            return object;
        }
        if (params.argCount == 2)
        {
            if (params.args == nullptr) return nullptr;
            if (params.args[0].data == nullptr) return nullptr;
            if (params.args[0].type != ::toolkit::TypeHandle()) return nullptr;
            if (params.args[0].passing != toolkit::ArgPassing::kValue) return nullptr;
            auto& initialHealth = *reinterpret_cast<toolkit::i32*>(params.args[0].data);
            if (params.args[1].data == nullptr) return nullptr;
            if (params.args[1].type != ::toolkit::TypeHandle()) return nullptr;
            if (params.args[1].passing != toolkit::ArgPassing::kValue) return nullptr;
            auto& initialEnergy = *reinterpret_cast<toolkit::f32*>(params.args[1].data);
            auto* object = ::toolkit::TkMemory::newObject<test::Actor>(::toolkit::RkManager::getAllocator(), initialHealth, initialEnergy);
            ::toolkit::RkObjectSpawnAccess::apply(object, params);
            return object;
        }
        return nullptr;
    }
    
    void rk_destroy_test__Actor(::toolkit::RkObject* object)
    {
        ::toolkit::TkMemory::deletePtr(::toolkit::RkManager::getAllocator(), static_cast<test::Actor*>(object));
    }
    
    void* rk_field_accessor_test__Actor_health(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::Actor*>(instance)->health;
    }
    
    void* rk_field_accessor_test__Actor_energy(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::Actor*>(instance)->GetEnergy();
    }
    
    bool rk_invoke_test__Actor_SetHealth(::toolkit::InvokeContext& ctx)
    {
        if (ctx.argCount != 1) return false;
        auto* self = static_cast<test::Actor*>(ctx.instance);
        if (self == nullptr) return false;
        auto& value = *reinterpret_cast<toolkit::i32*>(ctx.args[0].data);
        self->SetHealth(value);
        return true;
    }
    
    bool rk_invoke_test__Actor_GetHealth(::toolkit::InvokeContext& ctx)
    {
        if (ctx.argCount != 0) return false;
        auto* self = static_cast<test::Actor*>(ctx.instance);
        if (self == nullptr) return false;
        if (ctx.result == nullptr) return false;
        *reinterpret_cast<toolkit::i32*>(ctx.result) = self->GetHealth();
        return true;
    }
    
    bool rk_invoke_test__Actor_ScaleEnergy(::toolkit::InvokeContext& ctx)
    {
        if (ctx.argCount != 1) return false;
        auto* self = static_cast<test::Actor*>(ctx.instance);
        if (self == nullptr) return false;
        auto& factor = *reinterpret_cast<toolkit::f32*>(ctx.args[0].data);
        if (ctx.result == nullptr) return false;
        *reinterpret_cast<toolkit::f32*>(ctx.result) = self->ScaleEnergy(factor);
        return true;
    }
    
    bool rk_invoke_test__Actor_Sum(::toolkit::InvokeContext& ctx)
    {
        if (ctx.argCount != 2) return false;
        auto& lhs = *reinterpret_cast<toolkit::i32*>(ctx.args[0].data);
        auto& rhs = *reinterpret_cast<toolkit::i32*>(ctx.args[1].data);
        if (ctx.result == nullptr) return false;
        *reinterpret_cast<toolkit::i32*>(ctx.result) = test::Actor::Sum(lhs, rhs);
        return true;
    }
    
    const ::toolkit::RkField s_fields_test__Actor[] = {
        { "health", ::toolkit::TypeHandle(), 0, toolkit::FieldFlags::kNone | toolkit::FieldFlags::kOwnerClass, {}, &rk_field_accessor_test__Actor_health },
        { "energy", ::toolkit::TypeHandle(), 0, toolkit::FieldFlags::kNone | toolkit::FieldFlags::kOwnerClass, {}, &rk_field_accessor_test__Actor_energy },
    };
    
    const ::toolkit::RkParam s_params_test__Actor_SetHealth[] = {
        { "value", ::toolkit::TypeHandle(), toolkit::ArgPassing::kValue },
    };
    const ::toolkit::RkParam s_params_test__Actor_GetHealth[] = {
        {},
    };
    const ::toolkit::RkParam s_params_test__Actor_ScaleEnergy[] = {
        { "factor", ::toolkit::TypeHandle(), toolkit::ArgPassing::kValue },
    };
    const ::toolkit::RkParam s_params_test__Actor_Sum[] = {
        { "lhs", ::toolkit::TypeHandle(), toolkit::ArgPassing::kValue },
        { "rhs", ::toolkit::TypeHandle(), toolkit::ArgPassing::kValue },
    };
    const ::toolkit::RkMethod s_methods_test__Actor[] = {
        { "SetHealth", 0, ::toolkit::TypeHandle(), toolkit::ReturnKind::kVoid, ::toolkit::TkSpan<const ::toolkit::RkParam>(s_params_test__Actor_SetHealth, 1), 1, toolkit::MethodFlags::kNone, &rk_invoke_test__Actor_SetHealth, {} },
        { "GetHealth", 0, ::toolkit::TypeHandle(), toolkit::ReturnKind::kValue, ::toolkit::TkSpan<const ::toolkit::RkParam>(s_params_test__Actor_GetHealth, 0), 0, toolkit::MethodFlags::kNone | toolkit::MethodFlags::kConst, &rk_invoke_test__Actor_GetHealth, {} },
        { "ScaleEnergy", 0, ::toolkit::TypeHandle(), toolkit::ReturnKind::kValue, ::toolkit::TkSpan<const ::toolkit::RkParam>(s_params_test__Actor_ScaleEnergy, 1), 1, toolkit::MethodFlags::kNone, &rk_invoke_test__Actor_ScaleEnergy, {} },
        { "Sum", 0, ::toolkit::TypeHandle(), toolkit::ReturnKind::kValue, ::toolkit::TkSpan<const ::toolkit::RkParam>(s_params_test__Actor_Sum, 2), 2, toolkit::MethodFlags::kNone | toolkit::MethodFlags::kStatic, &rk_invoke_test__Actor_Sum, {} },
    };
    
    void* rk_field_accessor_test__nested__Bounds_minX(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::nested::Bounds*>(instance)->minX;
    }
    
    void* rk_field_accessor_test__nested__Bounds_minY(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::nested::Bounds*>(instance)->minY;
    }
    
    void* rk_field_accessor_test__nested__Bounds_maxX(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::nested::Bounds*>(instance)->maxX;
    }
    
    void* rk_field_accessor_test__nested__Bounds_maxY(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::nested::Bounds*>(instance)->maxY;
    }
    
    const ::toolkit::RkField s_fields_test__nested__Bounds[] = {
        { "minX", ::toolkit::typeOf<float>(), static_cast<::toolkit::u32>(offsetof(test::nested::Bounds, minX)), toolkit::FieldFlags::kNone, {}, nullptr },
        { "minY", ::toolkit::typeOf<float>(), static_cast<::toolkit::u32>(offsetof(test::nested::Bounds, minY)), toolkit::FieldFlags::kNone, {}, nullptr },
        { "maxX", ::toolkit::typeOf<float>(), static_cast<::toolkit::u32>(offsetof(test::nested::Bounds, maxX)), toolkit::FieldFlags::kNone, {}, nullptr },
        { "maxY", ::toolkit::typeOf<float>(), static_cast<::toolkit::u32>(offsetof(test::nested::Bounds, maxY)), toolkit::FieldFlags::kNone, {}, nullptr },
    };
    
    const ::toolkit::RkMethod s_methods_test__nested__Bounds[] = {
        {},
    };
    
}

void rkgen_register_test__Color(::toolkit::RkModule& module)
{
    ::toolkit::RkEnumDesc desc{};
    desc.name = "Color";
    desc.fullName = "test::Color";
    desc.size = sizeof(test::Color);
    desc.align = alignof(test::Color);
    desc.items = ::toolkit::TkSpan<const ::toolkit::EnumItem>(s_enum_items_test__Color, 3);
    module.registerType(desc);
}

void rkgen_register_test__Vec3(::toolkit::RkModule& module)
{
    ::toolkit::RkStructDesc desc{};
    desc.name = "Vec3";
    desc.fullName = "test::Vec3";
    desc.size = sizeof(test::Vec3);
    desc.align = alignof(test::Vec3);
    desc.fields = ::toolkit::TkSpan<const ::toolkit::RkField>(s_fields_test__Vec3, 3);
    module.registerType(desc);
}

void rkgen_register_test__Actor(::toolkit::RkModule& module)
{
    ::toolkit::RkClassDesc desc{};
    desc.name = "Actor";
    desc.fullName = "test::Actor";
    desc.size = sizeof(test::Actor);
    desc.align = alignof(test::Actor);
    desc.baseType = ::toolkit::typeOf<::toolkit::RkObject>();
    desc.spawn = &rk_spawn_test__Actor;
    desc.destroy = &rk_destroy_test__Actor;
    desc.fields = ::toolkit::TkSpan<const ::toolkit::RkField>(s_fields_test__Actor, 2);
    desc.methods = ::toolkit::TkSpan<const ::toolkit::RkMethod>(s_methods_test__Actor, 4);
    module.registerType(desc);
}

void rkgen_register_test__nested__Bounds(::toolkit::RkModule& module)
{
    ::toolkit::RkStructDesc desc{};
    desc.name = "Bounds";
    desc.fullName = "test::nested::Bounds";
    desc.size = sizeof(test::nested::Bounds);
    desc.align = alignof(test::nested::Bounds);
    desc.fields = ::toolkit::TkSpan<const ::toolkit::RkField>(s_fields_test__nested__Bounds, 4);
    module.registerType(desc);
}

void rkgen_register_file_Test(::toolkit::RkModule& module)
{
    rkgen_register_test__Color(module);
    rkgen_register_test__Vec3(module);
    rkgen_register_test__Actor(module);
    rkgen_register_test__nested__Bounds(module);
}
