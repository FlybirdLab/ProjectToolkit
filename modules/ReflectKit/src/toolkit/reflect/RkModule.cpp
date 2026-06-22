/*
 * Copyright (c) 2026 Flybird Games. All rights reserved.
 */

#include <toolkit/reflect/RkModule.hpp>
#include <toolkit/reflect/RkEvents.hpp>
#include <toolkit/reflect/RkManager.hpp>
#include <toolkit/reflect/RkObject.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/platforms/System.hpp>

namespace toolkit
{
    namespace
    {
        TkArray<RkModule*>& getModuleStorage()
        {
            static TkArray<RkModule*> modules;
            return modules;
        }

        TkAction<const RkModuleEvent&>& getModuleEvents()
        {
            static TkAction<const RkModuleEvent&> events;
            return events;
        }

        u64 hashModuleName(const TkStringView& name)
        {
            u64 hash = 1469598103934665603ull;
            for (i32 i = 0; i < name.length(); ++i)
            {
                hash ^= static_cast<u8>(name[i]);
                hash *= 1099511628211ull;
            }
            return hash != 0 ? hash : 1;
        }

        TkStringView makeLeafName(const TkStringView& fullName)
        {
            const i32 colonIndex = fullName.findLast(':');
            if (colonIndex >= 1 && fullName[colonIndex - 1] == ':')
                return fullName.substring(colonIndex + 1);

            const i32 dotIndex = fullName.findLast('.');
            if (dotIndex >= 0)
                return fullName.substring(dotIndex + 1);

            return fullName;
        }

        const RkField* findFieldInSpan(const TkSpan<const RkField>& fields, const TkStringView& fieldName)
        {
            for (const RkField& field : fields)
            {
                if (field.name == fieldName)
                    return &field;
            }
            return nullptr;
        }

        const RkMethod* findUniqueMethodInSpan(const TkSpan<const RkMethod>& methods, const TkStringView& methodName)
        {
            const RkMethod* result = nullptr;
            for (const RkMethod& method : methods)
            {
                if (method.name != methodName)
                    continue;
                if (result != nullptr)
                    return nullptr;
                result = &method;
            }
            return result;
        }

        i32 countMethodsByNameInSpan(const TkSpan<const RkMethod>& methods, const TkStringView& methodName)
        {
            i32 count = 0;
            for (const RkMethod& method : methods)
            {
                if (method.name == methodName)
                    ++count;
            }
            return count;
        }

        const RkMethod* findMethodBySignatureInSpan(const TkSpan<const RkMethod>& methods, const TkStringView& methodName, const u64 signatureHash)
        {
            for (const RkMethod& method : methods)
            {
                if (method.name == methodName && method.signatureHash == signatureHash)
                    return &method;
            }
            return nullptr;
        }

        const RkMethod* findMethodByParamsInSpan(const TkSpan<const RkMethod>& methods, const TkStringView& methodName, const TkSpan<const TypeHandle> paramTypes)
        {
            const RkMethod* result = nullptr;
            for (const RkMethod& method : methods)
            {
                if (method.name != methodName || method.paramCount != static_cast<u32>(paramTypes.length()))
                    continue;

                bool match = true;
                for (u32 i = 0; i < method.paramCount; ++i)
                {
                    if (method.params[static_cast<i32>(i)].type != paramTypes[static_cast<i32>(i)])
                    {
                        match = false;
                        break;
                    }
                }

                if (!match)
                    continue;
                if (result != nullptr)
                    return nullptr;
                result = &method;
            }
            return result;
        }

        i32 countMethodByParamsInSpan(const TkSpan<const RkMethod>& methods, const TkStringView& methodName, const TkSpan<const TypeHandle> paramTypes)
        {
            i32 count = 0;
            for (const RkMethod& method : methods)
            {
                if (method.name != methodName || method.paramCount != static_cast<u32>(paramTypes.length()))
                    continue;

                bool match = true;
                for (u32 i = 0; i < method.paramCount; ++i)
                {
                    if (method.params[static_cast<i32>(i)].type != paramTypes[static_cast<i32>(i)])
                    {
                        match = false;
                        break;
                    }
                }
                if (match)
                    ++count;
            }
            return count;
        }

        const RkMethod* findMethodByResolvedParamsInSpan(const TkSpan<const RkMethod>& methods, const TkStringView& methodName, const TkSpan<const RkParam> params)
        {
            const RkMethod* result = nullptr;
            for (const RkMethod& method : methods)
            {
                if (method.name != methodName || method.paramCount != static_cast<u32>(params.length()))
                    continue;

                bool match = true;
                for (u32 i = 0; i < method.paramCount; ++i)
                {
                    const RkParam& lhs = method.params[static_cast<i32>(i)];
                    const RkParam& rhs = params[static_cast<i32>(i)];
                    if (lhs.type != rhs.type || lhs.passing != rhs.passing)
                    {
                        match = false;
                        break;
                    }
                }

                if (!match)
                    continue;
                if (result != nullptr)
                    return nullptr;
                result = &method;
            }
            return result;
        }

        i32 countMethodByResolvedParamsInSpan(const TkSpan<const RkMethod>& methods, const TkStringView& methodName, const TkSpan<const RkParam> params)
        {
            i32 count = 0;
            for (const RkMethod& method : methods)
            {
                if (method.name != methodName || method.paramCount != static_cast<u32>(params.length()))
                    continue;

                bool match = true;
                for (u32 i = 0; i < method.paramCount; ++i)
                {
                    const RkParam& lhs = method.params[static_cast<i32>(i)];
                    const RkParam& rhs = params[static_cast<i32>(i)];
                    if (lhs.type != rhs.type || lhs.passing != rhs.passing)
                    {
                        match = false;
                        break;
                    }
                }
                if (match)
                    ++count;
            }
            return count;
        }

        void hashAppendByte(u64& hash, const char value)
        {
            hash ^= static_cast<u8>(value);
            hash *= 1099511628211ull;
        }

        template <class TEnum>
        void hashAppendEnum(u64& hash, const TEnum value)
        {
            hashAppendByte(hash, static_cast<char>(value));
        }

        void hashAppendView(u64& hash, const TkStringView& value)
        {
            for (i32 i = 0; i < value.length(); ++i)
                hashAppendByte(hash, value[i]);
        }
    }

    u64 makeMethodSignatureHash(
        const TkStringView& ownerFullName,
        const TkStringView& methodName,
        const TypeHandle returnType,
        const ReturnKind returnKind,
        const TkSpan<const RkParam> params)
    {
        u64 hash = 1469598103934665603ull;
        hashAppendView(hash, ownerFullName);
        hashAppendByte(hash, ':');
        hashAppendByte(hash, ':');
        hashAppendView(hash, methodName);
        hashAppendByte(hash, '(');
        for (i32 i = 0; i < params.length(); ++i)
        {
            if (i != 0)
                hashAppendByte(hash, ',');
            const RkParam& param = params[i];
            const TypeHandle type = param.type;
            hashAppendView(hash, type.isValid() ? type.getType().fullName : TkStringView("void"));
            hashAppendByte(hash, ':');
            hashAppendEnum(hash, param.passing);
        }
        hashAppendByte(hash, ')');
        hashAppendByte(hash, ':');
        hashAppendView(hash, returnType.isValid() ? returnType.getType().fullName : TkStringView("void"));
        hashAppendByte(hash, ':');
        hashAppendEnum(hash, returnKind);
        return hash != 0 ? hash : 1;
    }

    TypeHandle::TypeHandle(RkModule* inModule, const i32 inTypeIndex)
        : module(inModule)
        , moduleId(inModule != nullptr ? inModule->getId() : 0)
        , moduleGeneration(inModule != nullptr ? inModule->getGeneration() : 0)
        , typeIndex(inTypeIndex)
    {
        const RkType* type = inModule != nullptr ? inModule->tryGetType(inTypeIndex) : nullptr;
        typeId = type != nullptr ? inModule->makeTypeId(type->fullName) : 0;
    }

    TypeHandle::TypeHandle(const TypeInitializer& initializer)
        : module(initializer.getHandle().module)
        , moduleId(initializer.getHandle().moduleId)
        , moduleGeneration(initializer.getHandle().moduleGeneration)
        , typeId(initializer.getHandle().typeId)
        , typeIndex(initializer.getHandle().typeIndex)
    {
    }

    bool TypeHandle::isValid() const
    {
        return module != nullptr && typeIndex >= 0 && isAlive();
    }

    bool TypeHandle::isAlive() const
    {
        return module != nullptr && module->isHandleAlive(*this);
    }

    const RkType& TypeHandle::getType() const
    {
        TK_ASSERT(isValid());
        return module->getType(typeIndex);
    }

    TkString TypeHandle::toString(const bool withModuleName) const
    {
        if (!isValid())
            return "InvalidType";

        const RkType& type = getType();
        if (!withModuleName)
            return type.fullName.toString();

        return TkString::format("{}:{}", module->getName(), type.fullName);
    }

    bool TypeHandle::isSubclassOf(const TypeHandle other) const
    {
        if (!isValid() || !other.isValid() || *this == other)
            return false;

        const RkClass* klass = getType().asClass();
        while (klass != nullptr && klass->baseType.isValid())
        {
            if (klass->baseType == other)
                return true;
            klass = klass->baseType.getType().asClass();
        }
        return false;
    }

    bool TypeHandle::isAssignableFrom(const TypeHandle other) const
    {
        if (*this == other)
            return true;
        if (!isValid() || !other.isValid())
            return false;

        const RkType& target = getType();
        const RkType& source = other.getType();

        if (target.kind == TypeKind::kInterface)
        {
            const RkClass* sourceClass = source.asClass();
            return sourceClass != nullptr && sourceClass->implements(*this);
        }

        if (target.kind != TypeKind::kClass)
            return false;

        const RkClass* sourceClass = source.asClass();
        while (sourceClass != nullptr)
        {
            if (sourceClass->handle == *this)
                return true;
            sourceClass = sourceClass->baseType.isValid() ? sourceClass->baseType.getType().asClass() : nullptr;
        }
        return false;
    }

    const RkField* RkClass::findField(const TkStringView& fieldName) const
    {
        if (const RkField* field = findFieldInSpan(fields, fieldName))
            return field;
        return baseType.isValid() ? baseType.getType().findField(fieldName) : nullptr;
    }

    const RkMethod* RkClass::findMethod(const TkStringView& methodName) const
    {
        if (countMethodsByNameInSpan(methods, methodName) > 1)
            return nullptr;
        if (const RkMethod* method = findUniqueMethodInSpan(methods, methodName))
            return method;
        return baseType.isValid() ? baseType.getType().findMethod(methodName) : nullptr;
    }

    const RkMethod* RkClass::findMethod(const TkStringView& methodName, const u64 signatureHash) const
    {
        if (const RkMethod* method = findMethodBySignatureInSpan(methods, methodName, signatureHash))
            return method;
        return baseType.isValid() ? baseType.getType().findMethod(methodName, signatureHash) : nullptr;
    }

    const RkMethod* RkClass::findMethod(const TkStringView& methodName, const TkSpan<const TypeHandle> paramTypes) const
    {
        if (countMethodByParamsInSpan(methods, methodName, paramTypes) > 1)
            return nullptr;
        if (const RkMethod* method = findMethodByParamsInSpan(methods, methodName, paramTypes))
            return method;
        return baseType.isValid() ? baseType.getType().findMethod(methodName, paramTypes) : nullptr;
    }

    const RkMethod* RkClass::findMethod(const TkStringView& methodName, const TkSpan<const RkParam> params) const
    {
        if (countMethodByResolvedParamsInSpan(methods, methodName, params) > 1)
            return nullptr;
        if (const RkMethod* method = findMethodByResolvedParamsInSpan(methods, methodName, params))
            return method;
        return baseType.isValid() ? baseType.getType().findMethod(methodName, params) : nullptr;
    }

    bool RkClass::implements(const TypeHandle interfaceType) const
    {
        for (const TypeHandle& value : interfaces)
        {
            if (value == interfaceType)
                return true;
        }
        return baseType.isValid() && baseType.getType().asClass() != nullptr
            ? baseType.getType().asClass()->implements(interfaceType)
            : false;
    }

    const RkField* RkStruct::findField(const TkStringView& fieldName) const
    {
        return findFieldInSpan(fields, fieldName);
    }

    const RkMethod* RkStruct::findMethod(const TkStringView& methodName) const
    {
        return findUniqueMethodInSpan(methods, methodName);
    }

    const RkMethod* RkStruct::findMethod(const TkStringView& methodName, const u64 signatureHash) const
    {
        return findMethodBySignatureInSpan(methods, methodName, signatureHash);
    }

    const RkMethod* RkStruct::findMethod(const TkStringView& methodName, const TkSpan<const TypeHandle> paramTypes) const
    {
        return findMethodByParamsInSpan(methods, methodName, paramTypes);
    }

    const RkMethod* RkStruct::findMethod(const TkStringView& methodName, const TkSpan<const RkParam> params) const
    {
        return findMethodByResolvedParamsInSpan(methods, methodName, params);
    }

    const RkMethod* RkInterface::findMethod(const TkStringView& methodName) const
    {
        return findUniqueMethodInSpan(methods, methodName);
    }

    const RkMethod* RkInterface::findMethod(const TkStringView& methodName, const u64 signatureHash) const
    {
        return findMethodBySignatureInSpan(methods, methodName, signatureHash);
    }

    const RkMethod* RkInterface::findMethod(const TkStringView& methodName, const TkSpan<const TypeHandle> paramTypes) const
    {
        return findMethodByParamsInSpan(methods, methodName, paramTypes);
    }

    const RkMethod* RkInterface::findMethod(const TkStringView& methodName, const TkSpan<const RkParam> params) const
    {
        return findMethodByResolvedParamsInSpan(methods, methodName, params);
    }

    TypeInitializer::TypeInitializer(RkModule& module, const RkClassDesc& desc)
        : m_handle(module.registerType(desc))
    {
    }

    TypeInitializer::TypeInitializer(RkModule& module, const RkStructDesc& desc)
        : m_handle(module.registerType(desc))
    {
    }

    TypeInitializer::TypeInitializer(RkModule& module, const RkEnumDesc& desc)
        : m_handle(module.registerType(desc))
    {
    }

    TypeInitializer::TypeInitializer(RkModule& module, const RkInterfaceDesc& desc)
        : m_handle(module.registerType(desc))
    {
    }

    const RkType& TypeInitializer::getType() const
    {
        return m_handle.getType();
    }

    RkModule::RkModule(const TkStringView& moduleName)
        : m_name(moduleName)
        , m_id(hashModuleName(moduleName))
    {
        getModuleStorage().add(this);
        getModuleEvents()(RkModuleEvent{ this, RkModuleEventKind::kLoaded });
    }

    RkModule::~RkModule()
    {
        unload();
        getModuleStorage().remove(this);
    }

    void RkModule::clearTypes()
    {
        for (RkType* type : m_types)
            TkMemory::deletePtr(RkManager::getAllocator(), type);
        m_types.clear();
        m_typeIndexByName.clear();
    }

    TypeHandle RkModule::registerType(const RkClassDesc& desc)
    {
        RkClassDesc local = desc;
        if (local.name.isEmpty())
            local.name = makeLeafName(local.fullName);
        return registerTypeInternal(TkMemory::newObject<RkClass>(RkManager::getAllocator(), local));
    }

    TypeHandle RkModule::registerType(const RkStructDesc& desc)
    {
        RkStructDesc local = desc;
        if (local.name.isEmpty())
            local.name = makeLeafName(local.fullName);
        return registerTypeInternal(TkMemory::newObject<RkStruct>(RkManager::getAllocator(), local));
    }

    TypeHandle RkModule::registerType(const RkEnumDesc& desc)
    {
        RkEnumDesc local = desc;
        if (local.name.isEmpty())
            local.name = makeLeafName(local.fullName);
        return registerTypeInternal(TkMemory::newObject<RkEnum>(RkManager::getAllocator(), local));
    }

    TypeHandle RkModule::registerType(const RkInterfaceDesc& desc)
    {
        RkInterfaceDesc local = desc;
        if (local.name.isEmpty())
            local.name = makeLeafName(local.fullName);
        return registerTypeInternal(TkMemory::newObject<RkInterface>(RkManager::getAllocator(), local));
    }

    TypeHandle RkModule::findType(const TkStringView& fullName) const
    {
        const auto iterator = m_typeIndexByName.find(fullName.toString());
        if (iterator.isEnd())
            return TypeHandle();
        const i32 index = iterator->value;
        const u64 typeId = m_types.isValidIndex(index) ? makeTypeId(m_types[index]->fullName) : 0;
        return TypeHandle(const_cast<RkModule*>(this), index, m_id, m_generation, typeId);
    }

    const RkType* RkModule::findTypePtr(const TkStringView& fullName) const
    {
        const TypeHandle handle = findType(fullName);
        return handle.isValid() ? &handle.getType() : nullptr;
    }

    const RkType* RkModule::tryGetType(const i32 index) const
    {
        return m_types.isValidIndex(index) ? m_types[index] : nullptr;
    }

    const RkType& RkModule::getType(const i32 index) const
    {
        TK_ASSERT(m_types.isValidIndex(index));
        return *m_types[index];
    }

    bool RkModule::isHandleAlive(const TypeHandle& handle) const
    {
        if (!m_loaded || handle.module != this || handle.moduleId != m_id || handle.moduleGeneration != m_generation)
            return false;
        if (!m_types.isValidIndex(handle.typeIndex))
            return false;
        return handle.typeId == 0 || handle.typeId == makeTypeId(m_types[handle.typeIndex]->fullName);
    }

    u64 RkModule::makeTypeId(const TkStringView& fullName) const
    {
        u64 hash = m_id;
        hashAppendByte(hash, ':');
        hashAppendView(hash, fullName);
        return hash != 0 ? hash : 1;
    }

    const RkField* RkModule::findField(const TypeHandle owner, const TkStringView& fieldName) const
    {
        if (!owner.isValid())
            return nullptr;
        return owner.getType().findField(fieldName);
    }

    const RkMethod* RkModule::findMethod(const TypeHandle owner, const TkStringView& methodName) const
    {
        if (!owner.isValid())
            return nullptr;
        return owner.getType().findMethod(methodName);
    }

    const RkMethod* RkModule::findMethod(const TypeHandle owner, const TkStringView& methodName, const u64 signatureHash) const
    {
        if (!owner.isValid())
            return nullptr;
        return owner.getType().findMethod(methodName, signatureHash);
    }

    const RkMethod* RkModule::findMethod(const TypeHandle owner, const TkStringView& methodName, const TkSpan<const TypeHandle> paramTypes) const
    {
        if (!owner.isValid())
            return nullptr;
        return owner.getType().findMethod(methodName, paramTypes);
    }

    const RkMethod* RkModule::findMethod(const TypeHandle owner, const TkStringView& methodName, const TkSpan<const RkParam> params) const
    {
        if (!owner.isValid())
            return nullptr;
        return owner.getType().findMethod(methodName, params);
    }

    const TkArray<RkModule*>& RkModule::getModules()
    {
        return getModuleStorage();
    }

    RkModule* RkModule::findModule(const TkStringView& moduleName)
    {
        for (RkModule* module : getModuleStorage())
        {
            if (module->getName() == moduleName)
                return module;
        }
        return nullptr;
    }

    RkModule* RkModule::findModuleById(const u64 moduleId)
    {
        for (RkModule* module : getModuleStorage())
        {
            if (module->getId() == moduleId)
                return module;
        }
        return nullptr;
    }

    TkAction<const RkModuleEvent&>& RkModule::events()
    {
        return getModuleEvents();
    }

    bool RkModule::load(const TkStringView& libraryPath)
    {
        if (m_libraryHandle != nullptr)
            unload();

        m_libraryPath = libraryPath;
        m_libraryHandle = System::loadLib(m_libraryPath.getText());
        if (m_libraryHandle == nullptr)
        {
            m_loaded = false;
            getModuleEvents()(RkModuleEvent{ this, RkModuleEventKind::kLoadFailed });
            return false;
        }

        m_loaded = true;
        ++m_generation;
        TypeRegistry::markDirty();
        getModuleEvents()(RkModuleEvent{ this, RkModuleEventKind::kLoaded });
        return true;
    }

    void RkModule::attachLibrary(void* handle, const TkStringView& libraryPath)
    {
        if (m_libraryHandle != nullptr && m_libraryHandle != handle)
            System::freeLib(m_libraryHandle);

        m_libraryHandle = handle;
        m_libraryPath = libraryPath;
        m_loaded = handle != nullptr;
        ++m_generation;
        TypeRegistry::markDirty();
        if (m_loaded)
            getModuleEvents()(RkModuleEvent{ this, RkModuleEventKind::kLoaded });
    }

    void RkModule::addDependency(RkModule* module)
    {
        if (module == nullptr || module == this || m_dependencies.find(module) >= 0)
            return;
        m_dependencies.add(module);
    }

    bool RkModule::dependsOn(const RkModule* module) const
    {
        if (module == nullptr)
            return false;
        for (RkModule* dependency : m_dependencies)
        {
            if (dependency == module)
                return true;
            if (dependency != nullptr && dependency->dependsOn(module))
                return true;
        }
        return false;
    }

    void RkModule::unload()
    {
        if (!m_loaded)
            return;

        getModuleEvents()(RkModuleEvent{ this, RkModuleEventKind::kBeforeUnload });
        TypeRegistry::destroyObjectsFromModule(this);
        RkEvents::unregisterBindingsForModule(this);
        clearTypes();
        if (m_libraryHandle != nullptr)
        {
            System::freeLib(m_libraryHandle);
            m_libraryHandle = nullptr;
        }
        m_dependencies.clear();
        m_loaded = false;
        ++m_generation;
        TypeRegistry::markDirty();
        getModuleEvents()(RkModuleEvent{ this, RkModuleEventKind::kUnloaded });
    }

    void RkModule::reload()
    {
        getModuleEvents()(RkModuleEvent{ this, RkModuleEventKind::kBeforeReload });
        const TkString libraryPath = m_libraryPath;
        if (libraryPath.hasChars())
        {
            unload();
            if (!load(libraryPath))
                return;
        }
        else if (m_loaded)
            unload();
        m_loaded = true;
        ++m_generation;
        TypeRegistry::markDirty();
        getModuleEvents()(RkModuleEvent{ this, RkModuleEventKind::kReloaded });
    }

    TypeHandle RkModule::registerTypeInternal(RkType* type)
    {
        TK_ASSERT(type != nullptr);
        TK_ASSERT(type->fullName.hasChars());
        TK_ASSERT(!m_typeIndexByName.containsKey(type->fullName.toString()));

        const i32 index = m_types.count();
        m_loaded = true;
        type->handle = TypeHandle(this, index, m_id, m_generation, makeTypeId(type->fullName));
        m_types.add(type);
        m_typeIndexByName.add(type->fullName.toString(), index);
        TypeRegistry::markDirty();
        return type->handle;
    }

    RkModule& getReflectCoreModule()
    {
        static RkModule module("ReflectKit.Core");
        return module;
    }
}
