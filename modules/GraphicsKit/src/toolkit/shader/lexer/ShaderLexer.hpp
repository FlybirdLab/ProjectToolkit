/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/shader/lexer/ShaderToken.hpp>
#include <toolkit/shader/source/SourceFile.hpp>

namespace toolkit
{
    class DiagnosticSink;

    // Hand-written lexer for the .tkshader manifest DSL.
    //
    // The lexer operates in two modes:
    //   Normal           -- recognises DSL tokens (keywords, identifiers,
    //                       string/number literals, punctuation).
    //   EmbeddedSource   -- entered on HLSLPROGRAM / GLSLPROGRAM / MSLPROGRAM;
    //                       consumes raw text verbatim until the matching
    //                       ENDHLSL / ENDGLSL / ENDMSL marker and emits a
    //                       single EmbeddedSourceBlock token.
    //
    // The lexer reports errors via the DiagnosticSink passed at construction
    // and produces an EndOfFile token at the end of input. It is single-use:
    // create one lexer per source file.
    class ShaderLexer
    {
    public:
        ShaderLexer(const SourceFile *file, DiagnosticSink *sink);

        ShaderToken next();
        bool isAtEnd() const { return m_offset >= m_content.length(); }

    private:
        enum class Mode
        {
            Normal,
            EmbeddedSource,
        };

        void skipWhitespaceAndComments();
        ShaderToken lexIdentifierOrKeyword();
        ShaderToken lexNumber();
        ShaderToken lexString();
        ShaderToken lexEmbeddedSourceBody(ShaderSourceLanguage lang, u32 startOffset);

        SourceLocation locationAt(u32 offset) const;
        SourceRange rangeBetween(u32 begin, u32 end) const;
        ShaderToken makeSingleCharToken(ShaderTokenKind kind);

        char peek(i32 ahead = 0) const;
        bool matches(const char *literal) const;

        void reportLexerError(const SourceRange &range, TkStringView code, TkStringView message);

    private:
        const SourceFile *m_file;
        DiagnosticSink *m_sink;
        TkStringView m_content;
        u32 m_offset = 0;
        Mode m_mode = Mode::Normal;
        // When in EmbeddedSource mode this is the language we expect to close.
        ShaderSourceLanguage m_pendingEmbedded = ShaderSourceLanguage::Unknown;
        u32 m_pendingEmbeddedStartOffset = 0;
    };
}
