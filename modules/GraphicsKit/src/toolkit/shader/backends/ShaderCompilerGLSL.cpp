/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "ShaderCompilerGLSL.hpp"

#include "toolkit/shader/core/Context.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/GLShaderSourceFormat.hpp"
#include "toolkit/graphics/GfxShaderProgram.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/platforms/FileSystem.hpp"
#include "toolkit/platforms/System.hpp"
#include "toolkit/types/Path.hpp"
#include "toolkit/io/MemoryStream.hpp"

#ifdef COMPILE_WITH_SPIRV_CROSS
// glslang's HLSL frontend is behind ENABLE_HLSL; the Vulkan backend defines
// this the same way (see ShaderCompilerVulkan.cpp). Must come before any
// glslang header include.
#define ENABLE_HLSL 1

// glslang -- compile HLSL/GLSL -> SPIR-V.
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

// SPIRV-Cross -- decompile SPIR-V -> GLSL 410.
#include <spirv_cross/spirv_glsl.hpp>

#include <cstring>
#include <string>
#include <vector>
#endif

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
        //
        // Per the GLSL spec, `#version` must be the first non-comment / non-
        // whitespace directive, but it MAY have leading inline whitespace on
        // its line -- which is the common case when the source comes from an
        // indented `GLSLPROGRAM` block inside a `.tkshader` manifest. The
        // matcher therefore walks line-by-line and skips leading spaces / tabs
        // before testing the `#version` prefix.
        u32 findVersionLineEnd(const char *src, u32 size)
        {
            constexpr const char *kVersionTag = "#version";
            constexpr u32 kVersionTagLen = 8u;

            u32 i = 0;
            while (i < size)
            {
                // Skip leading inline whitespace on this line.
                u32 lineStart = i;
                while (lineStart < size && (src[lineStart] == ' ' || src[lineStart] == '\t'))
                    ++lineStart;

                if (lineStart + kVersionTagLen <= size)
                {
                    bool match = true;
                    for (u32 k = 0; k < kVersionTagLen; ++k)
                    {
                        if (src[lineStart + k] != kVersionTag[k])
                        {
                            match = false;
                            break;
                        }
                    }
                    if (match)
                    {
                        // Advance to end of this line (include the trailing
                        // newline so the injected stage define sits on its
                        // own line below the directive).
                        u32 j = lineStart + kVersionTagLen;
                        while (j < size && src[j] != '\n')
                            ++j;
                        if (j < size)
                            ++j; // include the '\n'
                        return j;
                    }
                }

                // Advance to the next line.
                while (i < size && src[i] != '\n')
                    ++i;
                if (i < size)
                    ++i;
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

        // Quick heuristic: does this source look like HLSL? Anything WITHOUT
        // a `#version` directive (anywhere -- line-start with optional
        // leading whitespace) gets routed through the HLSL -> SPIR-V -> GLSL
        // cross compiler. Real GLSL must declare a `#version` per spec, so
        // the absence of one is a strong signal the author handed us HLSL.
        bool looksLikeHlsl(const char *src, u32 size)
        {
            return findVersionLineEnd(src, size) == 0;
        }

#ifdef COMPILE_WITH_SPIRV_CROSS
        // Glslang has a process-wide global init counter. Multiple compiles
        // are fine as long as Initialize/Finalize are paired.
        struct GlslangScope
        {
            GlslangScope() { glslang::InitializeProcess(); }
            ~GlslangScope() { glslang::FinalizeProcess(); }
        };

        // Compile HLSL source text -> SPIR-V bytecode using glslang's HLSL
        // frontend. Targets Vulkan 1.0 semantics (which is what glslang knows
        // how to emit); SPIRV-Cross will translate semantics during the next
        // step. Returns false on parse / link / SPIR-V emit failure and
        // routes the glslang diagnostic message into the compilation
        // context's error channel.
        bool hlslToSpirv(
            ShaderCompilationContext *context,
            ShaderStage stage,
            const char *source,
            i32 sourceLen,
            const char *entryName,
            std::vector<u32> &outSpirv)
        {
            EShLanguage lang;
            switch (stage)
            {
            case ShaderStage::kVertex:   lang = EShLangVertex; break;
            case ShaderStage::kPixel:    lang = EShLangFragment; break;
            case ShaderStage::kGeometry: lang = EShLangGeometry; break;
            case ShaderStage::kHull:     lang = EShLangTessControl; break;
            case ShaderStage::kDomain:   lang = EShLangTessEvaluation; break;
            case ShaderStage::kCompute:  lang = EShLangCompute; break;
            default:
                context->onError("OpenGL cross-compile: unsupported shader stage.");
                return false;
            }

            // glslang requires the process to be initialized. We pair Init
            // and Finalize within the helper's scope so it works even when
            // the Vulkan backend hasn't been touched yet this run.
            GlslangScope scope;

            glslang::TShader shader(lang);
            glslang::TProgram program;
            shader.setEntryPoint(entryName);
            shader.setSourceEntryPoint(entryName);

            const int len = sourceLen;
            const char *name = "<hlsl-for-glsl>";
            shader.setStringsWithLengthsAndNames(&source, &len, &name, 1);

            // Mirror the Vulkan backend's setup: HLSL input, SPIR-V output,
            // Vulkan-1.0 client semantics. We deliberately don't set
            // `setInvertY` here -- the OpenGL convention is Y-up which
            // matches HLSL's NDC, while Vulkan is Y-down (hence the flip
            // there).
            //
            // setAutoMapLocations/Bindings: unlike the Vulkan backend (which
            // hands the raw SPIR-V to the GL/Vk runtime and links by name),
            // this path goes through SPIRV-Cross to emit GLSL 410 source --
            // and GLSL 410 cross-stage in/out linkage is by `layout(location=N)`,
            // not by name. Auto-mapping ensures every varying and every
            // resource gets an explicit location/binding in the SPIR-V, so
            // SPIRV-Cross can faithfully emit matching `layout(...)` on both
            // sides. Without these, varyings would lose location annotations
            // through the round-trip and VS->PS linkage would silently break
            // on the driver side.
            shader.setHlslIoMapping(true);
            shader.setAutoMapLocations(true);
            shader.setAutoMapBindings(true);
            shader.setEnvInput(glslang::EShSourceHlsl, lang, glslang::EShClientVulkan, 100);
            shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
            shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

            const EShMessages messages = static_cast<EShMessages>(
                EShMsgReadHlsl | EShMsgSpvRules | EShMsgVulkanRules | EShMsgHlslLegalization);

            if (!shader.parse(GetDefaultResources(), 450, false, messages))
            {
                TkString msg = TkString::format(
                    "OpenGL cross-compile: HLSL parse failed for entry '%s':\n%s\n%s",
                    entryName, shader.getInfoLog(), shader.getInfoDebugLog());
                context->onError(msg.get());
                return false;
            }

            program.addShader(&shader);
            if (!program.link(messages))
            {
                TkString msg = TkString::format(
                    "OpenGL cross-compile: HLSL link failed for entry '%s':\n%s",
                    entryName, program.getInfoLog());
                context->onError(msg.get());
                return false;
            }
            if (program.getIntermediate(lang) == nullptr)
            {
                context->onError("OpenGL cross-compile: glslang produced no intermediate.");
                return false;
            }

            spv::SpvBuildLogger logger;
            glslang::SpvOptions spvOptions;
            spvOptions.generateDebugInfo = false;
            spvOptions.disassemble = false;
            spvOptions.disableOptimizer = true;
            spvOptions.optimizeSize = false;
            spvOptions.stripDebugInfo = true;
            spvOptions.validate = false;

            glslang::GlslangToSpv(*program.getIntermediate(lang), outSpirv, &logger, &spvOptions);
            const std::string log = logger.getAllMessages();
            if (!log.empty())
                LogWarn("OpenGL cross-compile SPIR-V log: %s", log.c_str());
            if (outSpirv.empty())
            {
                context->onError("OpenGL cross-compile: SPIR-V emitter produced nothing.");
                return false;
            }
            return true;
        }

        // Decompile SPIR-V to a single GLSL 410 source string via SPIRV-Cross.
        // The OpenGL semantics (no Vulkan descriptor sets, location-only
        // bindings) are toggled via Options::vulkan_semantics = false. The
        // returned source is plain GLSL ready for `glShaderSource`.
        //
        // Side effect: reflection from SPIRV-Cross's `get_shader_resources()`
        // is appended to `outBindings` (CB / SRV / UAV / Sampler) and
        // `outVertexInputs` (VS stage inputs only). Other stages append
        // nothing to `outVertexInputs`. These outputs are consumed by the
        // manifest compiler to populate the .tksd's Bindings / VertexInput
        // chunks so the runtime can validate ABI without re-parsing source.
        bool spirvToGlsl410(
            ShaderCompilationContext *context,
            const std::vector<u32> &spirv,
            const char *entryName,
            ShaderStage stage,
            std::string &outGlsl,
            TkArray<ShaderBindingInfo> &outBindings,
            TkArray<ShaderVertexInputInfo> &outVertexInputs)
        {
            spv::ExecutionModel execModel;
            switch (stage)
            {
            case ShaderStage::kVertex:   execModel = spv::ExecutionModelVertex; break;
            case ShaderStage::kPixel:    execModel = spv::ExecutionModelFragment; break;
            case ShaderStage::kGeometry: execModel = spv::ExecutionModelGeometry; break;
            case ShaderStage::kHull:     execModel = spv::ExecutionModelTessellationControl; break;
            case ShaderStage::kDomain:   execModel = spv::ExecutionModelTessellationEvaluation; break;
            case ShaderStage::kCompute:  execModel = spv::ExecutionModelGLCompute; break;
            default:
                context->onError("OpenGL cross-compile: unsupported execution model.");
                return false;
            }

            try
            {
                spirv_cross::CompilerGLSL compiler(spirv);

                spirv_cross::CompilerGLSL::Options opts;
                opts.version = 410;
                opts.es = false;
                opts.vulkan_semantics = false;
                opts.separate_shader_objects = false;
                opts.enable_420pack_extension = false;
                opts.flatten_multidimensional_arrays = false;
                opts.emit_push_constant_as_uniform_buffer = false;
                opts.emit_uniform_buffer_as_plain_uniforms = false;
                compiler.set_common_options(opts);
                compiler.set_entry_point(entryName, execModel);

                // Strip Vulkan descriptor-set decorations -- OpenGL has no
                // descriptor sets and SPIRV-Cross would otherwise leave
                // `layout(set=N, binding=M)` qualifiers that GLSL 410 doesn't
                // understand. We collapse to plain `binding = M` by clearing
                // the DescriptorSet decoration on every resource we see.
                spirv_cross::ShaderResources res = compiler.get_shader_resources();
                auto stripSet = [&](const spirv_cross::Resource &r) {
                    if (compiler.has_decoration(r.id, spv::DecorationDescriptorSet))
                        compiler.unset_decoration(r.id, spv::DecorationDescriptorSet);
                };
                for (const auto &r : res.uniform_buffers)       stripSet(r);
                for (const auto &r : res.storage_buffers)       stripSet(r);
                for (const auto &r : res.sampled_images)        stripSet(r);
                for (const auto &r : res.separate_images)       stripSet(r);
                for (const auto &r : res.separate_samplers)     stripSet(r);
                for (const auto &r : res.storage_images)        stripSet(r);
                for (const auto &r : res.subpass_inputs)        stripSet(r);

                // HLSL represents textures and samplers independently, while
                // desktop GLSL 410 samples through combined sampler uniforms.
                // Build those uniforms and name them from the SRV slot so the
                // OpenGL runtime can bind texture units through SR0..SR15.
                compiler.build_combined_image_samplers();
                for (const auto &combined : compiler.get_combined_image_samplers())
                {
                    const u32 slot = compiler.has_decoration(combined.image_id, spv::DecorationBinding)
                                         ? compiler.get_decoration(combined.image_id, spv::DecorationBinding)
                                         : 0;
                    compiler.set_name(combined.combined_id, std::string("SR") + std::to_string(slot));
                }

                // ---- Pull reflection out of the SPIRV-Cross resource list. ----
                //
                // We record (name, slot, stage, type) for every resource and
                // (name-from-semantic, location) for every VS stage input.
                // Type tags match the .tksd ShaderBindingInfo convention:
                //   "CB"      = constant buffer / uniform buffer
                //   "SRV"     = sampled texture / storage buffer (read)
                //   "UAV"     = storage image (read-write)
                //   "Sampler" = standalone sampler
                auto recordBinding = [&](const spirv_cross::Resource &r, const char *typeTag) {
                    ShaderBindingInfo info;
                    // SPIRV-Cross stores the "logical" resource name on `name`
                    // (the variable in HLSL), while `id` references the SSA
                    // ID. Prefer the variable name -- the manifest's UBO/SRV
                    // tables use names too.
                    info.name = TkString(r.name.c_str());
                    info.slot = compiler.has_decoration(r.id, spv::DecorationBinding)
                                ? compiler.get_decoration(r.id, spv::DecorationBinding)
                                : 0;
                    info.stage = stage;
                    info.type = TkString(typeTag);
                    outBindings.add(info);
                };
                for (const auto &r : res.uniform_buffers)   recordBinding(r, "CB");
                for (const auto &r : res.storage_buffers)   recordBinding(r, "SRV");
                for (const auto &r : res.sampled_images)    recordBinding(r, "SRV");
                for (const auto &r : res.separate_images)   recordBinding(r, "SRV");
                for (const auto &r : res.storage_images)    recordBinding(r, "UAV");
                for (const auto &r : res.separate_samplers) recordBinding(r, "Sampler");

                if (stage == ShaderStage::kVertex)
                {
                    // VS stage inputs are the only thing the runtime vertex
                    // buffer layout cares about. glslang's HLSL frontend
                    // mangles semantics into names like `in.var.POSITION` --
                    // we strip the prefix so the output reads as the bare
                    // HLSL semantic. Format / slot / offset stay 0 in v1
                    // (proper type reflection lands with the per-stage
                    // bytecode split).
                    for (const auto &in : res.stage_inputs)
                    {
                        ShaderVertexInputInfo info;
                        TkString rawName(in.name.c_str());
                        // Strip the conventional `in.var.` / `var.` prefix
                        // glslang emits when `setHlslIoMapping(true)` is on.
                        static const char *kPrefixes[] = {"in.var.", "var."};
                        for (const char *prefix : kPrefixes)
                        {
                            const u32 prefixLen = static_cast<u32>(::strlen(prefix));
                            if (rawName.length() > static_cast<i32>(prefixLen)
                                && ::strncmp(rawName.get(), prefix, prefixLen) == 0)
                            {
                                rawName = TkString(rawName.get() + prefixLen);
                                break;
                            }
                        }
                        info.semantic = rawName;
                        info.location = compiler.has_decoration(in.id, spv::DecorationLocation)
                                        ? compiler.get_decoration(in.id, spv::DecorationLocation)
                                        : 0;
                        info.format = 0;
                        info.slot = 0;
                        info.offset = 0;
                        outVertexInputs.add(info);
                    }
                }
                outGlsl = compiler.compile();
            }
            catch (const std::exception &e)
            {
                TkString msg = TkString::format("OpenGL cross-compile: SPIRV-Cross threw: %s", e.what());
                context->onError(msg.get());
                return false;
            }
            if (outGlsl.empty())
            {
                context->onError("OpenGL cross-compile: SPIRV-Cross produced empty GLSL.");
                return false;
            }
            return true;
        }
#endif // COMPILE_WITH_SPIRV_CROSS

        // Phase 8/9 debug aid: when `TK_SHADER_DUMP_INTERMEDIATE` is set in the
        // environment, write the FINAL glsl source (post-#version-anchor,
        // post-stage-define) to disk so the user can diff what we actually
        // hand to the GL driver. Filename:
        //   <env-dir>/<sanitized-target>_<stage>_<profile>.glsl
        // The env var must be an absolute or workdir-relative directory. If
        // the directory cannot be created the dump is silently skipped (we
        // don't want to fail the build over a debug hook).
        void dumpIntermediateGlsl(
            const char *targetName,
            ShaderStage stage,
            GfxShaderProfile profile,
            const byte *finalSource,
            u32 finalSize)
        {
            TkString dumpDir;
            if (!System::getEnvVar(TkStringView("TK_SHADER_DUMP_INTERMEDIATE"), dumpDir))
                return;
            if (dumpDir.isEmpty())
                return;

            TkPath::normalize(dumpDir);
            if (!TkFileSystem::isDirExist(dumpDir))
            {
                if (!TkFileSystem::makeDir(dumpDir))
                {
                    LogWarn("[ShaderDump] makeDir failed: %s", dumpDir.get());
                    return;
                }
            }

            // Sanitize the target name -- it can contain '/' or ':' from e.g.
            // "Sandbox/Triangle/Forward".
            TkString sanitized;
            for (i32 i = 0; targetName != nullptr && targetName[i] != '\0'; i++)
            {
                const char c = targetName[i];
                if (c == '/' || c == '\\' || c == ':' || c == ' ' || c == '"' || c == '*')
                    sanitized += TkString("_");
                else
                    sanitized += TkString(&c, 1);
            }
            if (sanitized.isEmpty())
                sanitized = TkString("shader");

            const char *stageTag = "stage";
            switch (stage)
            {
            case ShaderStage::kVertex:   stageTag = "VS"; break;
            case ShaderStage::kPixel:    stageTag = "PS"; break;
            case ShaderStage::kGeometry: stageTag = "GS"; break;
            case ShaderStage::kHull:     stageTag = "HS"; break;
            case ShaderStage::kDomain:   stageTag = "DS"; break;
            case ShaderStage::kCompute:  stageTag = "CS"; break;
            default:                     stageTag = "XX"; break;
            }

            const char *profileTag = "unknown";
            switch (profile)
            {
            case GfxShaderProfile::kGLSL_410:    profileTag = "glsl410"; break;
            case GfxShaderProfile::kVulkan_SM5:  profileTag = "vk_sm5";  break;
            case GfxShaderProfile::kDirectX_SM4: profileTag = "d3d_sm4"; break;
            case GfxShaderProfile::kDirectX_SM5: profileTag = "d3d_sm5"; break;
            case GfxShaderProfile::kDirectX_SM6: profileTag = "dx_sm6";  break;
            default: break;
            }

            const TkString fileName = TkString::format(
                "%s_%s_%s.glsl", sanitized.get(), stageTag, profileTag);
            const TkString outPath = dumpDir / fileName;

            MemoryOStream out(finalSize);
            if (finalSize > 0)
                out.writeBytes(finalSource, finalSize);
            if (out.saveToFile(outPath))
                LogInfo("[ShaderDump] %s (%u bytes)", outPath.get(), finalSize);
            else
                LogWarn("[ShaderDump] failed to write %s", outPath.get());
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

        // `effectiveSource` / `effectiveSize` point at what we actually pass
        // to the GL driver. For native GLSL input (the `GLSLPROGRAM` block
        // path) this aliases `options->source`. For HLSL input we route
        // through glslang + SPIRV-Cross below and rebind these pointers to
        // the cross-compiled GLSL.
        const char *effectiveSource = options->source;
        u32 effectiveSize = options->srcLen;

#ifdef COMPILE_WITH_SPIRV_CROSS
        std::string crossCompiled; // kept alive for the duration of this call
        if (looksLikeHlsl(options->source, options->srcLen))
        {
            // HLSL detected -> compile to SPIR-V via glslang then decompile
            // to GLSL 410 via SPIRV-Cross. The entry name comes from
            // `meta.name`, populated by the manifest's `Entry = "..."`.
            //
            // Reflection (bindings + vertex inputs) lands on the context's
            // own arrays. ShadersCompilation copies them into
            // `options.reflectionBindings` / `reflectionVertexInputs` after
            // the backend returns -- the context is the backend's writable
            // workspace, options is the caller-facing in/out bundle.

            std::vector<u32> spirv;
            if (!hlslToSpirv(m_context, stage, options->source,
                             static_cast<i32>(options->srcLen),
                             meta.name.isEmpty() ? "main" : meta.name.get(),
                             spirv))
            {
                return false;
            }
            if (!spirvToGlsl410(m_context, spirv,
                                meta.name.isEmpty() ? "main" : meta.name.get(),
                                stage,
                                crossCompiled,
                                m_context->reflectionBindings,
                                m_context->reflectionVertexInputs))
            {
                return false;
            }
            effectiveSource = crossCompiled.c_str();
            effectiveSize = static_cast<u32>(crossCompiled.size());
            LogInfo("HLSL -> SPIR-V -> GLSL 410 ok (entry=%s, %u bytes out, +%i bindings, +%i vertex inputs)",
                    meta.name.isEmpty() ? "main" : meta.name.get(),
                    effectiveSize,
                    m_context->reflectionBindings.count(),
                    m_context->reflectionVertexInputs.count());
        }
#else
        if (looksLikeHlsl(options->source, options->srcLen))
        {
            m_context->onError(
                "OpenGL backend received HLSL source but SPIRV-Cross is not linked. "
                "Rebuild TkShaderCompilerCore with COMPILE_WITH_SPIRV_CROSS or supply a "
                "GLSL `GLSLPROGRAM` block via the manifest's OpenGL SubShader.");
            return false;
        }
#endif

        const u32 sourceSize = effectiveSize;
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
        const u32 versionEnd = findVersionLineEnd(effectiveSource, sourceSize);
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
            TkMemory::memoryCopy(p, effectiveSource, versionEnd);
            p += versionEnd;
            TkMemory::memoryCopy(p, stageDefine, stageDefineLen);
            p += stageDefineLen;
            TkMemory::memoryCopy(p, effectiveSource + versionEnd, sourceSize - versionEnd);
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
            TkMemory::memoryCopy(p, effectiveSource, sourceSize);
        }

        // Phase 8/9 debug hook: dump the final GLSL the GL driver will see.
        // Opt-in via the TK_SHADER_DUMP_INTERMEDIATE env var; no-op otherwise.
        dumpIntermediateGlsl(
            options->targetName.get(),
            stage,
            m_profile,
            blob + sizeof(gfx::GLShaderSourceHeader),
            transformedSize);

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
