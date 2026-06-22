/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>
#include <toolkit/shader/common.hpp>
#include <toolkit/shader/manifest/ShaderManifest.hpp>

namespace toolkit
{
    class SourceManager;
    class DiagnosticSink;
    class ShaderJobCache;

    struct ShaderManifestCompileOptions
    {
        // Absolute or workdir-relative path to the .tkshader file.
        TkString manifestPath;
        // Directory where output files will be written.
        TkString outputDir;
        // CLI-selected profiles (GfxShaderProfile::kGLSL_410, kVulkan_SM5...).
        TkArray<GfxShaderProfile> profiles;

        // Required pipeline plumbing. Both objects must outlive the call.
        SourceManager *sources = nullptr;
        DiagnosticSink *diagnostics = nullptr;

        // Optional job-cache plumbing. When provided, every (pass, profile)
        // compile checks the cache first and stores its result back on miss.
        ShaderJobCache *cache = nullptr;

        // User-supplied include search directories (Phase 8). The manifest
        // compiler always prepends the manifest's own directory first, so
        // these are searched after the manifest dir and after the dir of the
        // file currently being processed.
        TkArray<TkString> includeDirs;

        // Used purely as part of generated TkUuid()s so different inputs in
        // the same CLI invocation get distinct ids.
        u32 inputIndex = 0;

        // Skip compilation when an existing .tksd is newer than the manifest
        // and every recorded source dependency.
        bool incremental = true;
    };

    struct ShaderManifestCompileResult
    {
        // Number of (profile, pass) cases attempted.
        i32 caseCount = 0;
        // How many of those failed.
        i32 failedCount = 0;
        // Cache stats (populated when ShaderManifestCompileOptions::cache is set).
        i32 cacheHit = 0;
        i32 cacheMiss = 0;
        i32 compiled = 0; // == caseCount - cacheHit - failed (approximately)
        i32 skipped = 0;
        // Phase 8 dependency-scan stats.
        i32 includeMissingCount = 0;
        i32 includeCycleCount = 0;
        i32 dependencyTotalCount = 0;
    };

    // Drives a .tkshader through the lexer/parser/sema pipeline, dispatches
    // each resolved (profile, subshader, pass) combination to the inner
    // ShadersCompilation backend, then accumulates the per-pass results into
    // a single `.tksd` container written to `outputDir`.
    //
    // The compiler never invokes DXC / glslang / D3DCompiler directly; it
    // composes source strings and hands them to ShadersCompilation::compile().
    class ShaderManifestCompiler
    {
    public:
        bool compile(const ShaderManifestCompileOptions &options,
                     ShaderManifestCompileResult &outResult);
    };
}
