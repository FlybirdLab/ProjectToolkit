/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/types/String.hpp"
#include "toolkit/types/UUID.hpp"
#include "toolkit/collection/Array.hpp"
#include <toolkit/common.hpp>
#include <toolkit/graphics/common.hpp>
#include <toolkit/utils/Tokenizer.hpp>
#include <sdhc/environment.hpp>

namespace toolkit
{
    #define GFX_SHADER_CACHE_VERSION 1
    class MemoryOStream;

    // Shader compilation options container
    struct ShaderCompilationOptions
    {
    public:
        // Name of the target object (name of the shader or material for better logging readability)
        TkString targetName;
        // Unique ID of the target object
        TkUuid targetID = TkUuid::Empty;
        // Shader source code (null terminated)
        const char *source = nullptr;
        // Shader source code length
        u32 srcLen = 0;

    public:
        // Target shader profile
        GfxShaderProfile profile = GfxShaderProfile::kUnknown;
        // Disables shaders compiler optimizations. Can be used to debug shaders on a target platform or to speed up the shaders compilation time.
        bool noOptimize = false;
        // Enables shader debug data generation (depends on the target platform rendering backend).
        bool generateDebugData = false;
        // Enable/disable promoting warnings to compilation errors
        bool treatWarningsAsErrors = false;
        // Custom macros for the shader compilation
        TkArray<ShaderMacro> macros;

    public:
        // Output stream to write compiled shader cache to
        MemoryOStream *output = nullptr;
    };

    typedef Tokenizer Reader;
    typedef TextToken Token;
    typedef SeparatorData Separator;

// Don't count ending '\0' character
#define MACRO_LENGTH(macro) (lengthOf(macro) - 1)
}