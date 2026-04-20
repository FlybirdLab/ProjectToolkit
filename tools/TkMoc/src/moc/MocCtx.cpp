/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */
/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */
#include "MocCtx.hpp"

#include "toolkit/debug/Logger.hpp"
#include "toolkit/platforms/FileSystem.hpp"
#include "toolkit/platforms/System.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/types/StringBuilder.hpp"

#include "moc/CmdLine.hpp"

#include "purc/PurcCtx.hpp"

namespace toolkit {

    MocCtx::MocCtx(int argc, char** argv) {
        // g_deamonAppMode = true;

        // platform::init();
        // platform::setUp();

        TkStringBuilder bd{};

        for (int idx = 1; idx < argc; idx++) {
            bd.append(argv[idx]).append(" ");
        }

        if (!parseCmdLine(bd.getCharArray().get())) {
            // platform::cleanUp();
            // platform::exit();
        }
    }


    MocCtx::~MocCtx() {
        // platform::cleanUp();
        // platform::exit();
    }


    int MocCtx::runMoc() {
        auto* cmdLine = getCmdLine();

        if (cmdLine->sourcePath == TkString::Empty) {
#if CB_DEBUG
            cmdLine->sourcePath = "/Users/cubevlmu/Documents/cobalt/docs";
#else
            LogError("Source file path is empty, cmoc will halt.");
            return -1;
#endif
        }
        if (cmdLine->outputPath == TkString::Empty) {
            cmdLine->outputPath = TkString::format("%s/moc_obj", System::getWorkDir());
        }

        LogInfo("File path {0}", cmdLine->sourcePath);
        LogInfo("Output path {0}", cmdLine->outputPath);

        if (!TkFileSystem::isDirExist(cmdLine->outputPath)) {
            if (!TkFileSystem::makeDir(cmdLine->outputPath)) {
                LogError("Failed to create data directory at %s", cmdLine->outputPath);
                return -1;
            }
        }

        PurcCtx ctx {{
            cmdLine->sourcePath,
            cmdLine->outputPath
        }};
        if (!ctx.setUp()) {
            LogError("Failed to scan files in source dir!");
            return -1;
        }
        if (!ctx.processAll()) {
            LogError("Failed to parse files in source dir!");
            return -1;
        }

        return 0;
    }

} // namespace cobalt