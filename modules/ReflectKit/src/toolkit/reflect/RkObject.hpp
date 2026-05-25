/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/reflect/RkType.hpp>
#include <toolkit/base/Function.hpp>
#include <toolkit/serialization/ISerializable.hpp>

namespace toolkit
{
    struct RkObjectSpawnAccess;

    class TK_API RkObject
    {
    public:
        static TypeInitializer TypeInitializer;
        using SpawnParams = ObjectSpawnParams;

        static TypeHandle staticTypeHandle()
        {
            return TypeInitializer.getHandle();
        }

        static const RkClass& staticClass()
        {
            return static_cast<const RkClass&>(TypeInitializer.getType());
        }

        static const RkClass& staticType()
        {
            return staticClass();
        }

    public:
        explicit RkObject(const ObjectSpawnParams& params);
        virtual ~RkObject();

        virtual TypeHandle getTypeHandle() const
        {
            return m_type;
        }

        virtual const RkClass& getClass() const
        {
            return static_cast<const RkClass&>(m_type.getType());
        }

        const RkType& getType() const
        {
            return m_type.getType();
        }

        const TkUuid& getId() const
        {
            return m_id;
        }

        ObjectHandle getObjectHandle() const
        {
            return m_handle;
        }

        bool isRegistered() const
        {
            return m_isRegistered;
        }

        bool is(const TypeHandle& type) const;

        template <class T>
        bool is() const
        {
            return is(T::staticTypeHandle());
        }

        template <class T>
        T* cast()
        {
            return is<T>() ? static_cast<T*>(this) : nullptr;
        }

        template <class T>
        const T* cast() const
        {
            return is<T>() ? static_cast<const T*>(this) : nullptr;
        }

        void registerObject();
        void unregisterObject();
        void changeId(const TkUuid& newId);

        static bool canCast(const TypeHandle& from, const TypeHandle& to);
        static RkObject* newObject(TypeHandle type);
        static RkObject* newObject(TypeHandle type, ArgValue* args, u32 argCount);
        static void destroyObject(RkObject* object);
        static ObjectHandle newObjectHandle(TypeHandle type);
        static ObjectHandle newObjectHandle(TypeHandle type, ArgValue* args, u32 argCount);

        template <class T>
        static T* newObject()
        {
            return static_cast<T*>(newObject(T::staticTypeHandle()));
        }

        template <class T>
        static T* newObject(ArgValue* args, const u32 argCount)
        {
            return static_cast<T*>(newObject(T::staticTypeHandle(), args, argCount));
        }

        template <class T>
        static ObjectHandle newObjectHandle()
        {
            return newObjectHandle(T::staticTypeHandle());
        }

        template <class T>
        static ObjectHandle newObjectHandle(ArgValue* args, const u32 argCount)
        {
            return newObjectHandle(T::staticTypeHandle(), args, argCount);
        }

    protected:
        RkObject();

        virtual void onRegistered()
        {
        }

        virtual void onUnregistered()
        {
        }

        virtual void onDeleting()
        {
        }

        virtual TkString toString() const {
            return TkString(getClass().fullName) + "@" + m_id.toString();
        }

    private:
        friend class ObjectRegistry;
        friend class TypeRegistry;
        friend struct RkObjectSpawnAccess;

        void applySpawnParams(const ObjectSpawnParams& params);

    public:
        TkAction<RkObject*> onDeleted;

    private:
        TkUuid m_id;
        TypeHandle m_type;
        ObjectHandle m_handle;
        bool m_isRegistered = false;
    };

    struct RkObjectSpawnAccess
    {
        static void apply(RkObject* object, const ObjectSpawnParams& params)
        {
            object->applySpawnParams(params);
        }
    };

    class SerializableRkObject : public RkObject, public ISerializable
    {
    public:
        using RkObject::RkObject;

        void serialize(SerializeStream& stream, const void* otherObj = nullptr) override
        {
            (void)stream;
            (void)otherObj;
        }

        void deserialize(DeserializeStream stream, ISerializeModifier* modifier = nullptr) override
        {
            (void)stream;
            (void)modifier;
        }
    };
}
