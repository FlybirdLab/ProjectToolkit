/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "moc/MocCtx.hpp"

#include <cstdio>

#include <toolkit/collection/Array.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/types/Path.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/utils/CommandLine.hpp>

using namespace toolkit;

namespace toolkit
{
    void setupRkGenLogger()
    {
        LoggerInitParams params;
        params.name = "RkGen";
        params.format = "%^%l:%$ %v";
        params.infoFormat = "info: %v";
        params.successFormat = "\033[32msuccess:\033[0m %v";
        params.fileFormat = "%l: %v";
        TkLogger::getShared()->setup(params);
    }

    void printUsage()
    {
        std::printf(
            "Usage:\n"
            "  RkGen --file <header> --out <dir>\n"
            "  RkGen --source-dir <dir> --out <dir>\n"
            "  RkGen --source <dir> --output <dir>\n"
            "\n"
            "Aliases:\n"
            "  --input <header>  same as --file\n"
            "  --dir <dir>       same as --source-dir\n"
            "  --source <dir>    same as --source-dir\n"
            "  -s <dir>          same as --source-dir\n"
            "  --output <dir>    same as --out\n"
            "  -o <dir>          same as --out\n"
            "  --out-name <name> generated output stem for --file mode\n"
            "  --output-name <name> same as --out-name\n"
            "  --no-incremental  always regenerate even when output is up to date\n"
            "  --force           same as --no-incremental\n"
            "\n");
    }

    TkString makeAbsolutePath(TkStringView path)
    {
        TkString result(path);
        if (TkPath::isRelative(result))
            result = System::getWorkDir() / result;
        TkPath::normalize(result);
        return result;
    }
}

int main(int argc, char** argv)
{
    if (!System::init())
        return 1;
    System::beforeRun();
    setupRkGenLogger();

    TkCommandLine args(argc, argv);
    if (args.has("help") || args.has("h"))
    {
        printUsage();
        System::beforeQuit();
        System::quit();
        return 0;
    }

    TkString sourceDir = args.get("source-dir");
    if (sourceDir.isEmpty())
        sourceDir = args.get("source");
    if (sourceDir.isEmpty())
        sourceDir = args.get("dir");
    if (sourceDir.isEmpty())
        sourceDir = args.get("s");

    TkString inputFile = args.get("file");
    if (inputFile.isEmpty())
        inputFile = args.get("input");

    const auto& positionals = args.getPositionals();
    if (sourceDir.isEmpty() && inputFile.isEmpty() && !positionals.isEmpty())
        sourceDir = positionals[0];

    TkString outputDir;
    if (!args.get("out", outputDir) &&
        !args.get("output", outputDir) &&
        !args.get("o", outputDir))
    {
        outputDir = System::getWorkDir() / "rkgen-output";
    }

    if (sourceDir.isEmpty() && inputFile.isEmpty())
    {
        printUsage();
        System::beforeQuit();
        System::quit();
        return 1;
    }

    TkArray<TkString> sourceFiles;
    if (!inputFile.isEmpty())
    {
        inputFile = makeAbsolutePath(inputFile);
        sourceFiles.add(inputFile);
        sourceDir = TkString(TkPath::getDir(inputFile));
    }
    else
    {
        sourceDir = makeAbsolutePath(sourceDir);
    }
    outputDir = makeAbsolutePath(outputDir);

    TkString outputName = args.get("out-name");
    if (outputName.isEmpty())
        outputName = args.get("output-name");

    const bool incremental = !args.has("no-incremental") && !args.has("force");
    MocCtx ctx(sourceDir, outputDir, sourceFiles, outputName, incremental);
    const int result = ctx.runMoc();

    if (result != 0)
    {
        LogError("Processing failed.");
        System::beforeQuit();
        System::quit();
        return 1;
    }

    LogSuccess("Processing passed. Output dir: %s", outputDir.getText());
    System::beforeQuit();
    System::quit();
    return 0;
}
