/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/shader/parser/ShaderAST.hpp>
#include <toolkit/shader/source/SourceFile.hpp>

namespace toolkit
{
    // Resolved manifest data: the AST as produced by the parser plus
    // back-references to its origin SourceFile so we can anchor downstream
    // diagnostics at the right location.
    //
    // Phase 3 keeps this thin -- it is just a convenience bundle, not a fully
    // normalized intermediate form. Properties/variants live here verbatim and
    // are not yet consumed by the compile path.
    struct ShaderManifest
    {
        // The source file that was lexed/parsed. Owned by SourceManager.
        const SourceFile *file = nullptr;
        ShaderDecl declaration;
    };
}
