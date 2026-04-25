/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "GfxBuffer.hpp"
#include <toolkit/graphics/common.hpp>

namespace toolkit
{

    // Which resources are supported for a given format and given device.
    enum class FormatSupport : i32
    {
        // No features supported.
        kNone = 0,
        // Buffer resources supported.
        kBuffer = 1,
        // Vertex buffers supported.
        kInputAssemblyVertexBuffer = 2,
        // Index buffers supported.
        kInputAssemblyIndexBuffer = 4,
        // Streaming output buffers supported.
        kStreamOutputBuffer = 8,
        // 1D texture resources supported.
        kTexture1D = 16,
        // 2D texture resources supported.
        kTexture2D = 32,
        // 3D texture resources supported.
        kTexture3D = 64,
        // Cube texture resources supported.
        kTextureCube = 128,
        // The shader Load function for texture objects is supported.
        kShaderLoad = 256,
        // The shader Sample function for texture objects is supported.
        kShaderSample = 512,
        // The shader SampleCmp and SampleCmpLevelZero functions for texture objects are supported.
        kShaderSampleComparison = 1024,
        // Unused.
        kShaderSampleMonoText = 2048,
        // Mipmaps are supported.
        kMip = 4096,
        // Automatic generation of mipmaps is supported.
        kMipAutogen = 8192,
        // Render targets are supported.
        kRenderTarget = 16384,
        // Blend operations supported.
        kBlendable = 32768,
        // Depth stencils supported.
        kDepthStencil = 65536,
        // CPU locking supported.
        kCpuLockable = 131072,
        // Multisample antialiasing (MSAA) resolve operations are supported.
        kMultisampleResolve = 262144,
        // Format can be displayed on screen.
        kDisplay = 524288,
        // Format can't be cast to another format.
        kCastWithinBitLayout = 1048576,
        // Format can be used as a multi-sampled render target.
        kMultisampleRenderTarget = 2097152,
        // Format can be used as a multi-sampled texture and read into a shader with the shader Load function.
        kMultisampleLoad = 4194304,
        // Format can be used with the shader gather function.
        kShaderGather = 8388608,
        // Format supports casting when the resource is a back buffer.
        kBackBufferCast = 16777216,
        // Format can be used for an unordered access view.
        kTypedUnorderedAccessView = 33554432,
        // Format can be used with the shader gather with comparison function.
        kShaderGatherComparison = 67108864,
        // Format can be used with the decoder output.
        kDecoderOutput = 134217728,
        // Format can be used with the video processor output.
        kVideoProcessorOutput = 268435456,
        // Format can be used with the video processor input.
        kVideoProcessorInput = 536870912,
        // Format can be used with the video encoder.
        kVideoEncoder = 1073741824,
    };
    TK_DECLARE_ENUM_OPERATORS(FormatSupport);

    // The features exposed for a particular format
    struct FormatFeatures
    {
    public:
        FormatFeatures() = default;
        FormatFeatures(const PixelFormat format, GfxMSAALevel msaaLevelMax, FormatSupport formatSupport)
            : levelMaxMSAA(msaaLevelMax), support(formatSupport)
        {
        }

    public:
        // Get the maximum MSAA sample count for a particular <see cref="PixelFormat"/>.
        GfxMSAALevel levelMaxMSAA;
        // Support of a given format on the installed video device.
        FormatSupport support;
    };

}