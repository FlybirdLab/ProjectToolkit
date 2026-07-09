/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/types/String.hpp"
#include "toolkit/types/UUID.hpp"
#include "toolkit/collection/Array.hpp"
#include <toolkit/common.hpp>
#include <toolkit/graphics/common.hpp>
#include <toolkit/shader/core/environment.hpp>
#include <toolkit/shader/format/ShaderAsset.hpp>

namespace toolkit
{
    #define GFX_SHADER_CACHE_VERSION 1
    class MemoryOStream;
    class SourceManager;
    class DiagnosticSink;

    // Authoritative shader entry point declaration. When the .tkshader
    // manifest knows the entry name + stage upfront (the common case), the
    // compiler bypasses the legacy `META_VS` / `META_PS` marker auto-
    // discovery in ShadersCompilation::Parser and pre-populates ShaderMeta
    // from these entries. Avoids the brittle "scan tokens until EOF" name
    // extraction that doesn't work for hand-authored HLSL without META
    // decorators.
    struct PrebuiltShaderEntry
    {
        ShaderStage stage = ShaderStage::kVertex;
        TkString name;     // function name in the source (e.g. "VSMain")
    };

    // Shader compilation options container
    struct ShaderCompilationOptions
    {
    public:
        // Name of the target object (name of the shader or material for better logging readability)
        TkString targetName;
        // Unique ID of the target object
        TkUuid targetID = TkUuid::Empty;
        // Shader source code (null terminated)
        const char *source = nullptr;
        // Shader source code length
        u32 srcLen = 0;

    public:
        // Target shader profile
        GfxShaderProfile profile = GfxShaderProfile::kUnknown;
        // Disables shaders compiler optimizations. Can be used to debug shaders on a target platform or to speed up the shaders compilation time.
        bool noOptimize = false;
        // Enables shader debug data generation (depends on the target platform rendering backend).
        bool generateDebugData = false;
        // Enable/disable promoting warnings to compilation errors
        bool treatWarningsAsErrors = false;
        // Custom macros for the shader compilation
        TkArray<ShaderMacro> macros;

    public:
        // Output stream to write compiled shader cache to
        MemoryOStream *output = nullptr;

    public:
        // Optional diagnostic plumbing. When provided, the compiler routes structured
        // diagnostics (with error codes / source ranges) through these objects instead
        // of just logging to stderr. Phase 2 adds these as opt-in; future phases will
        // make them the primary error channel.
        SourceManager *sourceManager = nullptr;
        DiagnosticSink *diagnostics = nullptr;
        // File id (within `sourceManager`) of the primary shader source for `source`.
        // Used to anchor diagnostics that don't have a finer source location yet.
        u32 primaryFileId = 0;

        // User-supplied include search directories (forwarded from CLI -I /
        // --include-dir flags). Order is preserved -- earlier entries shadow
        // later ones at resolve time. Used both for IncludeResolver routing
        // and for cache hashing in Phase 8+ so the cache becomes invalid when
        // the include set changes.
        TkArray<TkString> includeDirs;

        // When non-empty, bypass the legacy META-marker Parser in
        // ShadersCompilation::compile and use these as the authoritative
        // entry-point list. ShaderManifestCompiler populates this from each
        // pass's `Vertex { Entry = ... }` / `Pixel { Entry = ... }` AST
        // nodes so external HLSL / GLSL sources without META decorators
        // still get their function names registered.
        TkArray<PrebuiltShaderEntry> prebuiltEntries;

        // Reflection sidechannel. Backends that produce a reflection-capable
        // intermediate (the GLSL backend's SPIRV-Cross pass is the only one
        // in v1) append per-resource records here so ShaderManifestCompiler
        // can fold them into the .tksd's Bindings / VertexInput chunks
        // without the manifest having to declare resources upfront. Other
        // backends leave these empty -- they're additive, not authoritative,
        // so the asset still reflects whatever the most-recently-compiled
        // (pass, profile) produced. Real cross-backend reflection unification
        // is a Phase 11+ task.
        TkArray<ShaderBindingInfo> reflectionBindings;
        TkArray<ShaderVertexInputInfo> reflectionVertexInputs;
    };
}