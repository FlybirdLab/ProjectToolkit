/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/locale/common.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/base/Format.hpp>

namespace toolkit
{
    // The string container that supports using localized text.
    class TK_API I18nString
    {
    public:
        I18nString() = default;
        I18nString(const I18nString &other);
        I18nString(I18nString &&other) noexcept;
        I18nString(const TkStringView &value);
        I18nString(TkString &&value) noexcept;

        I18nString &operator=(const I18nString &other);
        I18nString &operator=(I18nString &&other) noexcept;
        I18nString &operator=(const TkStringView &value);
        I18nString &operator=(TkString &&value) noexcept;

        bool operator==(const I18nString &other) const
        {
            return id == other.id && value == other.value;
        }

        friend bool operator!=(const I18nString &a, const I18nString &b)
        {
            return !(a == b);
        }

        friend bool operator==(const I18nString &a, const TkStringView &b)
        {
            return a.value == b || a.toString() == b;
        }

        friend bool operator!=(const I18nString &a, const TkStringView &b)
        {
            return !(a == b);
        }

    public:
        TkString toString() const;
        TkString toStringPlural(i32 n) const;

    public:
        // The localized string identifier. Used to lookup text value for a current language (via <see cref="Localization::GetString"/>).
        TkString id;
        // The overriden string value to use. If empty, the localized string will be used.
        TkString value;
    };

    template <>
    struct HashFunc<I18nString>
    {
        static u32 get(const I18nString &k)
        {
            return HashFunc<TkString>::get(k.toString());
        }
    };
}

FORMAT_CUSTOM(toolkit::I18nString, "%s", t.toString().getText());
