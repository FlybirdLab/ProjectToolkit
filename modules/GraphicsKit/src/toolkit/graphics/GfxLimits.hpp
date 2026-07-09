/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/common.hpp>

namespace toolkit
{
    // Graphics Device limits and constraints descriptor
    struct GfxLimits
    {
        // True if device supports Compute shaders.
        bool hasCompute;
        // True if device supports Tessellation shaders (domain and hull shaders).
        bool hasTessellation;
        // True if device supports Geometry shaders.
        bool hasGeometryShaders;
        // True if device supports hardware geometry instancing.
        bool hasInstancing;
        // True if device supports rendering to volume textures using Geometry shaders.
        bool hasVolumeTextureRendering;
        // True if device supports indirect drawing (including pixel shader write to UAV).
        bool hasDrawIndirect;
        // True if device supports append/consume buffers with counters.
        bool hasAppendConsumeBuffers;
        // True if device supports separate render target blending states.
        bool hasSeparateRenderTargetBlendState;
        // True if device supports depth buffer texture as a shader resource view.
        bool hasDepthAsSRV;
        // True if device supports depth buffer clipping (see GPUPipelineState::Description::DepthClipEnable).
        bool hasDepthClip;
        // True if device supports depth buffer texture as a readonly depth buffer (can be sampled in the shader while performing depth-test).
        bool hasReadOnlyDepth;
        // True if device supports multisampled depth buffer texture as a shader resource view.
        bool hasMultisampleDepthAsSRV;
        // True if device supports reading from typed UAV in shader (common types such as R32G32B32A32, R16G16B16A16, R16, R8). This doesn't apply to single-component 32-bit formats.
        bool hasTypedUAVLoad;
        // The maximum amount of texture mip levels.
        i32 maxMipLevelsCount;
        // The maximum size of the 1D texture.
        i32 maxTexture1DSize;
        // The maximum length of 1D textures array.
        i32 maxTexture1DArraySize;
        // The maximum size of the 2D texture.
        i32 maxTexture2DSize;
        // The maximum length of 2D textures array.
        i32 maxTexture2DArraySize;
        // The maximum size of the 3D texture.
        i32 maxTexture3DSize;
        // The maximum size of the cube texture (both width and height).
        i32 maxTextureCubeSize;
        // The maximum degree of anisotropic filtering used for texture sampling.
        f32 maxSamplerAnisotropy;
    };

}