/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */
#include "Tokens.hpp"

#include "parser/diagnose/SourceLoc.hpp"
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
        "Arrow",
        "LogicAnd",
        "LogicOr",
        "BitOr",
        "Not",
        "BitNot",
        "Question",
        "Percent",
        "Xor",
        "Ellipsis",
        "HashHash",
        "EndOfFile",
        "LeftAngleBracket",
        "RightAngleBracket"
    };


    Token Token::Empty{ TokenType::kUnknown, TkString::Empty };


    TkStringView Token::getTypeName() const {
        return s_tokenTypeStr[(u32)type];
    }


    TkString Token::toString() const {
        const TkString typeName = getTypeName().toString();
        return TkString::format("[%s : %s]", typeName.getText(), value.getText());
    }


    TokenType Token::parse(const TkStringView& str)
    {
        for (u32 idx = 0; idx < sizeof(s_tokenTypeStr) / sizeof(s_tokenTypeStr[0]); idx++) {
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
