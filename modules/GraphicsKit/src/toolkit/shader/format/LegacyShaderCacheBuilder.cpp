/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/format/LegacyShaderCacheBuilder.hpp>
#include <toolkit/shader/format/TksdFormat.hpp>

namespace toolkit
{
    bool LegacyShaderCacheBuilder::buildFromStages(
        const ShaderAsset &asset,
        const ShaderPass &pass,
        GfxShaderProfile profile,
        TkArray<byte> &outBlob)
    {
        outBlob.clear();

        if (pass.stageIndices.isEmpty())
        {
            m_lastError = TkString::format(
                "LegacyShaderCacheBuilder: pass '%s' has no stage entries",
                pass.name.isEmpty() ? "<unnamed>" : pass.name.get());
            return false;
        }

        // v1 stores one self-contained legacy bundle per (pass, profile).
        // A .tksd may hold several backend profiles; select the one required
        // by the active device rather than assuming the first entry belongs
        // to it.
        const ShaderStageData *stage = nullptr;
        for (i32 i = 0; i < pass.stageIndices.count(); i++)
        {
            const u32 stageIdx = pass.stageIndices[i];
            if (stageIdx >= static_cast<u32>(asset.stages.count()))
            {
                m_lastError = TkString::format(
                    "LegacyShaderCacheBuilder: stage index %u out of range (have %i stages)",
                    stageIdx, asset.stages.count());
                return false;
            }

            const ShaderStageData &candidate = asset.stages[stageIdx];
            if (candidate.profile != profile)
                continue;

            if (stage != nullptr)
            {
                m_lastError = TkString::format(
                    "LegacyShaderCacheBuilder: pass '%s' has multiple bundles for profile '%s'",
                    pass.name.isEmpty() ? "<unnamed>" : pass.name.get(),
                    toString(profile));
                return false;
            }
            stage = &candidate;
        }

        if (stage == nullptr)
        {
            m_lastError = TkString::format(
                "LegacyShaderCacheBuilder: pass '%s' has no bundle for profile '%s'",
                pass.name.isEmpty() ? "<unnamed>" : pass.name.get(),
                toString(profile));
            return false;
        }

        if (stage->bytecode.isEmpty())
        {
            m_lastError = TkString::format(
                "LegacyShaderCacheBuilder: pass '%s' stage payload is empty",
                pass.name.isEmpty() ? "<unnamed>" : pass.name.get());
            return false;
        }

        switch (stage->kind)
        {
        case TKSD_STAGE_KIND_RAW:
            // v1: payload is a self-contained `.shadercache`-wire-format
            // blob covering every stage of the pass. Identity-pass it
            // through so the runtime can call `GfxShader::create()` on it.
            // When the compiler grows true per-stage outputs this branch
            // will re-emit a fresh wire blob from VS+PS+CB metadata
            // instead -- the call site upstream will not have to change.
            outBlob.ensureCapacity(stage->bytecode.count(), false);
            for (i32 i = 0; i < stage->bytecode.count(); i++)
                outBlob.add(stage->bytecode[i]);
            return true;
        default:
            m_lastError = TkString::format(
                "LegacyShaderCacheBuilder: unsupported stage kind %u "
                "(only TKSD_STAGE_KIND_RAW is defined)", stage->kind);
            return false;
        }
    }
}
