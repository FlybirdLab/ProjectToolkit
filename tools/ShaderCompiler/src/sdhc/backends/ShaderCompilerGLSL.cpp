/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "ShaderCompilerGLSL.hpp"

#include "sdhc/Context.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/GLShaderSourceFormat.hpp"
#include "toolkit/graphics/GfxShaderProgram.hpp"
#include "toolkit/memory/Memory.hpp"

namespace toolkit
{

    namespace
    {
        // Inline u32 -> 4 bytes little-endian writer. Avoids depending on host
        // endianness when emitting the GLShaderSourceHeader.
        void writeU32LE(byte *dst, u32 value)
        {
            dst[0] = static_cast<byte>(value & 0xffu);
            dst[1] = static_cast<byte>((value >> 8) & 0xffu);
            dst[2] = static_cast<byte>((value >> 16) & 0xffu);
            dst[3] = static_cast<byte>((value >> 24) & 0xffu);
        }

        // Locate the end-of-line for the GLSL `#version` directive inside the
        // source. Returns 0 when no `#version` line is found, in which case the
        // caller is expected to prepend the stage define at the start of the
        // source (this is invalid GLSL, but matches the user's intent of
        // forcing a clear runtime error rather than silently corrupting).
        u32 findVersionLineEnd(const char *src, u32 size)
        {
            constexpr const char *kVersionTag = "#version";
            constexpr u32 kVersionTagLen = 8u;
            if (size < kVersionTagLen)
                return 0;
            for (u32 i = 0; i + kVersionTagLen <= size; ++i)
            {
                // Match `#version` at start-of-line only (column 0 or after a
                // newline). Avoids false positives inside string-like content.
                if (i != 0 && src[i - 1] != '\n' && src[i - 1] != '\r')
                    continue;
                bool match = true;
                for (u32 k = 0; k < kVersionTagLen; ++k)
                {
                    if (src[i + k] != kVersionTag[k])
                    {
                        match = false;
                        break;
                    }
                }
                if (!match)
                    continue;

                // Found `#version` -- advance to end of this line (consume the
                // trailing newline so the injected define sits on its own line).
                u32 j = i + kVersionTagLen;
                while (j < size && src[j] != '\n')
                    ++j;
                if (j < size)
                    ++j; // include the '\n'
                return j;
            }
            return 0;
        }

        const char *stageDefineFor(ShaderStage stage)
        {
            // Single-source dual-stage discrimination contract:
            //   - VS bytecode gets TK_SHADER_STAGE_VERTEX defined
            //   - PS bytecode gets TK_SHADER_STAGE_PIXEL  defined
            // Authors use #ifdef in their GLSL to select per-stage code paths.
            switch (stage)
            {
            case ShaderStage::kVertex:
                return "#define TK_SHADER_STAGE_VERTEX 1\n";
            case ShaderStage::kPixel:
                return "#define TK_SHADER_STAGE_PIXEL 1\n";
            default:
                return nullptr;
            }
        }

        u32 strLen(const char *s)
        {
            u32 n = 0;
            while (s && s[n])
                ++n;
            return n;
        }
    }

    ShaderCompilerGLSL::ShaderCompilerGLSL(GfxShaderProfile profile)
        : ShaderCompiler(profile)
    {
    }

    bool ShaderCompilerGLSL::onCompileBegin()
    {
        if (!ShaderCompiler::onCompileBegin())
            return false;

        // Surface the profile to the GLSL source via a define. Users can guard
        // GLSL-only blocks with #ifdef GLSL_410 / TK_SHADER_PROFILE_GLSL_410.
        m_globalMacros.add({"GLSL_410", "1"});
        m_globalMacros.add({"TK_SHADER_PROFILE_GLSL_410", "1"});
        m_globalMacros.add({"OPENGL", "1"});

        return true;
    }

    bool ShaderCompilerGLSL::compileShader(ShaderFunctionMeta &meta, WritePermutationData customDataWrite)
    {
        if (!writeShaderFunctionBegin(m_context, meta))
            return false;

        const auto stage = meta.getStage();

        // Only VS/PS reach this fallback. Anything else is a hard error so the
        // user knows they hit the fallback ceiling rather than silently
        // producing an empty cache entry.
        switch (stage)
        {
        case ShaderStage::kVertex:
        case ShaderStage::kPixel:
            break;
        case ShaderStage::kHull:
        case ShaderStage::kDomain:
        case ShaderStage::kGeometry:
        case ShaderStage::kCompute:
            m_context->onError("OpenGL GLSL_410 fallback compiler only supports VS/PS in this stage.");
            return false;
        default:
            m_context->onError("OpenGL GLSL_410 fallback compiler: unknown shader stage.");
            return false;
        }

        auto *options = m_context->options;
        if (options->source == nullptr || options->srcLen == 0)
        {
            m_context->onError("OpenGL GLSL_410 fallback compiler: empty source.");
            return false;
        }

        const u32 sourceSize = options->srcLen;
        const char *stageDefine = stageDefineFor(stage);
        if (!stageDefine)
        {
            m_context->onError("OpenGL GLSL_410 fallback compiler: missing stage define mapping.");
            return false;
        }
        const u32 stageDefineLen = strLen(stageDefine);

        // Insert the stage define on the line immediately after `#version` so
        // a single GLSL source can serve both VS and PS via #ifdef guards.
        // GLSL spec requires `#version` to be the first non-comment directive,
        // so we must NOT prepend before it.
        const u32 versionEnd = findVersionLineEnd(options->source, sourceSize);
        const u32 transformedSize = sourceSize + stageDefineLen;
        const u32 totalSize = static_cast<u32>(sizeof(gfx::GLShaderSourceHeader)) + transformedSize;

        byte *blob = TkMemory::newArray<byte>(totalSize);
        if (!blob)
        {
            m_context->onError("OpenGL GLSL_410 fallback compiler: out of memory.");
            return false;
        }

        writeU32LE(blob + 0, gfx::GL_SHADER_SOURCE_MAGIC);
        writeU32LE(blob + 4, gfx::GL_SHADER_SOURCE_VERSION);
        writeU32LE(blob + 8, static_cast<u32>(m_profile));
        writeU32LE(blob + 12, transformedSize);

        byte *p = blob + sizeof(gfx::GLShaderSourceHeader);
        if (versionEnd > 0)
        {
            // Layout: <up-to-and-including-version-line> + <stage define> + <rest of source>
            TkMemory::memoryCopy(p, options->source, versionEnd);
            p += versionEnd;
            TkMemory::memoryCopy(p, stageDefine, stageDefineLen);
            p += stageDefineLen;
            TkMemory::memoryCopy(p, options->source + versionEnd, sourceSize - versionEnd);
        }
        else
        {
            // No #version line found. Prepend the stage define and forward
            // the source -- the GLSL compiler will likely error out on the
            // missing #version directive, which is the correct signal that
            // the source is malformed for this backend.
            LogWarn("OpenGL GLSL_410 fallback compiler: source has no '#version' directive; injecting stage define at start.");
            TkMemory::memoryCopy(p, stageDefine, stageDefineLen);
            p += stageDefineLen;
            TkMemory::memoryCopy(p, options->source, sourceSize);
        }

        // Walk permutations so VS additional data and the shader-cache wire
        // format stay aligned with Vulkan/D3D output. The bytecode is shared
        // across permutations; runtime selection via macros is the user's
        // responsibility.
        AdditionalDataVS additionalDataVS;
        for (i32 permutationIndex = 0; permutationIndex < meta.permutations.count(); permutationIndex++)
        {
            m_macros.clear();
            meta.getDefinitionsForPermutation(permutationIndex, m_macros);
            getDefineForFunction(meta, m_macros);
            m_macros.add(m_context->options->macros);
            m_macros.add(m_globalMacros);

            // bindings = all zero. The GL runtime binds CB0..CB15 / SR0..SR15
            // by uniform name in GLPipelineState::init regardless of these
            // masks, so leaving them empty is safe. A future phase that
            // introduces real GLSL reflection should populate them.
            ShaderBindings bindings = {0, 0, 0, 0};

            void *additionalData = nullptr;
            if (stage == ShaderStage::kVertex)
            {
                additionalData = &additionalDataVS;
                additionalDataVS.inputs.clear();
                // Passthrough mode has no source-level reflection, so we
                // intentionally emit an empty vertex input list. The runtime
                // vertex layout still comes from the bound vertex buffers
                // (see GfxVertexLayout::Merge in the runtime draw path).
            }

            if (!writeShaderFunctionPermutation(m_context, meta, permutationIndex, bindings, blob, static_cast<i32>(totalSize)))
            {
                TkMemory::deleteArray(blob, totalSize);
                return false;
            }

            if (customDataWrite && !customDataWrite(m_context, meta, permutationIndex, m_macros, additionalData))
            {
                TkMemory::deleteArray(blob, totalSize);
                return false;
            }
        }

        TkMemory::deleteArray(blob, totalSize);

        // Mark all declared constant buffers as "used" with unknown size.
        // Plan B cannot reflect the GLSL source, so we conservatively trust
        // META_CB_BEGIN markers parsed from the .shader file. Real sizes are
        // supplied at runtime by GLDevice::createConstantBuffer(size,...).
        for (i32 i = 0; i < m_constantBuffers.count(); i++)
        {
            auto &cb = m_constantBuffers[i];
            cb.isUsed = true;
            cb.size = 0;
        }

        return writeShaderFunctionEnd(m_context, meta);
    }

}
