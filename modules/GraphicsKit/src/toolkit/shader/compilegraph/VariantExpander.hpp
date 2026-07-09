/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/shader/compilegraph/ShaderCompileGraph.hpp>
#include <toolkit/shader/parser/ShaderAST.hpp>

namespace toolkit
{
    // Variant expansion stub.
    //
    // The .tkshader grammar accepts `Variants { NAME = [0, 1] }` blocks, but
    // v1 of the manifest compiler does not consume them. This class exists so
    // ShaderManifestCompiler / CompilerScheduler can call into a single,
    // documented seam when variant explosion lands.
    //
    // Current behavior: emits one job per (pass, profile) with an empty
    // variantKey. Future phases will produce 2^N jobs for a SubShader with N
    // boolean variants.
    class VariantExpander
    {
    public:
        // No-op for v1. Returns true so callers can chain it unconditionally.
        bool expand(const ShaderDecl &shader, ShaderCompileGraph &graph) const;
    };
}
