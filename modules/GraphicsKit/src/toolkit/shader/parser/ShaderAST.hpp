/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/shader/common.hpp>            // ShaderStage
#include <toolkit/shader/lexer/ShaderToken.hpp> // ShaderSourceLanguage
#include <toolkit/shader/source/SourceRange.hpp>

namespace toolkit
{
    struct ShaderAstNode
    {
        SourceRange range;
    };

    // Property "Color MainColor = (1, 1, 1, 1)" -- the default expression is
    // captured verbatim as text for now (Phase 5 / .tksd will parse values).
    struct PropertyDecl : ShaderAstNode
    {
        TkString type;
        TkString name;
        TkString defaultValue;
    };

    struct TagDecl : ShaderAstNode
    {
        TkString key;
        TkString value;
    };

    // "USE_TEXTURE = [0, 1]"
    struct VariantDecl : ShaderAstNode
    {
        TkString name;
        TkArray<TkString> values;
    };

    // "dx12 = \"sm_6_0\""
    struct TargetEntryDecl : ShaderAstNode
    {
        TkString backend;
        TkString profile;
    };

    struct TargetDecl : ShaderAstNode
    {
        TkArray<TargetEntryDecl> entries;
    };

    // External stage declaration: "Vertex { Source = "..."; Entry = "..."; }"
    struct StageDecl : ShaderAstNode
    {
        ShaderStage stage = ShaderStage::kVertex;
        TkString source;
        TkString entry;
    };

    // Inline source block (HLSLPROGRAM / GLSLPROGRAM / MSLPROGRAM).
    struct EmbeddedSourceDecl : ShaderAstNode
    {
        ShaderSourceLanguage language = ShaderSourceLanguage::Unknown;
        TkString sourceText;
    };

    // "ZWrite On" / "Blend Alpha" / etc.
    struct RenderStateDecl : ShaderAstNode
    {
        TkString name;
        TkString value;
    };

    struct PassDecl : ShaderAstNode
    {
        TkString name;
        TkArray<TagDecl> tags;
        TkArray<StageDecl> stages;
        TkArray<EmbeddedSourceDecl> embeddedSources;
        TkArray<RenderStateDecl> renderStates;
    };

    struct SubShaderDecl : ShaderAstNode
    {
        TargetDecl target;
        bool hasTarget = false;
        TkArray<TagDecl> tags;
        TkArray<PassDecl> passes;
    };

    struct ShaderDecl : ShaderAstNode
    {
        TkString name;
        TkArray<PropertyDecl> properties;
        TkArray<TagDecl> tags;
        TkArray<VariantDecl> variants;
        TkArray<SubShaderDecl> subShaders;
    };
}
