/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/shader/common.hpp> // ShaderStage, GfxShaderProfile

namespace toolkit
{
    // ---------- Manifest-mirrored metadata ----------

    struct ShaderProperty
    {
        TkString type;          // Color / Float / Texture2D / Int / ...
        TkString name;
        TkString defaultValue;  // raw textual form for now
    };

    struct ShaderTag
    {
        TkString key;
        TkString value;
    };

    struct ShaderRenderState
    {
        TkString name;   // Blend / Cull / ZWrite / ZTest / ...
        TkString value;  // On / Off / Back / Alpha / ...
    };

    // ---------- Stage bytecode ----------

    // Stage container used by the manifest -> .tksd path. v1 stores one
    // ShaderStageData per (pass, profile) with `kind == TKSD_STAGE_KIND_RAW`;
    // `bytecode` holds a self-contained `.shadercache`-wire-format blob
    // covering every stage in the pass. The runtime extracts a usable
    // `GfxShader` from it via `LegacyShaderCacheBuilder`. A future split
    // into per-stage bytecode + reflection will refine this without
    // bumping the format major.
    struct ShaderStageData
    {
        u32 kind = 0;                       // TKSD_STAGE_KIND_*
        ShaderStage stage = ShaderStage::kVertex;
        GfxShaderProfile profile = GfxShaderProfile::kUnknown;
        TkString entry;                     // optional, kept for forward-compat
        TkArray<byte> bytecode;             // payload (legacy `.shadercache` wire format in v1)
    };

    // ---------- Reflection ----------

    struct ShaderBindingInfo
    {
        TkString name;
        u32 slot = 0;
        ShaderStage stage = ShaderStage::kVertex;
        TkString type;  // "CB" / "SRV" / "UAV" / "Sampler"
    };

    struct ShaderVertexInputInfo
    {
        TkString semantic;
        u32 location = 0;
        u32 format = 0;     // erased PixelFormat (decoded at runtime)
        u32 slot = 0;
        u32 offset = 0;
    };

    // ---------- Pass ----------

    struct ShaderPass
    {
        TkString name;
        TkArray<ShaderTag> tags;
        TkArray<ShaderRenderState> renderStates;
        // Indices into ShaderAsset::stages. Each entry corresponds to one
        // GfxShaderProfile produced for this pass.
        TkArray<u32> stageIndices;
    };

    // ---------- Top-level asset ----------

    struct ShaderAsset
    {
        // Manifest-level data
        TkString name;
        TkArray<ShaderProperty> properties;
        TkArray<ShaderTag> tags;
        TkArray<ShaderPass> passes;

        // Bytecode + reflection
        TkArray<ShaderStageData> stages;
        TkArray<ShaderBindingInfo> bindings;
        TkArray<ShaderVertexInputInfo> vertexInputs;

        // Source dependency closure (normalized absolute paths). Populated by
        // the manifest compiler from DependencyScanner output and emitted as
        // a Dependencies chunk in the .tksd. Editor / build-system can use
        // this to wire up hot-reload watchers without re-scanning sources.
        TkArray<TkString> dependencies;
    };
}
