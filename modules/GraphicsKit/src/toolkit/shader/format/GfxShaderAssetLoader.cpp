/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/format/GfxShaderAssetLoader.hpp>
#include <toolkit/shader/format/GfxShaderAsset.hpp>
#include <toolkit/shader/format/TksdReader.hpp>

namespace toolkit
{
    GfxShader *GfxShaderAssetLoader::loadShaderFromTksd(
        GfxDevice *device,
        TkStringView path,
        TkStringView passName,
        TkString *outError)
    {
        GfxShaderAsset asset;
        if (!asset.loadFromFile(path))
        {
            if (outError != nullptr)
                *outError = asset.lastError();
            return nullptr;
        }

        GfxShader *shader = asset.createShaderForPass(device, passName);
        if (shader == nullptr && outError != nullptr)
            *outError = asset.lastError();
        return shader;
    }

    bool GfxShaderAssetLoader::loadAssetFromTksd(
        TkStringView path,
        ShaderAsset &outAsset,
        TkString *outError)
    {
        TksdReader reader;
        outAsset = ShaderAsset();
        const TksdReadResult r = reader.readFromFile(path, outAsset);
        if (r == TksdReadResult::Ok)
            return true;
        if (outError != nullptr)
            *outError = TkString::format("tksd read failed: %s (%s)",
                                         toString(r),
                                         reader.lastError().get());
        return false;
    }
}
