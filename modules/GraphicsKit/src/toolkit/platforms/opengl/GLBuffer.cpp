/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GLBuffer.hpp"

#if GFX_API_OPENGL

#include "GLNative.hpp"
#include "GLUtils.hpp"
#include "toolkit/debug/Logger.hpp"

namespace toolkit
{
    bool resolveTarget(GfxBufferFlags flags, GLenum &outTarget)
    {
        if ((flags & GfxBufferFlags::kVertexBuffer) != GfxBufferFlags::kNone)
        {
            outTarget = GL_ARRAY_BUFFER;
            return true;
        }
        if ((flags & GfxBufferFlags::kIndexBuffer) != GfxBufferFlags::kNone)
        {
            outTarget = GL_ELEMENT_ARRAY_BUFFER;
            return true;
        }
        // Some callers may flag a UB via kShaderResource on a non-structured buffer.
        // True constant buffers go through GfxConstantBuffer which is unimplemented
        // in Phase 2 (createConstantBuffer returns nullptr). Treat any kShaderResource
        // without structured/raw as a uniform buffer.
        if ((flags & GfxBufferFlags::kShaderResource) != GfxBufferFlags::kNone)
        {
            outTarget = GL_UNIFORM_BUFFER;
            return true;
        }
        return false;
    }

    GLenum resolveUsage(GfxResourceUsage usage)
    {
        switch (usage)
        {
        case GfxResourceUsage::kDynamic:
            return GL_DYNAMIC_DRAW;
        case GfxResourceUsage::kStagingUpload:
        case GfxResourceUsage::kStagingReadback:
        case GfxResourceUsage::kStaging:
            return GL_STREAM_DRAW;
        case GfxResourceUsage::kDefault:
        default:
            return GL_STATIC_DRAW;
        }
    }

    bool GLBuffer::onInit()
    {
        const auto flags = m_desc.flags;

        // Reject UA / structured / raw / append / counter / argument buffers up front.
        constexpr GfxBufferFlags kUnsupportedMask =
            GfxBufferFlags::kUnorderedAccess |
            GfxBufferFlags::kStructured |
            GfxBufferFlags::kRawBuffer |
            GfxBufferFlags::kAppend |
            GfxBufferFlags::kCounter |
            GfxBufferFlags::kArgument;
        if ((flags & kUnsupportedMask) != GfxBufferFlags::kNone)
        {
            GLReportUnsupported("GLBuffer with UA/Structured/Raw/Append/Counter/Argument flags");
            return false;
        }

        GLenum target = 0;
        if (!resolveTarget(flags, target))
        {
            GLReportUnsupported("GLBuffer with no recognised usage flag");
            return false;
        }

        if (m_desc.size == 0)
        {
            LogError("GLBuffer::onInit: zero-sized buffer");
            return false;
        }

        GLuint handle = 0;
        glGenBuffers(1, &handle);
        if (handle == 0)
        {
            LogError("glGenBuffers failed (current context?)");
            return false;
        }

        glBindBuffer(target, handle);
        glBufferData(target, static_cast<GLsizeiptr>(m_desc.size), m_desc.initData, resolveUsage(m_desc.usage));
        glBindBuffer(target, 0);

        if (!GLCheckError("GLBuffer::onInit"))
        {
            glDeleteBuffers(1, &handle);
            return false;
        }

        m_handle = static_cast<u32>(handle);
        m_target = static_cast<u32>(target);
        return true;
    }

    void GLBuffer::onReleaseData()
    {
        if (m_handle)
        {
            const GLuint h = static_cast<GLuint>(m_handle);
            glDeleteBuffers(1, &h);
            m_handle = 0;
        }
        m_target = 0;
        m_mapped = false;
        GfxBuffer::onReleaseData();
    }

    void *GLBuffer::map(GfxResourceMapMode mode)
    {
        if (!m_handle || m_mapped)
            return nullptr;

        GLbitfield access = 0;
        if ((static_cast<u32>(mode) & static_cast<u32>(GfxResourceMapMode::kRead)) != 0)
            access |= GL_MAP_READ_BIT;
        if ((static_cast<u32>(mode) & static_cast<u32>(GfxResourceMapMode::kWrite)) != 0)
            access |= GL_MAP_WRITE_BIT;
        if (access == 0)
            access = GL_MAP_WRITE_BIT;

        const GLenum target = static_cast<GLenum>(m_target);
        glBindBuffer(target, static_cast<GLuint>(m_handle));
        void *ptr = glMapBufferRange(target, 0, static_cast<GLsizeiptr>(m_desc.size), access);
        if (ptr)
            m_mapped = true;
        return ptr;
    }

    void GLBuffer::unmap()
    {
        if (!m_handle || !m_mapped)
            return;
        const GLenum target = static_cast<GLenum>(m_target);
        glUnmapBuffer(target);
        glBindBuffer(target, 0);
        m_mapped = false;
    }

}

#elif GFX_API_OPENGLES

// OpenGL ES path is intentionally inert in Phase 2. The OPENGLES backend is
// disabled at CMake-level so callers should never link this stub.

namespace toolkit
{
    bool GLBuffer::onInit() { return false; }
    void GLBuffer::onReleaseData() { GfxBuffer::onReleaseData(); }
    void *GLBuffer::map(GfxResourceMapMode) { return nullptr; }
    void GLBuffer::unmap() {}
}

#endif
