/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/graphics/GfxShaderProgram.hpp"
#include <toolkit/graphics/common.hpp>
#include <toolkit/graphics/GfxResource.hpp>

namespace toolkit
{
    // Stencil operation modes.
    enum class StencilOperation : byte
    {
        // Keep the existing stencil data.
        kKeep,
        // Set the stencil data to 0.
        kZero,
        // Set the stencil data to the reference value (set via GPUContext::SetStencilRef).
        kReplace,
        // Increment the stencil value by 1, and clamp the result.
        kIncrementSaturated,
        // Decrement the stencil value by 1, and clamp the result.
        kDecrementSaturated,
        // Invert the stencil data.
        kInvert,
        // Increment the stencil value by 1, and wrap the result if necessary.
        kIncrement,
        // Decrement the stencil value by 1, and wrap the result if necessary.
        kDecrement,
        MAX
    };

    // Describes full graphics pipeline state within single object.
    class TK_API GfxPipelineState : public GfxResource
    {
        static GfxPipelineState *newPipeline();

    public:
        // Pipeline state description
        struct TK_API Description
        {
            // Enable/disable depth (DepthFunc and DepthWriteEnable)
            bool depthEnable;
            // Enable/disable depth write
            bool depthWriteEnable;
            // Enable/disable depth clipping
            bool depthClipEnable;
            // A function that compares depth data against existing depth data
            GfxComparisonFunc depthFunc;

            // Enable/disable stencil buffer usage
            bool stencilEnable;
            // The read mask applied to the reference value and each stencil buffer entry to determine the significant bits for the stencil test.
            u8 stencilReadMask;
            // The write mask applied to values written into the stencil buffer.
            u8 stencilWriteMask;

            // The comparison function for the stencil test.
            GfxComparisonFunc stencilFunc;
            // The stencil operation to perform when stencil testing fails.
            StencilOperation stencilFailOp;
            // The stencil operation to perform when stencil testing passes and depth testing fails.
            StencilOperation stencilDepthFailOp;
            // The stencil operation to perform when stencil testing and depth testing both pass.
            StencilOperation stencilPassOp;

            // Vertex shader program
            GfxShaderProgramVS *vs;
            // Hull shader program
            GfxShaderProgramHS *hs;
            // Domain shader program
            GfxShaderProgramDS *ds;
            // Geometry shader program
            GfxShaderProgramGS *gs;
            // Pixel shader program
            GfxShaderProgramPS *ps;

            // Input primitives topology
            GfxPrimitiveTopologyType primitiveTopology;

            // True if use wireframe rendering, otherwise false
            bool wireframe;
            // Primitives culling mode
            GfxCullMode cullMode;

            // Colors blending mode
            GfxBlendingMode blendMode;

        public:
            // Default description
            static Description Default;
            // Default description without using depth buffer at all
            static Description DefaultNoDepth;
            // Default description for fullscreen triangle rendering
            static Description DefaultFullscreenTriangle;
        };

    protected:
        ShaderBindings m_meta;

        GfxPipelineState();

    public:
#if TK_DEBUG
        // The description of the pipeline state cached on creation in debug builds. Can be used to help with rendering crashes or issues and validation.
        Description debugDesc;
#endif

    public:
        // Gets constant buffers usage mask (each set bit marks usage of the constant buffer at the bit index slot). Combined from all the used shader stages.
        TK_FORCE_INLINE u32 getUsedCBsMask() const
        {
            return m_meta.usedCBsMask;
        }

        // Gets shader resources usage mask (each set bit marks usage of the shader resource slot at the bit index slot). Combined from all the used shader stages.
        TK_FORCE_INLINE u32 getUsedSRsMask() const
        {
            return m_meta.usedSRsMask;
        }

        // Gets unordered access usage mask (each set bit marks usage of the unordered access slot at the bit index slot). Combined from all the used shader stages.
        TK_FORCE_INLINE u32 getUsedUAsMask() const
        {
            return m_meta.usedUAsMask;
        }

    public:
        // Returns true if pipeline state is valid and ready to use
        virtual bool isValid() const = 0;
        // Create new state data)
        virtual bool init(const Description &desc);

        GfxResType getType() const final override;
    };

}