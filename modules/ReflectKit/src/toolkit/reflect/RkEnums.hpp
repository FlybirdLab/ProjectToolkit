/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/reflect/RkType.hpp>

namespace toolkit
{
    class RkEnums
    {
    public:
        template <class EnumType>
        static TkSpan<const EnumItem> getItems()
        {
            const TypeHandle typeHandle = typeOf<EnumType>();
            if (!typeHandle.isValid())
                return {};

            const RkEnum* enumType = typeHandle.getType().asEnum();
            return enumType != nullptr ? enumType->items : TkSpan<const EnumItem>();
        }

        template <class EnumType>
        static const char* getName(const EnumType value)
        {
            for (const EnumItem& item : getItems<EnumType>())
            {
                if (item.value == static_cast<i64>(value))
                    return item.name.getText();
            }
            return nullptr;
        }

        template <class EnumType>
        static TkString toString(const EnumType value)
        {
            const char* name = getName(value);
            return name != nullptr ? TkString(name) : TkString::Empty;
        }

        template <class EnumType>
        static EnumType fromString(const TkStringView& name)
        {
            for (const EnumItem& item : getItems<EnumType>())
            {
                if (item.name == name)
                    return static_cast<EnumType>(item.value);
            }
            return static_cast<EnumType>(0);
        }

        template <class EnumType>
        static TkString toStringFlags(const EnumType value, const char separator = '|')
        {
            TkString result;
            const i64 mask = static_cast<i64>(value);
            for (const EnumItem& item : getItems<EnumType>())
            {
                if (mask == 0 && item.value == 0)
                    return item.name.toString();

                if (item.value != 0 && (mask & item.value) == item.value)
                {
                    if (result.hasChars())
                        result += separator;
                    result += item.name;
                }
            }
            return result;
        }
    };
}
