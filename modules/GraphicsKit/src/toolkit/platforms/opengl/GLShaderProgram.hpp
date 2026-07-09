/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxShader.hpp>
#include <toolkit/platforms/opengl/common.hpp>

#if GFX_API_OPENGL || GFX_API_OPENGLES

namespace toolkit
{

    // Common base for OpenGL shader programs. Owns one compiled GL shader
    // object that is later attached to a program by GLPipelineState. The
    // shader handle is destroyed when the program is destroyed; flag-detach
    // happens after link.
    template <typename BaseType>
    class GLShaderProgramBase : public BaseType
    {
    public:
        GLShaderProgramBase(const GfxShaderProgramInitializer &initializer, u32 shaderHandle, u32 sourceSize)
            : m_shader(shaderHandle), m_sourceSize(sourceSize)
        {
            BaseType::init(initializer);
        }

        ~GLShaderProgramBase()
        {
            destroyShader();
        }

        void *getBufferHandle() const override
        {
            // Returning the address of the GL handle storage is enough for any
            // caller that simply wants an identifier; the link path goes
            // through shaderHandle() directly.
            return const_cast<void *>(static_cast<const void *>(&m_shader));
        }

        u32 getBufferSize() const override
        {
            return m_sourceSize;
        }

        // GL shader object name (== GLuint). 0 once destroyShader has run.
        u32 shaderHandle() const { return m_shader; }

        // Free the GL shader object. Safe to call multiple times. Pipeline
        // states may detach + flag-delete shaders before the program object
        // itself is gone -- this lets that flow re-finalize cleanly.
        void destroyShader();

    private:
        u32 m_shader = 0;
        u32 m_sourceSize = 0;
    };

    class GLShaderProgramVS : public GLShaderProgramBase<GfxShaderProgramVS>
    {
    public:
        GLShaderProgramVS(const GfxShaderProgramInitializer &initializer, u32 shaderHandle, u32 sourceSize, GfxVertexLayout *inputLayoutIn, GfxVertexLayout *vertexLayoutIn)
            : GLShaderProgramBase<GfxShaderProgramVS>(initializer, shaderHandle, sourceSize)
        {
            this->inputLayout = inputLayoutIn;
            this->layout = vertexLayoutIn;
        }
    };

    class GLShaderProgramPS : public GLShaderProgramBase<GfxShaderProgramPS>
    {
    public:
        GLShaderProgramPS(const GfxShaderProgramInitializer &initializer, u32 shaderHandle, u32 sourceSize)
            : GLShaderProgramBase<GfxShaderProgramPS>(initializer, shaderHandle, sourceSize)
        {
        }
    };

}

#endif
