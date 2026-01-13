/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "StringBuilder.hpp"

#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>

namespace toolkit {

    TkStringBuilder::TkStringBuilder(IAllocator& alloc)
        : m_alloc {alloc}
        , m_data {alloc}
    {}

    TkStringBuilder::TkStringBuilder(i32 capacity, IAllocator& alloc)
        : m_data {capacity, alloc}
        , m_alloc {alloc}
    {}

    void TkStringBuilder::toString(TkString& result) const
    {
        result = TkString(m_data.get(), m_data.count(), m_alloc);
    }


    TkStringBuilder& TkStringBuilder::append(const char c)
    {
        m_data.add(c);
        return *this;
    }
    TkStringBuilder& TkStringBuilder::append(const char* str)
    {
        const i32 length = TkStringUtils::length(str);
        m_data.add(str, length);
        return *this;
    }
    TkStringBuilder& TkStringBuilder::append(const char* str, int len)
    {
        m_data.add(str, len);
        return *this;
    }
    TkStringBuilder& TkStringBuilder::append(const TkString& str)
    {
        m_data.add(*str, str.length());
        return *this;
    }
    TkStringBuilder& TkStringBuilder::append(const TkStringView& str)
    {
        m_data.add(*str, str.length());
        return *this;
    }
    TkStringBuilder& TkStringBuilder::append(i32 v)
    {
        auto str = TkStringUtils::toString(v);
        m_data.add(*str, str.length());
        return *this;
    }
    TkStringBuilder& TkStringBuilder::append(u32 v)
    {
        auto str = TkStringUtils::toString(v);
        m_data.add(*str, str.length());
        return *this;
    }


    TkStringBuilder& TkStringBuilder::appendLine()
    {
        append(TK_LINE_TERMINATOR);
        return *this;
    }
    TkStringBuilder& TkStringBuilder::appendLine(i32 v)
    {
        append(v);
        append(TK_LINE_TERMINATOR);
        return *this;
    }
    TkStringBuilder& TkStringBuilder::appendLine(u32 v)
    {
        append(v);
        append(TK_LINE_TERMINATOR);
        return *this;
    }
    TkStringBuilder& TkStringBuilder::appendLine(const char* str)
    {
        const i32 length = TkStringUtils::length(str);
        m_data.add(str, length);
        append(TK_LINE_TERMINATOR);
        return *this;
    }
    TkStringBuilder& TkStringBuilder::appendLine(const TkString& str)
    {
        m_data.add(*str, str.length());
        append(TK_LINE_TERMINATOR);
        return *this;
    }

    TkString TkStringBuilder::toString() const
    {
        return TkString(m_data.get(), m_data.count());
    }
}