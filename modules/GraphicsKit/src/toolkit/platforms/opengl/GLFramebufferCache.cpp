/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GLFramebufferCache.hpp"

#if GFX_API_OPENGL

#include "GLNative.hpp"
#include "GLTexture.hpp"
#include "GLUtils.hpp"
#include "toolkit/debug/Logger.hpp"

namespace toolkit
{

    namespace
    {
        // Pull width/height from the attached texture so the caller of
        // getOrCreate can program a matching glViewport without having to
        // re-read GfxTexture state.
        void resolveSize(GLTextureView *view, i32 &outW, i32 &outH)
        {
            if (!view)
                return;
            GLTexture *owner = view->owner();
            if (!owner)
                return;
            i32 w = 0, h = 0;
            owner->getMipSize(view->mipMapIndex(), w, h);
            if (w > 0)
                outW = w;
            if (h > 0)
                outH = h;
        }

        bool viewIsDepthStencilFormat(GLTextureView *depth)
        {
            if (!depth)
                return false;
            GLTextureFormatInfo info;
            if (!GLGetTextureFormatInfo(depth->getFormat(), info))
                return false;
            return info.stencil;
        }
    }

    bool GLFramebufferCache::init()
    {
        return true;
    }

    void GLFramebufferCache::shutdown()
    {
        clear();
    }

    void GLFramebufferCache::clear()
    {
        for (i32 i = 0; i < m_entries.count(); ++i)
        {
            const GLuint fbo = static_cast<GLuint>(m_entries[i].fbo);
            if (fbo != 0)
                glDeleteFramebuffers(1, &fbo);
        }
        m_entries.clear();
    }

    void GLFramebufferCache::invalidateForView(GLTextureView *view)
    {
        if (!view)
            return;
        for (i32 i = m_entries.count() - 1; i >= 0; --i)
        {
            auto &e = m_entries[i];
            if (e.key.color == view || e.key.depth == view)
            {
                const GLuint fbo = static_cast<GLuint>(e.fbo);
                if (fbo != 0)
                    glDeleteFramebuffers(1, &fbo);
                m_entries.removeAt(i);
            }
        }
    }

    u32 GLFramebufferCache::getOrCreate(GLTextureView *color, GLTextureView *depth)
    {
        if (!color && !depth)
        {
            LogError("GLFramebufferCache::getOrCreate: no attachments");
            return 0;
        }

        const Key key{color, depth};
        for (i32 i = 0; i < m_entries.count(); ++i)
        {
            if (m_entries[i].key == key)
                return m_entries[i].fbo;
        }

        i32 width = 0, height = 0;
        const u32 fbo = createFramebuffer(color, depth, width, height);
        if (fbo == 0)
            return 0;

        Entry entry;
        entry.key = key;
        entry.fbo = fbo;
        entry.width = width;
        entry.height = height;
        m_entries.add(entry);
        return fbo;
    }

    u32 GLFramebufferCache::createFramebuffer(GLTextureView *color, GLTextureView *depth, i32 &outWidth, i32 &outHeight)
    {
        // Phase 5: refuse non-zero layer attachments.
        if (color && color->arrayOrDepthIndex() != 0)
        {
            GLReportUnsupported("framebuffer layer attachment");
            return 0;
        }
        if (depth && depth->arrayOrDepthIndex() != 0)
        {
            GLReportUnsupported("framebuffer layer attachment");
            return 0;
        }

        GLuint fbo = 0;
        glGenFramebuffers(1, &fbo);
        if (fbo == 0)
        {
            LogError("glGenFramebuffers failed");
            return 0;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        if (color)
        {
            glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0,
                static_cast<GLenum>(color->target()),
                static_cast<GLuint>(color->texture()),
                color->mipMapIndex());
            const GLenum drawBuf = GL_COLOR_ATTACHMENT0;
            glDrawBuffers(1, &drawBuf);
            resolveSize(color, outWidth, outHeight);
        }
        else
        {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }

        if (depth)
        {
            const GLenum attachment = viewIsDepthStencilFormat(depth)
                ? GL_DEPTH_STENCIL_ATTACHMENT
                : GL_DEPTH_ATTACHMENT;
            glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                attachment,
                static_cast<GLenum>(depth->target()),
                static_cast<GLuint>(depth->texture()),
                depth->mipMapIndex());
            if (outWidth == 0 || outHeight == 0)
                resolveSize(depth, outWidth, outHeight);
        }

        const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            LogError("GLFramebufferCache: framebuffer incomplete (status=0x%X)", status);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &fbo);
            return 0;
        }

        // Leave the FBO bound on success; GLContext::setRenderTarget rebinds
        // anyway, but this avoids an extra bind round-trip on first use.
        return static_cast<u32>(fbo);
    }

}

#elif GFX_API_OPENGLES

namespace toolkit
{
    bool GLFramebufferCache::init() { return false; }
    void GLFramebufferCache::shutdown() {}
    void GLFramebufferCache::clear() {}
    void GLFramebufferCache::invalidateForView(GLTextureView *) {}
    u32 GLFramebufferCache::getOrCreate(GLTextureView *, GLTextureView *) { return 0; }
    u32 GLFramebufferCache::createFramebuffer(GLTextureView *, GLTextureView *, i32 &, i32 &) { return 0; }
}

#endif
