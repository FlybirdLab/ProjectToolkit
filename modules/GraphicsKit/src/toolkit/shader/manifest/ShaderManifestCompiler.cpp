/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/manifest/ShaderManifestCompiler.hpp>

#include <toolkit/shader/lexer/ShaderLexer.hpp>
#include <toolkit/shader/parser/ShaderParser.hpp>
#include <toolkit/shader/sema/ShaderSema.hpp>
#include <toolkit/shader/source/SourceManager.hpp>
#include <toolkit/shader/source/IncludeResolver.hpp>
#include <toolkit/shader/diagnostics/DiagnosticSink.hpp>
#include <toolkit/shader/diagnostics/Diagnostic.hpp>
#include <toolkit/shader/core/Compilation.hpp>
#include <toolkit/shader/format/ShaderAsset.hpp>
#include <toolkit/shader/format/TksdWriter.hpp>
#include <toolkit/shader/format/TksdFormat.hpp>
#include <toolkit/shader/compilegraph/ShaderCompileJob.hpp>
#include <toolkit/shader/compilegraph/DependencyScanner.hpp>
#include <toolkit/shader/cache/ShaderJobCache.hpp>
#include <toolkit/shader/cache/ShaderHash.hpp>

#include <toolkit/debug/Logger.hpp>
#include <toolkit/io/MemoryStream.hpp>
#include <toolkit/platforms/FileSystem.hpp>
#include <toolkit/types/Path.hpp>
#include <toolkit/types/UUID.hpp>

namespace toolkit
{
    namespace
    {
        const char *profileShortName(GfxShaderProfile profile)
        {
            switch (profile)
            {
            case GfxShaderProfile::kDirectX_SM4: return "d3d_sm4";
            case GfxShaderProfile::kDirectX_SM5: return "d3d_sm5";
            case GfxShaderProfile::kDirectX_SM6: return "dx_sm6";
            case GfxShaderProfile::kVulkan_SM5:  return "vk_sm5";
            case GfxShaderProfile::kGLSL_410:    return "glsl410";
            default:                             return "unknown";
            }
        }

        // Map CLI profile -> the manifest backend identifier expected in
        // Target { ... }. Keep this in sync with ShaderSema::isKnownBackend.
        const char *manifestBackendFor(GfxShaderProfile profile)
        {
            switch (profile)
            {
            case GfxShaderProfile::kDirectX_SM4:
            case GfxShaderProfile::kDirectX_SM5: return "dx11";
            case GfxShaderProfile::kDirectX_SM6: return "dx12";
            case GfxShaderProfile::kVulkan_SM5:  return "vulkan";
            case GfxShaderProfile::kGLSL_410:    return "opengl";
            default:                             return "";
            }
        }

        // Embedded-source META stub. Phase 2 of GraphicsKit already injects
        // TK_SHADER_STAGE_VERTEX / TK_SHADER_STAGE_PIXEL into the GLSL source
        // right after #version, so all we need to do here is give the existing
        // Parser virtual entry points it can latch onto.
        const char *kGlslMetaStub =
            "#define META_VS(visible, level)\n"
            "#define META_PS(visible, level)\n"
            "#define META_CB_BEGIN(slot, name)\n"
            "#define META_CB_END\n"
            "#define META_FLAG(flag)\n"
            "META_VS(true, FEATURE_LEVEL_SM5)\n"
            "void VSMain() {}\n"
            "META_PS(true, FEATURE_LEVEL_SM5)\n"
            "void PSMain() {}\n";

        // Splice the META stub into a GLSL source so it sits right after the
        // #version directive (GLSL forbids anything before #version).
        TkString wrapEmbeddedGlsl(const TkString &userSource)
        {
            const char *data = userSource.get();
            const i32 length = userSource.length();

            // Find the end of the first non-blank/non-comment line. If it is
            // #version, splice the stub right after; otherwise prepend.
            i32 lineStart = 0;
            i32 i = 0;
            // Skip leading whitespace.
            while (i < length && (data[i] == ' ' || data[i] == '\t' || data[i] == '\r' || data[i] == '\n'))
                i++;
            lineStart = i;
            // Scan to end of that line.
            while (i < length && data[i] != '\n')
                i++;

            const bool hasVersion = (i > lineStart + 8)
                && data[lineStart] == '#'
                && (data[lineStart + 1] == 'v' || data[lineStart + 1] == 'V');
            if (hasVersion)
            {
                const i32 insertAt = (i < length) ? i + 1 : length;
                TkString head(data, insertAt);
                TkString tail(data + insertAt, length - insertAt);
                return head + TkString("\n") + TkString(kGlslMetaStub) + tail;
            }

            return TkString(kGlslMetaStub) + TkString("\n") + userSource;
        }

        bool isProfileCompatibleWithEmbedded(GfxShaderProfile profile, ShaderSourceLanguage lang)
        {
            switch (lang)
            {
            case ShaderSourceLanguage::GLSL:
                return profile == GfxShaderProfile::kGLSL_410;
            case ShaderSourceLanguage::HLSL:
                return profile == GfxShaderProfile::kDirectX_SM4
                    || profile == GfxShaderProfile::kDirectX_SM5
                    || profile == GfxShaderProfile::kDirectX_SM6
                    || profile == GfxShaderProfile::kVulkan_SM5;
            case ShaderSourceLanguage::MSL:
                return false; // No Metal backend in Phase 1-4.
            default:
                return false;
            }
        }

        // Pick the SubShader to use for a given backend. We use the first
        // SubShader that explicitly declares that backend in its Target block;
        // returns -1 if none match.
        i32 pickSubShaderIndex(const ShaderDecl &shader, const char *backend)
        {
            for (i32 i = 0; i < shader.subShaders.count(); i++)
            {
                const SubShaderDecl &sub = shader.subShaders[i];
                for (i32 j = 0; j < sub.target.entries.count(); j++)
                {
                    if (sub.target.entries[j].backend == backend)
                        return i;
                }
            }
            return -1;
        }

        TkString sanitizeForFileName(const TkString &in)
        {
            // Replace path separators and quotes so a shader named "Foo/Bar"
            // doesn't produce a weird output file name.
            TkString out;
            for (i32 i = 0; i < in.length(); i++)
            {
                const char c = in[i];
                if (c == '/' || c == '\\' || c == ' ' || c == '"' || c == ':' || c == '*')
                    out += TkString("_");
                else
                    out += TkString(&c, 1);
            }
            return out;
        }
    }

    bool ShaderManifestCompiler::compile(
        const ShaderManifestCompileOptions &options,
        ShaderManifestCompileResult &outResult)
    {
        outResult.caseCount = 0;
        outResult.failedCount = 0;

        SourceManager *sources = options.sources;
        DiagnosticSink *sink = options.diagnostics;
        if (sources == nullptr || sink == nullptr)
        {
            // Pure programming error; the CLI must always provide both.
            LogError("[ShaderManifestCompiler] sources/diagnostics required");
            return false;
        }

        // ---- 1. Load the manifest into the SourceManager. ----
        SourceFile *manifestFile = sources->loadFile(options.manifestPath);
        if (manifestFile == nullptr)
        {
            sink->error(
                SourceRange::invalid(),
                TkStringView("TKSHD0001"),
                TkStringView(TkString::format("failed to read manifest: %s", options.manifestPath.get())));
            return false;
        }

        // ---- 2. Lex + parse + sema. ----
        ShaderLexer lexer(manifestFile, sink);
        ShaderParser parser(lexer, sink);

        ShaderDecl decl;
        if (!parser.parse(decl))
            return false;

        ShaderSema sema(sink);
        if (!sema.validate(decl))
            return false;

        // ---- 3. For each selected CLI profile, drive the legacy compile path. ----
        const TkString manifestDir(TkPath::getDir(TkStringView(options.manifestPath)));
        const TkString shaderBaseName = sanitizeForFileName(decl.name);

        // v1 stops short of variant expansion -- if the manifest declares any
        // Variants, surface the fact that we're silently picking the "all-off"
        // permutation so users don't believe their `Variants { ... }` block
        // is producing per-variant bytecode.
        if (!decl.variants.isEmpty())
        {
            sink->warning(
                decl.range,
                TkStringView(TKSHD_FEATURE_NOT_IMPLEMENTED),
                TkStringView(TkString::format(
                    "manifest declares %i variant(s) but variant expansion is not implemented in v1; "
                    "compiling the 'all-off' permutation only. See VariantExpander for the future seam.",
                    decl.variants.count())));
        }

        // ---- Include resolver shared across all passes in this manifest.
        //      Order: manifest dir first (so siblings of the .tkshader resolve
        //      without -I), then user-supplied -I dirs in CLI order.
        //      Resolver also tries `currentFile`'s dir at resolve time, which
        //      gives external sources a relative-to-self lookup automatically.
        IncludeResolver includeResolver;
        if (!manifestDir.isEmpty())
            includeResolver.addIncludeDirectory(TkStringView(manifestDir));
        for (i32 i = 0; i < options.includeDirs.count(); i++)
            includeResolver.addIncludeDirectory(TkStringView(options.includeDirs[i]));

        // Snapshot the normalized include dir list once -- this is what we
        // mix into the job hash.
        TkArray<TkString> hashIncludeDirs;
        const TkArray<TkString> &resolverDirs = includeResolver.getIncludeDirectories();
        for (i32 i = 0; i < resolverDirs.count(); i++)
            hashIncludeDirs.add(resolverDirs[i]);

        // Accumulator used only when emitting a .tksd container. For
        // ShaderCache mode we still touch it (cheap), but skip the final
        // write at the end.
        ShaderAsset accumulatedAsset;
        accumulatedAsset.name = decl.name;
        for (i32 i = 0; i < decl.properties.count(); i++)
        {
            ShaderProperty out;
            out.type = decl.properties[i].type;
            out.name = decl.properties[i].name;
            out.defaultValue = decl.properties[i].defaultValue;
            accumulatedAsset.properties.add(out);
        }
        for (i32 i = 0; i < decl.tags.count(); i++)
        {
            ShaderTag out;
            out.key = decl.tags[i].key;
            out.value = decl.tags[i].value;
            accumulatedAsset.tags.add(out);
        }
        // Map (subIndex * 1024 + passIndex) -> accumulatedAsset.passes index.
        TkArray<u32> passKeyArr;
        TkArray<i32> passSlotArr;
        auto findOrAddPass = [&](i32 subIndex, i32 passIndex, const PassDecl &pass) -> i32
        {
            const u32 key = static_cast<u32>(subIndex * 1024 + passIndex);
            for (i32 i = 0; i < passKeyArr.count(); i++)
            {
                if (passKeyArr[i] == key)
                    return passSlotArr[i];
            }
            ShaderPass out;
            out.name = pass.name;
            for (i32 i = 0; i < pass.tags.count(); i++)
            {
                ShaderTag tag;
                tag.key = pass.tags[i].key;
                tag.value = pass.tags[i].value;
                out.tags.add(tag);
            }
            for (i32 i = 0; i < pass.renderStates.count(); i++)
            {
                ShaderRenderState rs;
                rs.name = pass.renderStates[i].name;
                rs.value = pass.renderStates[i].value;
                out.renderStates.add(rs);
            }
            accumulatedAsset.passes.add(out);
            const i32 slot = accumulatedAsset.passes.count() - 1;
            passKeyArr.add(key);
            passSlotArr.add(slot);
            return slot;
        };

        bool anyAttempted = false;

        for (i32 p = 0; p < options.profiles.count(); p++)
        {
            const GfxShaderProfile profile = options.profiles[p];
            const char *backend = manifestBackendFor(profile);
            if (backend == nullptr || backend[0] == 0)
            {
                sink->warning(
                    decl.range,
                    TkStringView("TKSHD0003"),
                    TkStringView(TkString::format(
                        "no manifest backend mapping for profile %s", toolkit::toString(profile))));
                continue;
            }

            const i32 subIndex = pickSubShaderIndex(decl, backend);
            if (subIndex < 0)
            {
                sink->warning(
                    decl.range,
                    TkStringView("TKSHD1006"),
                    TkStringView(TkString::format(
                        "no SubShader targets backend '%s' -- skipping %s", backend, profileShortName(profile))));
                continue;
            }
            const SubShaderDecl &sub = decl.subShaders[subIndex];

            for (i32 pi = 0; pi < sub.passes.count(); pi++)
            {
                const PassDecl &pass = sub.passes[pi];
                outResult.caseCount++;
                anyAttempted = true;

                // ---- 3a. Construct the source string for this pass. ----
                TkString effectiveSource;
                u32 sourceFileId = manifestFile->getId();
                // `scanAnchorPath` is what DependencyScanner uses to seed
                // relative include resolution. For external sources this is
                // the absolute path on disk; for embedded sources it's the
                // virtual file name we synthesize below.
                TkString scanAnchorPath;
                // `cacheSourcePath` is what the cache key includes as the
                // "where did this source come from" identifier. Distinct
                // from scanAnchorPath because the anchor for embedded blobs
                // is virtual.
                TkString cacheSourcePath;

                // Entry points pulled from the manifest. Populated by both
                // the embedded-source path (fixed convention: VSMain/PSMain
                // under #ifdef gates) and the external-source path (whatever
                // `Vertex { Entry = "..." }` declared). Threaded into
                // ShadersCompilation::compile so it can bypass the legacy
                // META-marker scanner -- which can't name-extract from
                // hand-authored HLSL without META decorators.
                TkArray<PrebuiltShaderEntry> manifestEntries;

                if (!pass.embeddedSources.isEmpty())
                {
                    // Pick the first embedded block compatible with the chosen profile.
                    const EmbeddedSourceDecl *picked = nullptr;
                    for (i32 e = 0; e < pass.embeddedSources.count(); e++)
                    {
                        if (isProfileCompatibleWithEmbedded(profile, pass.embeddedSources[e].language))
                        {
                            picked = &pass.embeddedSources[e];
                            break;
                        }
                    }
                    if (picked == nullptr)
                    {
                        sink->error(
                            pass.range,
                            TkStringView("TKSHD1009"),
                            TkStringView(TkString::format(
                                "Pass '%s' has no embedded source compatible with profile %s",
                                pass.name.isEmpty() ? "<unnamed>" : pass.name.get(),
                                profileShortName(profile))));
                        outResult.failedCount++;
                        continue;
                    }

                    if (picked->language == ShaderSourceLanguage::GLSL)
                    {
                        effectiveSource = wrapEmbeddedGlsl(picked->sourceText);
                        // Convention: embedded GLSL blocks always expose
                        // VSMain / PSMain entry points, with stage selection
                        // driven by TK_SHADER_STAGE_VERTEX / _PIXEL #ifdef
                        // gates injected by ShaderCompilerGLSL.
                        PrebuiltShaderEntry vsEntry;
                        vsEntry.stage = ShaderStage::kVertex;
                        vsEntry.name = TkString("VSMain");
                        manifestEntries.add(vsEntry);
                        PrebuiltShaderEntry psEntry;
                        psEntry.stage = ShaderStage::kPixel;
                        psEntry.name = TkString("PSMain");
                        manifestEntries.add(psEntry);
                    }
                    else if (picked->language == ShaderSourceLanguage::HLSL)
                    {
                        // First-version policy: inline HLSL not supported.
                        // The runtime test setup uses external Source/Entry
                        // for HLSL; embedded HLSL would require a parser
                        // upgrade to discover VSMain/PSMain conventions and
                        // isn't worth doing until someone needs it.
                        sink->error(
                            picked->range,
                            TkStringView("TKSHD1009"),
                            TkStringView("embedded HLSL is not supported yet -- use external Source/Entry"));
                        outResult.failedCount++;
                        continue;
                    }
                    else
                    {
                        sink->error(
                            picked->range,
                            TkStringView("TKSHD1009"),
                            TkStringView(TkString::format(
                                "embedded %s source not supported in this build",
                                toString(picked->language))));
                        outResult.failedCount++;
                        continue;
                    }

                    // Register the wrapped source as a virtual file so future
                    // diagnostics can point users at "manifest/pass" instead of
                    // the raw .tkshader range.
                    const TkString virtualName = TkString::format(
                        "%s/%s.%s",
                        shaderBaseName.isEmpty() ? "shader" : shaderBaseName.get(),
                        pass.name.isEmpty() ? "pass" : sanitizeForFileName(pass.name).get(),
                        toString(picked->language));
                    SourceFile *virt = sources->addVirtualFile(virtualName, effectiveSource);
                    if (virt != nullptr)
                        sourceFileId = virt->getId();
                    scanAnchorPath = TkString(manifestDir) / virtualName;
                    TkPath::normalize(scanAnchorPath);
                    cacheSourcePath = virtualName;
                }
                else
                {
                    // External Source / Entry. First-version policy: all stages
                    // in a pass must reference the same Source file. Each
                    // stage's `Entry = "..."` populates manifestEntries; with
                    // that the compiler no longer scans for META_VS / META_PS
                    // markers.
                    if (pass.stages.isEmpty())
                    {
                        sink->error(
                            pass.range,
                            TkStringView("TKSHD1008"),
                            TkStringView("pass has neither embedded source nor stage declarations"));
                        outResult.failedCount++;
                        continue;
                    }

                    TkString firstSource;
                    bool stagesAgree = true;
                    for (i32 s = 0; s < pass.stages.count(); s++)
                    {
                        const StageDecl &st = pass.stages[s];
                        if (firstSource.isEmpty())
                            firstSource = st.source;
                        else if (st.source != firstSource)
                        {
                            stagesAgree = false;
                            sink->error(
                                st.range,
                                TkStringView("TKSHD1008"),
                                TkStringView(
                                    "stages with different Source files in one Pass are not supported yet "
                                    "(first version requires a single shared HLSL/GLSL file)"));
                        }

                        if (st.entry.isEmpty())
                        {
                            sink->error(
                                st.range,
                                TkStringView("TKSHD1008"),
                                TkStringView("stage declaration missing `Entry = \"...\";`"));
                            stagesAgree = false;
                        }
                        else
                        {
                            PrebuiltShaderEntry pe;
                            pe.stage = st.stage;
                            pe.name = st.entry;
                            manifestEntries.add(pe);
                        }
                    }
                    if (!stagesAgree)
                    {
                        outResult.failedCount++;
                        continue;
                    }

                    TkString resolved = TkString(manifestDir) / firstSource;
                    TkPath::normalize(resolved);
                    if (!TkFileSystem::isFileExist(resolved))
                    {
                        sink->error(
                            pass.stages[0].range,
                            TkStringView("TKSHD0001"),
                            TkStringView(TkString::format("shader source not found: %s", resolved.get())));
                        outResult.failedCount++;
                        continue;
                    }

                    SourceFile *src = sources->loadFile(resolved);
                    if (src == nullptr)
                    {
                        sink->error(
                            pass.stages[0].range,
                            TkStringView("TKSHD0001"),
                            TkStringView(TkString::format("failed to read shader source: %s", resolved.get())));
                        outResult.failedCount++;
                        continue;
                    }

                    effectiveSource = src->getContent();
                    sourceFileId = src->getId();
                    scanAnchorPath = resolved;
                    cacheSourcePath = resolved;
                }

                // ---- 3a'. Dependency scan (Phase 8). ----
                //   Recursively walks #include "..." / #include <...> from
                //   the effective source, resolves each through the shared
                //   IncludeResolver, and records (path, contentHash) pairs.
                //   Failure modes:
                //     - Missing include  -> TKSHD0301 + outResult.failedCount++
                //     - Cycle            -> TKSHD0302 + outResult.failedCount++
                //   The scanner still walks the whole tree even on failure so
                //   the user gets a complete list of broken includes in one
                //   pass instead of one-per-rerun.
                DependencyScanner depScanner;
                DependencyScanResult depResult = depScanner.scan(
                    *sources, includeResolver,
                    TkStringView(scanAnchorPath),
                    TkStringView(effectiveSource));
                outResult.dependencyTotalCount += depResult.dependencies.count();
                for (i32 i = 0; i < depResult.missingIncludes.count(); i++)
                {
                    outResult.includeMissingCount++;
                    sink->error(
                        SourceRange::invalid(),
                        TkStringView(TKSHD_INCLUDE_NOT_FOUND2),
                        TkStringView(TkString::format(
                            "include file not found: %s (while scanning '%s')",
                            depResult.missingIncludes[i].get(),
                            scanAnchorPath.isEmpty() ? "<inline>" : scanAnchorPath.get())));
                }
                for (i32 i = 0; i < depResult.includeCycles.count(); i++)
                {
                    outResult.includeCycleCount++;
                    sink->error(
                        SourceRange::invalid(),
                        TkStringView(TKSHD_INCLUDE_CYCLE),
                        TkStringView(TkString::format(
                            "include cycle detected: %s",
                            depResult.includeCycles[i].get())));
                }
                if (!depResult.success)
                {
                    outResult.failedCount++;
                    continue;
                }

                // ---- 3b. Hand the effective source to the legacy pipeline. ----
                MemoryOStream output(1024 * 1024 * 8);

                ShaderCompilationOptions compileOpts;
                compileOpts.targetName = pass.name.isEmpty()
                    ? (decl.name.isEmpty() ? TkString("ShaderManifest") : decl.name)
                    : (decl.name + TkString("/") + pass.name);
                compileOpts.targetID = TkUuid(
                    0x544B5348, // 'TKSH'
                    static_cast<u32>(options.inputIndex + 1),
                    static_cast<u32>(profile),
                    static_cast<u32>(0x20260000 | ((subIndex & 0xff) << 8) | (pi & 0xff)));
                compileOpts.source = effectiveSource.get();
                compileOpts.srcLen = effectiveSource.length();
                compileOpts.profile = profile;
                compileOpts.noOptimize = true;
                compileOpts.generateDebugData = true;
                compileOpts.treatWarningsAsErrors = true;
                compileOpts.output = &output;
                compileOpts.sourceManager = sources;
                compileOpts.diagnostics = sink;
                compileOpts.primaryFileId = sourceFileId;
                compileOpts.includeDirs = hashIncludeDirs;
                compileOpts.prebuiltEntries = manifestEntries;

                LogInfo("[ShaderManifestCompiler] compiling '%s' pass '%s' as %s",
                        decl.name, pass.name, profileShortName(profile));

                // ---- 3b'. Job cache lookup (Phase 6B, extended in Phase 8). ----
                //   The job descriptor carries every input that influences the
                //   compiled bytecode: source text, dependency closure, include
                //   dirs, defines, profile/stage/entry. ShaderHash::hashJob mixes
                //   all of them so editing an included header invalidates the
                //   cache automatically.
                ShaderCompileJob job;
                job.shaderName = decl.name;
                job.passName = pass.name;
                job.stage = ShaderStage::kVertex; // job-level (per pass, not per stage in v1)
                job.profile = profile;
                job.sourcePath = cacheSourcePath;
                job.virtualSourceName = TkString::format("%s/%s",
                    shaderBaseName.isEmpty() ? "shader" : shaderBaseName.get(),
                    pass.name.isEmpty() ? "pass" : pass.name.get());
                job.sourceText = effectiveSource;
                job.includeDirs = hashIncludeDirs;
                job.dependencies = depResult.dependencies;
                job.dependencyHash = depResult.dependencyHash;

                bool cacheHit = false;
                CachedShaderJobResult cached;
                if (options.cache != nullptr)
                {
                    if (options.cache->tryLoad(job, cached))
                    {
                        // Replay the cached bytecode back into `output`.
                        // Downstream code (legacy save / tksd accumulator)
                        // reads from `output.getHandle()` so this is byte-
                        // equal to the fresh-compile path. Reflection and
                        // dependencies come from `cached` directly.
                        output.reset(static_cast<u32>(cached.bytecode.count()));
                        if (!cached.bytecode.isEmpty())
                            output.writeBytes(cached.bytecode.get(),
                                              static_cast<u32>(cached.bytecode.count()));
                        outResult.cacheHit++;
                        cacheHit = true;
                        LogInfo("[ShaderManifestCompiler] cache hit '%s' (%i bytes, %i bindings, %i vertex inputs, %i deps)",
                                pass.name.get(),
                                output.getPosition(),
                                cached.reflectionBindings.count(),
                                cached.reflectionVertexInputs.count(),
                                cached.dependencies.count());
                    }
                    else
                    {
                        outResult.cacheMiss++;
                    }
                }

                if (!cacheHit)
                {
                    if (!ShadersCompilation::compile(compileOpts))
                    {
                        outResult.failedCount++;
                        continue;
                    }
                    outResult.compiled++;

                    // Build the cache result from the fresh compile. The
                    // reflection arrays on compileOpts were populated by
                    // backends that have a reflection-capable IR (today
                    // only the OpenGL HLSL->SPIRV-Cross path). Dependencies
                    // come from the dep scan above so they survive a cache
                    // hit on the next run.
                    cached.bytecode.clear();
                    cached.bytecode.ensureCapacity(static_cast<i32>(output.getPosition()), false);
                    {
                        const byte *src = output.getHandle();
                        for (u32 b = 0; b < output.getPosition(); b++)
                            cached.bytecode.add(src[b]);
                    }
                    cached.reflectionBindings = compileOpts.reflectionBindings;
                    cached.reflectionVertexInputs = compileOpts.reflectionVertexInputs;
                    cached.dependencies.clear();
                    for (i32 i = 0; i < depResult.dependencies.count(); i++)
                        cached.dependencies.add(depResult.dependencies[i].resolvedPath);

                    if (options.cache != nullptr)
                    {
                        // `job` was already populated above and its hash was
                        // computed on tryLoad; the store call reuses it.
                        options.cache->store(job, cached);
                    }
                }

                // Every (pass, profile) emits a TKSD_STAGE_KIND_RAW entry.
                // The bytecode payload is a self-contained `.shadercache`-wire-
                // format blob covering every stage of the pass; the runtime's
                // LegacyShaderCacheBuilder is the chokepoint that turns it
                // back into something `GfxShader::create()` can read.
                ShaderStageData stageData;
                stageData.kind = TKSD_STAGE_KIND_RAW;
                stageData.stage = ShaderStage::kVertex; // per-pass marker; per-stage split deferred
                stageData.profile = profile;
                stageData.entry = TkString();
                const u32 byteCount = output.getPosition();
                stageData.bytecode.ensureCapacity(static_cast<i32>(byteCount), false);
                const byte *src = output.getHandle();
                for (u32 b = 0; b < byteCount; b++)
                    stageData.bytecode.add(src[b]);
                accumulatedAsset.stages.add(stageData);
                const i32 stageSlot = accumulatedAsset.stages.count() - 1;

                const i32 passSlot = findOrAddPass(subIndex, pi, pass);
                accumulatedAsset.passes[passSlot].stageIndices.add(static_cast<u32>(stageSlot));

                // ---- Reflection sidechannel. ----
                //
                // Backends that produce reflection-capable IR (currently just
                // the OpenGL HLSL->SPIRV-Cross path) populate
                // `cached.reflectionBindings` / `reflectionVertexInputs` on
                // fresh compile; cache hits load the same arrays from the
                // .jobcache file. Either path produces the same data, so
                // hit/miss don't diverge in the resulting .tksd.
                //
                // The .tksd has one combined bindings / vertexInputs list,
                // not per-stage, so dedupe by (name, slot, stage, type) /
                // (semantic, location) to avoid the same CB showing up once
                // per stage that referenced it.
                for (i32 i = 0; i < cached.reflectionBindings.count(); i++)
                {
                    const ShaderBindingInfo &incoming = cached.reflectionBindings[i];
                    bool already = false;
                    for (i32 j = 0; j < accumulatedAsset.bindings.count(); j++)
                    {
                        const ShaderBindingInfo &existing = accumulatedAsset.bindings[j];
                        if (existing.name == incoming.name
                            && existing.slot == incoming.slot
                            && existing.stage == incoming.stage
                            && existing.type == incoming.type)
                        {
                            already = true;
                            break;
                        }
                    }
                    if (!already)
                        accumulatedAsset.bindings.add(incoming);
                }
                for (i32 i = 0; i < cached.reflectionVertexInputs.count(); i++)
                {
                    const ShaderVertexInputInfo &incoming = cached.reflectionVertexInputs[i];
                    bool already = false;
                    for (i32 j = 0; j < accumulatedAsset.vertexInputs.count(); j++)
                    {
                        const ShaderVertexInputInfo &existing = accumulatedAsset.vertexInputs[j];
                        if (existing.semantic == incoming.semantic
                            && existing.location == incoming.location)
                        {
                            already = true;
                            break;
                        }
                    }
                    if (!already)
                        accumulatedAsset.vertexInputs.add(incoming);
                }

                // ---- Dependency closure recording. ----
                //
                // Cache miss: `cached.dependencies` was built from
                // `depResult.dependencies` before the store. Cache hit: it
                // was loaded from the .jobcache file. Either way it matches
                // what the dep scan would have produced (the scan also ran
                // before lookup to compute the cache key, so we double-check
                // by preferring `cached.dependencies` -- the authoritative
                // record of "what this entry was built against"). Dedupe
                // across all (pass, profile) iterations.
                for (i32 i = 0; i < cached.dependencies.count(); i++)
                {
                    const TkString &p = cached.dependencies[i];
                    if (p.isEmpty())
                        continue;
                    bool already = false;
                    for (i32 j = 0; j < accumulatedAsset.dependencies.count(); j++)
                    {
                        if (accumulatedAsset.dependencies[j] == p)
                        {
                            already = true;
                            break;
                        }
                    }
                    if (!already)
                        accumulatedAsset.dependencies.add(p);
                }

                LogInfo("[ShaderManifestCompiler] queued pass '%s' (%i bytes) for tksd",
                        pass.name.get(), byteCount);
            }
        }

        if (!anyAttempted)
        {
            sink->warning(
                decl.range,
                TkStringView("TKSHD0003"),
                TkStringView("no compatible profile/backend pairs in this manifest"));
        }

        // ---- 4. Emit the .tksd container. ----
        if (anyAttempted)
        {
            if (accumulatedAsset.stages.isEmpty())
            {
                sink->error(
                    decl.range,
                    TkStringView("TKSHD0200"),
                    TkStringView("no stages were compiled successfully; refusing to emit empty .tksd"));
                return false;
            }

            const TkString tksdPath = TkString(options.outputDir)
                / TkString::format("%s.tksd",
                    shaderBaseName.isEmpty() ? "shader" : shaderBaseName.get());

            TksdWriter writer;
            if (!writer.writeToFile(accumulatedAsset, tksdPath))
            {
                sink->error(
                    decl.range,
                    TkStringView("TKSHD0200"),
                    TkStringView(TkString::format("failed to write tksd: %s", tksdPath.get())));
                outResult.failedCount++;
                return false;
            }

            LogInfo("[ShaderManifestCompiler] wrote %s (%i passes, %i stages)",
                    tksdPath.get(),
                    accumulatedAsset.passes.count(),
                    accumulatedAsset.stages.count());
        }

        return outResult.failedCount == 0;
    }
}
