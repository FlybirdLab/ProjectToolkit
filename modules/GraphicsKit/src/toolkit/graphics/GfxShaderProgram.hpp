/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/types/String.hpp"
#include <toolkit/graphics/common.hpp>

namespace toolkit
{
    class GfxShader;
    class GfxVertexLayout;

    // The shader program metadata container. Contains description about resources used by the shader.
    struct TK_API ShaderBindings
    {
    public:
        u32 instructionsCount;
        u32 usedCBsMask;
        u32 usedSRsMask;
        u32 usedUAsMask;

    public:
        TK_FORCE_INLINE bool isUsingCB(u32 slotIndex) const
        {
            return (usedCBsMask & (1u << slotIndex)) != 0u;
        }

        TK_FORCE_INLINE bool isUsingSR(u32 slotIndex) const
        {
            return (usedSRsMask & (1u << slotIndex)) != 0u;
        }

        TK_FORCE_INLINE bool isUsingUA(u32 slotIndex) const
        {
            return (usedUAsMask & (1u << slotIndex)) != 0u;
        }
    };

    struct GfxShaderProgramInitializer
    {
        TkString name;
        ShaderBindings bindings;
        GfxShaderFlags flags;
#if TK_DEBUG
        GfxShader *owner;
#endif
    };

    // Mini program that can run on the GPU.
    class TK_API GfxShaderProgram
    {
    public:
        virtual ~GfxShaderProgram() = default;

    public:
        // Get name of the shader program.
        TK_FORCE_INLINE const TkString &getName() const
        {
            return m_name;
        }

        // Get the shader resource bindings.
        TK_FORCE_INLINE const ShaderBindings &getBindings() const
        {
            return m_bindings;
        }

        // Get the shader flags.
        TK_FORCE_INLINE GfxShaderFlags getFlags() const
        {
            return m_flags;
        }

    public:
        // Get shader program stage type.
        virtual ShaderStage getStage() const = 0;
        // Get buffer handle (platform dependent).
        virtual void *getBufferHandle() const = 0;
        // Get buffer size (in bytes).
        virtual u32 getBufferSize() const = 0;

    protected:
        void init(const GfxShaderProgramInitializer &initializer);

    protected:
        TkString m_name;
        ShaderBindings m_bindings;
        GfxShaderFlags m_flags;
#if TK_DEBUG
        GfxShader *m_owner;
#endif
    };
    
    // Vertex Shader program
    class GfxShaderProgramVS : public GfxShaderProgram
    {
    public:
        // Vertex shader inputs layout. Used to ensure that binded vertex buffers provide all required elements.
        GfxVertexLayout *InputLayout = nullptr;

    public:
        ShaderStage getStage() const override
        {
            return ShaderStage::kVertex;
        }
    };

    // Geometry Shader program
    class GfxShaderProgramGS : public GfxShaderProgram
    {
    public:
        ShaderStage getStage() const override
        {
            return ShaderStage::kGeometry;
        }
    };

    // Hull Shader program
    class GfxShaderProgramHS : public GfxShaderProgram
    {
    public:
        // Get the input control points count (valid range: 1-32).
        TK_FORCE_INLINE i32 GetControlPointsCount() const
        {
            return m_controlPointsCount;
        }

        ShaderStage getStage() const override
        {
            return ShaderStage::kHull;
        }

    protected:
        i32 m_controlPointsCount = 0;

    };

    // Domain Shader program
    class GfxShaderProgramDS : public GfxShaderProgram
    {
    public:
        ShaderStage getStage() const override
        {
            return ShaderStage::kDomain;
        }
    };

    // Pixel Shader program
    class GfxShaderProgramPS : public GfxShaderProgram
    {
    public:
        ShaderStage getStage() const override
        {
            return ShaderStage::kPixel;
        }
    };
    
    // Compute Shader program
    class GfxShaderProgramCS : public GfxShaderProgram
    {
    public:
        ShaderStage getStage() const override
        {
            return ShaderStage::kCompute;
        }
    };

}