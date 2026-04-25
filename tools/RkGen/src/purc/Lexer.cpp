/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

 #include "Lexer.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/types/StringBuilder.hpp"

namespace toolkit {

    const std::map<char, TokenType> CXXLexer::s_token_types = {
        {'+', TokenType::kAdd},
        {'-', TokenType::kSub},
        {'*', TokenType::kMultiply},
       //  {'/', TokenType::Divide},
        {';', TokenType::kSemiColon},
        {'<', TokenType::kLAngleBracket},
        {'>', TokenType::kRAngleBracket},
        {'(', TokenType::kLParen},
        {')', TokenType::kRParen},
        {'{', TokenType::kLCurlyBrace},
        {'}', TokenType::kRCurlyBrace},
        {'[', TokenType::kLBracket},
        {']', TokenType::kRBracket},
        {'.', TokenType::kDot},
        {'&', TokenType::kAnd},
        {':', TokenType::kColon},
        {'#', TokenType::kPreprocessor},
        {';', TokenType::kSemiColon},
        {',', TokenType::kComma},
        {'=', TokenType::kEqual},
        {'\\', TokenType::kSlash}
    };

    CXXLexer::CXXLexer(const TkString& src)
        : m_src(src)
        , m_max(src.length())
        , m_idx(0)
        , m_line(0)
    {
        m_src_data = m_src.get();
    }


    CXXLexer::~CXXLexer() {
        m_tokens.clear();
    }


    bool CXXLexer::lex() {
        if (!m_tokens.isEmpty()) {
            LogError("Current source has been lexed!");
            return false;
        }

        while (m_idx < m_max) {
            skipWhiteSpace();
            char c = m_src_data[m_idx];

            if (s_token_types.find(c) != s_token_types.end()) {
                TokenType type = s_token_types.at(c);
                if (type == TokenType::kColon && m_idx + 1 < m_max) {
                    char next = m_src_data[m_idx + 1];
                    if (next == ':') {
                        m_tokens.add(Token{ .type = TokenType::kDoubleColon, .value = "::" });
                        m_idx += 2;
                        continue;
                    }
                }

                m_tokens.add(Token{ .type = type, .value = TkString::Empty });
                m_idx += 1;
            }
            else if (c == '/') {
                char next = getChar(m_idx + 1);
                if (next == '/') {
                    m_idx += 2;
                    auto e = lexComment(true);
                    if (!e) {
                        LogError("[Lexer] Failed to lex comment");
                    }
                }
                else if (next == '*') {
                    m_idx += 2;
                    auto e = lexComment(false);
                    if (!e) {
                        LogError("[Lexer] Failed to lex comment");
                    }
                }
                else {
                    m_idx += 1;
                    pushToken(TokenType::kDivide, "/");
                }
                continue;
            }
            else if (c == '\'') {
                if (m_idx + 2 < m_max) {
                    char c2 = m_src_data[m_idx + 2];
                    if (c2 == '\'') {
                        m_idx += 2;
                        m_tokens.add(Token{ .type = TokenType::kChar, .value = TkString(&m_src_data[m_idx + 1]) });
                        continue;
                    }
                }
            }
            else if (c == '\"') {
                auto txt = lexText();
                if(!txt) {
                    LogError("Failed to lex text");
                }
            }
            else {
                if (isDigit(c)) {
                    bool e = lexNumber();
                    if (!e) {
                        LogError("[Lexer] scan number error -> ", c);
                    }
                }
                else if (isLetter(c)) {
                    bool e = lexIdentifier();
                    if (!e) {
                        LogError("[Lexer] scan identifier error -> ", c);
                    }
                }
                else if (c == CHAR_EOF) {
                    pushToken(TokenType::kEOF);
                    return false;
                }
                else {
                    LogError("[Lexer] unexpected symbol near -> ", c);
                    return false;
                }
                continue;
            }
        }

        return true;
    }


    bool CXXLexer::lexText() {
        u32 start = m_idx + 1;
        bool escape = false;
        char current = getChar(start);
        char next = ' ';
        TkStringBuilder result {};
        m_idx++;

        while (m_idx < m_max) {
            current = getChar(m_idx);
            if (current == 0 || current == '\r' || current == '\n') {
                char prev = getChar(m_idx - 1);
                if (prev != '\\') {
                    LogError("[Lexer] expect another quotation mark before end of file or newline");
                    return NLexTextEscape;
                }
                u32 endLine = m_idx;
                auto linePart = subString(start, m_idx);
                result.append(linePart);
                m_idx++;
                start = m_idx;
                continue;
            }
            if (current == '\"') {
                auto str = subString(start, m_idx);
                result.append(str);
                m_idx++;
                pushToken(TokenType::kString, result.toString());
                return true;
            }
            m_idx++;
        }

        pushToken(TokenType::kString, result.toString());
        return true;
    }


    bool CXXLexer::lexNumber() {
        u32 start = m_idx;
        bool isFloat = false;

        char first = getChar(m_idx);
        if (first == '0') {
            char next = getChar(m_idx + 1);
            if (next == 'X' || next == 'x') {
                m_idx += 2;
                while (m_idx < m_max && isHexDigit(m_src[m_idx]) && m_src[m_idx] != '\n') {
                    m_idx++;
                }
                auto hex = subString(start, m_idx);
                pushToken(TokenType::kHex, hex);
                return true;
            }
        }
        if (!isDigit(first)) {
            return false;
        }
        while (m_idx < m_max) {
            char current = m_src[m_idx];
            if (!isDigit(current) || current == '\n' || current == '\r' || current == 0) {
                break;
            }
            else if (current == '.') {
                if (isFloat) {
                    return false;
                }
            }
            m_idx++;
        }
        auto number = subString(start, m_idx);
        pushToken(TokenType::kNumber, number);
        return true;
    }


    bool CXXLexer::lexIdentifier() {
        u32 start = m_idx;

        while (m_idx < m_max && isLetter(m_src[m_idx]) || isDigit(m_src[m_idx])) {
            m_idx++;
        }

        auto idType = subString(start, m_idx);
        auto type = Token::parse(idType.get());
        if (type != TokenType::kUnknown) {
            pushToken(type, idType);
            return true;
        }

        pushToken(TokenType::kIdentifier, idType);
        return true;
    }


    bool CXXLexer::lexComment(bool doubleSlash) {
        if (doubleSlash) {
            u32 start = m_idx;
            while (m_idx < m_max) {
                char ahead = getChar(m_idx);
                if (ahead == CHAR_EOF || ahead == CHAR_CR || ahead == CHAR_LF) {
                    break;
                }
                m_idx++;
            }
            // pushToken(TokenType::CommentSingleLine, m_src.substr(start, m_idx - start));
            return true;
        }
        /* */

        u32 start = m_idx;
        while (m_idx < m_max) {
            char current = getChar(m_idx);
            if (current == '*') {
                char next = getChar(m_idx + 1);
                if (next == '/') {
                    break;
                }
            }
            m_idx++;
        } //TODO how about colum ? line ? fix it later
        // pushToken(TokenType::CommentMultiLine, m_src.substr(start, m_idx - start));
        return true;
    }

} // namespace cobalt