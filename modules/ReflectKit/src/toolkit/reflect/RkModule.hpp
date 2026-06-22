/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/reflect/RkType.hpp>
#include <toolkit/base/Function.hpp>
#include <toolkit/collection/HashMap.hpp>

namespace toolkit
{
    enum class RkModuleEventKind : u8
    {
        kLoaded = 0,
        kBeforeUnload,
        kUnloaded,
        kBeforeReload,
        kReloaded,
        kLoadFailed,
    };

    struct RkModuleEvent
    {
        RkModule *module = nullptr;
        RkModuleEventKind kind = RkModuleEventKind::kLoaded;
    };

    class TK_API RkModule
    {
    public:
        explicit RkModule(const TkStringView& moduleName);
        ~RkModule();

        const TkString& getName() const
        {
            return m_name;
        }

        u64 getId() const
        {
            return m_id;
        }

        u32 getGeneration() const
        {
            return m_generation;
        }

        bool isLoaded() const
        {
            return m_loaded;
        }

        i32 getTypeCount() const
        {
            return m_types.count();
        }

        const TkArray<RkType*>& getTypes() const
        {
            return m_types;
        }

        const TkString& getLibraryPath() const
        {
            return m_libraryPath;
        }

        void* getLibraryHandle() const
        {
            return m_libraryHandle;
        }

        const TkArray<RkModule*>& getDependencies() const
        {
            return m_dependencies;
        }

        TypeHandle registerType(const RkClassDesc& desc);
        TypeHandle registerType(const RkStructDesc& desc);
        TypeHandle registerType(const RkEnumDesc& desc);
        TypeHandle registerType(const RkInterfaceDesc& desc);

        TypeHandle findType(const TkStringView& fullName) const;
        const RkType* findTypePtr(const TkStringView& fullName) const;
        const RkType* tryGetType(i32 index) const;
        const RkType& getType(i32 index) const;
        bool isHandleAlive(const TypeHandle& handle) const;
        u64 makeTypeId(const TkStringView& fullName) const;

        const RkField* findField(TypeHandle owner, const TkStringView& fieldName) const;
        const RkMethod* findMethod(TypeHandle owner, const TkStringView& methodName) const;
        const RkMethod* findMethod(TypeHandle owner, const TkStringView& methodName, u64 signatureHash) const;
        const RkMethod* findMethod(TypeHandle owner, const TkStringView& methodName, TkSpan<const TypeHandle> paramTypes) const;
        const RkMethod* findMethod(TypeHandle owner, const TkStringView& methodName, TkSpan<const RkParam> params) const;

        bool findReflectType(const TkStringView& fullName, i32& typeIndex) const
        {
            const TypeHandle handle = findType(fullName);
            typeIndex = handle.typeIndex;
            return handle.isValid();
        }

        static const TkArray<RkModule*>& getModules();
        static RkModule* findModule(const TkStringView& moduleName);
        static RkModule* findModuleById(u64 moduleId);
        static TkAction<const RkModuleEvent&>& events();

        bool load(const TkStringView& libraryPath);
        void attachLibrary(void* handle, const TkStringView& libraryPath = TkStringView());
        void addDependency(RkModule* module);
        bool dependsOn(const RkModule* module) const;
        void unload();
        void reload();

    private:
        TypeHandle registerTypeInternal(RkType* type);
        void clearTypes();

    private:
        TkString m_name;
        u64 m_id = 0;
        u32 m_generation = 1;
        bool m_loaded = true;
        void* m_libraryHandle = nullptr;
        TkString m_libraryPath;
        TkArray<RkModule*> m_dependencies;
        TkArray<RkType*> m_types;
        TkHashMap<TkString, i32> m_typeIndexByName;
    };

    TK_API RkModule& getReflectCoreModule();
}
