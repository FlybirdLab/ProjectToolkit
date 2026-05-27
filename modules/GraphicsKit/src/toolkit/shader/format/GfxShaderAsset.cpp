/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/format/GfxShaderAsset.hpp>
#include <toolkit/shader/format/TksdReader.hpp>
#include <toolkit/shader/format/TksdFormat.hpp>
#include <toolkit/shader/format/LegacyShaderCacheBuilder.hpp>

#include <toolkit/graphics/GfxDevice.hpp>
#include <toolkit/graphics/GfxShader.hpp>
#include <toolkit/io/MemoryStream.hpp>

namespace toolkit
{
    bool GfxShaderAsset::loadFromFile(TkStringView path)
    {
        TksdReader reader;
        m_asset = ShaderAsset();
        const TksdReadResult r = reader.readFromFile(path, m_asset);
        if (r != TksdReadResult::Ok)
        {
            m_lastError = TkString::format("tksd read failed: %s (%s)",
                                           toString(r),
                                           reader.lastError().get());
            return false;
        }
        return true;
    }

    bool GfxShaderAsset::loadFromMemory(const byte *data, u32 size)
    {
        TksdReader reader;
        m_asset = ShaderAsset();
        const TksdReadResult r = reader.readFromMemory(data, size, m_asset);
        if (r != TksdReadResult::Ok)
        {
            m_lastError = TkString::format("tksd read failed: %s (%s)",
                                           toString(r),
                                           reader.lastError().get());
            return false;
        }
        return true;
    }

    const ShaderPass *GfxShaderAsset::findPass(TkStringView passName) const
    {
        for (i32 i = 0; i < m_asset.passes.count(); i++)
        {
            if (m_asset.passes[i].name == TkString(passName))
                return &m_asset.passes[i];
        }
        return nullptr;
    }

    const ShaderPass *GfxShaderAsset::getDefaultPass() const
    {
        if (m_asset.passes.isEmpty())
            return nullptr;
        return &m_asset.passes[0];
    }

    GfxShader *GfxShaderAsset::createShaderForDefaultPass(GfxDevice *device) const
    {
        return createShaderForPass(device, TkStringView());
    }

    GfxShader *GfxShaderAsset::createShaderForPass(GfxDevice *device, TkStringView passName) const
    {
        if (device == nullptr)
        {
            m_lastError = TkString("createShaderForPass: device is null");
            return nullptr;
        }
        if (m_asset.passes.isEmpty())
        {
            m_lastError = TkString("createShaderForPass: ShaderAsset has no passes");
            return nullptr;
        }

        const ShaderPass *pass = passName.length() > 0
            ? findPass(passName)
            : getDefaultPass();
        if (pass == nullptr)
        {
            m_lastError = TkString::format("createShaderForPass: pass '%.*s' not found",
                                           passName.length(), passName.get());
            return nullptr;
        }
        if (pass->stageIndices.isEmpty())
        {
            m_lastError = TkString::format("createShaderForPass: pass '%s' has no stages",
                                           pass->name.get());
            return nullptr;
        }

        // A pass may include bundles for multiple backend profiles. Select
        // only the payload compatible with the device currently loading it.
        LegacyShaderCacheBuilder builder;
        TkArray<byte> assembled;
        if (!builder.buildFromStages(m_asset, *pass, device->getShaderProfile(), assembled))
        {
            m_lastError = TkString::format(
                "createShaderForPass: LegacyShaderCacheBuilder failed: %s",
                builder.lastError().get());
            return nullptr;
        }
        if (assembled.isEmpty())
        {
            m_lastError = TkString("createShaderForPass: assembled shadercache is empty");
            return nullptr;
        }

        // Build a name for the shader -- helps the existing debug-layer hookups
        // identify resources without leaking the on-disk pass naming.
        const TkString shaderName = TkString::format("%s/%s",
            m_asset.name.isEmpty() ? "ShaderAsset" : m_asset.name.get(),
            pass->name.isEmpty() ? "Default" : pass->name.get());

        GfxShader *shader = device->createShader(TkStringView(shaderName));
        if (shader == nullptr)
        {
            m_lastError = TkString("createShaderForPass: device->createShader returned null");
            return nullptr;
        }

        MemoryIStream stream(assembled.get(), static_cast<u32>(assembled.count()));
        if (!shader->create(stream))
        {
            m_lastError = TkString::format("createShaderForPass: GfxShader::create failed for pass '%s'",
                                           pass->name.get());
            GFX_DEL_RESOURCE(shader);
            return nullptr;
        }
        return shader;
    }
}
