/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

namespace toolkit
{
    class ShaderCompileGraph;
    class ShaderJobCache;
    class DiagnosticSink;

    struct ShaderCompileStats
    {
        u32 jobCount = 0;
        u32 cacheHit = 0;
        u32 cacheMiss = 0;
        u32 compiled = 0;
        u32 failed = 0;
        f64 totalTimeMs = 0.0;
    };

    // Stub scheduler.
    //
    // v1 does not run shader compiles in parallel: existing backends
    // (DXC / glslang / D3DCompiler) and the global ShadersCompilation
    // singleton are not yet audited for thread safety, and ShaderManifestCompiler
    // already drives compiles inline. The class exists so the CLI can accept
    // --jobs N and so a later phase can land a real implementation without
    // touching call sites.
    class CompilerScheduler
    {
    public:
        void setJobCount(u32 jobs);
        u32 getJobCount() const { return m_jobCount; }

        // Tells callers whether `setJobCount(>1)` is currently honored.
        bool isParallelEnabled() const { return false; }

    private:
        u32 m_jobCount = 1;
    };
}
