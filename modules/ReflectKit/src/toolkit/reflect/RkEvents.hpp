/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/reflect/invoking/ArgValue.hpp>
#include <toolkit/reflect/RkObject.hpp>
#include <toolkit/base/Function.hpp>
#include <toolkit/collection/HashMap.hpp>
#include <toolkit/types/Pair.hpp>

namespace toolkit
{
    using EventBindingFn = void (*)(RkObject* instance, void* listener, bool bind);
    using EventArgSpan = TkSpan<ArgValue>;
    using EventKey = TkPair<TypeHandle, TkString>;

    struct RkEventDesc
    {
        TypeHandle ownerType;
        TkStringView name;
        TkString nameStorage;
        EventBindingFn binding = nullptr;
        TkSpan<const RkParam> params;
        u32 paramCount = 0;
        u64 signatureHash = 0;
        AttributeSpan attributes;
    };

    class RkEvents
    {
    public:
        static void registerBinding(TypeHandle type, const TkStringView& eventName, EventBindingFn binding)
        {
            RkEventDesc desc;
            desc.ownerType = type;
            desc.name = eventName;
            desc.binding = binding;
            registerEvent(desc);
        }

        static void registerEvent(const RkEventDesc& desc)
        {
            RkEventDesc local = desc;
            local.nameStorage = local.name.toString();
            local.name = TkStringView(local.nameStorage);
            local.paramCount = static_cast<u32>(local.params.length());
            if (local.signatureHash == 0)
                local.signatureHash = makeMethodSignatureHash(local.ownerType.isValid() ? local.ownerType.getType().fullName : TkStringView(), local.name, TypeHandle(), ReturnKind::kVoid, local.params);

            const EventKey key(local.ownerType, local.nameStorage);
            bindings()[key] = local.binding;
            descriptors()[key] = local;
        }

        static void unregisterBinding(TypeHandle type, const TkStringView& eventName)
        {
            const EventKey key(type, eventName.toString());
            bindings().remove(key);
            descriptors().remove(key);
        }

        static void unregisterBindingsForModule(const RkModule* module)
        {
            TkArray<EventKey> keys;
            for (auto iterator = descriptors().Begin(); iterator.isNotEnd(); ++iterator)
            {
                if (iterator->key.first.module == module)
                    keys.add(iterator->key);
            }
            for (const EventKey& key : keys)
            {
                bindings().remove(key);
                descriptors().remove(key);
            }
        }

        static EventBindingFn findBinding(TypeHandle type, const TkStringView& eventName)
        {
            const auto iterator = bindings().find(EventKey(type, eventName.toString()));
            return iterator.isEnd() ? nullptr : iterator->value;
        }

        static const RkEventDesc* findEvent(TypeHandle type, const TkStringView& eventName)
        {
            const auto iterator = descriptors().find(EventKey(type, eventName.toString()));
            if (iterator.isEnd())
                return nullptr;
            iterator->value.name = TkStringView(iterator->key.second);
            return &iterator->value;
        }

        static TkArray<RkEventDesc> getEvents(TypeHandle type)
        {
            TkArray<RkEventDesc> result;
            for (auto iterator = descriptors().Begin(); iterator.isNotEnd(); ++iterator)
            {
                if (iterator->key.first == type)
                {
                    RkEventDesc desc = iterator->value;
                    desc.name = TkStringView(iterator->key.second);
                    result.add(desc);
                }
            }
            return result;
        }

        static bool validateSignature(TypeHandle type, const TkStringView& eventName, TkSpan<const RkParam> params)
        {
            const RkEventDesc* desc = findEvent(type, eventName);
            if (desc == nullptr || desc->paramCount != static_cast<u32>(params.length()))
                return false;
            for (u32 i = 0; i < desc->paramCount; ++i)
            {
                const RkParam& lhs = desc->params[static_cast<i32>(i)];
                const RkParam& rhs = params[static_cast<i32>(i)];
                if (lhs.type != rhs.type || lhs.passing != rhs.passing)
                    return false;
            }
            return true;
        }

        static TkAction<RkObject*, EventArgSpan, TypeHandle, TkStringView>& event()
        {
            static TkAction<RkObject*, EventArgSpan, TypeHandle, TkStringView> dispatcher;
            return dispatcher;
        }

    private:
        static TkHashMap<EventKey, EventBindingFn>& bindings()
        {
            static TkHashMap<EventKey, EventBindingFn> value;
            return value;
        }

        static TkHashMap<EventKey, RkEventDesc>& descriptors()
        {
            static TkHashMap<EventKey, RkEventDesc> value;
            return value;
        }
    };
}
