/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/shader/source/SourceRange.hpp>

namespace toolkit
{
    enum class ShaderTokenKind
    {
        EndOfFile,
        Unknown,

        Identifier,
        StringLiteral,
        NumberLiteral,

        LBrace,    // {
        RBrace,    // }
        LParen,    // (
        RParen,    // )
        LBracket,  // [
        RBracket,  // ]

        Equal,     // =
        Comma,     // ,
        Dot,       // .
        Colon,     // :
        Semicolon, // ;

        // Top-level structure keywords
        KeywordShader,
        KeywordProperties,
        KeywordTags,
        KeywordVariants,
        KeywordSubShader,
        KeywordPass,
        KeywordTarget,

        // Stage keywords
        KeywordVertex,
        KeywordPixel,
        KeywordCompute,

        // Render-state keywords
        KeywordBlend,
        KeywordCull,
        KeywordZWrite,
        KeywordZTest,

        // Embedded source markers (the BEGIN form is emitted by the lexer when
        // it transitions to EmbeddedSourceMode; the END form is consumed
        // internally and never surfaces).
        KeywordHLSLProgram,
        KeywordGLSLProgram,
        KeywordMSLProgram,

        // Raw payload between an *PROGRAM marker and its matching END* marker.
        // Its SourceRange covers the body only, not the markers themselves.
        EmbeddedSourceBlock,
    };

    const char *toString(ShaderTokenKind kind);

    enum class ShaderSourceLanguage
    {
        Unknown,
        HLSL,
        GLSL,
        MSL,
    };

    const char *toString(ShaderSourceLanguage lang);

    struct ShaderToken
    {
        ShaderTokenKind kind = ShaderTokenKind::Unknown;

        // Source range covered by the token (begin inclusive, end exclusive).
        SourceRange range;

        // Token text. For string literals this is the unescaped value (no
        // surrounding quotes). For embedded source blocks this is the raw block
        // body.
        TkString text;

        // Only meaningful for embedded source blocks.
        ShaderSourceLanguage language = ShaderSourceLanguage::Unknown;
    };
}
