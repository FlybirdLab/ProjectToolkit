/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/shader/core/common.hpp>
#include <toolkit/collection/HashSet.hpp>

namespace toolkit
{
    class ShaderMeta;
    class ShaderFunctionMeta;
    class SourceManager;
    class DiagnosticSink;

    // Shader compilation context container
    class ShaderCompilationContext
    {
    public:
        // The compilation options.
        const ShaderCompilationOptions *options;
        // The shader metadata container.
        ShaderMeta *meta;

    public:
        // Output stream to write compiled shader cache to.
        MemoryOStream *output;
        // All source files included by this file (absolute paths). Generated during shader compilation.
        TkHashSet<TkString> includes;

        // Backend-populated reflection sidechannel. Owned by the context (one
        // per ShadersCompilation::compile call) so backends can append freely
        // without touching the caller-facing `options`. ShadersCompilation
        // copies these back into `options.reflectionBindings` /
        // `options.reflectionVertexInputs` after the backend returns, which
        // keeps the public API on `options` and removes the const_cast that
        // an earlier version of the code used to write into the const-options
        // pointer from the backend side.
        TkArray<ShaderBindingInfo> reflectionBindings;
        TkArray<ShaderVertexInputInfo> reflectionVertexInputs;

    public:
        // Name of the target object (in ASCII)
        char targetNameAnsi[64];

    public:
        // Event called on compilation error
        void onError(const char *message);
        // Event called on compilation debug data collecting
        void onCollectDebugInfo(ShaderFunctionMeta &meta, i32 permutationIndex, const char *data, const i32 dataLength);

        // Diagnostic plumbing helpers. Forwarded from `options`. May return
        // nullptr / 0 when the caller did not provide a sink.
        DiagnosticSink *diagnostics() const;
        SourceManager *sourceManager() const;
        u32 primaryFileId() const;

    public:
        ShaderCompilationContext(const ShaderCompilationOptions *options, ShaderMeta *meta);
    };

}