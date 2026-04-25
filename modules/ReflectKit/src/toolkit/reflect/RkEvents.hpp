/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/reflect/RkObject.hpp>
#include <toolkit/base/Function.hpp>
#include <toolkit/collection/HashMap.hpp>
#include <toolkit/types/Pair.hpp>

namespace toolkit
{
    using EventBindingFn = void (*)(RkObject* instance, void* listener, bool bind);
    using EventKey = TkPair<TypeHandle, TkString>;

    class RkEvents
    {
    public:
        static void registerBinding(TypeHandle type, const TkStringView& eventName, EventBindingFn binding)
        {
            bindings()[EventKey(type, eventName.toString())] = binding;
        }

        static void unregisterBinding(TypeHandle type, const TkStringView& eventName)
        {
            bindings().remove(EventKey(type, eventName.toString()));
        }

        static EventBindingFn findBinding(TypeHandle type, const TkStringView& eventName)
        {
            const auto iterator = bindings().find(EventKey(type, eventName.toString()));
            return iterator.isEnd() ? nullptr : iterator->Value;
        }

        static TkAction<RkObject*, ArgSpan, TypeHandle, TkStringView>& event()
        {
            static TkAction<RkObject*, ArgSpan, TypeHandle, TkStringView> dispatcher;
            return dispatcher;
        }

    private:
        static TkHashMap<EventKey, EventBindingFn>& bindings()
        {
            static TkHashMap<EventKey, EventBindingFn> value;
            return value;
        }
    };
}
