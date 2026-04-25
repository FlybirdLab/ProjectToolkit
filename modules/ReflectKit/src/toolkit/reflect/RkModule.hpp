/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/reflect/RkType.hpp>
#include <toolkit/collection/HashMap.hpp>

namespace toolkit
{
    class TK_API RkModule
    {
    public:
        explicit RkModule(const TkStringView& moduleName);
        ~RkModule();

        const TkString& getName() const
        {
            return m_name;
        }

        i32 getTypeCount() const
        {
            return m_types.count();
        }

        const TkArray<RkType*>& getTypes() const
        {
            return m_types;
        }

        TypeHandle registerType(const RkClassDesc& desc);
        TypeHandle registerType(const RkStructDesc& desc);
        TypeHandle registerType(const RkEnumDesc& desc);
        TypeHandle registerType(const RkInterfaceDesc& desc);

        TypeHandle findType(const TkStringView& fullName) const;
        const RkType* findTypePtr(const TkStringView& fullName) const;
        const RkType* tryGetType(i32 index) const;
        const RkType& getType(i32 index) const;

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

    private:
        TypeHandle registerTypeInternal(RkType* type);

    private:
        TkString m_name;
        TkArray<RkType*> m_types;
        TkHashMap<TkString, i32> m_typeIndexByName;
    };

    TK_API RkModule& getReflectCoreModule();
}
