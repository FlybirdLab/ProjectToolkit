/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "MocCtx.hpp"

#include "toolkit/debug/Logger.hpp"
#include "toolkit/platforms/FileSystem.hpp"
#include "toolkit/types/String.hpp"

#include "moc/Generator.hpp"

#include "parser/Context.hpp"

namespace toolkit {

    MocCtx::MocCtx(const TkString& sourcePath, const TkString& outputPath)
        : m_sourcePath(sourcePath)
        , m_outputPath(outputPath)
    {
    }

    MocCtx::MocCtx(const TkString& sourcePath, const TkString& outputPath, const TkArray<TkString>& sourceFiles)
        : m_sourcePath(sourcePath)
        , m_outputPath(outputPath)
        , m_sourceFiles(sourceFiles)
    {
    }

    MocCtx::MocCtx(const TkString& sourcePath, const TkString& outputPath, const TkArray<TkString>& sourceFiles, const TkString& outputName)
        : m_sourcePath(sourcePath)
        , m_outputPath(outputPath)
        , m_outputName(outputName)
        , m_sourceFiles(sourceFiles)
    {
    }

    MocCtx::MocCtx(const TkString& sourcePath, const TkString& outputPath, const TkArray<TkString>& sourceFiles, const TkString& outputName, const bool incremental)
        : m_sourcePath(sourcePath)
        , m_outputPath(outputPath)
        , m_outputName(outputName)
        , m_sourceFiles(sourceFiles)
        , m_incremental(incremental)
    {
    }


    MocCtx::~MocCtx() {
        // platform::cleanUp();
        // platform::exit();
    }


    int MocCtx::runMoc() {
        if (m_sourcePath.isEmpty()) {
            LogError("Source path is empty, RkGen will halt.");
            return -1;
        }
        if (m_outputPath.isEmpty()) {
            LogError("Output path is empty, RkGen will halt.");
            return -1;
        }

        LogDebug("source path: %s", m_sourcePath.getText());
        LogDebug("output path: %s", m_outputPath.getText());

        if (!TkFileSystem::isDirExist(m_outputPath)) {
            if (!TkFileSystem::makeDir(m_outputPath)) {
                LogError("failed to create output directory: %s", m_outputPath.getText());
                return -1;
            }
        }

        ParserContext ctx {{
            m_sourcePath,
            m_outputPath,
            m_sourceFiles,
            m_outputName
        }};
        LogDebug("scanning source files...");
        if (!ctx.setUp()) {
            LogError("failed to scan files in source dir.");
            return -1;
        }
        if (m_incremental)
            ctx.filterUpToDateOutputs();

        LogDebug("parsing source files...");
        if (!ctx.processAll()) {
            LogError("failed to parse files in source dir.");
            return -1;
        }
        Generator generator;
        LogDebug("generating reflection sources...");
        if (!generator.generate(ctx)) {
            LogError("failed to generate files.");
            return -1;
        }
        LogDebug("generation finished.");

        return 0;
    }

} // namespace cobalt
