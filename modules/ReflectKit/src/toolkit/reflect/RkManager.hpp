/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/reflect/RkModule.hpp>
#include <toolkit/reflect/RkObject.hpp>
#include <toolkit/reflect/invoking/MethodCall.hpp>
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
        static RkObject* newObject(TypeHandle type, ArgValue* args, u32 argCount);
        static void destroyObject(RkObject* object);

        static ObjectHandle newObjectHandle(TypeHandle type);
        static ObjectHandle newObjectHandle(TypeHandle type, ArgValue* args, u32 argCount);
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

        template <class Ret = void, class Obj = RkObject*, class... Args>
        static Ret invoke(const RkMethod& method, Obj&& instance, Args&&... args)
        {
            return method.invokeTyped<Ret>(toolkit::forward<Obj>(instance), toolkit::forward<Args>(args)...);
        }

        template <class Ret = void, class Obj = RkObject*, class... Args>
        static Ret invoke(const RkMethod* method, Obj&& instance, Args&&... args)
        {
            TK_ASSERT(method != nullptr);
            if (method == nullptr)
            {
                if constexpr (!TIsVoidType<Ret>::Value)
                    return Ret{};
                else
                    return;
            }
            return invoke<Ret>(*method, toolkit::forward<Obj>(instance), toolkit::forward<Args>(args)...);
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

        static RkObject* newObject(TypeHandle type, ArgValue* args, const u32 argCount)
        {
            return TypeRegistry::newObject(type, args, argCount);
        }

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

        static void destroyObject(RkObject* object)
        {
            TypeRegistry::destroyObject(object);
        }

        static ObjectHandle newObjectHandle(TypeHandle type)
        {
            return TypeRegistry::newObjectHandle(type);
        }

        static ObjectHandle newObjectHandle(TypeHandle type, ArgValue* args, const u32 argCount)
        {
            return TypeRegistry::newObjectHandle(type, args, argCount);
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

        template <class Ret = void, class Obj = RkObject*, class... Args>
        static Ret invoke(const RkMethod& method, Obj&& instance, Args&&... args)
        {
            return TypeRegistry::invoke<Ret>(method, toolkit::forward<Obj>(instance), toolkit::forward<Args>(args)...);
        }

        template <class Ret = void, class Obj = RkObject*, class... Args>
        static Ret invoke(const RkMethod* method, Obj&& instance, Args&&... args)
        {
            return TypeRegistry::invoke<Ret>(method, toolkit::forward<Obj>(instance), toolkit::forward<Args>(args)...);
        }
    };
}
