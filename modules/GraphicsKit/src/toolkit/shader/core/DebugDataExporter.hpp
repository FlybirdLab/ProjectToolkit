/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "ShaderMeta.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "Context.hpp"
#include "toolkit/types/StringBuilder.hpp"
#include <sdhc/common.hpp>
#include "toolkit/io/MemoryStream.hpp"
#include <toolkit/platforms/System.hpp>
#include <toolkit/platforms/FileSystem.hpp>

namespace toolkit
{
    // Tool class used to export debug information about shaders
    class ShaderDebugDataExporter
    {
    public:
        // Exports compilation results info
        static bool exportData(ShaderCompilationContext *context)
        {
            PROFILE();
            static TkString dbgDir = System::getWorkDir() / "Shaders/Debug";

            // Create output folder
            if (!TkFileSystem::isDirExist(dbgDir))
            {
                if (!TkFileSystem::makeDir(dbgDir))
                    return false;
            }

            // Prepare
            auto options = context->options;
            TkString outputFilePath = dbgDir / TkString::format("ShaderDebug_%s_%i.txt", options->targetName, options->targetID);
            auto shadersCount = context->meta->getShadersCount();
            TkStringBuilder info(1024 * shadersCount);
            TkArray<const ShaderFunctionMeta *> functions(shadersCount);
            context->meta->getShaders(functions);

            // Generate output info
            info.appendFormat("Target shader: %s : %i\nProfile: %i\nCache size: {3} bytes\n",
                              options->targetName,
                              options->targetID,
                              options->profile,
                              options->output->getPosition());
            for (i32 i = 0; i < functions.count(); i++)
            {
                const auto f = functions[i];
                auto stageName = toString(f->getStage());
                for (i32 permutationIndex = 0; permutationIndex < f->permutations.count(); permutationIndex++)
                {
                    info.append("\n*********************************************************************\n");
                    info.appendFormat("%s Shader: %s, Permutation: %i\n", stageName, f->name, permutationIndex + 1);
                    info.append(f->permutations[permutationIndex].debugData.get());
                }
            }

// #if TK_WINDOWS

//             // Change line endings on Windows platform
//             TkStringBuilder winNewLineFix = info;
//             WindowsTkFileSystem::ConvertLineEndingsToDos(TkStringView(*winNewLineFix, winNewLineFix.length()), info.getCharArray());

// #endif

            OpenFileParams p{
                .path = outputFilePath,
            };
            auto handler = TkFileSystem::openFile(p);
            auto r = TkFileSystem::writeText(handler, info.getCharArray().get(), info.length(), Encoding::kUnicode);
            TkFileSystem::closeFile(handler);

            return r;
        }
    };

}