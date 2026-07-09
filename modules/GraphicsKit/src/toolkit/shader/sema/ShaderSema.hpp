/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/shader/parser/ShaderAST.hpp>

namespace toolkit
{
    class DiagnosticSink;

    // Minimal semantic validation for a parsed ShaderDecl.
    //
    // Phase 3 scope: structural rules only -- enough to keep manifest compile
    // from feeding garbage into the existing backend pipeline. Type/property
    // value validation will land alongside .tksd reflection (Phase 5/7).
    class ShaderSema
    {
    public:
        explicit ShaderSema(DiagnosticSink *sink);

        bool validate(const ShaderDecl &shader);

    private:
        bool validateSubShader(const SubShaderDecl &sub, i32 index);
        bool validatePass(const PassDecl &pass, i32 subIndex, i32 passIndex);
        bool validateTarget(const TargetDecl &target);
        bool validateRenderStates(const PassDecl &pass);
        bool checkLanguageMatchesTarget(const PassDecl &pass, const TargetDecl &target);

        void error(const SourceRange &range, TkStringView code, TkStringView message);
        void error(const SourceRange &range, TkStringView code, const TkString &message);
        void warning(const SourceRange &range, TkStringView code, TkStringView message);
        void warning(const SourceRange &range, TkStringView code, const TkString &message);

        DiagnosticSink *m_sink;
    };
}
