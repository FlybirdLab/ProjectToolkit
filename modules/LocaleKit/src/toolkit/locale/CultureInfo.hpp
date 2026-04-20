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
    // The information about a specific culture (aka locale). The information includes the names for the culture, the writing system, the calendar used, the sort order of strings, and formatting for dates and numbers.
    class TK_API CultureInfo
    {
    private:
        void *m_data;
        i32 m_lcid;
        i32 m_lcidParent;
        TkString m_name;
        TkString m_nativeName;
        TkString m_englishName;

    public:
        CultureInfo(i32 lcid);
        CultureInfo(const TkStringView &localeName);

    public:
        // Gets the culture identifier.
        i32 getLCID() const;
        // Gets the parent culture identifier.
        i32 getParentLCID() const;
        // Gets the culture name (eg. pl-PL).
        const TkString &getName() const;
        // Gets the full localized culture name (eg. Simplified Chinese (PRC)).
        const TkString &getNativeName() const;
        // Gets the culture name in English (eg. Chinese (Simplified) (PRC)).
        const TkString &getEnglishName() const;

        TkString toString() const;
        bool operator==(const CultureInfo &other) const;
    };

}

FORMAT_CUSTOM(CultureInfo, "%s", t.toString().getText());
