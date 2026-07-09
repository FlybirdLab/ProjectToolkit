/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/shader/common.hpp> // ShaderStage, GfxShaderProfile, ShaderMacro
#include <toolkit/shader/cache/ShaderHash.hpp>
#include <toolkit/shader/compilegraph/DependencyScanner.hpp>

namespace toolkit
{
    // A single unit of compile work fed to the scheduler / cache. The struct
    // is intentionally flat -- everything that influences the produced
    // bytecode lives here so ShaderHash::hashJob() is the single source of
    // cache key truth.
    //
    // v1 limitation: the compile path still goes through
    // ShadersCompilation::compile() which produces a per-pass .shadercache
    // bundle (not raw per-stage bytecode), so for the manifest path one Job
    // is created per (pass, profile) -- not per stage. The struct keeps the
    // `stage` field so a later phase can split this further without changing
    // the cache layout.
    struct ShaderCompileJob
    {
        // Identity
        TkString shaderName;     // manifest Shader name (e.g. "Sandbox/Triangle")
        TkString passName;       // pass name (may be empty for legacy paths)

        ShaderStage stage = ShaderStage::kVertex;
        GfxShaderProfile profile = GfxShaderProfile::kUnknown;

        // Sources -- exactly one of (sourcePath, sourceText) is the primary
        // input. `virtualSourceName` is what diagnostics will display when
        // the input is purely in-memory (e.g. wrapped GLSL from inline block).
        TkString sourcePath;
        TkString virtualSourceName;
        TkString sourceText;

        // Entry point. May be empty for paths that derive entries from
        // META_VS / META_PS markers inside `sourceText`.
        TkString entryPoint;

        // Permutation key -- v1 manifests never set variants, so this is empty.
        TkString variantKey;

        // Defines forwarded to the backend compiler.
        TkArray<ShaderMacro> defines;

        // Phase 8: include search paths active for this job. Each entry is an
        // absolute, normalized directory. The set participates in the hash so
        // adding/removing -I dirs invalidates the cache.
        TkArray<TkString> includeDirs;

        // Phase 8: dependency closure resolved by DependencyScanner. Sorted
        // by resolvedPath so the on-wire hash is order-stable.
        TkArray<ShaderDependency> dependencies;
        ShaderHash128 dependencyHash;

        // Pre-computed job hash (filled by ShaderHash::hashJob). Zero means
        // "not hashed yet" -- the cache layer fills this lazily.
        u64 jobHashLow = 0;
        u64 jobHashHigh = 0;

        // Optional human-friendly identifier for log output.
        TkString outputName;
    };
}
