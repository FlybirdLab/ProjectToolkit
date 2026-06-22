#include "D:/Projects/ToolkitProject/tests/RkGenTest/src/Test.hpp"
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

namespace test {
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
        { "x", ::toolkit::typeOf<float>(), static_cast<::toolkit::u32>(offsetof(test::Vec3, x)), toolkit::FieldFlags::kNone, ::toolkit::AttributeSpan(), nullptr, { toolkit::FieldSerializationFlags::kNone, "", "", "" } },
        { "y", ::toolkit::typeOf<float>(), static_cast<::toolkit::u32>(offsetof(test::Vec3, y)), toolkit::FieldFlags::kNone, ::toolkit::AttributeSpan(), nullptr, { toolkit::FieldSerializationFlags::kNone, "", "", "" } },
        { "z", ::toolkit::typeOf<float>(), static_cast<::toolkit::u32>(offsetof(test::Vec3, z)), toolkit::FieldFlags::kNone, ::toolkit::AttributeSpan(), nullptr, { toolkit::FieldSerializationFlags::kNone, "", "", "" } },
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
            if (params.args[0].type != ::toolkit::typeOf<toolkit::i32>()) return nullptr;
            if (params.args[0].passing != toolkit::ArgPassing::kValue) return nullptr;
            auto& _p_initialHealth = *reinterpret_cast<toolkit::i32*>(params.args[0].data);
            if (params.args[1].data == nullptr) return nullptr;
            if (params.args[1].type != ::toolkit::typeOf<toolkit::f32>()) return nullptr;
            if (params.args[1].passing != toolkit::ArgPassing::kValue) return nullptr;
            auto& _p_initialEnergy = *reinterpret_cast<toolkit::f32*>(params.args[1].data);
            auto* object = ::toolkit::TkMemory::newObject<test::Actor>(::toolkit::RkManager::getAllocator(), _p_initialHealth, _p_initialEnergy);
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
        { "health", ::toolkit::typeOf<toolkit::i32>(), 0, toolkit::FieldFlags::kNone | toolkit::FieldFlags::kOwnerClass, ::toolkit::AttributeSpan(), &rk_field_accessor_test__Actor_health, { toolkit::FieldSerializationFlags::kNone, "", "", "" } },
        { "energy", ::toolkit::typeOf<toolkit::f32>(), 0, toolkit::FieldFlags::kNone | toolkit::FieldFlags::kOwnerClass, ::toolkit::AttributeSpan(), &rk_field_accessor_test__Actor_energy, { toolkit::FieldSerializationFlags::kNone, "", "", "" } },
    };

    const ::toolkit::RkParam s_params_test__Actor_SetHealth[] = {
        { "value", ::toolkit::typeOf<toolkit::i32>(), toolkit::ArgPassing::kValue },
    };
    const ::toolkit::RkParam s_params_test__Actor_GetHealth[] = {
        {},
    };
    const ::toolkit::RkParam s_params_test__Actor_ScaleEnergy[] = {
        { "factor", ::toolkit::typeOf<toolkit::f32>(), toolkit::ArgPassing::kValue },
    };
    const ::toolkit::RkParam s_params_test__Actor_Sum[] = {
        { "lhs", ::toolkit::typeOf<toolkit::i32>(), toolkit::ArgPassing::kValue },
        { "rhs", ::toolkit::typeOf<toolkit::i32>(), toolkit::ArgPassing::kValue },
    };
    const ::toolkit::RkMethod s_methods_test__Actor[] = {
        { "SetHealth", 0, ::toolkit::typeOf<void>(), toolkit::ReturnKind::kVoid, ::toolkit::TkSpan<const ::toolkit::RkParam>(s_params_test__Actor_SetHealth, 1), 1, toolkit::MethodFlags::kNone, &rk_invoke_test__Actor_SetHealth, ::toolkit::AttributeSpan() },
        { "GetHealth", 0, ::toolkit::typeOf<toolkit::i32>(), toolkit::ReturnKind::kValue, ::toolkit::TkSpan<const ::toolkit::RkParam>(s_params_test__Actor_GetHealth, 0), 0, toolkit::MethodFlags::kNone | toolkit::MethodFlags::kConst, &rk_invoke_test__Actor_GetHealth, ::toolkit::AttributeSpan() },
        { "ScaleEnergy", 0, ::toolkit::typeOf<toolkit::f32>(), toolkit::ReturnKind::kValue, ::toolkit::TkSpan<const ::toolkit::RkParam>(s_params_test__Actor_ScaleEnergy, 1), 1, toolkit::MethodFlags::kNone, &rk_invoke_test__Actor_ScaleEnergy, ::toolkit::AttributeSpan() },
        { "Sum", 0, ::toolkit::typeOf<toolkit::i32>(), toolkit::ReturnKind::kValue, ::toolkit::TkSpan<const ::toolkit::RkParam>(s_params_test__Actor_Sum, 2), 2, toolkit::MethodFlags::kNone | toolkit::MethodFlags::kStatic, &rk_invoke_test__Actor_Sum, ::toolkit::AttributeSpan() },
    };

    void* rk_field_accessor_test__RuntimePayload_migratedValue(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::RuntimePayload*>(instance)->migratedValue;
    }

    void* rk_field_accessor_test__RuntimePayload_label(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::RuntimePayload*>(instance)->label;
    }

    void* rk_field_accessor_test__RuntimePayload_labelView(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::RuntimePayload*>(instance)->labelView;
    }

    void* rk_field_accessor_test__RuntimePayload_uuid(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::RuntimePayload*>(instance)->uuid;
    }

    void* rk_field_accessor_test__RuntimePayload_dateTime(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::RuntimePayload*>(instance)->dateTime;
    }

    void* rk_field_accessor_test__RuntimePayload_timeSpan(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::RuntimePayload*>(instance)->timeSpan;
    }

    void* rk_field_accessor_test__RuntimePayload_version(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::RuntimePayload*>(instance)->version;
    }

    void* rk_field_accessor_test__RuntimePayload_position(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::RuntimePayload*>(instance)->position;
    }

    void* rk_field_accessor_test__RuntimePayload_rotation(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::RuntimePayload*>(instance)->rotation;
    }

    void* rk_field_accessor_test__RuntimePayload_owner(void* instance)
    {
        if (instance == nullptr) return nullptr;
        return &static_cast<test::RuntimePayload*>(instance)->owner;
    }

    bool rk_invoke_test__RuntimePayload_Describe(::toolkit::InvokeContext& ctx)
    {
        if (ctx.argCount != 2) return false;
        auto* self = static_cast<test::RuntimePayload*>(ctx.instance);
        if (self == nullptr) return false;
        auto& prefix = *reinterpret_cast<TkStringView*>(ctx.args[0].data);
        auto& offset = *reinterpret_cast<FVec3*>(ctx.args[1].data);
        if (ctx.result == nullptr) return false;
        *reinterpret_cast<TkString*>(ctx.result) = self->Describe(prefix, offset);
        return true;
    }

    const ::toolkit::Attribute s_attrs_test__RuntimePayload[] = {
        { "LoadOrder", "10" },
        { "Plugin", "Core" },
    };

    const ::toolkit::Attribute s_attrs_test__RuntimePayload_migratedValue[] = {
        { "NoSerialize", "" },
        { "OldName", "legacyName" },
        { "Default", "42" },
    };

    const ::toolkit::Attribute s_attrs_test__RuntimePayload_label[] = {
        { "EditorOnly", "" },
        { "ReadOnly", "" },
    };

    const ::toolkit::Attribute s_attrs_test__RuntimePayload_owner[] = {
        { "SoftObjectReference", "" },
        { "ReferencePolicy", "Weak" },
    };

    const ::toolkit::Attribute s_attrs_test__RuntimePayload_Describe[] = {
        { "RuntimeCallable", "" },
    };

    const ::toolkit::RkField s_fields_test__RuntimePayload[] = {
        { "migratedValue", ::toolkit::typeOf<toolkit::i32>(), static_cast<::toolkit::u32>(offsetof(test::RuntimePayload, migratedValue)), toolkit::FieldFlags::kNone | toolkit::FieldFlags::kNoSerialize, ::toolkit::AttributeSpan(s_attrs_test__RuntimePayload_migratedValue, 3), nullptr, { toolkit::FieldSerializationFlags::kNone | toolkit::FieldSerializationFlags::kNoSerialize, "42", "legacyName", "" } },
        { "label", ::toolkit::typeOf<::toolkit::TkString>(), static_cast<::toolkit::u32>(offsetof(test::RuntimePayload, label)), toolkit::FieldFlags::kNone | toolkit::FieldFlags::kEditorOnly, ::toolkit::AttributeSpan(s_attrs_test__RuntimePayload_label, 2), nullptr, { toolkit::FieldSerializationFlags::kNone | toolkit::FieldSerializationFlags::kEditorOnly | toolkit::FieldSerializationFlags::kReadOnly, "", "", "" } },
        { "labelView", ::toolkit::typeOf<::toolkit::TkStringView>(), static_cast<::toolkit::u32>(offsetof(test::RuntimePayload, labelView)), toolkit::FieldFlags::kNone, ::toolkit::AttributeSpan(), nullptr, { toolkit::FieldSerializationFlags::kNone, "", "", "" } },
        { "uuid", ::toolkit::typeOf<::toolkit::TkUuid>(), static_cast<::toolkit::u32>(offsetof(test::RuntimePayload, uuid)), toolkit::FieldFlags::kNone, ::toolkit::AttributeSpan(), nullptr, { toolkit::FieldSerializationFlags::kNone, "", "", "" } },
        { "dateTime", ::toolkit::typeOf<::toolkit::TkDateTime>(), static_cast<::toolkit::u32>(offsetof(test::RuntimePayload, dateTime)), toolkit::FieldFlags::kNone, ::toolkit::AttributeSpan(), nullptr, { toolkit::FieldSerializationFlags::kNone, "", "", "" } },
        { "timeSpan", ::toolkit::typeOf<::toolkit::TkTimeSpan>(), static_cast<::toolkit::u32>(offsetof(test::RuntimePayload, timeSpan)), toolkit::FieldFlags::kNone, ::toolkit::AttributeSpan(), nullptr, { toolkit::FieldSerializationFlags::kNone, "", "", "" } },
        { "version", ::toolkit::typeOf<::toolkit::TkVersion>(), static_cast<::toolkit::u32>(offsetof(test::RuntimePayload, version)), toolkit::FieldFlags::kNone, ::toolkit::AttributeSpan(), nullptr, { toolkit::FieldSerializationFlags::kNone, "", "", "" } },
        { "position", ::toolkit::typeOf<::toolkit::FVec3>(), static_cast<::toolkit::u32>(offsetof(test::RuntimePayload, position)), toolkit::FieldFlags::kNone, ::toolkit::AttributeSpan(), nullptr, { toolkit::FieldSerializationFlags::kNone, "", "", "" } },
        { "rotation", ::toolkit::typeOf<::toolkit::Quat>(), static_cast<::toolkit::u32>(offsetof(test::RuntimePayload, rotation)), toolkit::FieldFlags::kNone, ::toolkit::AttributeSpan(), nullptr, { toolkit::FieldSerializationFlags::kNone, "", "", "" } },
        { "owner", ::toolkit::typeOf<::toolkit::RkObjectRef<test::Actor>>(), static_cast<::toolkit::u32>(offsetof(test::RuntimePayload, owner)), toolkit::FieldFlags::kNone | toolkit::FieldFlags::kSoftObjectReference, ::toolkit::AttributeSpan(s_attrs_test__RuntimePayload_owner, 2), nullptr, { toolkit::FieldSerializationFlags::kNone | toolkit::FieldSerializationFlags::kSoftObjectReference, "", "", "Weak" } },
    };

    const ::toolkit::RkParam s_params_test__RuntimePayload_Describe[] = {
        { "prefix", ::toolkit::typeOf<::toolkit::TkStringView>(), toolkit::ArgPassing::kValue },
        { "offset", ::toolkit::typeOf<::toolkit::FVec3>(), toolkit::ArgPassing::kValue },
    };
    const ::toolkit::RkMethod s_methods_test__RuntimePayload[] = {
        { "Describe", 0, ::toolkit::typeOf<::toolkit::TkString>(), toolkit::ReturnKind::kValue, ::toolkit::TkSpan<const ::toolkit::RkParam>(s_params_test__RuntimePayload_Describe, 2), 2, toolkit::MethodFlags::kNone | toolkit::MethodFlags::kConst, &rk_invoke_test__RuntimePayload_Describe, ::toolkit::AttributeSpan(s_attrs_test__RuntimePayload_Describe, 1) },
    };

}

void rkgen_register_test__Color(::toolkit::RkModule& module)
{
    ::toolkit::RkEnumDesc desc{};
    desc.name = "Color";
    desc.fullName = "test::Color";
    desc.size = sizeof(test::Color);
    desc.align = alignof(test::Color);
    desc.attributes = ::toolkit::AttributeSpan();
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
    desc.attributes = ::toolkit::AttributeSpan();
    desc.fields = ::toolkit::TkSpan<const ::toolkit::RkField>(s_fields_test__Vec3, 3);
    desc.methods = ::toolkit::TkSpan<const ::toolkit::RkMethod>(s_methods_test__Vec3, 0);
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
    desc.attributes = ::toolkit::AttributeSpan();
    desc.spawn = &rk_spawn_test__Actor;
    desc.destroy = &rk_destroy_test__Actor;
    desc.fields = ::toolkit::TkSpan<const ::toolkit::RkField>(s_fields_test__Actor, 2);
    desc.methods = ::toolkit::TkSpan<const ::toolkit::RkMethod>(s_methods_test__Actor, 4);
    module.registerType(desc);
}

void rkgen_register_test__RuntimePayload(::toolkit::RkModule& module)
{
    ::toolkit::RkStructDesc desc{};
    desc.name = "RuntimePayload";
    desc.fullName = "test::RuntimePayload";
    desc.size = sizeof(test::RuntimePayload);
    desc.align = alignof(test::RuntimePayload);
    desc.attributes = ::toolkit::AttributeSpan(s_attrs_test__RuntimePayload, 2);
    desc.fields = ::toolkit::TkSpan<const ::toolkit::RkField>(s_fields_test__RuntimePayload, 10);
    desc.methods = ::toolkit::TkSpan<const ::toolkit::RkMethod>(s_methods_test__RuntimePayload, 1);
    module.registerType(desc);
}

} // namespace test

namespace test::nested {
namespace {
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
        { "minX", ::toolkit::typeOf<float>(), static_cast<::toolkit::u32>(offsetof(test::nested::Bounds, minX)), toolkit::FieldFlags::kNone, ::toolkit::AttributeSpan(), nullptr, { toolkit::FieldSerializationFlags::kNone, "", "", "" } },
        { "minY", ::toolkit::typeOf<float>(), static_cast<::toolkit::u32>(offsetof(test::nested::Bounds, minY)), toolkit::FieldFlags::kNone, ::toolkit::AttributeSpan(), nullptr, { toolkit::FieldSerializationFlags::kNone, "", "", "" } },
        { "maxX", ::toolkit::typeOf<float>(), static_cast<::toolkit::u32>(offsetof(test::nested::Bounds, maxX)), toolkit::FieldFlags::kNone, ::toolkit::AttributeSpan(), nullptr, { toolkit::FieldSerializationFlags::kNone, "", "", "" } },
        { "maxY", ::toolkit::typeOf<float>(), static_cast<::toolkit::u32>(offsetof(test::nested::Bounds, maxY)), toolkit::FieldFlags::kNone, ::toolkit::AttributeSpan(), nullptr, { toolkit::FieldSerializationFlags::kNone, "", "", "" } },
    };

    const ::toolkit::RkMethod s_methods_test__nested__Bounds[] = {
        {},
    };

}

void rkgen_register_test__nested__Bounds(::toolkit::RkModule& module)
{
    ::toolkit::RkStructDesc desc{};
    desc.name = "Bounds";
    desc.fullName = "test::nested::Bounds";
    desc.size = sizeof(test::nested::Bounds);
    desc.align = alignof(test::nested::Bounds);
    desc.attributes = ::toolkit::AttributeSpan();
    desc.fields = ::toolkit::TkSpan<const ::toolkit::RkField>(s_fields_test__nested__Bounds, 4);
    desc.methods = ::toolkit::TkSpan<const ::toolkit::RkMethod>(s_methods_test__nested__Bounds, 0);
    module.registerType(desc);
}

} // namespace test::nested

void rkgen_register_file_Test(::toolkit::RkModule& module)
{
    test::rkgen_register_test__Color(module);
    test::rkgen_register_test__Vec3(module);
    test::rkgen_register_test__Actor(module);
    test::rkgen_register_test__RuntimePayload(module);
    test::nested::rkgen_register_test__nested__Bounds(module);
}
