/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GLTexture.hpp"

#if GFX_API_OPENGL

#include "GLNative.hpp"
#include "GLUtils.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/memory/Memory.hpp"

namespace toolkit
{

    // ---- GLTextureView ---------------------------------------------------

    void GLTextureView::initShaderResource(GLTexture *owner, u32 texture, u32 target, PixelFormat format, i32 arrayOrDepthIndex, i32 mipMapIndex)
    {
        m_kind = Kind::kShaderResource;
        m_owner = owner;
        m_swapChain = nullptr;
        m_texture = texture;
        m_target = target;
        m_arrayOrDepthIndex = arrayOrDepthIndex;
        m_mipMapIndex = mipMapIndex;
        init(owner, format, owner ? owner->multiSampleLevel() : GfxMSAALevel::kNone);
    }

    void GLTextureView::initRenderTarget(GLTexture *owner, u32 texture, u32 target, PixelFormat format, i32 arrayOrDepthIndex, i32 mipMapIndex)
    {
        m_kind = Kind::kRenderTarget;
        m_owner = owner;
        m_swapChain = nullptr;
        m_texture = texture;
        m_target = target;
        m_arrayOrDepthIndex = arrayOrDepthIndex;
        m_mipMapIndex = mipMapIndex;
        init(owner, format, owner ? owner->multiSampleLevel() : GfxMSAALevel::kNone);
    }

    void GLTextureView::initDepthStencil(GLTexture *owner, u32 texture, u32 target, PixelFormat format, i32 arrayOrDepthIndex, i32 mipMapIndex)
    {
        m_kind = Kind::kDepthStencil;
        m_owner = owner;
        m_swapChain = nullptr;
        m_texture = texture;
        m_target = target;
        m_arrayOrDepthIndex = arrayOrDepthIndex;
        m_mipMapIndex = mipMapIndex;
        init(owner, format, owner ? owner->multiSampleLevel() : GfxMSAALevel::kNone);
    }

    // ---- GLTexture -------------------------------------------------------

    GLTexture::~GLTexture()
    {
        GLTexture::onReleaseData();
    }

    bool GLTexture::onInit()
    {
        const auto &desc = m_desc;

        if (desc.dimensions != TextureDimensions::kTexture)
        {
            GLReportUnsupported("GLTexture: dimensions other than Texture2D");
            return false;
        }
        if (desc.isUnorderedAccess())
        {
            GLReportUnsupported("GLTexture: unordered-access flag");
            return false;
        }
        if (desc.isCubeMap() || desc.isVolume() || desc.isArray())
        {
            GLReportUnsupported("GLTexture: cube/volume/array dimensions");
            return false;
        }
        if (desc.isMultiSample())
        {
            GLReportUnsupported("GLTexture: multisample");
            return false;
        }

        GLTextureFormatInfo info;
        if (!GLGetTextureFormatInfo(desc.format, info))
            return false;

        const bool isDepth = desc.isDepthStencil();
        const bool isColorRT = desc.isRenderTarget();
        const bool isShaderResource = desc.isShaderResource();

        if (isDepth && isColorRT)
        {
            LogError("GLTexture: cannot be both color RenderTarget and DepthStencil");
            return false;
        }
        if (isDepth && !info.depth)
        {
            LogError("GLTexture: DepthStencil flag set but format %i is not a depth format", static_cast<i32>(desc.format));
            return false;
        }
        if (!isDepth && info.depth)
        {
            LogError("GLTexture: depth format %i requested but DepthStencil flag is not set", static_cast<i32>(desc.format));
            return false;
        }

        GLuint h = 0;
        glGenTextures(1, &h);
        if (h == 0)
        {
            LogError("glGenTextures failed");
            return false;
        }

        m_target = GL_TEXTURE_2D;
        glBindTexture(GL_TEXTURE_2D, h);

        // Basic default parameters; samplers override these when bound for SR.
        if (isDepth)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        else
        {
            const bool hasMips = desc.mipLevels > 1;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, hasMips ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            if (isColorRT)
            {
                // RT textures default to clamp; samplers can override later.
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
            else
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, desc.mipLevels > 0 ? desc.mipLevels - 1 : 0);

        // Allocate storage for every mip level. Upload happens later via
        // GLContext::updateTexture / GfxTexture::uploadMipMapAsync.
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        for (i32 mip = 0; mip < desc.mipLevels; ++mip)
        {
            i32 mipW = 0, mipH = 0;
            getMipSize(mip, mipW, mipH);
            if (mipW <= 0)
                mipW = 1;
            if (mipH <= 0)
                mipH = 1;
            glTexImage2D(
                GL_TEXTURE_2D,
                mip,
                static_cast<GLint>(info.internalFormat),
                static_cast<GLsizei>(mipW),
                static_cast<GLsizei>(mipH),
                0,
                info.format,
                info.type,
                nullptr);
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        if (!GLCheckError("GLTexture::onInit"))
        {
            glDeleteTextures(1, &h);
            m_target = 0;
            return false;
        }

        m_handle = static_cast<u32>(h);

        // Create the typed views. A texture may sit on multiple roles:
        //   - DepthStencil texture: dsView only (no SR sampling in Phase 5)
        //   - Color RT texture: rtView always; srView too if kShaderResource flag
        //   - Plain SR texture: srView only
        if (isDepth)
        {
            m_dsView = TkMemory::newObject<GLTextureView>();
            if (m_dsView)
                m_dsView->initDepthStencil(this, m_handle, m_target, desc.format, 0, 0);
        }
        else if (isColorRT)
        {
            m_rtView = TkMemory::newObject<GLTextureView>();
            if (m_rtView)
                m_rtView->initRenderTarget(this, m_handle, m_target, desc.format, 0, 0);
            if (isShaderResource)
            {
                m_srView = TkMemory::newObject<GLTextureView>();
                if (m_srView)
                    m_srView->initShaderResource(this, m_handle, m_target, desc.format, 0, 0);
            }
        }
        else
        {
            m_srView = TkMemory::newObject<GLTextureView>();
            if (m_srView)
                m_srView->initShaderResource(this, m_handle, m_target, desc.format, 0, 0);
        }
        return true;
    }

    void GLTexture::onReleaseData()
    {
        TK_SAFE_DELETE(m_srView);
        TK_SAFE_DELETE(m_rtView);
        TK_SAFE_DELETE(m_dsView);
        if (m_handle)
        {
            const GLuint h = static_cast<GLuint>(m_handle);
            glDeleteTextures(1, &h);
            m_handle = 0;
        }
        m_target = 0;
        GfxTexture::onReleaseData();
    }

    GfxTextureView *GLTexture::view(i32 arrayOrDepthIndex) const
    {
        if (arrayOrDepthIndex != 0)
            return nullptr;
        // Prefer SR view; fall back to RT view (which is sampleable when the
        // RT also has kShaderResource flag).
        if (m_srView)
            return m_srView;
        if (m_rtView)
            return m_rtView;
        if (m_dsView)
            return m_dsView;
        return nullptr;
    }

    GfxTextureView *GLTexture::view(i32 arrayOrDepthIndex, i32 mipMapIndex) const
    {
        if (arrayOrDepthIndex != 0 || mipMapIndex != 0)
            return nullptr;
        return view(arrayOrDepthIndex);
    }

}

#elif GFX_API_OPENGLES

namespace toolkit
{
    void GLTextureView::initShaderResource(GLTexture *, u32, u32, PixelFormat, i32, i32) {}
    void GLTextureView::initRenderTarget(GLTexture *, u32, u32, PixelFormat, i32, i32) {}
    void GLTextureView::initDepthStencil(GLTexture *, u32, u32, PixelFormat, i32, i32) {}
    GLTexture::~GLTexture() {}
    bool GLTexture::onInit() { return false; }
    void GLTexture::onReleaseData() { GfxTexture::onReleaseData(); }
    GfxTextureView *GLTexture::view(i32) const { return nullptr; }
    GfxTextureView *GLTexture::view(i32, i32) const { return nullptr; }
}

#endif
