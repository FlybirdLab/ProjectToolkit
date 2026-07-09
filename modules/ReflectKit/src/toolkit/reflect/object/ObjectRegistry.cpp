/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include <toolkit/reflect/object/ObjectRegistry.hpp>

namespace toolkit
{
    ObjectHandle ObjectRegistry::add(RkObject* object)
    {
        TK_ASSERT(object != nullptr);

        u32 index = 0;
        if (m_freeList.count() > 0)
        {
            index = m_freeList.last();
            m_freeList.removeLast();
        }
        else
        {
            index = static_cast<u32>(m_slots.count());
            m_slots.add(Slot());
        }

        Slot& slot = m_slots[static_cast<i32>(index)];
        TK_ASSERT(slot.object == nullptr);
        if (slot.generation == 0)
            slot.generation = 1;

        slot.object = object;
        return ObjectHandle{ index, slot.generation };
    }

    void ObjectRegistry::remove(const ObjectHandle handle)
    {
        if (!handle.isValid() || !m_slots.isValidIndex(static_cast<i32>(handle.index)))
            return;

        Slot& slot = m_slots[static_cast<i32>(handle.index)];
        if (slot.generation != handle.generation || slot.object == nullptr)
            return;

        slot.object = nullptr;
        ++slot.generation;
        if (slot.generation == 0)
            slot.generation = 1;
        m_freeList.add(handle.index);
    }

    RkObject* ObjectRegistry::resolve(const ObjectHandle handle)
    {
        return const_cast<RkObject*>(const_cast<const ObjectRegistry*>(this)->resolve(handle));
    }

    const RkObject* ObjectRegistry::resolve(const ObjectHandle handle) const
    {
        if (!handle.isValid() || !m_slots.isValidIndex(static_cast<i32>(handle.index)))
            return nullptr;

        const Slot& slot = m_slots[static_cast<i32>(handle.index)];
        if (slot.generation != handle.generation)
            return nullptr;
        return slot.object;
    }

    bool ObjectRegistry::isAlive(const ObjectHandle handle) const
    {
        return resolve(handle) != nullptr;
    }

    ObjectRegistry& getObjectRegistry()
    {
        static ObjectRegistry registry;
        return registry;
    }
}
