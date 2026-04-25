#include "Tokens.hpp"

#include "purc/diagnose/SourceLoc.hpp"
#include <toolkit/types/StringView.hpp>

namespace toolkit {

    static const TkStringView s_tokenTypeStr[] = {
        "Unknown",
        "EOL",
        "Identifier",
        "Hex",
        "Number",
        "Equal",
        "Comma",
        "SemiColon",
        "LeftCurlyBrace",
        "RightCurlyBrace",
        "LeftParen",
        "RightParen",
        "LeftBracket",
        "RightBracket",
        "String",
        "Character",
        "Preprocessor",
        "Colon",
        "DoubleColon",
        "Dot",
        "LessThan",
        "GreaterThan",
        "And",
        "Multiply",
        "Divide",
        "Add",
        "Sub",
        "Slash",
        "EndOfFile",
        "LeftAngleBracket",
        "RightAngleBracket"
    };


    Token Token::Empty{ TokenType::kUnknown, TkString::Empty };


    TkStringView Token::getTypeName() const {
        return s_tokenTypeStr[(u32)type];
    }


    TkString Token::toString() const {
        return TkString::format("[{0} : {1}]", getTypeName().toString(), value);
    }


    TokenType Token::parse(const TkStringView& str)
    {
        for (u32 idx = 0; idx < 33; idx++) {
            if (s_tokenTypeStr[idx] == str) {
                return (TokenType)idx;
            }
        }
        return TokenType::kUnknown;
    }


    SourceLoc Token::locate(CXXSourceFile* file) const {
        return SourceLoc{ line, column, file };
    }

} // namespace cobalt