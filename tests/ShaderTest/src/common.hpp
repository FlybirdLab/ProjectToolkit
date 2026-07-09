/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

// Silence MSVC's deprecated-CRT warnings (std::fopen etc.) -- the tests
// favour the portable C stdio for write paths so we get clean output across
// platforms. Must be defined before any system header pulls in <cstdio>.
#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <toolkit/common.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/io/MemoryStream.hpp>
#include <toolkit/platforms/FileSystem.hpp>
#include <toolkit/types/Path.hpp>

// Compiler library surface under test.
#include <toolkit/shader/common.hpp>
#include <toolkit/shader/core/common.hpp>
#include <toolkit/shader/lexer/ShaderLexer.hpp>
#include <toolkit/shader/parser/ShaderParser.hpp>
#include <toolkit/shader/sema/ShaderSema.hpp>
#include <toolkit/shader/source/SourceManager.hpp>
#include <toolkit/shader/source/IncludeResolver.hpp>
#include <toolkit/shader/diagnostics/DiagnosticSink.hpp>
#include <toolkit/shader/diagnostics/Diagnostic.hpp>
#include <toolkit/shader/diagnostics/TextDiagnosticPrinter.hpp>
#include <toolkit/shader/diagnostics/JsonDiagnosticPrinter.hpp>
#include <toolkit/shader/cache/ShaderHash.hpp>
#include <toolkit/shader/cache/ShaderJobCache.hpp>
#include <toolkit/shader/compilegraph/ShaderCompileJob.hpp>
#include <toolkit/shader/compilegraph/DependencyScanner.hpp>

// Runtime / format surface under test.
#include <toolkit/shader/format/ShaderAsset.hpp>
#include <toolkit/shader/format/TksdFormat.hpp>
#include <toolkit/shader/format/TksdReader.hpp>
#include <toolkit/shader/format/TksdWriter.hpp>
#include <toolkit/shader/format/GfxShaderAsset.hpp>
#include <toolkit/shader/format/GfxShaderAssetLoader.hpp>

#define GTEST_HAS_SEH 0
#include <gtest/gtest.h>

// Test-data roots are normally injected by CMake (see tests/ShaderTest/
// CMakeLists.txt). The fallback values keep the IDE / linter happy when the
// translation unit is parsed without the CMake-driven defines -- they're
// never the values used at runtime since the CMake definitions take
// precedence.
#ifndef SHADER_TEST_DOC_DIR
    #define SHADER_TEST_DOC_DIR "tests/ShaderTest/testdata"
#endif
#ifndef SHADER_TEST_TMP_DIR
    #define SHADER_TEST_TMP_DIR "build/tests/ShaderTest/tmp"
#endif

using namespace toolkit;

namespace shader_test
{
    // ---------- Test-data path helpers ----------

    // Absolute path to <repo>/tests/ShaderTest/testdata. SHADER_TEST_DOC_DIR
    // is injected by CMake (see tests/ShaderTest/CMakeLists.txt).
    inline TkString docPath(const char *relative)
    {
        return TkString(SHADER_TEST_DOC_DIR) / relative;
    }

    // Scratch directory under the build tree -- safe to create / delete files
    // under here without affecting the source tree.
    inline TkString tmpPath(const char *relative)
    {
        return TkString(SHADER_TEST_TMP_DIR) / relative;
    }

    // Generate a unique scratch directory for tests that need a fresh slate
    // (cache directories, generated includes, etc). Uses the test name +
    // a process-local counter so test fixtures don't collide.
    inline TkString uniqueScratchDir(const char *tag)
    {
        static u32 counter = 0;
        const u32 n = ++counter;
        // tmpPath returns SHADER_TEST_TMP_DIR / "<tag>_<N>" -- we then
        // normalize so the result is a clean forward-slash path the
        // platform file system is happy with.
        TkString p = tmpPath(TkString::format("%s_%u", tag, n).get());
        TkPath::normalize(p);

        // SHCreateDirectoryExW (used by TkFileSystem::makeDir on Windows)
        // creates intermediate dirs, so callers don't need to MAKE_DIRECTORY
        // the parents. We only call makeDir if the dir isn't already there.
        if (!TkFileSystem::isDirExist(p))
        {
            const bool created = TkFileSystem::makeDir(p);
            (void)created; // tests assert post-condition below
        }
        // Hard-stop if the dir still isn't there -- otherwise every
        // subsequent file write in the test silently lands at a broken
        // path and the failure mode is confusing.
        if (!TkFileSystem::isDirExist(p))
        {
            ADD_FAILURE() << "uniqueScratchDir failed to create '" << p.get() << "'";
        }
        return p;
    }

    // ---------- I/O helpers ----------

    inline bool writeTextFile(TkStringView path, TkStringView content)
    {
        MemoryOStream out(static_cast<u32>(content.length()));
        if (content.length() > 0)
            out.writeBytes(content.get(), static_cast<u32>(content.length()));
        return out.saveToFile(TkString(path));
    }

    inline TkString readTextFile(TkStringView path)
    {
        OpenFileParams params{
            .path = path,
            .mode = FileMode::kOpenExisting,
            .access = FileAccess::kRead,
            .share = FileShare::kRead,
        };
        FileHandle handle = TkFileSystem::openFile(params);
        if (!TkFileSystem::isFileOpened(handle))
            return TkString();
        TkString content;
        TkFileSystem::readAsText(handle, content);
        TkFileSystem::closeFile(handle);
        return content;
    }

    // ---------- Lex/Parse helpers ----------

    // Run the lexer once and collect every token (including EOF). Helpful for
    // assertions on the token stream without re-doing the loop in every test.
    inline TkArray<ShaderToken> collectTokens(SourceManager &sm, TkStringView name,
                                              TkStringView content, DiagnosticSink &sink)
    {
        SourceFile *file = sm.addVirtualFile(name, content);
        TkArray<ShaderToken> out;
        if (file == nullptr)
            return out;
        ShaderLexer lexer(file, &sink);
        while (true)
        {
            ShaderToken tok = lexer.next();
            const bool atEnd = (tok.kind == ShaderTokenKind::EndOfFile);
            out.add(tok);
            if (atEnd)
                break;
        }
        return out;
    }

    // Lex + parse an in-memory .tkshader source. Returns true on a clean parse
    // (no errors in the sink). `outShader` is the parsed AST.
    inline bool parseManifest(SourceManager &sm, TkStringView name,
                              TkStringView content, DiagnosticSink &sink,
                              ShaderDecl &outShader)
    {
        SourceFile *file = sm.addVirtualFile(name, content);
        if (file == nullptr)
            return false;
        ShaderLexer lexer(file, &sink);
        ShaderParser parser(lexer, &sink);
        const bool ok = parser.parse(outShader);
        return ok && !sink.hasError();
    }

    // ---------- Diagnostic predicates ----------

    inline bool hasDiagnosticWithCode(const DiagnosticSink &sink, TkStringView code)
    {
        const TkArray<Diagnostic> &all = sink.getDiagnostics();
        for (i32 i = 0; i < all.count(); i++)
        {
            if (all[i].code == TkString(code))
                return true;
        }
        return false;
    }

    inline i32 countDiagnosticsWithSeverity(const DiagnosticSink &sink,
                                            DiagnosticSeverity sev)
    {
        const TkArray<Diagnostic> &all = sink.getDiagnostics();
        i32 n = 0;
        for (i32 i = 0; i < all.count(); i++)
            if (all[i].severity == sev)
                n++;
        return n;
    }

    // ---------- Job cache helpers ----------

    // Build a sane minimal ShaderCompileJob for cache round-trip tests. The
    // values match what ShaderManifestCompiler would produce for a one-pass
    // single-stage compile.
    inline ShaderCompileJob makeMinimalJob(TkStringView name, GfxShaderProfile profile,
                                           TkStringView sourceText)
    {
        ShaderCompileJob job;
        job.shaderName = TkString(name);
        job.passName = TkString("Forward");
        job.stage = ShaderStage::kVertex;
        job.profile = profile;
        job.sourcePath = TkString();
        job.virtualSourceName = TkString::format("%.*s/Forward",
                                                 name.length(), name.get());
        job.sourceText = TkString(sourceText);
        job.entryPoint = TkString("VSMain");
        job.variantKey = TkString();
        return job;
    }

    inline CachedShaderJobResult makeCacheResult(TkStringView bytes,
                                                  bool withBindings,
                                                  bool withVertexInputs,
                                                  bool withDependencies)
    {
        CachedShaderJobResult r;
        if (bytes.length() > 0)
        {
            r.bytecode.ensureCapacity(static_cast<i32>(bytes.length()), false);
            for (i32 i = 0; i < bytes.length(); i++)
                r.bytecode.add(static_cast<byte>(bytes[i]));
        }
        if (withBindings)
        {
            ShaderBindingInfo b;
            b.name = TkString("FrameCB");
            b.slot = 0;
            b.stage = ShaderStage::kVertex;
            b.type = TkString("CB");
            r.reflectionBindings.add(b);

            ShaderBindingInfo b2;
            b2.name = TkString("MainTex");
            b2.slot = 1;
            b2.stage = ShaderStage::kPixel;
            b2.type = TkString("SRV");
            r.reflectionBindings.add(b2);
        }
        if (withVertexInputs)
        {
            ShaderVertexInputInfo v;
            v.semantic = TkString("POSITION");
            v.location = 0;
            v.format = 0;
            v.slot = 0;
            v.offset = 0;
            r.reflectionVertexInputs.add(v);
        }
        if (withDependencies)
        {
            r.dependencies.add(TkString("C:/abs/path/header.hlsl"));
            r.dependencies.add(TkString("C:/abs/path/inner/common.hlsl"));
        }
        return r;
    }

    // ---------- .tksd helpers ----------

    // Build a tiny but complete ShaderAsset with at least one pass + stage so
    // writer/reader round-trips have non-trivial content to chew on.
    inline ShaderAsset makeMinimalAsset(TkStringView name = TkStringView("Test/Asset"))
    {
        ShaderAsset asset;
        asset.name = TkString(name);

        ShaderProperty p;
        p.type = TkString("Color");
        p.name = TkString("MainColor");
        p.defaultValue = TkString("(1, 1, 1, 1)");
        asset.properties.add(p);

        ShaderTag t;
        t.key = TkString("RenderType");
        t.value = TkString("Opaque");
        asset.tags.add(t);

        ShaderPass pass;
        pass.name = TkString("Forward");

        ShaderTag pt;
        pt.key = TkString("LightMode");
        pt.value = TkString("ForwardBase");
        pass.tags.add(pt);

        ShaderRenderState rs;
        rs.name = TkString("ZWrite");
        rs.value = TkString("On");
        pass.renderStates.add(rs);
        ShaderRenderState rs2;
        rs2.name = TkString("Cull");
        rs2.value = TkString("Back");
        pass.renderStates.add(rs2);

        pass.stageIndices.add(0);
        asset.passes.add(pass);

        ShaderStageData stage;
        stage.kind = TKSD_STAGE_KIND_RAW;
        stage.stage = ShaderStage::kVertex;
        stage.profile = GfxShaderProfile::kGLSL_410;
        stage.entry = TkString();
        const byte payload[] = {'T', 'K', 'S', 'D', 'd', 'a', 't', 'a', 0x01, 0x02, 0x03, 0x04};
        for (u32 i = 0; i < sizeof(payload); i++)
            stage.bytecode.add(payload[i]);
        asset.stages.add(stage);

        ShaderBindingInfo b;
        b.name = TkString("FrameCB");
        b.slot = 0;
        b.stage = ShaderStage::kVertex;
        b.type = TkString("CB");
        asset.bindings.add(b);

        ShaderVertexInputInfo vi;
        vi.semantic = TkString("POSITION");
        vi.location = 0;
        vi.format = 0;
        vi.slot = 0;
        vi.offset = 0;
        asset.vertexInputs.add(vi);

        asset.dependencies.add(TkString("C:/abs/test/header.hlsl"));
        return asset;
    }
}
