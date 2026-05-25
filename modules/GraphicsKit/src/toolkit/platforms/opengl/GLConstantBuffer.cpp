/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GLConstantBuffer.hpp"

#if GFX_API_OPENGL

#include "GLNative.hpp"
#include "GLUtils.hpp"
#include "toolkit/debug/Logger.hpp"

namespace toolkit
{

    GLConstantBuffer::GLConstantBuffer(u32 size)
    {
        m_size = size;
    }

    bool GLConstantBuffer::init()
    {
        if (m_size == 0)
        {
            LogError("GLConstantBuffer::init: zero size");
            return false;
        }
        GLuint h = 0;
        glGenBuffers(1, &h);
        if (h == 0)
        {
            LogError("GLConstantBuffer::init: glGenBuffers failed");
            return false;
        }
        glBindBuffer(GL_UNIFORM_BUFFER, h);
        glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(m_size), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        if (!GLCheckError("GLConstantBuffer::init"))
        {
            glDeleteBuffers(1, &h);
            return false;
        }
        m_handle = static_cast<u32>(h);
        return true;
    }

    void GLConstantBuffer::update(const void *data)
    {
        if (!m_handle || !data)
            return;
        const GLuint h = static_cast<GLuint>(m_handle);
        glBindBuffer(GL_UNIFORM_BUFFER, h);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>(m_size), data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void GLConstantBuffer::onReleaseData()
    {
        if (m_handle)
        {
            const GLuint h = static_cast<GLuint>(m_handle);
            glDeleteBuffers(1, &h);
            m_handle = 0;
        }
        GfxResource::onReleaseData();
    }

}

#elif GFX_API_OPENGLES

namespace toolkit
{
    GLConstantBuffer::GLConstantBuffer(u32 size) { m_size = size; }
    bool GLConstantBuffer::init() { return false; }
    void GLConstantBuffer::update(const void *) {}
    void GLConstantBuffer::onReleaseData() { GfxResource::onReleaseData(); }
}

#endif
