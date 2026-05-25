/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include <toolkit/graphics/common.hpp>

namespace toolkit {
    
    // Shader profile types.
    static const char* s_GfxShaderProfile[] =
    {
        "Unknown",
        "DirectX SM4",
        "DirectX SM5",
        "GLSL 410",
        "GLSL 440",
        "Vulkan SM5",
        "DirectX SM6",
        "Max"
    };
    const char* toString(GfxShaderProfile s) {
        return s_GfxShaderProfile[(int)s];
    }

    static const char* s_GfxBackendType[] =
    {
        "Unknown",
        "DirectX 10",
        "DirectX 10.1",
        "DirectX 11",
        "DirectX 12",
        "OpenGL 4.1",
        "OpenGL 4.4",
        "OpenGL ES3",
        "OpenGL ES3.1",
        "Null",
        "Vulkan",
        "Metal",

        "Max"
    };
    const char* toString(GfxBackendType s) {
        return s_GfxBackendType[(int)s];
    }

    
    // Graphics feature levels.
    static const char* s_GfxFeatureLevel[] =
    {
        "es2",
        "es3",
        "es3.1",
        "sm4",
        "sm5",
        "sm6",

        "Max"
    };
    const char* toString(GfxFeatureLevel s) {
        return s_GfxFeatureLevel[(int)s];
    }

    static const char* s_FatalErrorType[] =
    {
        "None",
        "Unknown",
        "Exception",
        "Assertion",
        "OutOfMemory",
        "GPUCrash",
        "GPUHang",
        "GPUOutOfMemory",
    };
    const char* toString(FatalErrorType s) {
        return s_FatalErrorType[(int)s];
    }
}