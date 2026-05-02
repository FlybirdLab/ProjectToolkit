/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <sdhc/common.hpp>
#include <toolkit/collection/HashSet.hpp>

namespace toolkit
{
    class ShaderMeta;
    class ShaderFunctionMeta;

    // Shader compilation context container
    class ShaderCompilationContext
    {
    public:
        // The compilation options.
        const ShaderCompilationOptions *options;
        // The shader metadata container.
        ShaderMeta *meta;

    public:
        // Output stream to write compiled shader cache to.
        MemoryOStream *output;
        // All source files included by this file (absolute paths). Generated during shader compilation.
        TkHashSet<TkString> includes;

    public:
        // Name of the target object (in ASCII)
        char targetNameAnsi[64];

    public:
        // Event called on compilation error
        void onError(const char *message);
        // Event called on compilation debug data collecting
        void onCollectDebugInfo(ShaderFunctionMeta &meta, i32 permutationIndex, const char *data, const i32 dataLength);

    public:
        ShaderCompilationContext(const ShaderCompilationOptions *options, ShaderMeta *meta);
    };

}