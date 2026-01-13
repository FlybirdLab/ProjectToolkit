/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/base/Assert.hpp>
#include <toolkit/common.hpp>

namespace toolkit
{
    // Represents a value type that can be assigned null. A nullable type can represent the correct range of values for its underlying value type, plus an additional null value
    template <typename T>
    class TkNullable
    {
    public:
        TkNullable()
        {
            m_hasValue = false;
        }
        TkNullable<T>(const T &value)
        {
            m_value = value;
            m_hasValue = true;
        }
        TkNullable(const TkNullable &other)
        {
            m_value = other.m_value;
            m_hasValue = other.m_hasValue;
        }

    public:
        TK_FORCE_INLINE bool hasValue() const
        {
            return m_hasValue;
        }
        TK_FORCE_INLINE const T &getValue() const
        {
            TK_ASSERT(m_hasValue);
            return m_value;
        }
        TK_FORCE_INLINE T getValue()
        {
            TK_ASSERT(m_hasValue);
            return m_value;
        }

        void setValue(const T &value)
        {
            m_value = value;
            m_hasValue = true;
        }

        void reset()
        {
            m_hasValue = false;
        }

        bool operator==(const TkNullable &other) const
        {
            if (m_hasValue)
            {
                return other.m_hasValue && m_value == other.m_value;
            }

            return !other.m_hasValue;
        }
        TK_FORCE_INLINE bool operator!=(const TkNullable &other) const
        {
            return !operator==(other);
        }

    private:
        bool m_hasValue;
        T m_value;
    };

}