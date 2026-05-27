/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <cstdio>

#include <toolkit/shader/core/Compilation.hpp>
#include <toolkit/shader/source/SourceManager.hpp>
#include <toolkit/shader/diagnostics/DiagnosticSink.hpp>
#include <toolkit/shader/diagnostics/Diagnostic.hpp>
#include <toolkit/shader/diagnostics/TextDiagnosticPrinter.hpp>
#include <toolkit/shader/diagnostics/JsonDiagnosticPrinter.hpp>
#include <toolkit/shader/manifest/ShaderManifestCompiler.hpp>
#include <toolkit/shader/cache/ShaderJobCache.hpp>
#include <toolkit/shader/compilegraph/CompilerScheduler.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/io/MemoryStream.hpp>
#include <toolkit/platforms/FileSystem.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/types/Path.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/UUID.hpp>
#include <toolkit/types/DateTime.hpp>
#include <toolkit/utils/CommandLine.hpp>

using namespace toolkit;

namespace toolkit
{
    enum class DiagnosticFormat
    {
        Text,
        Json,
    };

    struct CompileInput
    {
        TkString path;
    };

    const char *profileName(GfxShaderProfile profile)
    {
        switch (profile)
        {
        case GfxShaderProfile::kDirectX_SM4:
            return "d3d_sm4";
        case GfxShaderProfile::kDirectX_SM5:
            return "d3d_sm5";
        case GfxShaderProfile::kDirectX_SM6:
            return "dx_sm6";
        case GfxShaderProfile::kVulkan_SM5:
            return "vk_sm5";
        case GfxShaderProfile::kGLSL_410:
            return "glsl410";
        default:
            return "unknown";
        }
    }

    void printUsage()
    {
        std::printf(
            "Usage:\n"
            "  ShaderCompiler --file <shader.tkshader> --backend <d3d|dx|vk|opengl|all> --out <dir>\n"
            "  ShaderCompiler --source-dir <dir>      --backend <d3d,dx,vk,opengl> --out <dir>\n"
            "\n"
            "Inputs:\n"
            "  *.tkshader               Toolkit shader manifest (only accepted input)\n"
            "\n"
            "Aliases:\n"
            "  --input <file>           same as --file\n"
            "  --type <targets>         same as --backend\n"
            "  --target <targets>       same as --backend\n"
            "  opengl|gl|ogl            OpenGL 4.1 (GLSL 410) backend\n"
            "\n"
            "Includes:\n"
            "  -I <path>                add an include search directory (repeatable)\n"
            "  --include-dir <path>     same as -I\n"
            "                           Search order: dir of the file doing the #include,\n"
            "                           manifest dir, then -I dirs in CLI order. The set\n"
            "                           participates in the cache hash, so changing -I\n"
            "                           invalidates the cache.\n"
            "\n"
            "Cache:\n"
            "  --cache-dir=<path>       job cache directory (default: .shader-cache)\n"
            "  --no-cache               disable read/write of the job cache\n"
            "  --jobs=<N>               parallel job count (N>1 falls back to serial)\n"
            "  --show-stats             print compile stats at the end\n"
            "\n"
            "Diagnostics:\n"
            "  --diagnostics=text       human-readable output (default)\n"
            "  --diagnostics=json       structured JSON array on stdout\n"
            "\n"
            "Output:\n"
            "  Each .tkshader produces one `<name>.tksd` file in --out.\n"
            "\n"
            "Notes:\n"
            "  'all' covers d3d/dx/vk; pass 'opengl' explicitly to emit GLSL 410.\n"
            "  metal is recognized but not implemented yet.\n");
    }

    TkString makeAbsolutePath(TkStringView path)
    {
        TkString result(path);
        if (TkPath::isRelative(result))
            result = System::getWorkDir() / result;
        TkPath::normalize(result);
        return result;
    }

    bool ensureDirectory(TkStringView path)
    {
        if (TkFileSystem::isDirExist(path))
            return true;
        return TkFileSystem::makeDir(path);
    }

    bool isManifestFile(TkStringView path)
    {
        return TkPath::hasExtension(path, "tkshader");
    }

    bool collectInputsFromFile(
        DiagnosticSink &diagnostics,
        TkStringView path,
        TkArray<CompileInput> &inputs)
    {
        CompileInput input;
        input.path = makeAbsolutePath(path);
        if (!isManifestFile(input.path))
        {
            LogError("[ShaderCompiler] only .tkshader inputs are accepted: %s", input.path);
            diagnostics.error(
                SourceRange::invalid(),
                TKSHD_INVALID_CLI,
                TkString::format("only .tkshader inputs are accepted: %s", input.path));
            return false;
        }
        if (!TkFileSystem::isFileExist(input.path))
        {
            LogError("[ShaderCompiler] input file not found: %s", input.path);
            diagnostics.error(
                SourceRange::invalid(),
                TKSHD_READ_SOURCE_FAILED,
                TkString::format("input file not found: %s", input.path));
            return false;
        }
        inputs.add(input);
        return true;
    }

    bool collectInputsFromDirectory(
        DiagnosticSink &diagnostics,
        TkStringView path,
        TkArray<CompileInput> &inputs)
    {
        const TkString dir = makeAbsolutePath(path);
        if (!TkFileSystem::isDirExist(dir))
        {
            LogError("[ShaderCompiler] source directory not found: %s", dir);
            diagnostics.error(
                SourceRange::invalid(),
                TKSHD_NO_INPUT,
                TkString::format("source directory not found: %s", dir));
            return false;
        }

        TkArray<TkString> files;
        if (!TkFileSystem::listDir(files, dir, "*", true))
        {
            LogError("[ShaderCompiler] failed to list source directory: %s", dir);
            diagnostics.error(
                SourceRange::invalid(),
                TKSHD_NO_INPUT,
                TkString::format("failed to list source directory: %s", dir));
            return false;
        }

        for (const auto &file : files)
        {
            if (!isManifestFile(file))
                continue;

            CompileInput input;
            input.path = file;
            inputs.add(input);
        }

        return true;
    }

    void addBackendProfile(
        DiagnosticSink &diagnostics,
        TkStringView backend,
        TkArray<GfxShaderProfile> &profiles)
    {
        i32 start = 0;
        i32 end = backend.length();
        while (start < end && (backend[start] == ' ' || backend[start] == '\t'))
            start++;
        while (end > start && (backend[end - 1] == ' ' || backend[end - 1] == '\t'))
            end--;
        if (start == end)
            return;

        TkString normalized(backend.get() + start, end - start);
        normalized = normalized.toLowerAscii();

        if (normalized == "all")
        {
#if TK_WINDOWS
            profiles.addUnique(GfxShaderProfile::kDirectX_SM5);
            profiles.addUnique(GfxShaderProfile::kDirectX_SM6);
#endif
            profiles.addUnique(GfxShaderProfile::kVulkan_SM5);
            return;
        }
        if (normalized == "d3d" || normalized == "d3d11" || normalized == "sm5")
        {
            profiles.addUnique(GfxShaderProfile::kDirectX_SM5);
            return;
        }
        if (normalized == "dx" || normalized == "dxc" || normalized == "d3d12" || normalized == "sm6")
        {
            profiles.addUnique(GfxShaderProfile::kDirectX_SM6);
            return;
        }
        if (normalized == "vk" || normalized == "vulkan")
        {
            profiles.addUnique(GfxShaderProfile::kVulkan_SM5);
            return;
        }
        if (normalized == "opengl" || normalized == "gl" || normalized == "ogl")
        {
            profiles.addUnique(GfxShaderProfile::kGLSL_410);
            return;
        }
        if (normalized == "metal" || normalized == "mtl")
        {
            LogWarn("[ShaderCompiler] backend '%s' is recognized but not implemented yet.", normalized);
            diagnostics.warning(
                SourceRange::invalid(),
                TKSHD_UNSUPPORTED_BACKEND,
                TkString::format("backend '%s' is recognized but not implemented yet", normalized));
            return;
        }

        LogWarn("[ShaderCompiler] unknown backend '%s'.", normalized);
        diagnostics.warning(
            SourceRange::invalid(),
            TKSHD_UNSUPPORTED_BACKEND,
            TkString::format("unknown backend '%s'", normalized));
    }

    void parseBackends(
        DiagnosticSink &diagnostics,
        TkStringView value,
        TkArray<GfxShaderProfile> &profiles)
    {
        TkString text(value);
        TkArray<TkString> parts;
        text.split(',', parts);
        if (parts.isEmpty())
            addBackendProfile(diagnostics, text, profiles);
        for (const auto &part : parts)
            addBackendProfile(diagnostics, part, profiles);
    }

    DiagnosticFormat parseDiagnosticFormat(const TkCommandLine &args)
    {
        TkString value;
        if (args.get("diagnostics", value))
        {
            const TkString normalized = value.toLowerAscii();
            if (normalized == "json")
                return DiagnosticFormat::Json;
        }
        return DiagnosticFormat::Text;
    }

    void emitDiagnostics(
        const DiagnosticSink &sink,
        const SourceManager &sources,
        DiagnosticFormat format)
    {
        if (sink.getDiagnostics().count() == 0)
            return;

        if (format == DiagnosticFormat::Json)
        {
            JsonDiagnosticPrinter printer;
            printer.print(sink, sources);
        }
        else
        {
            TextDiagnosticPrinter printer;
            printer.print(sink, sources);
        }
    }
}

int main(int argc, char **argv)
{
    if (!System::init())
        return 1;
    System::beforeRun();

    TkCommandLine args(argc, argv);
    if (args.has("help") || args.has("h"))
    {
        printUsage();
        System::beforeQuit();
        System::quit();
        return 0;
    }

    const DiagnosticFormat diagFormat = parseDiagnosticFormat(args);
    SourceManager sources;
    DiagnosticSink diagnostics;

    // ---- Job cache plumbing. ----
    ShaderJobCache jobCache;
    TkString cacheDir;
    if (args.get("cache-dir", cacheDir))
        jobCache.setCacheDirectory(TkStringView(cacheDir));
    else
        jobCache.setCacheDirectory(TkStringView(TkString(".shader-cache")));
    if (args.has("no-cache"))
        jobCache.setEnabled(false);

    // ---- Scheduler stub. ----
    CompilerScheduler scheduler;
    TkString jobsValue;
    if (args.get("jobs", jobsValue))
    {
        i32 jobsCount = 0;
        for (i32 i = 0; i < jobsValue.length(); i++)
        {
            const char c = jobsValue[i];
            if (c < '0' || c > '9') { jobsCount = 0; break; }
            jobsCount = jobsCount * 10 + (c - '0');
        }
        if (jobsCount <= 0) jobsCount = 1;
        scheduler.setJobCount(static_cast<u32>(jobsCount));
    }

    const bool showStats = args.has("show-stats");
    ShaderManifestCompileResult aggregateStats;

    // Tie cache verbose-logging to --show-stats so the same flag the user
    // already uses to investigate cache hit/miss numbers also shows the
    // underlying hash / path / outcome for every lookup.
    jobCache.setVerbose(showStats);

    // ---- Include search directories. ----
    TkArray<TkString> includeDirs;
    args.getAll(TkStringView("I"), includeDirs);
    args.getAll(TkStringView("include-dir"), includeDirs);
    for (i32 i = 0; i < includeDirs.count(); i++)
    {
        TkString abs = makeAbsolutePath(TkStringView(includeDirs[i]));
        includeDirs[i] = abs;
    }

    TkString inputFile = args.get("file");
    if (inputFile.isEmpty())
        inputFile = args.get("input");
    TkString sourceDir = args.get("source-dir");
    if (sourceDir.isEmpty())
        sourceDir = args.get("dir");
    const auto &positionals = args.getPositionals();
    if (inputFile.isEmpty() && sourceDir.isEmpty() && !positionals.isEmpty())
        inputFile = positionals[0];

    TkString defaultOutputDir = System::getWorkDir() / "shader-output";
    TkString outputDir;
    if (!args.get("out", outputDir))
        outputDir = defaultOutputDir;
    outputDir = makeAbsolutePath(outputDir);

    TkString backendText = args.get("backend");
    if (backendText.isEmpty())
        backendText = args.get("type");
    if (backendText.isEmpty())
        backendText = args.get("target");
    if (backendText.isEmpty())
        backendText = "all";

    if (inputFile.isEmpty() && sourceDir.isEmpty())
    {
        printUsage();
        emitDiagnostics(diagnostics, sources, diagFormat);
        System::beforeQuit();
        System::quit();
        return 1;
    }
    if (!inputFile.isEmpty() && !sourceDir.isEmpty())
    {
        LogError("[ShaderCompiler] use either --file or --source-dir, not both.");
        diagnostics.error(
            SourceRange::invalid(),
            TKSHD_INVALID_CLI,
            "use either --file or --source-dir, not both");
        emitDiagnostics(diagnostics, sources, diagFormat);
        System::beforeQuit();
        System::quit();
        return 1;
    }
    if (!ensureDirectory(outputDir))
    {
        LogError("[ShaderCompiler] failed to create output directory: %s", outputDir);
        diagnostics.error(
            SourceRange::invalid(),
            TKSHD_WRITE_OUTPUT_FAILED,
            TkString::format("failed to create output directory: %s", outputDir));
        emitDiagnostics(diagnostics, sources, diagFormat);
        System::beforeQuit();
        System::quit();
        return 1;
    }

    TkArray<CompileInput> inputs;
    const bool collectedInputs = !inputFile.isEmpty()
        ? collectInputsFromFile(diagnostics, inputFile, inputs)
        : collectInputsFromDirectory(diagnostics, sourceDir, inputs);
    if (!collectedInputs || inputs.isEmpty())
    {
        LogError("[ShaderCompiler] no .tkshader input files found.");
        if (!diagnostics.hasError())
        {
            diagnostics.error(
                SourceRange::invalid(),
                TKSHD_NO_INPUT,
                "no .tkshader input files found");
        }
        emitDiagnostics(diagnostics, sources, diagFormat);
        System::beforeQuit();
        System::quit();
        return 1;
    }

    TkArray<GfxShaderProfile> profiles;
    parseBackends(diagnostics, backendText, profiles);
    if (profiles.isEmpty())
    {
        LogError("[ShaderCompiler] no supported backend selected.");
        diagnostics.error(
            SourceRange::invalid(),
            TKSHD_UNSUPPORTED_BACKEND,
            "no supported backend selected");
        emitDiagnostics(diagnostics, sources, diagFormat);
        System::beforeQuit();
        System::quit();
        return 1;
    }

    ShadersCompilation::init();
    ShadersCompilation::onIncludeChanged.bind([](const TkString &path, FileAction action)
    {
        LogInfo("[ShaderCompiler] include changed: %s action=%i", path, static_cast<i32>(action));
    });

    const TkDateTime statsStart = TkDateTime::nowUTC();

    i32 failed = 0;
    u32 inputIndex = 0;
    for (const auto &input : inputs)
    {
        ShaderManifestCompileOptions mopts;
        mopts.manifestPath = input.path;
        mopts.outputDir = outputDir;
        mopts.profiles = profiles;
        mopts.sources = &sources;
        mopts.diagnostics = &diagnostics;
        mopts.cache = jobCache.isEnabled() ? &jobCache : nullptr;
        mopts.inputIndex = inputIndex;
        mopts.includeDirs = includeDirs;

        ShaderManifestCompileResult mresult;
        ShaderManifestCompiler manifestCompiler;
        manifestCompiler.compile(mopts, mresult);
        failed += mresult.failedCount;

        aggregateStats.caseCount += mresult.caseCount;
        aggregateStats.failedCount += mresult.failedCount;
        aggregateStats.cacheHit += mresult.cacheHit;
        aggregateStats.cacheMiss += mresult.cacheMiss;
        aggregateStats.compiled += mresult.compiled;
        aggregateStats.dependencyTotalCount += mresult.dependencyTotalCount;
        aggregateStats.includeMissingCount += mresult.includeMissingCount;
        aggregateStats.includeCycleCount += mresult.includeCycleCount;
        inputIndex++;
    }

    ShadersCompilation::dispose();

    emitDiagnostics(diagnostics, sources, diagFormat);

    if (showStats)
    {
        const TkDateTime statsEnd = TkDateTime::nowUTC();
        const f64 elapsedMs = static_cast<f64>((statsEnd - statsStart).getTotalMilliseconds());
        std::printf("Shader compile stats:\n");
        std::printf("  jobs:       %i\n", aggregateStats.caseCount);
        std::printf("  cache hit:  %i\n", aggregateStats.cacheHit);
        std::printf("  cache miss: %i\n", aggregateStats.cacheMiss);
        std::printf("  compiled:   %i\n", aggregateStats.compiled);
        std::printf("  failed:     %i\n", aggregateStats.failedCount);
        std::printf("  deps:       %i (missing=%i, cycles=%i)\n",
                    aggregateStats.dependencyTotalCount,
                    aggregateStats.includeMissingCount,
                    aggregateStats.includeCycleCount);
        std::printf("  time:        %.1f ms\n", elapsedMs);
    }

    System::beforeQuit();
    System::quit();

    if (failed != 0 || diagnostics.hasError())
    {
        std::printf("ShaderCompiler failed: %i case(s)\n", failed);
        return 1;
    }

    std::printf("ShaderCompiler passed. Output: %s\n", outputDir.getText());
    return 0;
}
