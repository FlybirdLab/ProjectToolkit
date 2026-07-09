/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "CultureInfo.hpp"

#include "CultureTable.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/types/StringView.hpp"

namespace toolkit
{

    const CultureInfoEntry* findEntry(const TkStringView &name)
    {
        for (i32 i = 0; i < NUM_CULTURE_ENTRIES; i++)
        {
            const CultureInfoEntry &e = culture_entries[i];
            if (name == idx2string(e.name))
                return &e;
        }
        return nullptr;
    }

    CultureInfo::CultureInfo(i32 lcid)
    {
        m_lcid = lcid;
        m_lcidParent = 0;
        m_data = nullptr;
        if (lcid == 0)
            return;
        if (lcid == 127)
        {
            m_englishName = "Invariant Culture";
            return;
        }
        for (i32 i = 0; i < NUM_CULTURE_ENTRIES; i++)
        {
            auto &e = culture_entries[i];
            if (e.lcid == lcid)
            {
                m_data = (void *)&e;
                m_lcidParent = (i32)e.parent_lcid;
                const char *name = idx2string(e.name);
                m_name.set(name, TkStringUtils::length(name));
                const char *nativename = idx2string(e.nativename);
                m_nativeName.set(nativename, TkStringUtils::length(nativename));
                const char *englishname = idx2string(e.englishname);
                m_englishName.set(englishname, TkStringUtils::length(englishname));
                break;
            }
        }
        if (!m_data)
        {
            m_lcid = 127;
            m_lcidParent = 0;
            m_englishName = "Invariant Culture";
            LogError("Unknown LCID %d for CultureInfo", lcid);
        }
    }

    CultureInfo::CultureInfo(const TkStringView &name)
    {
        m_data = nullptr;
        if (name.isEmpty())
        {
            m_lcid = 127;
            m_lcidParent = 0;
            m_englishName = "Invariant Culture";
            return;
        }
        const CultureInfoEntry *e = findEntry(name);
        if (!e && name.find('-') != -1)
        {
            e = findEntry(name.substring(0, name.find('-')));
        }
        if (e)
        {
            m_data = (void *)e;
            m_lcid = (i32)e->lcid;
            m_lcidParent = (i32)e->parent_lcid;
            const char *ename = idx2string(e->name);
            m_name.set(ename, TkStringUtils::length(ename));
            const char *nativename = idx2string(e->nativename);
            m_nativeName.set(nativename, TkStringUtils::length(nativename));
            const char *englishname = idx2string(e->englishname);
            m_englishName.set(englishname, TkStringUtils::length(englishname));
        }
        else
        {
            m_lcid = 127;
            m_lcidParent = 0;
            m_englishName = "Invariant Culture";
            LogError("Unknown name %s for CultureInfo", name);
        }
    }

    i32 CultureInfo::getLCID() const
    {
        return m_lcid;
    }

    i32 CultureInfo::getParentLCID() const
    {
        return m_lcidParent;
    }

    const TkString &CultureInfo::getName() const
    {
        return m_name;
    }

    const TkString &CultureInfo::getNativeName() const
    {
        return m_nativeName;
    }

    const TkString &CultureInfo::getEnglishName() const
    {
        return m_englishName;
    }

    TkString CultureInfo::toString() const
    {
        return m_name;
    }

    bool CultureInfo::operator==(const CultureInfo &other) const
    {
        return m_lcid == other.m_lcid;
    }

}