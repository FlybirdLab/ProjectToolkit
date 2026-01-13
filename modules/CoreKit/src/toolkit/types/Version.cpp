/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Version.hpp"

#include "toolkit/base/Math.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/types/StringView.hpp"
#include "toolkit/types/StringBuilder.hpp"

namespace toolkit
{

    TkVersion TkVersion::Empty{0, 0, 0, 0};

    TkVersion::TkVersion(i32 major, i32 minor, i32 build, i32 revision)
    {
        m_major = TkMath::max(major, 0);
        m_minor = TkMath::max(minor, 0);
        m_build = TkMath::max(build, -1);
        m_revision = TkMath::max(revision, -1);
    }

    TkVersion::TkVersion(i32 major, i32 minor, i32 build)
    {
        m_major = TkMath::max(major, 0);
        m_minor = TkMath::max(minor, 0);
        m_build = TkMath::max(build, -1);
        m_revision = -1;
    }

    TkVersion::TkVersion(i32 major, i32 minor)
    {
        m_major = TkMath::max(major, 0);
        m_minor = TkMath::max(minor, 0);
        m_build = -1;
        m_revision = -1;
    }

    i32 TkVersion::compareTo(const TkVersion &value) const
    {
        if (m_major != value.m_major)
        {
            if (m_major > value.m_major)
            {
                return 1;
            }
            return -1;
        }
        if (m_minor != value.m_minor)
        {
            if (m_minor > value.m_minor)
            {
                return 1;
            }
            return -1;
        }
        if (m_build != value.m_build)
        {
            if (m_build > value.m_build)
            {
                return 1;
            }
            return -1;
        }
        if (m_revision == value.m_revision)
        {
            return 0;
        }
        if (m_revision > value.m_revision)
        {
            return 1;
        }
        return -1;
    }

    TkString TkVersion::toString(i32 fieldCount) const
    {
        TkStringBuilder stringBuilder(32);
        switch (fieldCount)
        {
        case 0:
            break;
        case 1:
            stringBuilder.append(TkMath::max(m_major, 0));
            break;
        case 2:
            stringBuilder.append(TkMath::max(m_major, 0));
            stringBuilder.append('.');
            stringBuilder.append(TkMath::max(m_minor, 0));
            break;
        case 3:
            stringBuilder.append(TkMath::max(m_major, 0));
            stringBuilder.append('.');
            stringBuilder.append(TkMath::max(m_minor, 0));
            stringBuilder.append('.');
            stringBuilder.append(TkMath::max(m_build, 0));
            break;
        default:
            stringBuilder.append(TkMath::max(m_major, 0));
            stringBuilder.append('.');
            stringBuilder.append(TkMath::max(m_minor, 0));
            stringBuilder.append('.');
            stringBuilder.append(TkMath::max(m_build, 0));
            stringBuilder.append('.');
            stringBuilder.append(TkMath::max(m_revision, 0));
            break;
        }
        return stringBuilder.toString();
    }

    TkString TkVersion::toString() const
    {
        if (m_build == -1)
        {
            return toString(2);
        }
        if (m_revision == -1)
        {
            return toString(3);
        }
        return toString(4);
    }

    bool TkVersion::parse(const TkString &text, TkVersion &value, IAllocator& alloc)
    {
        value = Empty;

        i32 major, minor, build, revision;

        TkArray<TkString> parsedComponents(4, alloc);
        text.split('.', parsedComponents);

        i32 parsedComponentsLength = parsedComponents.count();
        if (parsedComponentsLength < 2 || parsedComponentsLength > 4)
        {
            return false;
        }

        if (!TkStringUtils::fromString(parsedComponents[0].get(), major))
        {
            return false;
        }

        if (!TkStringUtils::fromString(parsedComponents[1].get(), minor))
        {
            return false;
        }

        parsedComponentsLength -= 2;

        if (parsedComponentsLength > 0)
        {
            if (!TkStringUtils::fromString(parsedComponents[2].get(), build))
            {
                return false;
            }

            parsedComponentsLength--;

            if (parsedComponentsLength > 0)
            {
                if (!TkStringUtils::fromString(parsedComponents[3].get(), revision))
                {
                    return false;
                }

                value = TkVersion {major, minor, build, revision};
            }
            else
            {
                value = TkVersion {major, minor, build};
            }
        }
        else
        {
            value = TkVersion {major, minor};
        }

        return true;
    }

}