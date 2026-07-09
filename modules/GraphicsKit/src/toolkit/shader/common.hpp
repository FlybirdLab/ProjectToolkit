/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/Enum.hpp>

namespace toolkit
{
    // Shader program type
    TK_DECLARE_ENUM_EX_6(ShaderStage, i32, 0, Vertex, Hull, Domain, Geometry, Pixel, Compute);
    static constexpr i32 ShaderStage_Count = ShaderStage_const_size;
    static constexpr i32 kShaderStageCount = ShaderStage_const_size;

    // Shader macro definition structure
    struct ShaderMacro
    {
        // Macro name
        const char *name;
        // Macro value
        const char *definition;
    };

    // Shader profile types.
    enum class GfxShaderProfile
    {
        kUnknown = 0,
        kDirectX_SM4,
        kDirectX_SM5,
        kGLSL_410,
        kGLSL_440,
        kVulkan_SM5,
        kDirectX_SM6,

        kMax
    };

    inline const char *toString(GfxShaderProfile s)
    {
        switch (s)
        {
        case GfxShaderProfile::kUnknown:     return "Unknown";
        case GfxShaderProfile::kDirectX_SM4: return "DirectX SM4";
        case GfxShaderProfile::kDirectX_SM5: return "DirectX SM5";
        case GfxShaderProfile::kGLSL_410:    return "GLSL 410";
        case GfxShaderProfile::kGLSL_440:    return "GLSL 440";
        case GfxShaderProfile::kVulkan_SM5:  return "Vulkan SM5";
        case GfxShaderProfile::kDirectX_SM6: return "DirectX SM6";
        case GfxShaderProfile::kMax:         return "Max";
        }
        return "Unknown";
    }

    // Shader compilation flags.
    enum class GfxShaderFlags : u32
    {
        kDefault = 0,
        kHidden = 1,
        kNoFastMath = 2,
        kVertexToGeometryShader = 4,
    };
    TK_DECLARE_ENUM_OPERATORS(GfxShaderFlags);
}
