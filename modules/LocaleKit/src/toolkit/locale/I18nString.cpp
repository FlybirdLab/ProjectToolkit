/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "I18nString.hpp"

#include "I18nManager.hpp"

namespace toolkit
{

    I18nString::I18nString(const I18nString &other)
        : id(other.id), value(other.value)
    {
    }

    I18nString::I18nString(I18nString &&other) noexcept
        : id(moveTemp(other.id)), value(moveTemp(other.value))
    {
    }

    I18nString::I18nString(const TkStringView &value)
        : value(value)
    {
    }

    I18nString::I18nString(TkString &&value) noexcept
        : value(moveTemp(value))
    {
    }

    I18nString &I18nString::operator=(const I18nString &other)
    {
        if (this != &other)
        {
            id = other.id;
            value = other.value;
        }
        return *this;
    }

    I18nString &I18nString::operator=(I18nString &&other) noexcept
    {
        if (this != &other)
        {
            id = moveTemp(other.id);
            value = moveTemp(other.value);
        }
        return *this;
    }

    I18nString &I18nString::operator=(const TkStringView &value)
    {
        id.clear();
        this->value = value;
        return *this;
    }

    I18nString &I18nString::operator=(TkString &&value) noexcept
    {
        id.clear();
        this->value = moveTemp(value);
        return *this;
    }

    TkString I18nString::toString() const
    {
        return I18nManager::getString(id, value);
    }

    TkString I18nString::toStringPlural(i32 n) const
    {
        return I18nManager::getPluralString(id, n, value);
    }

}