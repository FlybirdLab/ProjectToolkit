/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/graphics/typedef.hpp>
#include <toolkit/graphics/environment.hpp>
#include <toolkit/shader/common.hpp>

#include <toolkit/base/Enum.hpp>
#include <toolkit/base/Hash.hpp>

namespace toolkit
{
    // Helper type used to synchronize CPU and GPU work
    typedef u64 GfxSyncPoint;
    // Default latency (in frames) between CPU and GPU used for async gpu jobs
    static constexpr u32 kGfxAsyncLatency = 2;

    // Graphics resource types.
    enum class GfxResType
    {
        // GPU render target texture
        kRenderTarget = 0,
        // GPU texture
        kTexture,
        // GPU cube texture (cubemap)
        kCubeTexture,
        // GPU volume texture (3D)
        kVolumeTexture,
        // GPU buffer
        kBuffer,
        // GPU shader
        kShader,
        // GPU pipeline state object (PSO)
        kPipelineState,
        // GPU binding descriptor
        kDescriptor,
        // GPU timer query
        kQuery,
        // GPU texture sampler
        kSampler,
        MAX
    };

    // The GPU buffer usage flags.
    enum class GfxBufferFlags
    {
        // Nothing
        kNone = 0x000,
        // Create a buffer that can be bound as a shader resource.
        kShaderResource = 0x001,
        // Create a buffer that can be bound as a vertex buffer.
        kVertexBuffer = 0x002,
        // Create a buffer that can be bound as a index buffer.
        kIndexBuffer = 0x004,
        // Create a buffer that can be bound as a unordered access.
        kUnorderedAccess = 0x008,
        // Flag for unordered access buffers that will use append feature.
        kAppend = 0x010,
        // Flag for unordered access buffers that will use counter feature.
        kCounter = 0x020,
        // Flag for unordered access buffers that will be used as draw indirect argument buffer.
        kArgument = 0x040,
        // Flag for structured buffers.
        kStructured = 0x080,
        // Flag for raw buffers.
        kRawBuffer = 0x100,
        // Creates a structured buffer that supports unordered access and append.
        kStructuredAppendBuffer = kUnorderedAccess | kStructured | kAppend,
        // Creates a structured buffer that supports unordered access and counter.
        kStructuredCounterBuffer = kUnorderedAccess | kStructured | kCounter
    };
    TK_DECLARE_ENUM_OPERATORS(GfxBufferFlags);

    // Graphics backend types.
    enum class GfxBackendType
    {
        kUnknown = 0,
        kDirectX10,
        kDirectX10_1,
        kDirectX11,
        kDirectX12,
        kOpenGL4_1,
        kOpenGL4_4,
        kOpenGLES3,
        kOpenGLES3_1,
        kNull,
        kVulkan,
        kMetal,

        kMax
    };
    const char* toString(GfxBackendType);

    // Graphics feature levels.
    enum class GfxFeatureLevel
    {
        kES2 = 0,
        kES3,
        kES3_1,
        kSM4,
        kSM5,
        kSM6,

        kMax
    };
    const char* toString(GfxFeatureLevel);

    // Multisample count level.
    enum class GfxMSAALevel : i32
    {
        kNone = 1,
        kX2 = 2,
        kX4 = 4,
        kX8 = 8,
    };

    // GPU resource usage.
    enum class GfxResourceUsage
    {
        kDefault = 0,
        kDynamic,
        kStagingUpload,
        kStagingReadback,
        kStaging,
    };

    // GPU resource map mode.
    enum class GfxResourceMapMode
    {
        kRead = 0x01,
        kWrite = 0x02,
        kReadWrite = kRead | kWrite,
    };

    // Primitive topology type.
    enum class GfxPrimitiveTopologyType
    {
        kUndefined = 0,
        kPoint,
        kLine,
        kTriangle,
    };

    // Primitive culling mode.
    enum class GfxCullMode : byte
    {
        kNormal = 0,
        kInverted = 1,
        kTwoSided = 2,
    };

    // Comparison function mode.
    enum class GfxComparisonFunc : byte
    {
        kNever = 1,
        kLess,
        kEqual,
        kLessEqual,
        kGreater,
        kNotEqual,
        kGreaterEqual,
        kAlways,

        kMax
    };

    // Possible fatal error types that cause engine exit.
    enum class FatalErrorType
    {
        // No fatal error set.
        kNone,
        // Not defined or custom error.
        kUnknown,
        // Runtime exception caught by the handler (eg. stack overflow, invalid memory address access).
        kException,
        // Data assertion failed (eg. invalid value or code usage).
        kAssertion,
        // Program run out of memory to allocate.
        kOutOfMemory,
        // The graphics device crashed, has been removed or restarted.
        kGPUCrash,
        // The graphics device stopped responding (eg. incorrect rendering code or bug in driver).
        kGPUHang,
        // The graphics device run out of video memory to allocate.
        kGPUOutOfMemory,
    };
    const char* toString(FatalErrorType);

    // Render target blending mode descriptor.
    struct TK_API GfxBlendingMode
    {
        enum class Blend
        {
            kZero = 1,
            kOne,
            kSrcColor,
            kInvSrcColor,
            kSrcAlpha,
            kInvSrcAlpha,
            kDestAlpha,
            kInvDestAlpha,
            kDestColor,
            kInvDestColor,
            kSrcAlphaSat,

            kBlendFactor = 14,
            kBlendInvFactor,
            kSrc1Color,
            kInvSrc1Color,
            kSrc1Alpha,
            kInvSrc1Alpha,

            kMax
        };

        enum class Operation
        {
            kAdd = 1,
            kSubtract,
            kRevSubtract,
            kMin,
            kMax,
        };

        enum class ColorWrite
        {
            kNone = 0,
            kRed = 1,
            kGreen = 2,
            kBlue = 4,
            kAlpha = 8,

            kAll = kRed | kGreen | kBlue | kAlpha,
            kRG = kRed | kGreen,
            kRGB = kRed | kGreen | kBlue,
            kRGBA = kAll,
        };

        bool alphaToCoverageEnable = false;
        bool blendEnable = false;

        Blend srcBlend = Blend::kOne;
        Blend destBlend = Blend::kZero;
        Operation blendOp = Operation::kAdd;

        Blend srcBlendAlpha = Blend::kOne;
        Blend destBlendAlpha = Blend::kZero;
        Operation blendOpAlpha = Operation::kAdd;

        ColorWrite renderTargetWriteMask = ColorWrite::kAll;

        bool operator==(const GfxBlendingMode &other) const;
        bool operator!=(const GfxBlendingMode &other) const
        {
            return !(*this == other);
        }

        static GfxBlendingMode sOpaque;
        static GfxBlendingMode sAdditive;
        static GfxBlendingMode sAlphaBlend;
        static GfxBlendingMode sAdd;
        static GfxBlendingMode sMultiply;
    };
    u32 getHash(const GfxBlendingMode &key);

    template <>
    struct HashFunc<GfxBlendingMode>
    {
        TK_FORCE_INLINE static u32 get(const GfxBlendingMode &k)
        {
            return getHash(k);
        }
    };

}
