/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/compilegraph/VariantExpander.hpp>

namespace toolkit
{
    bool VariantExpander::expand(const ShaderDecl &shader, ShaderCompileGraph &graph) const
    {
        // v1: variants are parsed but not consumed. The caller has already
        // populated `graph` with one job per (pass, profile); no expansion to
        // do here yet.
        (void)shader;
        (void)graph;
        return true;
    }
}
