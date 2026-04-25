/*
 * Copyright (c) 2026 Flybird Games. All rights reserved.
 */

#include <toolkit/reflect/RkObject.hpp>
#include <toolkit/reflect/RkManager.hpp>
#include <toolkit/reflect/object/ObjectRegistry.hpp>
#include <toolkit/memory/Memory.hpp>

namespace toolkit
{
    static RkObject *spawnObject(const ObjectSpawnParams &params)
    {
        return TkMemory::newObject<RkObject>(RkManager::getAllocator(), params);
    }

    static void destroyObjectFn(RkObject *object)
    {
        TkMemory::deletePtr(RkManager::getAllocator(), object);
    }

    RkObject::RkObject(const ObjectSpawnParams &params)
        : m_id(params.id.isValid() ? params.id : TkUuid::create()), m_type(params.type)
    {
    }

    RkObject::~RkObject()
    {
        onDeleting();
        onDeleted(this);

        TK_ASSERT(!m_handle.isValid() && "RkObject with live handle must be destroyed through TypeRegistry::destroyObject.");

        if (m_isRegistered)
            unregisterObject();
    }

    bool RkObject::is(const TypeHandle &type) const
    {
        return canCast(getTypeHandle(), type);
    }

    void RkObject::registerObject()
    {
        if (m_isRegistered)
            return;

        m_isRegistered = true;
        TypeRegistry::registerObject(this);
        onRegistered();
    }

    void RkObject::unregisterObject()
    {
        if (!m_isRegistered)
            return;

        TypeRegistry::unregisterObject(this);
        m_isRegistered = false;
        onUnregistered();
    }

    void RkObject::changeId(const TkUuid &newId)
    {
        TK_ASSERT(newId.isValid());
        if (newId == m_id)
            return;

        const TkUuid oldId = m_id;
        m_id = newId;

        if (m_isRegistered)
            TypeRegistry::onObjectIdChanged(this, oldId);
    }

    bool RkObject::canCast(const TypeHandle &from, const TypeHandle &to)
    {
        if (from == to)
            return true;
        if (!from.isValid() || !to.isValid())
            return false;
        return to.isAssignableFrom(from);
    }

    RkObject *RkObject::newObject(const TypeHandle type)
    {
        return TypeRegistry::newObject(type);
    }

    void RkObject::destroyObject(RkObject *object)
    {
        TypeRegistry::destroyObject(object);
    }

    ObjectHandle RkObject::newObjectHandle(const TypeHandle type)
    {
        return TypeRegistry::newObjectHandle(type);
    }

    TypeInitializer RkObject::TypeInitializer(
        getReflectCoreModule(),
        RkClassDesc{
            .name = "RkObject",
            .fullName = "toolkit.RkObject",
            .size = static_cast<u32>(sizeof(RkObject)),
            .align = static_cast<u32>(alignof(RkObject)),
            .spawn = &spawnObject,
            .destroy = &destroyObjectFn,
        });
}
