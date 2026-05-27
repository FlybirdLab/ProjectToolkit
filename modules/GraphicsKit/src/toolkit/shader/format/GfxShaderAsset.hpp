/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>
#include <toolkit/shader/format/ShaderAsset.hpp>

namespace toolkit
{
    class GfxDevice;
    class GfxShader;

    // Runtime-only wrapper around a parsed ShaderAsset.
    //
    // Lives in TkGraphics (no dependency on the compiler library). Hands out
    // GfxShader instances for individual passes by feeding the per-pass legacy
    // bundle blob into the existing GfxShader::create(MemoryIStream&) path.
    // Future phases can switch to true per-stage bytecode without breaking
    // this surface (kind == TKSD_STAGE_KIND_RAW returns nullptr for now).
    class GfxShaderAsset
    {
    public:
        bool loadFromFile(TkStringView path);
        bool loadFromMemory(const byte *data, u32 size);

        const ShaderAsset &asset() const { return m_asset; }

        // Look up a pass by name. Returns nullptr if the name is missing.
        const ShaderPass *findPass(TkStringView passName) const;

        // Convenience: returns the first pass, or nullptr if the asset has no
        // passes at all.
        const ShaderPass *getDefaultPass() const;

        // Create a GfxShader for the named pass. Passing an empty passName
        // falls back to getDefaultPass(). The returned shader is owned by the
        // caller (matches the existing device->createShader() semantics).
        GfxShader *createShaderForPass(GfxDevice *device, TkStringView passName) const;

        GfxShader *createShaderForDefaultPass(GfxDevice *device) const;

        // Editor / tooling accessors. These surface the reflection and
        // dependency closure baked into the .tksd so a host doesn't need to
        // re-parse the source: shader graphs / inspectors can read the
        // declared bindings/vertex inputs, and file watchers can subscribe to
        // every dependency at load time.
        const TkArray<ShaderBindingInfo> &bindings() const { return m_asset.bindings; }
        const TkArray<ShaderVertexInputInfo> &vertexInputs() const { return m_asset.vertexInputs; }
        const TkArray<TkString> &dependencies() const { return m_asset.dependencies; }

        const TkString &lastError() const { return m_lastError; }

    private:
        ShaderAsset m_asset;
        mutable TkString m_lastError;
    };
}
