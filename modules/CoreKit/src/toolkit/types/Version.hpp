/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/memory/Memory.hpp"
#include <toolkit/common.hpp>
#include <toolkit/base/TypeTraits.hpp>
#include <toolkit/base/Hash.hpp>

namespace toolkit
{
    // Represents the version number made of major, minor, build and revision numbers.
    class TkVersion
    {
    public:
        static TkVersion Empty;

        static bool parse(const TkString &text, TkVersion &value, IAllocator &alloc = TkMemory::getDefault());

    public:
        TkVersion(i32 major, i32 minor, i32 build, i32 revision);
        TkVersion(i32 major, i32 minor, i32 build);
        TkVersion(i32 major, i32 minor);

        TkVersion()
        {
            m_major = 0;
            m_minor = 0;
            m_revision = -1;
            m_build = -1;
        }

        TK_FORCE_INLINE i32 build() const
        {
            return m_build;
        }
        TK_FORCE_INLINE i32 major() const
        {
            return m_major;
        }
        TK_FORCE_INLINE i32 minor() const
        {
            return m_minor;
        }
        TK_FORCE_INLINE i32 revision() const
        {
            return m_revision;
        }

        // Compares the current TkVersion object to a specified TkVersion object and returns an indication of their relative values.
        i32 compareTo(const TkVersion &value) const;

        // Returns a value indicating whether the current TkVersion object and a specified TkVersion object represent the same value.
        bool equals(const TkVersion &obj) const
        {
            return m_major == obj.m_major && m_minor == obj.m_minor && m_build == obj.m_build && m_revision == obj.m_revision;
        }

        TK_FORCE_INLINE bool operator==(const TkVersion &other) const
        {
            return equals(other);
        }
        TK_FORCE_INLINE bool operator>(const TkVersion &other) const
        {
            return other < *this;
        }
        TK_FORCE_INLINE bool operator>=(const TkVersion &other) const
        {
            return other <= *this;
        }
        TK_FORCE_INLINE bool operator!=(const TkVersion &other) const
        {
            return !(*this == other);
        }
        TK_FORCE_INLINE bool operator<(const TkVersion &other) const
        {
            return compareTo(other) < 0;
        }
        TK_FORCE_INLINE bool operator<=(const TkVersion &other) const
        {
            return compareTo(other) <= 0;
        }

        TkString toString(i32 fieldCount) const;
        TkString toString() const;

    private:
        i32 m_major;
        i32 m_minor;
        i32 m_build;
        i32 m_revision;
    };

    template <>
    struct HashFunc<TkVersion>
    {
        static u32 get(const TkVersion &key)
        {
            return ((key.major() & 15) << 28) | ((key.minor() & 255) << 20) | ((key.build() & 255) << 12) | (key.revision() & 4095);
        }
    };

    template <>
    struct TIsPODType<TkVersion>
    {
        enum
        {
            Value = true
        };
    };
}