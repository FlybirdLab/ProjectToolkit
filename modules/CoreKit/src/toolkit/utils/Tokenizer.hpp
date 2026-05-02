/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/types/String.hpp"
#include <toolkit/common.hpp>
#include <toolkit/utils/StringUtils.hpp>
#include <toolkit/collection/Array.hpp>

namespace toolkit
{
    // Separator structure.
    struct SeparatorData
    {
    public:
        char c0;
        char c1;

    public:
        SeparatorData()
            : c0(0), c1(0)
        {
        }

        SeparatorData(char c0)
            : c0(c0), c1(0)
        {
        }

        SeparatorData(char c0, char c1)
            : c0(c0), c1(c1)
        {
        }

        SeparatorData(const SeparatorData &other)
            : c0(other.c0), c1(other.c1)
        {
        }

    public:
        bool isWhiteSpace() const
        {
            return *this == SeparatorData('\r', '\n') || *this == SeparatorData('\n') || *this == SeparatorData('\t') || *this == SeparatorData(' ');
        }

    public:
        bool operator==(const SeparatorData &other) const
        {
            return c0 == other.c0 && c1 == other.c1;
        }

        bool operator!=(const SeparatorData &other) const
        {
            return c0 != other.c0 || c1 != other.c1;
        }
    };

    // TextToken structure
    struct TextToken
    {
    public:
        const char *start;
        i32 length;
        SeparatorData separator;

    public:
        TextToken()
            : start(nullptr), length(0), separator()
        {
        }

        explicit TextToken(const char *text)
            : start(text), length(TkStringUtils::length(text)), separator()
        {
        }

        explicit TextToken(const TkString &text)
            : start(text.get()), length(text.length()), separator()
        {
        }

        TextToken(const char *text, i32 length)
            : start(text), length(length), separator()
        {
        }

        TextToken(const char *text, i32 length, const SeparatorData &separator)
            : start(text), length(length), separator(separator)
        {
        }

        TextToken(const TextToken &other)
            : start(other.start), length(other.length), separator(other.separator)
        {
        }

    public:
        TkString toString() const
        {
            return TkString(start, length);
        }

    public:
        TK_FORCE_INLINE bool equals(const TextToken &other) const
        {
            return equals(other.start, other.length);
        }

        TK_FORCE_INLINE bool equals(const char *text) const
        {
            return equals(text, TkStringUtils::length(text));
        }

        bool equals(const char *text, i32 l) const
        {
            return length == l && TkStringUtils::compareIStrings(start, text, l) == 0;
        }

    public:
        TK_FORCE_INLINE bool equalsIgnoreCase(const TextToken &other) const
        {
            return equalsIgnoreCase(other.start, other.length);
        }

        TK_FORCE_INLINE bool equalsIgnoreCase(const char *text) const
        {
            return equalsIgnoreCase(text, TkStringUtils::length(text));
        }

        bool equalsIgnoreCase(const char *text, i32 l) const
        {
            return length == l && TkStringUtils::compareIStrings(start, text, l) == 0;
        }

    public:
        TK_FORCE_INLINE bool operator==(const char *text) const
        {
            auto token = TextToken(text);
            return equals(token);
        }

        TK_FORCE_INLINE bool operator==(const TextToken &other) const
        {
            return equals(other);
        }

        TK_FORCE_INLINE bool operator!=(const TextToken &other) const
        {
            return !equals(other);
        }
    };

    // Helper class to fast ANSI text processing (tokenization, reading, streaming etc.).
    class TK_API Tokenizer : public NonCopyable
    {
    public:
        Tokenizer(const char *input, i32 length);

    public:
        // Array with all token separators.
        TkArray<SeparatorData> separators;
        // Array with all white characters.
        TkArray<char> whitespaces;

        SeparatorData singleLineComment;
        SeparatorData multiLineCommentSeparator;

    public:
        // Sets up separators and white chars for HLSL language.
        void setupHLSL();

    public:
        // Returns true if there are still characters in the buffer and can read data from it
        TK_FORCE_INLINE bool canRead() const
        {
            return m_pos < m_length;
        }

        // Peeks a single character without moving forward in the buffer.
        TK_FORCE_INLINE char peekChar() const
        {
            return *m_cursor;
        }

        // Gets the current line number.
        TK_FORCE_INLINE i32 getLine() const
        {
            return m_line;
        }

    public:
        // Read single character from the buffer
        char readChar();
        // Read all whitespace chars like spaces, tabs, new lines
        void eatWhiteSpaces();
        // Reads next token
        void readToken(TextToken *token);
        // Read whole line until new line char '\n'
        void readLine(TextToken *token);
        // Read whole line until new line char '\n'
        void readLine();

    private:
        char moveForward();
        char moveBack();

    private:
        const char *m_buffer;
        i32 m_length;
        char *m_cursor;
        i32 m_pos;
        i32 m_line;
    };

}