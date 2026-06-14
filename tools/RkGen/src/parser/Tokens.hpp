/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */
#pragma once

#include <toolkit/types/String.hpp>

namespace toolkit {

    enum class TokenType : u8 {
        kUnknown,
        kEOL,
        kIdentifier,
        kHex,
        kNumber,
        kEqual,
        kComma,
        kSemiColon,
        kLCurlyBrace,
        kRCurlyBrace,
        kLParen,
        kRParen,
        kLBracket,
        kRBracket,
        kString,
        kChar,
        kPreprocessor,
        kColon,
        kDoubleColon,
        kDot,
        kLessThan,
        kGreaterThan,
        kAnd,
        kMultiply,
        kDivide,
        kAdd,
        kSub,
        kSlash,
        kArrow,
        kLogicAnd,
        kLogicOr,
        kBitOr,
        kNot,
        kBitNot,
        kQuestion,
        kPercent,
        kXor,
        kEllipsis,
        kHashHash,
        kEOF,
        kLAngleBracket = kLessThan,
        kRAngleBracket = kGreaterThan,
    };


    struct Token
    {
        TokenType type;
        TkString value = TkString::Empty;
        u32 line;
        u32 column;

        TkStringView getTypeName() const;
        TkString toString() const;

        bool operator==(const Token& other) const {
            return type == other.type && value == other.value;
        }
        bool operator!=(const Token& other) const {
            return !(*this == other);
        }

        static TokenType parse(const TkStringView& str);
        struct SourceLoc locate(class CXXSourceFile* file = nullptr) const;

        static Token Empty;
    };
}
