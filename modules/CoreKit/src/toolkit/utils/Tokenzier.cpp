/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Tokenizer.hpp"

#include "toolkit/collection/Array.hpp"
#include "toolkit/debug/Logger.hpp"

namespace toolkit
{
    Tokenizer::Tokenizer(const char *input, i32 length)
        : m_buffer(input), m_length(length), m_cursor(const_cast<char *>(input)), m_pos(0), m_line(1), separators(32), whitespaces(8)
    {
    }

    void Tokenizer::setupHLSL()
    {
        static SeparatorData separators[] =
            {
                {'\r', '\n'},
                {'/', '/'},
                {'/', '*'},
                {'\n', 0},
                {'\t', 0},
                {' ', 0},
                {'.', 0},
                {',', 0},
                {':', 0},
                {';', 0},
                {'+', 0},
                {'-', 0},
                {'(', 0},
                {')', 0},
                {'!', 0},
                {'=', 0},
                {'&', 0},
                {'%', 0},
                {'*', 0},
                {'<', 0},
                {'>', 0},
                {'[', 0},
                {']', 0},
                {'{', 0},
                {'}', 0},
            };
        this->separators.set(separators, lengthOf(separators));
        static char whitespaces[] =
            {
                9,
                10,
                11,
                12,
                13,
                32,
            };
        this->whitespaces.set(whitespaces, lengthOf(whitespaces));
        singleLineComment = SeparatorData('/', '/');
        multiLineCommentSeparator = SeparatorData('/', '*');
    }

    char Tokenizer::readChar()
    {
        if (m_pos >= m_length)
            return 0;
        return moveForward();
    }

    void Tokenizer::eatWhiteSpaces()
    {
        // Read until can
        while (m_pos < m_length)
        {
            // Peek char
            char c = peekChar();

            // Check if is whitespace
            bool isToken = true;
            for (i32 i = 0; i < whitespaces.count(); i++)
            {
                if (whitespaces[i] == c)
                {
                    // Read it
                    moveForward();
                    isToken = false;
                    break;
                }
            }
            if (isToken)
            {
                // End
                return;
            }
        }
    }

    void Tokenizer::readToken(TextToken *token)
    {
        // Eat whitespaces
        eatWhiteSpaces();

        // Set token start but reset its length
        token->start = m_cursor;
        token->length = 0;
        token->separator = {0, 0};

        // Read until can
        while (m_pos < m_length)
        {
            // Read char
            char c = moveForward();

            // Check if it's a separator char
            char c1 = peekChar();
            for (i32 s = 0; s < separators.count(); s++)
            {
                if (separators[s].c0 == c && separators[s].c1 == c1)
                {
                    /*if (*length == 0)
                    {
                        // Skip separator after separator case and start searching for new token
                        *token = m_cursor;
                        *length = -1;
                        break;
                    }*/

                    token->separator = separators[s];
                    readChar();

                    // Check for comments
                    if (token->separator == singleLineComment)
                    {
                        // Read whole line
                        readLine();

                        // Read another token
                        readToken(token);
                    }
                    else if (token->separator == multiLineCommentSeparator)
                    {
                        // Read tokens until end sequence
                        char prev = ' ';
                        while (canRead())
                        {
                            c = readChar();
                            if (prev == '*' && c == '/')
                                break;
                            prev = c;
                        }

                        // Check if comment is valid (has end before file end)
                        if (!canRead())
                        {
                            LogWarn("Missing multiline comment ending");
                        }

                        // Read another token
                        readToken(token);
                    }

                    return;
                }
            }
            for (i32 s = 0; s < separators.count(); s++)
            {
                if (separators[s].c0 == c && separators[s].c1 == 0)
                {
                    if (token->length == 0)
                    {
                        // Skip separator after separator case and start searching for new token
                        token->start = m_cursor;
                        token->length = -1;
                        break;
                    }

                    token->separator = separators[s];
                    return;
                }
            }

            // Move to the next character
            token->length++;
        }
    }

    void Tokenizer::readLine(TextToken *token)
    {
        // Set line start but reset its length
        token->start = m_cursor;
        token->length = 0;

        // Read until can
        while (m_pos < m_length)
        {
            // Read char
            char c = moveForward();

            // Check new-line char
            if (c == '\n')
            {
                return;
            }

            // Move to the next char
            token->length++;
        }
    }

    void Tokenizer::readLine()
    {
        // Read until can
        while (m_pos < m_length)
        {
            // Read char
            char c = moveForward();

            // Check new-line char
            if (c == '\n')
            {
                return;
            }
        }
    }

    char Tokenizer::moveForward()
    {
        char c = *m_cursor;
        if (c == '\n')
            m_line++;
        m_cursor++;
        m_pos++;
        return c;
    }

    char Tokenizer::moveBack()
    {
        m_pos--;
        m_cursor--;
        char c = *m_cursor;
        if (c == '\n')
            m_line--;
        return c;
    }

}