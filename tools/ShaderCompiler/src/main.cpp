/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include <cstdio>

#include <sdhc/Compilation.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/io/MemoryStream.hpp>
#include <toolkit/platforms/FileSystem.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/types/Path.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/types/UUID.hpp>
#include <toolkit/utils/CommandLine.hpp>

using namespace toolkit;

namespace toolkit
{
    struct CompileInput
    {
        TkString path;
        TkString source;
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
        default:
            return "unknown";
        }
    }

    void printUsage()
    {
        std::printf(
            "Usage:\n"
            "  ShaderCompiler --file <shader-file> --backend <d3d|dx|vk|all> --out <dir>\n"
            "  ShaderCompiler --source-dir <dir> --backend <d3d,dx,vk> --out <dir>\n"
            "\n"
            "Aliases:\n"
            "  --input <file>       same as --file\n"
            "  --type <targets>     same as --backend\n"
            "  --target <targets>   same as --backend\n"
            "\n"
            "Notes:\n"
            "  opengl and metal are recognized but not implemented yet.\n");
    }

    TkString makeAbsolutePath(TkStringView path)
    {
        TkString result(path);
        if (TkPath::isRelative(result))
            result = System::getWorkDir() / result;
        TkPath::normalize(result);
        return result;
    }

    bool readTextFile(TkStringView path, TkString &text)
    {
        OpenFileParams params{
            .path = path,
            .mode = FileMode::kOpenExisting,
            .access = FileAccess::kRead,
            .share = FileShare::kRead,
        };
        FileHandle file = TkFileSystem::openFile(params);
        if (!TkFileSystem::isFileOpened(file))
            return false;

        const bool result = TkFileSystem::readAsText(file, text);
        TkFileSystem::closeFile(file);
        return result;
    }

    bool ensureDirectory(TkStringView path)
    {
        if (TkFileSystem::isDirExist(path))
            return true;
        return TkFileSystem::makeDir(path);
    }

    bool isShaderSourceFile(TkStringView path)
    {
        return TkPath::hasExtension(path, "shader") || TkPath::hasExtension(path, "hlsl");
    }

    bool collectInputsFromFile(TkStringView path, TkArray<CompileInput> &inputs)
    {
        CompileInput input;
        input.path = makeAbsolutePath(path);
        if (!TkFileSystem::isFileExist(input.path))
        {
            LogError("[ShaderCompiler] input file not found: %s", input.path);
            return false;
        }
        if (!readTextFile(input.path, input.source))
        {
            LogError("[ShaderCompiler] failed to read input file: %s", input.path);
            return false;
        }
        inputs.add(input);
        return true;
    }

    bool collectInputsFromDirectory(TkStringView path, TkArray<CompileInput> &inputs)
    {
        const TkString dir = makeAbsolutePath(path);
        if (!TkFileSystem::isDirExist(dir))
        {
            LogError("[ShaderCompiler] source directory not found: %s", dir);
            return false;
        }

        TkArray<TkString> files;
        if (!TkFileSystem::listDir(files, dir, "*", true))
        {
            LogError("[ShaderCompiler] failed to list source directory: %s", dir);
            return false;
        }

        for (const auto &file : files)
        {
            if (!isShaderSourceFile(file))
                continue;

            CompileInput input;
            input.path = file;
            if (!readTextFile(input.path, input.source))
            {
                LogError("[ShaderCompiler] failed to read input file: %s", input.path);
                return false;
            }
            inputs.add(input);
        }

        return true;
    }

    void addBackendProfile(TkStringView backend, TkArray<GfxShaderProfile> &profiles)
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
        if (normalized == "opengl" || normalized == "gl" || normalized == "metal" || normalized == "mtl")
        {
            LogWarn("[ShaderCompiler] backend '%s' is recognized but not implemented yet.", normalized);
            return;
        }

        LogWarn("[ShaderCompiler] unknown backend '%s'.", normalized);
    }

    void parseBackends(TkStringView value, TkArray<GfxShaderProfile> &profiles)
    {
        TkString text(value);
        TkArray<TkString> parts;
        text.split(',', parts);
        if (parts.isEmpty())
            addBackendProfile(text, profiles);
        for (const auto &part : parts)
            addBackendProfile(part, profiles);
    }

    TkString makeTargetName(TkStringView inputPath)
    {
        TkString name(TkPath::getBaseName(inputPath));
        if (name.isEmpty())
            name = "ShaderCompilerInput";
        return name;
    }

    TkString makeOutputPath(TkStringView outputDir, TkStringView inputPath, GfxShaderProfile profile)
    {
        TkString base(TkPath::getBaseName(inputPath));
        if (base.isEmpty())
            base = "shader";
        return TkString(outputDir) / TkString::format("%s_%s.shadercache", base, profileName(profile));
    }

    bool compileOne(const CompileInput &input, GfxShaderProfile profile, TkStringView outputDir, u32 index)
    {
        MemoryOStream output(1024 * 1024 * 8);

        ShaderCompilationOptions options;
        options.targetName = makeTargetName(input.path);
        options.targetID = TkUuid(0x53444348, index + 1, static_cast<u32>(profile), 0x20260502);
        options.source = input.source.get();
        options.srcLen = input.source.length();
        options.profile = profile;
        options.noOptimize = true;
        options.generateDebugData = true;
        options.treatWarningsAsErrors = true;
        options.output = &output;

        LogInfo("[ShaderCompiler] compiling %s as %s", input.path, profileName(profile));
        if (!ShadersCompilation::compile(options))
            return false;

        const TkString outputPath = makeOutputPath(outputDir, input.path, profile);
        if (!output.saveToFile(outputPath))
        {
            LogError("[ShaderCompiler] failed to save %s", outputPath);
            return false;
        }

        TkArray<TkString> includes;
        ShadersCompilation::extractShaderIncludes(output.getHandle(), output.getPosition(), includes);
        LogInfo("[ShaderCompiler] wrote %s (%i bytes, includes=%i)", outputPath, output.getPosition(), includes.count());
        return true;
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
        System::beforeQuit();
        System::quit();
        return 1;
    }
    if (!inputFile.isEmpty() && !sourceDir.isEmpty())
    {
        LogError("[ShaderCompiler] use either --file or --source-dir, not both.");
        System::beforeQuit();
        System::quit();
        return 1;
    }
    if (!ensureDirectory(outputDir))
    {
        LogError("[ShaderCompiler] failed to create output directory: %s", outputDir);
        System::beforeQuit();
        System::quit();
        return 1;
    }

    TkArray<CompileInput> inputs;
    const bool collectedInputs = !inputFile.isEmpty()
        ? collectInputsFromFile(inputFile, inputs)
        : collectInputsFromDirectory(sourceDir, inputs);
    if (!collectedInputs || inputs.isEmpty())
    {
        LogError("[ShaderCompiler] no shader input files found.");
        System::beforeQuit();
        System::quit();
        return 1;
    }

    TkArray<GfxShaderProfile> profiles;
    parseBackends(backendText, profiles);
    if (profiles.isEmpty())
    {
        LogError("[ShaderCompiler] no supported backend selected.");
        System::beforeQuit();
        System::quit();
        return 1;
    }

    ShadersCompilation::init();
    ShadersCompilation::onIncludeChanged.bind([](const TkString &path, FileAction action)
    {
        LogInfo("[ShaderCompiler] include changed: %s action=%i", path, static_cast<i32>(action));
    });

    i32 failed = 0;
    u32 inputIndex = 0;
    for (const auto &input : inputs)
    {
        for (const auto profile : profiles)
        {
            if (!compileOne(input, profile, outputDir, inputIndex))
                failed++;
        }
        inputIndex++;
    }

    ShadersCompilation::dispose();
    System::beforeQuit();
    System::quit();

    if (failed != 0)
    {
        std::printf("ShaderCompiler failed: %i case(s)\n", failed);
        return 1;
    }

    std::printf("ShaderCompiler passed. Output: %s\n", outputDir.getText());
    return 0;
}
