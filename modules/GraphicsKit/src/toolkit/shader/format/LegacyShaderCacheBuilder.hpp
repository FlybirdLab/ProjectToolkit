/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/shader/format/ShaderAsset.hpp>

namespace toolkit
{
    // Reconstructs an in-memory legacy `.shadercache` blob from the per-stage
    // data stored in a `.tksd` ShaderAsset. The blob can then be fed straight
    // to `GfxShader::create(MemoryIStream&)`, which is what the runtime
    // shader-creation path already knows how to read.
    //
    // v1 scope (Phase 9, pragmatic):
    //   - LegacyBundle entries  -> identity pass-through (the payload IS the
    //                              legacy blob).
    //   - Raw entries (Phase 9) -> v1 also passes through, because the
    //                              ShaderCompiler hasn't been split per-stage
    //                              yet so each Raw entry happens to carry a
    //                              self-contained per-pass legacy bundle. A
    //                              future phase that finally splits per-stage
    //                              bytecode at the compiler will refine this
    //                              builder to actually re-emit a legacy
    //                              container from VS+PS+CB metadata.
    //
    // Lives in TkGraphics so the runtime can build a shader from a `.tksd`
    // without dragging the compiler library along.
    class LegacyShaderCacheBuilder
    {
    public:
        // Build the legacy `.shadercache` blob for `profile` from the bundle
        // stored for `pass`. Returns false and fills `lastError` on failure.
        bool buildFromStages(
            const ShaderAsset &asset,
            const ShaderPass &pass,
            GfxShaderProfile profile,
            TkArray<byte> &outBlob);

        const TkString &lastError() const { return m_lastError; }

    private:
        TkString m_lastError;
    };
}
