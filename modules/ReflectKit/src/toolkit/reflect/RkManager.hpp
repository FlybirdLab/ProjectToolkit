/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/reflect/RkModule.hpp>
#include <toolkit/reflect/RkObject.hpp>
#include <toolkit/reflect/object/ObjectRegistry.hpp>

namespace toolkit
{
    class TK_API TypeRegistry
    {
    public:
        static void finalize();
        static void markDirty();
        static bool isDirty();

        static TypeHandle findType(const TkStringView& fullName);
        // Preferred creation API is newObjectHandle(). newObject() is a pointer convenience that also registers a handle.
        static RkObject* newObject(TypeHandle type);
        static void destroyObject(RkObject* object);

        static ObjectHandle newObjectHandle(TypeHandle type);
        static void destroyObject(ObjectHandle handle);
        static RkObject* resolveObject(ObjectHandle handle);
        static bool isObjectAlive(ObjectHandle handle);

        template <class T>
        static ObjectHandle newObjectHandle()
        {
            return newObjectHandle(T::staticTypeHandle());
        }

        static void registerObject(RkObject* object);
        static void unregisterObject(RkObject* object);
        static void onObjectIdChanged(RkObject* object, const TkUuid& oldId);

        static RkObject* findObject(const TkUuid& id);
        static TkArray<RkObject*> getObjects();

        template <class T>
        static T* findObject(const TkUuid& id)
        {
            RkObject* object = findObject(id);
            return object != nullptr && object->is<T>() ? static_cast<T*>(object) : nullptr;
        }
    };

    class TK_API RkManager final
    {
    public:
        static IAllocator& getAllocator();

        static TypeHandle findType(const TkStringView& fullName)
        {
            return TypeRegistry::findType(fullName);
        }

        static RkObject* newObject(TypeHandle type)
        {
            return TypeRegistry::newObject(type);
        }

        static void destroyObject(RkObject* object)
        {
            TypeRegistry::destroyObject(object);
        }

        static ObjectHandle newObjectHandle(TypeHandle type)
        {
            return TypeRegistry::newObjectHandle(type);
        }

        template <class T>
        static ObjectHandle newObjectHandle()
        {
            return TypeRegistry::newObjectHandle<T>();
        }

        static void destroyObject(ObjectHandle handle)
        {
            TypeRegistry::destroyObject(handle);
        }

        static RkObject* resolveObject(ObjectHandle handle)
        {
            return TypeRegistry::resolveObject(handle);
        }

        static RkObject* findObject(const TkUuid& id)
        {
            return TypeRegistry::findObject(id);
        }

        template <class T>
        static T* findObject(const TkUuid& id)
        {
            return TypeRegistry::findObject<T>(id);
        }

        static TkArray<RkObject*> getObjects()
        {
            return TypeRegistry::getObjects();
        }
    };
}
