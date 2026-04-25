/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/reflect/RkManager.hpp>
#include <toolkit/reflect/type/TypeResolver.hpp>
#include <toolkit/reflect/object/ObjectRegistry.hpp>
#include <toolkit/memory/Memory.hpp>
#include "RkManager.hpp"
#include "toolkit/memory/PageAllocator.hpp"

namespace toolkit
{
    namespace
    {
        enum class RegistryState : u8
        {
            kNotFinalized = 0,
            kFinalizing = 1,
            kFinalized = 2,
        };

        TkHashMap<TkUuid, RkObject *> &getObjectStorage()
        {
            static TkHashMap<TkUuid, RkObject *> objects;
            return objects;
        }

        RegistryState &getRegistryState()
        {
            static RegistryState state = RegistryState::kNotFinalized;
            return state;
        }

        bool &getRegistryDirty()
        {
            static bool dirty = true;
            return dirty;
        }

        struct BuiltinTypeState
        {
            bool initialized = false;
            TypeHandle handles[static_cast<int>(BuiltinTypeId::kF64) + 1];
        };

        BuiltinTypeState &getBuiltinTypes()
        {
            static BuiltinTypeState state;
            return state;
        }

        void registerBuiltinStruct(
            RkModule &module,
            BuiltinTypeState &state,
            const BuiltinTypeId id,
            const char *fullName,
            const u32 size,
            const u32 align)
        {
            RkStructDesc desc;
            desc.name = fullName;
            desc.fullName = fullName;
            desc.size = size;
            desc.align = align;
            state.handles[static_cast<int>(id)] = module.registerType(desc);
        }

        void ensureBuiltinTypesRegistered()
        {
            BuiltinTypeState &state = getBuiltinTypes();
            if (state.initialized)
                return;

            static RkModule builtinModule("ReflectKit.Builtin");

            registerBuiltinStruct(builtinModule, state, BuiltinTypeId::kVoid, "toolkit.void", 0, 1);
            registerBuiltinStruct(builtinModule, state, BuiltinTypeId::kBool, "toolkit.bool", sizeof(bool), alignof(bool));
            registerBuiltinStruct(builtinModule, state, BuiltinTypeId::kChar, "toolkit.char", sizeof(char), alignof(char));
            registerBuiltinStruct(builtinModule, state, BuiltinTypeId::kU8, "toolkit.u8", sizeof(u8), alignof(u8));
            registerBuiltinStruct(builtinModule, state, BuiltinTypeId::kU16, "toolkit.u16", sizeof(u16), alignof(u16));
            registerBuiltinStruct(builtinModule, state, BuiltinTypeId::kU32, "toolkit.u32", sizeof(u32), alignof(u32));
            registerBuiltinStruct(builtinModule, state, BuiltinTypeId::kU64, "toolkit.u64", sizeof(u64), alignof(u64));
            registerBuiltinStruct(builtinModule, state, BuiltinTypeId::kI8, "toolkit.i8", sizeof(i8), alignof(i8));
            registerBuiltinStruct(builtinModule, state, BuiltinTypeId::kI16, "toolkit.i16", sizeof(i16), alignof(i16));
            registerBuiltinStruct(builtinModule, state, BuiltinTypeId::kI32, "toolkit.i32", sizeof(i32), alignof(i32));
            registerBuiltinStruct(builtinModule, state, BuiltinTypeId::kI64, "toolkit.i64", sizeof(i64), alignof(i64));
            registerBuiltinStruct(builtinModule, state, BuiltinTypeId::kF32, "toolkit.f32", sizeof(f32), alignof(f32));
            registerBuiltinStruct(builtinModule, state, BuiltinTypeId::kF64, "toolkit.f64", sizeof(f64), alignof(f64));

            state.initialized = true;
        }
    }

    TypeHandle getBuiltinTypeHandle(const BuiltinTypeId id)
    {
        ensureBuiltinTypesRegistered();
        return getBuiltinTypes().handles[static_cast<int>(id)];
    }

    RkObject *resolveObjectHandle(const ObjectHandle handle)
    {
        return TypeRegistry::resolveObject(handle);
    }

    TypeHandle TypeRef::resolve() const
    {
        if (!resolved.isValid() && fullName != nullptr)
            resolved = TypeRegistry::findType(fullName);
        return resolved;
    }

    void TypeRegistry::finalize()
    {
        RegistryState &state = getRegistryState();
        if (state == RegistryState::kFinalizing)
            return;
        if (state == RegistryState::kFinalized && !getRegistryDirty())
            return;

        state = RegistryState::kFinalizing;
        ensureBuiltinTypesRegistered();
        // TODO: Phase 2 resolve TypeRef -> TypeHandle once generated descriptors start storing unresolved refs.
        getRegistryDirty() = false;
        state = RegistryState::kFinalized;
    }

    void TypeRegistry::markDirty()
    {
        getRegistryDirty() = true;
        if (getRegistryState() != RegistryState::kFinalizing)
            getRegistryState() = RegistryState::kNotFinalized;
    }

    bool TypeRegistry::isDirty()
    {
        return getRegistryDirty();
    }

    namespace
    {
        RkObject *spawnRegisteredObject(const TypeHandle type)
        {
            TypeRegistry::finalize();
            if (!type.isValid())
                return nullptr;

            const RkClass *klass = type.getType().asClass();
            if (klass == nullptr || klass->spawn == nullptr || klass->isAbstract)
                return nullptr;

            ObjectSpawnParams params(type);
            RkObject *object = klass->spawn(params);
            if (object != nullptr && !object->isRegistered())
                object->registerObject();
            return object;
        }
    }

    RkObject *TypeRegistry::newObject(const TypeHandle type)
    {
        const ObjectHandle handle = newObjectHandle(type);
        return resolveObject(handle);
    }

    ObjectHandle TypeRegistry::newObjectHandle(const TypeHandle type)
    {
        RkObject *object = spawnRegisteredObject(type);
        if (object == nullptr)
            return ObjectHandle();

        TK_ASSERT(!object->m_handle.isValid());
        object->m_handle = getObjectRegistry().add(object);
        return object->m_handle;
    }

    TypeHandle TypeRegistry::findType(const TkStringView &fullName)
    {
        finalize();
        const TkArray<RkModule *> &modules = RkModule::getModules();
        for (RkModule *module : modules)
        {
            const TypeHandle handle = module->findType(fullName);
            if (handle.isValid())
                return handle;
        }
        return TypeHandle();
    }

    void TypeRegistry::destroyObject(RkObject *object)
    {
        if (object == nullptr)
            return;

        if (object->m_handle.isValid())
            getObjectRegistry().remove(object->m_handle);
        object->m_handle = ObjectHandle();

        const RkClass *klass = object->getType().asClass();
        if (klass != nullptr && klass->destroy != nullptr)
            klass->destroy(object);
        else
            TkMemory::deletePtr(RkManager::getAllocator(), object);
    }

    void TypeRegistry::destroyObject(const ObjectHandle handle)
    {
        RkObject *object = resolveObject(handle);
        if (object != nullptr)
            destroyObject(object);
    }

    RkObject *TypeRegistry::resolveObject(const ObjectHandle handle)
    {
        finalize();
        return getObjectRegistry().resolve(handle);
    }

    bool TypeRegistry::isObjectAlive(const ObjectHandle handle)
    {
        finalize();
        return getObjectRegistry().isAlive(handle);
    }

    void TypeRegistry::registerObject(RkObject *object)
    {
        TK_ASSERT(object != nullptr);
        TK_ASSERT(object->getId().isValid());
        getObjectStorage()[object->getId()] = object;
    }

    void TypeRegistry::unregisterObject(RkObject *object)
    {
        if (object == nullptr)
            return;
        getObjectStorage().remove(object->getId());
    }

    void TypeRegistry::onObjectIdChanged(RkObject *object, const TkUuid &oldId)
    {
        if (object == nullptr)
            return;
        getObjectStorage().remove(oldId);
        getObjectStorage()[object->getId()] = object;
    }

    RkObject *TypeRegistry::findObject(const TkUuid &id)
    {
        const auto iterator = getObjectStorage().find(id);
        return iterator.isEnd() ? nullptr : iterator->Value;
    }

    TkArray<RkObject *> TypeRegistry::getObjects()
    {
        TkArray<RkObject *> objects;
        for (auto iterator = getObjectStorage().Begin(); iterator.isNotEnd(); ++iterator)
            objects.add(iterator->Value);
        return objects;
    }

    static PageAllocator g_rkAllocator {};
    IAllocator& RkManager::getAllocator()
    {
        static auto intf = g_rkAllocator.asInterface();
        return intf;
    }

}
