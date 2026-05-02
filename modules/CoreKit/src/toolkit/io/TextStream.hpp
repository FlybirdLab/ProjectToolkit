/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/types/String.hpp"
#include "toolkit/types/StringView.hpp"
#include "toolkit/base/Format.hpp"
#include <toolkit/io/MemoryStream.hpp>
#include <toolkit/utils/StringUtils.hpp>

namespace toolkit
{

    // Useful tool to write many text data
    class TextWriter : public NonCopyable
    {
    private:
        MemoryOStream m_buffer;

    public:
        TextWriter(u32 capacity = 1024)
            : m_buffer(capacity)
        {
        }
        ~TextWriter() = default;

    public:
        // Gets writer private buffer.
        TK_FORCE_INLINE MemoryOStream *getBuffer()
        {
            return &m_buffer;
        }

        // Gets writer private buffer.
        TK_FORCE_INLINE const MemoryOStream *getBuffer() const
        {
            return &m_buffer;
        }

    public:
        // Write line terminator sign
        template <class Q = char>
        typename std::enable_if<std::is_same<Q, char>::value, void>::type writeLine()
        {
            m_buffer.writeU8('\n');
        }

        // Write line terminator sign
        template <class Q = char>
        typename std::enable_if<std::is_same<Q, wchar>::value, void>::type writeLine()
        {
            m_buffer.writeU16('\n');
        }

        // Write single line of text to the buffer
        void writeLine(const char *text)
        {
            m_buffer.writeBytes((void *)text, TkStringUtils::length(text) * sizeof(char));
            writeLine();
        }

        // Format text and write line to the buffer
        template <typename... Args>
        void writeLine(const char *format, const Args &...args)
        {
            FormatBuffer buf{TkMemory::getDefault()};
            Formatter::format(buf, format, args...);
            m_buffer.writeBytes(buf.getPointer(), static_cast<u32>(buf.getUsed() * sizeof(char)));
            writeLine();
        }

        // Write text to the buffer
        void write(const TkStringView &text)
        {
            m_buffer.writeBytes((void *)text.get(), text.length() * sizeof(char));
        }

        // Write text to the buffer
        void write(const TkString &text)
        {
            m_buffer.writeBytes((void *)text.get(), text.length() * sizeof(char));
        }

        // Write text to the buffer
        void write(const char *text)
        {
            m_buffer.writeBytes((void *)text, TkStringUtils::length(text) * sizeof(char));
        }

        // Format text and write it
        template <typename... Args>
        void write(const char *format, const Args &...args)
        {
            FormatBuffer buf{TkMemory::getDefault()};
            Formatter::format(buf, format, args...);
            m_buffer.writeBytes(buf.getPointer(), static_cast<u32>(buf.getUsed() * sizeof(char)));
        }

    public:
        // Clear whole data
        void clear()
        {
            m_buffer.setPos(0);
        }

        TkString toString()
        {
            return TkString((char *)m_buffer.getHandle(), m_buffer.pos() / sizeof(char));
        }
    };

}
