#pragma once

#include "Tokens.hpp"

#include "toolkit/collection/Array.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/types/StringView.hpp"

#include <map>

namespace toolkit {

#ifdef __WIN32
#define NewLineChar '\n\r'
#else
#define NewLineChar '\n'
#endif
const char CHAR_EOF = 0;
const char CHAR_CR = '\r';
const char CHAR_LF = '\n';
const char CHAR_SPACE = ' ';
const char CHAR_TAB = '\t';

#define NLexFailedErr 3
#define NLexTextEscape 4
#define NLexFloatbool 5
#define NLexNumberbool 6


    TK_FORCE_INLINE bool isDigit(char ch) {
        return ch >= '0' && ch <= '9';
    }

    TK_FORCE_INLINE bool isHexDigit(char ch) {
        return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
    }
    
    TK_FORCE_INLINE bool isLetter(char ch) {
        return ch == '_' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
    }

    TK_FORCE_INLINE i64 toNumber(char digit) {
        if (!isDigit(digit)) {
            return 9 + (digit & 15);
        }
        return digit & 15;
    }


    class CXXLexer
    {
        friend class CXXParser;
    public:
        CXXLexer(const TkString& src);
        ~CXXLexer();

        bool lex();

        TkArray<Token>& getTokens() {
            return m_tokens;
        }

    private:
        TK_FORCE_INLINE void skipWhiteSpace() {
            while (m_idx < m_max) {
                if (m_src_data[m_idx] == NewLineChar) {
                    m_line++;
                    pushToken(TokenType::kEOL);
                }
                if (!isspace(m_src_data[m_idx]))
                    break;
                m_idx++;
            }
        }

        TK_FORCE_INLINE bool checkMatched(const char* token) const {
            size_t size = TkStringUtils::length(token);
            if (m_idx + size > m_max)
                return false;
            const char* start = m_src_data + m_idx * sizeof(char);
            const char* end = m_src_data + (m_idx + size) * sizeof(char);
            TkStringView raw_token(start, end - start);
            return raw_token.compare( TkStringView{ token });
        }
        
        TK_FORCE_INLINE char getChar(u32 idx) const {
            return idx > m_max ? ' ' : m_src[idx];
        }

        TK_FORCE_INLINE TkString subString(u32 start, u32 end) const {
            if (end > m_max) {
                return TkString::Empty;
            }
            return m_src.substring(start, end - start);
        }

        TK_FORCE_INLINE void pushToken(TokenType type, const TkString& val = TkString::Empty) {
            m_tokens.add(Token { .type = type, .value = val });
        }

        bool lexText();
        bool lexNumber();
        bool lexIdentifier();
        bool lexComment(bool doubleSlash);

    private:
        TkString m_src;
        const char* m_src_data;
        u32 m_max;
        u32 m_idx;
        u32 m_line;

        TkArray<Token> m_tokens;
        const static std::map<char, TokenType> s_token_types;
    };
}