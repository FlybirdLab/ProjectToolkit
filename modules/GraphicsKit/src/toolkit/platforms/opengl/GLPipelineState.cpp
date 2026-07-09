/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GLPipelineState.hpp"

#if GFX_API_OPENGL

#include "GLNative.hpp"
#include "GLShaderProgram.hpp"
#include "GLUtils.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/memory/Memory.hpp"

namespace toolkit
{

    namespace
    {
        // Stable attribute-location table. Must match the slot order used by
        // the GLSL we emit (or that the user hand-writes for Phase 3 tests).
        struct AttribBinding
        {
            i32 location;
            const char *semantic;
        };

        constexpr AttribBinding kAttribBindings[] = {
            {0, "POSITION"},
            {1, "COLOR"},
            {2, "NORMAL"},
            {3, "TANGENT"},
            {4, "BLENDINDICES"},
            {5, "BLENDWEIGHTS"},
            {6, "TEXCOORD0"},
            {7, "TEXCOORD1"},
            {8, "TEXCOORD2"},
            {9, "TEXCOORD3"},
            {10, "TEXCOORD4"},
            {11, "TEXCOORD5"},
            {12, "TEXCOORD6"},
            {13, "TEXCOORD7"},
            {14, "ATTRIBUTE0"},
            {15, "ATTRIBUTE1"},
            {16, "ATTRIBUTE2"},
            {17, "ATTRIBUTE3"},
        };
    }

    bool GLPipelineState::isValid() const
    {
        return m_valid && m_program != 0;
    }

    bool GLPipelineState::init(const Description &desc)
    {
        // Merge metadata (used CB/SR/UA masks) and stash any debug copy.
        if (!GfxPipelineState::init(desc))
            return false;

        if (!desc.vs)
        {
            LogError("GLPipelineState::init: missing vertex shader");
            return false;
        }
        if (!desc.ps)
        {
            LogError("GLPipelineState::init: missing pixel shader");
            return false;
        }
        if (desc.hs || desc.ds || desc.gs)
        {
            GLReportUnsupported("GLPipelineState::init: HS/DS/GS stages");
            return false;
        }

        auto *vs = static_cast<GLShaderProgramVS *>(desc.vs);
        auto *ps = static_cast<GLShaderProgramPS *>(desc.ps);
        const GLuint vsShader = static_cast<GLuint>(vs->shaderHandle());
        const GLuint psShader = static_cast<GLuint>(ps->shaderHandle());
        if (vsShader == 0 || psShader == 0)
        {
            LogError("GLPipelineState::init: shader handles are zero");
            return false;
        }

        const GLuint program = glCreateProgram();
        if (program == 0)
        {
            LogError("glCreateProgram failed");
            return false;
        }

        glAttachShader(program, vsShader);
        glAttachShader(program, psShader);

        // Bind attribute locations BEFORE linking so the layout matches our
        // expected semantic table even when the GLSL didn't use
        // layout(location=N) qualifiers.
        for (const auto &b : kAttribBindings)
            glBindAttribLocation(program, static_cast<GLuint>(b.location), b.semantic);

        glLinkProgram(program);

        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE)
        {
            GLint logLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
            if (logLen > 0)
            {
                char *log = TkMemory::newArray<char>(static_cast<u32>(logLen));
                if (log)
                {
                    glGetProgramInfoLog(program, logLen, nullptr, log);
                    LogError("GLPipelineState::init: link failed:\n%s", log);
                    TkMemory::deleteArray(log, static_cast<u32>(logLen));
                }
            }
            else
            {
                LogError("GLPipelineState::init: link failed (no info log)");
            }
            glDetachShader(program, vsShader);
            glDetachShader(program, psShader);
            glDeleteProgram(program);
            return false;
        }

        // Shaders are linked into the program; we no longer need them attached
        // for further binding -- glDetach is safe and avoids accidental
        // double-free if a shader program object outlives the pipeline.
        glDetachShader(program, vsShader);
        glDetachShader(program, psShader);

        // Bind uniform blocks "CB0".."CB15" to fixed binding slots, matching
        // GLContext::bindCB() which uses glBindBufferBase(GL_UNIFORM_BUFFER, slot, ...).
        for (i32 slot = 0; slot < GFX_MAX_CB_BINDED; ++slot)
        {
            char name[8];
            // No printf dependency: small inline formatter is enough for "CB%d".
            name[0] = 'C';
            name[1] = 'B';
            if (slot < 10)
            {
                name[2] = static_cast<char>('0' + slot);
                name[3] = '\0';
            }
            else
            {
                name[2] = static_cast<char>('0' + slot / 10);
                name[3] = static_cast<char>('0' + slot % 10);
                name[4] = '\0';
            }
            const GLuint index = glGetUniformBlockIndex(program, name);
            if (index != GL_INVALID_INDEX)
                glUniformBlockBinding(program, index, static_cast<GLuint>(slot));
        }

        // Bind texture sampler uniforms "SR0".."SR15" to texture units 0..15.
        // glUniform1i needs the program to be current.
        glUseProgram(program);
        for (i32 slot = 0; slot < GFX_MAX_SR_BINDED; ++slot)
        {
            char name[8];
            name[0] = 'S';
            name[1] = 'R';
            if (slot < 10)
            {
                name[2] = static_cast<char>('0' + slot);
                name[3] = '\0';
            }
            else
            {
                name[2] = static_cast<char>('0' + slot / 10);
                name[3] = static_cast<char>('0' + slot % 10);
                name[4] = '\0';
            }
            const GLint loc = glGetUniformLocation(program, name);
            if (loc >= 0)
                glUniform1i(loc, slot);
        }
        glUseProgram(0);

        m_desc = desc;
        m_program = static_cast<u32>(program);
        m_valid = true;
        GLCheckError("GLPipelineState::init");
        return true;
    }

    void GLPipelineState::onReleaseData()
    {
        if (m_program)
        {
            glDeleteProgram(static_cast<GLuint>(m_program));
            m_program = 0;
        }
        m_valid = false;
        GfxResource::onReleaseData();
    }

}

#elif GFX_API_OPENGLES

namespace toolkit
{
    bool GLPipelineState::isValid() const { return false; }
    bool GLPipelineState::init(const Description &) { return false; }
    void GLPipelineState::onReleaseData() { GfxResource::onReleaseData(); }
}

#endif
