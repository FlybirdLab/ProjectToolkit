/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/reflect/RkManager.hpp>
#include <toolkit/reflect/RkObject.hpp>

namespace toolkit
{
    class SoftObjectReferenceBase
    {
        NON_COPYABLE(SoftObjectReferenceBase);
    public:
        SoftObjectReferenceBase() = default;

        ~SoftObjectReferenceBase()
        {
            if (m_object != nullptr)
                m_object->onDeleted.template unbind<SoftObjectReferenceBase, &SoftObjectReferenceBase::onDeleted>(this);
        }

        TkUuid getId() const
        {
            return m_id;
        }

    protected:
        void setObject(RkObject* object)
        {
            if (m_object == object)
                return;

            if (m_object != nullptr)
                m_object->onDeleted.template unbind<SoftObjectReferenceBase, &SoftObjectReferenceBase::onDeleted>(this);

            m_object = object;
            m_id = object != nullptr ? object->getId() : TkUuid();

            if (m_object != nullptr)
                m_object->onDeleted.template bind<SoftObjectReferenceBase, &SoftObjectReferenceBase::onDeleted>(this);
        }

        void setId(const TkUuid& id)
        {
            if (m_object != nullptr)
                m_object->onDeleted.template unbind<SoftObjectReferenceBase, &SoftObjectReferenceBase::onDeleted>(this);
            m_object = nullptr;
            m_id = id;
        }

        RkObject* resolve(const TypeHandle& expectedType)
        {
            if (m_object == nullptr && m_id.isValid())
            {
                RkObject* object = TypeRegistry::findObject(m_id);
                if (object != nullptr && (expectedType.isValid() ? object->is(expectedType) : true))
                    setObject(object);
            }
            return m_object;
        }

    private:
        void onDeleted(RkObject* object)
        {
            if (m_object == object)
            {
                m_object->onDeleted.template unbind<SoftObjectReferenceBase, &SoftObjectReferenceBase::onDeleted>(this);
                m_object = nullptr;
                m_id = TkUuid();
            }
        }

    protected:
        RkObject* m_object = nullptr;
        TkUuid m_id;
    };

    template <typename T>
    class SoftObjectReference : public SoftObjectReferenceBase
    {
    public:
        SoftObjectReference() = default;
        explicit SoftObjectReference(T* object)
        {
            setObject(object);
        }

        T* get() const
        {
            return static_cast<T*>(const_cast<SoftObjectReference*>(this)->resolve(T::staticTypeHandle()));
        }

        void set(T* object)
        {
            setObject(object);
        }

        void set(const TkUuid& id)
        {
            setId(id);
        }

        T* operator->() const
        {
            return get();
        }

        operator T*() const
        {
            return get();
        }

        explicit operator bool() const
        {
            return get() != nullptr;
        }
    };

    template <typename T>
    struct HashFunc<SoftObjectReference<T>>
    {
        static u32 get(const SoftObjectReference<T>& key)
        {
            return HashFunc<TkUuid>::get(key.getId());
        }
    };
}
