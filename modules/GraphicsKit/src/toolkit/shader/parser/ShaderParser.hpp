/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/shader/lexer/ShaderLexer.hpp>
#include <toolkit/shader/parser/ShaderAST.hpp>

namespace toolkit
{
    class DiagnosticSink;

    // Recursive-descent parser for the .tkshader manifest grammar.
    // Errors are reported through DiagnosticSink and the parser attempts a
    // coarse "skip to next top-level brace" recovery strategy so a single
    // syntax error doesn't suppress every diagnostic in the rest of the file.
    class ShaderParser
    {
    public:
        ShaderParser(ShaderLexer &lexer, DiagnosticSink *sink);

        bool parse(ShaderDecl &outShader);

    private:
        const ShaderToken &peek() const { return m_current; }
        ShaderToken consume();
        bool accept(ShaderTokenKind kind);
        bool expect(ShaderTokenKind kind, ShaderToken &out, TkStringView code, TkStringView what);
        bool expect(ShaderTokenKind kind, TkStringView code, TkStringView what);

        bool parseShader(ShaderDecl &out);
        bool parseProperties(ShaderDecl &out);
        bool parseTags(TkArray<TagDecl> &outTags);
        bool parseVariants(ShaderDecl &out);
        bool parseSubShader(SubShaderDecl &out);
        bool parseTarget(TargetDecl &out);
        bool parsePass(PassDecl &out);
        bool parseStage(ShaderStage stage, PassDecl &outPass);
        bool parseRenderState(PassDecl &outPass);

        void recoverToTopLevel();
        void recoverToBlockEnd();

        void error(const SourceRange &range, TkStringView code, TkStringView message);
        void error(const SourceRange &range, TkStringView code, const TkString &message);

    private:
        ShaderLexer &m_lexer;
        DiagnosticSink *m_sink;
        ShaderToken m_current;
        bool m_panic = false; // true while we are in error-recovery mode.
    };
}
