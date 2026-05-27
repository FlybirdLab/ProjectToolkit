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

    // Thin facade over GfxShaderAsset for sites that just want
    // "give me a GfxShader from this .tksd path" with no intermediate state.
    class GfxShaderAssetLoader
    {
    public:
        // Loads `path`, picks `passName` (default pass if empty), and creates
        // a GfxShader on the supplied device. Returns nullptr and writes the
        // reason into *outError on failure.
        static GfxShader *loadShaderFromTksd(
            GfxDevice *device,
            TkStringView path,
            TkStringView passName = TkStringView(),
            TkString *outError = nullptr);

        // Pure parse: load .tksd contents into a ShaderAsset without creating
        // any GPU resources. Useful for editors / tooling.
        static bool loadAssetFromTksd(
            TkStringView path,
            ShaderAsset &outAsset,
            TkString *outError = nullptr);
    };
}
