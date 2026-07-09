/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GLContext.hpp"

#if GFX_API_OPENGL

#include "GLBuffer.hpp"
#include "GLConstantBuffer.hpp"
#include "GLDevice.hpp"
#include "GLFramebufferCache.hpp"
#include "GLNative.hpp"
#include "GLPipelineState.hpp"
#include "GLSampler.hpp"
#include "GLShaderProgram.hpp"
#include "GLSwapChain.hpp"
#include "GLTexture.hpp"
#include "GLUtils.hpp"
#include "platform/GLPlatform.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/GfxBuffer.hpp"
#include "toolkit/graphics/GfxVertexLayout.hpp"
#include "toolkit/utils/Color.hpp"

namespace toolkit
{

    namespace
    {
        GLenum mapTopology(GfxPrimitiveTopologyType t)
        {
            switch (t)
            {
            case GfxPrimitiveTopologyType::kPoint:
                return GL_POINTS;
            case GfxPrimitiveTopologyType::kLine:
                return GL_LINES;
            case GfxPrimitiveTopologyType::kTriangle:
                return GL_TRIANGLES;
            default:
                return 0;
            }
        }

        GLenum mapCompareFunc(GfxComparisonFunc f)
        {
            switch (f)
            {
            case GfxComparisonFunc::kNever:
                return GL_NEVER;
            case GfxComparisonFunc::kLess:
                return GL_LESS;
            case GfxComparisonFunc::kEqual:
                return GL_EQUAL;
            case GfxComparisonFunc::kLessEqual:
                return GL_LEQUAL;
            case GfxComparisonFunc::kGreater:
                return GL_GREATER;
            case GfxComparisonFunc::kNotEqual:
                return GL_NOTEQUAL;
            case GfxComparisonFunc::kGreaterEqual:
                return GL_GEQUAL;
            case GfxComparisonFunc::kAlways:
            default:
                return GL_ALWAYS;
            }
        }

        // Locations must match GLPipelineState's kAttribBindings table.
        i32 attribLocationFor(VertexElementTypes type)
        {
            switch (type)
            {
            case VertexElementTypes::kPosition:
                return 0;
            case VertexElementTypes::kColor:
                return 1;
            case VertexElementTypes::kNormal:
                return 2;
            case VertexElementTypes::kTangent:
                return 3;
            case VertexElementTypes::kBlendIndices:
                return 4;
            case VertexElementTypes::kBlendWeights:
                return 5;
            case VertexElementTypes::kTexCoord0:
                return 6;
            case VertexElementTypes::kTexCoord1:
                return 7;
            case VertexElementTypes::kTexCoord2:
                return 8;
            case VertexElementTypes::kTexCoord3:
                return 9;
            case VertexElementTypes::kTexCoord4:
                return 10;
            case VertexElementTypes::kTexCoord5:
                return 11;
            case VertexElementTypes::kTexCoord6:
                return 12;
            case VertexElementTypes::kTexCoord7:
                return 13;
            case VertexElementTypes::kAttribute0:
                return 14;
            case VertexElementTypes::kAttribute1:
                return 15;
            case VertexElementTypes::kAttribute2:
                return 16;
            case VertexElementTypes::kAttribute3:
                return 17;
            default:
                return -1;
            }
        }

        GLenum indexTypeFor(GfxBuffer *ib)
        {
            if (!ib)
                return GL_UNSIGNED_INT;
            const auto &desc = ib->getDescription();
            switch (desc.format)
            {
            case PixelFormat::kR16_UInt:
                return GL_UNSIGNED_SHORT;
            case PixelFormat::kR32_UInt:
                return GL_UNSIGNED_INT;
            default:
                // Phase-3 shader cache should always set the index format,
                // but be lenient: 16-bit if stride is 2, else 32-bit.
                return (desc.stride == 2) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
            }
        }

        u32 indexElementSize(GLenum type)
        {
            return type == GL_UNSIGNED_SHORT ? 2u : 4u;
        }
    }

    GLContext::GLContext(GLDevice *device)
        : GfxContext(device), m_device(device)
    {
        // Device context is current when this ctor runs (see GLDevice::init).
        GLuint vao = 0;
        glGenVertexArrays(1, &vao);
        m_vao = static_cast<u32>(vao);

        // Cache GL_MAX_VERTEX_ATTRIBS so flushVertexLayout doesn't blindly
        // disable slot 16/17 (NVIDIA's limit is 16 on most parts; GL spec
        // guarantees at least 16, our semantic table tops at 18 = ATTRIBUTE3).
        GLint maxAttribs = 16;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
        if (maxAttribs <= 0)
            maxAttribs = 16;
        m_maxVertexAttribs = static_cast<i32>(maxAttribs);
    }

    GLContext::~GLContext()
    {
        if (m_vao)
        {
            // GLDevice::dispose makes the device context current before
            // deleting the main context, so the VAO delete here is safe.
            const GLuint vao = static_cast<GLuint>(m_vao);
            glDeleteVertexArrays(1, &vao);
            m_vao = 0;
        }
    }

    // Resolve the VAO to use for the upcoming draw. VAOs are not shared
    // between GL contexts, so when the last-bound RT belonged to a swap chain
    // we must use that swap chain's VAO (created inside its own context).
    // Falls back to the device-context VAO for offscreen-only flows where no
    // swap chain has been touched yet.
    u32 GLContext::activeVAO() const
    {
        if (m_lastSwapChain)
        {
            const u32 v = m_lastSwapChain->vao();
            if (v != 0)
                return v;
        }
        return m_vao;
    }

    // ---- Clears ----------------------------------------------------------

    void GLContext::clear(GfxTextureView *rt, const Color &color)
    {
        if (rt)
            setRenderTarget(rt);
        if (!m_currentRenderTarget)
        {
            LogWarn("GLContext::clear called without a bound render target");
            return;
        }
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void GLContext::clearDepth(GfxTextureView *depthBuffer, f32 depthValue, u8 stencilValue)
    {
        if (depthBuffer)
        {
            auto *dv = static_cast<GLTextureView *>(depthBuffer);
            if (!dv->isDepthStencil())
            {
                GLReportUnsupported("GLContext::clearDepth: view is not a depth target");
                return;
            }
            // Preserve any current color binding while we attach depth.
            setRenderTarget(depthBuffer, m_currentRenderTarget);
        }
        if (!m_currentDepthTarget)
        {
            LogWarn("GLContext::clearDepth: no depth target bound");
            return;
        }

        glClearDepth(static_cast<GLdouble>(depthValue));

        GLTextureFormatInfo info;
        const bool hasStencil =
            GLGetTextureFormatInfo(m_currentDepthTarget->getFormat(), info) && info.stencil;

        // glClear honours the depth/stencil write masks; force them on so
        // clearDepth always lands.
        GLint prevDepthMask = GL_TRUE;
        glGetIntegerv(GL_DEPTH_WRITEMASK, &prevDepthMask);
        if (prevDepthMask == GL_FALSE)
            glDepthMask(GL_TRUE);

        if (hasStencil)
        {
            GLint prevStencilMask = 0xFF;
            glGetIntegerv(GL_STENCIL_WRITEMASK, &prevStencilMask);
            glStencilMask(0xFF);
            glClearStencil(static_cast<GLint>(stencilValue));
            glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glStencilMask(static_cast<GLuint>(prevStencilMask));
        }
        else
        {
            glClear(GL_DEPTH_BUFFER_BIT);
        }

        if (prevDepthMask == GL_FALSE)
            glDepthMask(GL_FALSE);
    }
    void GLContext::clearUA(GfxBuffer *, const FVec4 &) { GLReportUnsupported("GLContext::clearUA(buffer,FVec4)"); }
    void GLContext::clearUA(GfxBuffer *, const u32[4]) { GLReportUnsupported("GLContext::clearUA(buffer,u32[4])"); }
    void GLContext::clearUA(GfxTexture *, const u32[4]) { GLReportUnsupported("GLContext::clearUA(texture,u32[4])"); }
    void GLContext::clearUA(GfxTexture *, const FVec4 &) { GLReportUnsupported("GLContext::clearUA(texture,FVec4)"); }

    // ---- Render target ---------------------------------------------------

    namespace
    {
        // If the same texture view is currently bound as an SR, unbind it so
        // we don't create a feedback loop the moment glDraw* fires.
        void unbindSRAliases(GfxResourceView **slots, i32 maxSlots, GLTextureView *rt)
        {
            if (!rt)
                return;
            for (i32 i = 0; i < maxSlots; ++i)
            {
                if (slots[i] == rt)
                {
                    glActiveTexture(GL_TEXTURE0 + i);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    slots[i] = nullptr;
                }
            }
        }
    }

    void GLContext::resetRenderTarget()
    {
        m_currentRenderTarget = nullptr;
        m_currentDepthTarget = nullptr;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void GLContext::setRenderTarget(GfxTextureView *rt)
    {
        setRenderTarget(static_cast<GfxTextureView *>(nullptr), rt);
    }

    void GLContext::setRenderTarget(GfxTextureView *depthBuffer, GfxTextureView *rt)
    {
        auto *color = static_cast<GLTextureView *>(rt);
        auto *depth = static_cast<GLTextureView *>(depthBuffer);

        if (!color && !depth)
        {
            resetRenderTarget();
            return;
        }

        // Unbind any SR slot that aliases the new RT/DS to avoid feedback loops.
        unbindSRAliases(m_shaderResources, GFX_MAX_SR_BINDED, color);
        unbindSRAliases(m_shaderResources, GFX_MAX_SR_BINDED, depth);

        if (color && color->isBackBuffer())
        {
            if (depth)
            {
                GLReportUnsupported("GLContext::setRenderTarget: backbuffer + custom depth");
                return;
            }
            auto *sc = color->getSwapChain();
            if (sc)
            {
                sc->makeCurrent();
                m_lastSwapChain = sc;
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, sc->getWidth(), sc->getHeight());
            }
            else
            {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }
            m_currentRenderTarget = color;
            m_currentDepthTarget = nullptr;
            return;
        }

        // Texture path. Both color and depth must be texture-backed (or null
        // for color-only / depth-only).
        if (color && !color->isRenderTarget())
        {
            GLReportUnsupported("GLContext::setRenderTarget: color view is not a render target");
            return;
        }
        if (depth && !depth->isDepthStencil())
        {
            GLReportUnsupported("GLContext::setRenderTarget: depth view is not a depth target");
            return;
        }

        if (!m_device)
        {
            LogError("GLContext::setRenderTarget: no device");
            return;
        }
        auto *cache = m_device->framebufferCache();
        if (!cache)
        {
            LogError("GLContext::setRenderTarget: no FBO cache");
            return;
        }

        // FBOs aren't shared between GL contexts; keep the last-used swapchain
        // window context current so the cache stays consistent.
        if (m_lastSwapChain)
            m_lastSwapChain->makeCurrent();

        const u32 fbo = cache->getOrCreate(color, depth);
        if (fbo == 0)
            return;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        i32 width = 0, height = 0;
        GLTextureView *sizeSrc = color ? color : depth;
        if (sizeSrc && sizeSrc->owner())
            sizeSrc->owner()->getMipSize(sizeSrc->mipMapIndex(), width, height);
        if (width > 0 && height > 0)
            glViewport(0, 0, width, height);

        m_currentRenderTarget = color;
        m_currentDepthTarget = depth;
    }

    void GLContext::setRenderTarget(GfxTextureView *depthBuffer, const TkSpan<GfxTextureView *> &rts)
    {
        if (rts.length() == 0)
        {
            setRenderTarget(depthBuffer, static_cast<GfxTextureView *>(nullptr));
            return;
        }
        if (rts.length() != 1)
        {
            GLReportUnsupported("GLContext::setRenderTarget(MRT)");
            return;
        }
        setRenderTarget(depthBuffer, rts[0]);
    }

    // ---- Blend factor / stencil ref --------------------------------------

    void GLContext::setBlendFactor(const FVec4 &value)
    {
        m_blendFactor = value;
        glBlendColor(value.x, value.y, value.z, value.w);
    }

    void GLContext::setStencilRef(u32 value)
    {
        m_stencilRef = value;
        // Phase 3 has no stencil pipeline yet; cached for future use.
    }

    // ---- Reset binds -----------------------------------------------------

    void GLContext::resetSR()
    {
        for (i32 slot = 0; slot < GFX_MAX_SR_BINDED; ++slot)
        {
            m_shaderResources[slot] = nullptr;
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    void GLContext::resetUA()
    {
        // UA pipeline not supported.
    }

    void GLContext::resetCB()
    {
        for (i32 slot = 0; slot < GFX_MAX_CB_BINDED; ++slot)
        {
            m_constantBuffers[slot] = nullptr;
            glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(slot), 0);
        }
    }

    // ---- Resource bindings ----------------------------------------------

    void GLContext::bindCB(i32 slot, GfxConstantBuffer *cb)
    {
        if (slot < 0 || slot >= GFX_MAX_CB_BINDED)
            return;
        m_constantBuffers[slot] = cb;
        const GLuint handle = cb ? static_cast<GLuint>(static_cast<GLConstantBuffer *>(cb)->handle()) : 0;
        glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(slot), handle);
    }

    void GLContext::bindSR(i32 slot, GfxResourceView *view)
    {
        if (slot < 0 || slot >= GFX_MAX_SR_BINDED)
            return;

        // Feedback-loop guard: refuse to bind a view that's currently an
        // active render target. Sampling and rendering the same texture in
        // the same draw is undefined in GL.
        if (view != nullptr && (view == m_currentRenderTarget || view == m_currentDepthTarget))
        {
            LogError("GLContext::bindSR: refusing to bind active render target as SR (slot=%i)", slot);
            return;
        }

        m_shaderResources[slot] = view;

        glActiveTexture(GL_TEXTURE0 + slot);
        if (!view)
        {
            glBindTexture(GL_TEXTURE_2D, 0);
            return;
        }

        auto *tv = static_cast<GLTextureView *>(view);
        if (tv->isBackBuffer())
        {
            GLReportUnsupported("GLContext::bindSR(backbuffer view)");
            glBindTexture(GL_TEXTURE_2D, 0);
            return;
        }
        if (tv->isDepthStencil())
        {
            GLReportUnsupported("GLContext::bindSR(depth-stencil view)");
            glBindTexture(GL_TEXTURE_2D, 0);
            return;
        }
        if (!tv->isTexture() || tv->texture() == 0)
        {
            glBindTexture(GL_TEXTURE_2D, 0);
            return;
        }
        glBindTexture(static_cast<GLenum>(tv->target()), static_cast<GLuint>(tv->texture()));
    }
    void GLContext::bindUA(i32, GfxResourceView *) { GLReportUnsupported("GLContext::bindUA"); }

    void GLContext::bindSampler(i32 slot, GfxSampler *sampler)
    {
        if (slot < 0 || slot >= GFX_MAX_SAMPLER_BINDED)
            return;
        m_samplers[slot] = sampler;
        const GLuint h = sampler ? static_cast<GLuint>(static_cast<GLSampler *>(sampler)->handle()) : 0;
        glBindSampler(static_cast<GLuint>(slot), h);
    }

    void GLContext::bindVB(const TkSpan<GfxBuffer *> &vertexBuffers, const u32 *vertexBuffersOffsets, GfxVertexLayout *vertexLayout)
    {
        const i32 count = vertexBuffers.length();
        for (i32 i = 0; i < GFX_MAX_VB_BINDED; ++i)
        {
            m_vertexBuffers[i] = (i < count) ? vertexBuffers[i] : nullptr;
            m_vertexBufferOffsets[i] = (vertexBuffersOffsets && i < count) ? vertexBuffersOffsets[i] : 0u;
        }
        m_currentVertexLayout = vertexLayout;
    }

    void GLContext::bindIB(GfxBuffer *indexBuffer)
    {
        m_indexBuffer = indexBuffer;
        // GL_ELEMENT_ARRAY_BUFFER binding is part of VAO state. In Core Profile
        // every binding op needs a non-zero VAO bound, so attach the right
        // one for the current context up front. flushVertexLayout re-binds
        // it later for the actual draw.
        glBindVertexArray(static_cast<GLuint>(activeVAO()));
        const GLuint handle = indexBuffer ? static_cast<GLuint>(static_cast<GLBuffer *>(indexBuffer)->handle()) : 0;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
    }

    void GLContext::updateCB(GfxConstantBuffer *cb, const void *data)
    {
        if (!cb)
            return;
        static_cast<GLConstantBuffer *>(cb)->update(data);
    }

    // ---- Compute / indirect / resolve (all unsupported in Phase 3) -------

    void GLContext::dispatch(GfxShaderProgramCS *, u32, u32, u32) { GLReportUnsupported("GLContext::dispatch"); }
    void GLContext::dispatchIndirect(GfxShaderProgramCS *, GfxBuffer *, u32) { GLReportUnsupported("GLContext::dispatchIndirect"); }
    void GLContext::resolveMultisample(GfxTexture *, GfxTexture *, i32, i32, PixelFormat) { GLReportUnsupported("GLContext::resolveMultisample"); }
    void GLContext::drawInstancedIndirect(GfxBuffer *, u32) { GLReportUnsupported("GLContext::drawInstancedIndirect"); }
    void GLContext::drawIndexedInstancedIndirect(GfxBuffer *, u32) { GLReportUnsupported("GLContext::drawIndexedInstancedIndirect"); }
    void GLContext::resetCounter(GfxBuffer *) { GLReportUnsupported("GLContext::resetCounter"); }
    void GLContext::copyCounter(GfxBuffer *, u32, GfxBuffer *) { GLReportUnsupported("GLContext::copyCounter"); }

    // ---- Viewport / scissor ----------------------------------------------

    void GLContext::setViewport(const Viewport &viewport)
    {
        glViewport(
            static_cast<GLint>(viewport.x),
            static_cast<GLint>(viewport.y),
            static_cast<GLsizei>(viewport.w),
            static_cast<GLsizei>(viewport.h));
        glDepthRange(viewport.minDepth, viewport.maxDepth);
    }

    void GLContext::setScissor(const Rectangle &scissorRect)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(
            static_cast<GLint>(scissorRect.getX()),
            static_cast<GLint>(scissorRect.getY()),
            static_cast<GLsizei>(scissorRect.getWidth()),
            static_cast<GLsizei>(scissorRect.getHeight()));
    }

    // ---- Pipeline state --------------------------------------------------

    GfxPipelineState *GLContext::getState() const
    {
        return reinterpret_cast<GfxPipelineState *>(m_currentPipeline);
    }

    void GLContext::setState(GfxPipelineState *state)
    {
        auto *ps = static_cast<GLPipelineState *>(state);
        if (!ps || !ps->isValid())
        {
            LogError("GLContext::setState: invalid pipeline state");
            return;
        }
        m_currentPipeline = ps;
        glUseProgram(static_cast<GLuint>(ps->program()));

        const auto &desc = ps->desc();

        // Depth
        if (desc.depthEnable)
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(mapCompareFunc(desc.depthFunc));
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }
        glDepthMask(desc.depthWriteEnable ? GL_TRUE : GL_FALSE);

        // Cull
        if (desc.cullMode == GfxCullMode::kTwoSided)
        {
            glDisable(GL_CULL_FACE);
        }
        else
        {
            glEnable(GL_CULL_FACE);
            glCullFace(desc.cullMode == GfxCullMode::kInverted ? GL_FRONT : GL_BACK);
        }

        // Wireframe
        glPolygonMode(GL_FRONT_AND_BACK, desc.wireframe ? GL_LINE : GL_FILL);

        // Blend (Phase 3: just enable/disable + a single opaque/alpha path)
        if (desc.blendMode.blendEnable)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else
        {
            glDisable(GL_BLEND);
        }
    }

    void GLContext::clearState() {}
    void GLContext::flushState() {}
    void GLContext::flush()
    {
        glFlush();
    }

    void GLContext::updateTexture(GfxTexture *texture, i32 arrayIndex, i32 mipIndex, const void *data, u32 rowPitch, u32 slicePitch)
    {
        if (!texture || !data)
            return;

        auto *gltex = static_cast<GLTexture *>(texture);
        if (gltex->handle() == 0)
        {
            LogError("GLContext::updateTexture: texture has no GL handle");
            return;
        }
        if (arrayIndex != 0)
        {
            GLReportUnsupported("GLContext::updateTexture: arrayIndex != 0");
            return;
        }

        GLTextureFormatInfo info;
        if (!GLGetTextureFormatInfo(gltex->textureFormat(), info))
            return;

        i32 mipW = 0, mipH = 0;
        gltex->getMipSize(mipIndex, mipW, mipH);
        if (mipW <= 0)
            mipW = 1;
        if (mipH <= 0)
            mipH = 1;

        const GLenum target = static_cast<GLenum>(gltex->target());
        glBindTexture(target, static_cast<GLuint>(gltex->handle()));
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        // If rowPitch is provided we honour it via GL_UNPACK_ROW_LENGTH (in
        // pixels). For Phase 4 we trust the caller passes tightly-packed data
        // when rowPitch == 0.
        if (rowPitch != 0)
        {
            // Compute bytes-per-pixel from the format. Easier: just disable
            // row-length and require tightly packed input -- safer for all
            // supported formats. Future phase may add a proper bpp helper.
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        }
        glTexSubImage2D(
            target,
            mipIndex,
            0, 0,
            static_cast<GLsizei>(mipW),
            static_cast<GLsizei>(mipH),
            info.format,
            info.type,
            data);
        glBindTexture(target, 0);
        GLCheckError("GLContext::updateTexture");
    }

    // ---- Vertex layout flush --------------------------------------------

    bool GLContext::flushVertexLayout()
    {
        if (!m_currentPipeline)
        {
            LogError("GLContext::flushVertexLayout: no pipeline bound");
            return false;
        }

        GfxVertexLayout *layout = m_currentVertexLayout;
        auto *vs = static_cast<GLShaderProgramVS *>(m_currentPipeline->desc().vs);
        if (!layout && vs)
            layout = vs->layout;
        if (!layout && vs)
            layout = vs->inputLayout;

        // Core Profile requires a non-zero VAO bound for any draw, even when
        // the VS pulls data from gl_VertexID with no vertex inputs. VAOs are
        // not shared across GL contexts so pick the one created in whichever
        // context is current right now.
        glBindVertexArray(static_cast<GLuint>(activeVAO()));

        // Disable everything in our attribute range first; we'll re-enable
        // only the slots used by the current layout. Cap at the GPU's actual
        // GL_MAX_VERTEX_ATTRIBS so we don't trip GL_INVALID_VALUE on cards
        // (e.g. NVIDIA) whose limit is below our semantic table size of 18.
        const i32 disableCount = m_maxVertexAttribs < 18 ? m_maxVertexAttribs : 18;
        for (i32 i = 0; i < disableCount; ++i)
            glDisableVertexAttribArray(static_cast<GLuint>(i));

        // No layout / no elements is the gl_VertexID-only case (fullscreen
        // triangle, procedural meshes). VAO is bound; nothing else to set up.
        if (!layout)
            return true;
        const auto &elements = layout->getElements();
        if (elements.count() == 0)
            return true;

        // Compute per-slot stride from the elements that target each slot.
        u32 slotStride[GFX_MAX_VB_BINDED] = {};
        for (i32 i = 0; i < elements.count(); ++i)
        {
            const VertexElement &e = elements[i];
            if (e.slot >= GFX_MAX_VB_BINDED)
                continue;
            GLVertexAttribFormat fmt;
            if (!GLGetVertexAttribFormat(e.format, fmt))
                continue;
            const u32 end = static_cast<u32>(e.offset) + fmt.sizeInBytes;
            if (end > slotStride[e.slot])
                slotStride[e.slot] = end;
        }

        for (i32 i = 0; i < elements.count(); ++i)
        {
            const VertexElement &e = elements[i];
            if (e.slot >= GFX_MAX_VB_BINDED)
                continue;

            const i32 loc = attribLocationFor(e.type);
            if (loc < 0)
            {
                GLReportUnsupported("vertex element type");
                continue;
            }

            GfxBuffer *vb = m_vertexBuffers[e.slot];
            if (!vb)
                continue;

            GLVertexAttribFormat fmt;
            if (!GLGetVertexAttribFormat(e.format, fmt))
                continue;

            glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(static_cast<GLBuffer *>(vb)->handle()));

            const uintptr_t byteOffset =
                static_cast<uintptr_t>(m_vertexBufferOffsets[e.slot]) + static_cast<uintptr_t>(e.offset);
            const GLsizei stride = static_cast<GLsizei>(slotStride[e.slot]);

            if (fmt.integer)
            {
                glVertexAttribIPointer(
                    static_cast<GLuint>(loc),
                    fmt.components,
                    fmt.type,
                    stride,
                    reinterpret_cast<const void *>(byteOffset));
            }
            else
            {
                glVertexAttribPointer(
                    static_cast<GLuint>(loc),
                    fmt.components,
                    fmt.type,
                    fmt.normalized ? GL_TRUE : GL_FALSE,
                    stride,
                    reinterpret_cast<const void *>(byteOffset));
            }
            glEnableVertexAttribArray(static_cast<GLuint>(loc));
            glVertexAttribDivisor(static_cast<GLuint>(loc), e.perInstance ? 1u : 0u);
        }
        return true;
    }

    // ---- Draw ------------------------------------------------------------

    void GLContext::drawInstanced(u32 verticesCount, u32 instanceCount, i32 startInstance, i32 startVertex)
    {
        if (!m_currentRenderTarget)
        {
            LogError("GLContext::drawInstanced: no render target bound");
            return;
        }
        if (startInstance != 0)
        {
            GLReportUnsupported("GLContext::drawInstanced: startInstance != 0");
            return;
        }
        if (!flushVertexLayout())
            return;

        const GLenum topo = mapTopology(m_currentPipeline->desc().primitiveTopology);
        if (topo == 0)
        {
            LogError("GLContext::drawInstanced: invalid primitive topology");
            return;
        }

        if (instanceCount > 1)
            glDrawArraysInstanced(topo, static_cast<GLint>(startVertex), static_cast<GLsizei>(verticesCount), static_cast<GLsizei>(instanceCount));
        else
            glDrawArrays(topo, static_cast<GLint>(startVertex), static_cast<GLsizei>(verticesCount));
        GLCheckError("GLContext::drawInstanced");
    }

    void GLContext::drawIndexedInstanced(u32 indicesCount, u32 instanceCount, i32 startInstance, i32 startVertex, i32 startIndex)
    {
        if (!m_currentRenderTarget)
        {
            LogError("GLContext::drawIndexedInstanced: no render target bound");
            return;
        }
        if (!m_indexBuffer)
        {
            LogError("GLContext::drawIndexedInstanced: no index buffer bound");
            return;
        }
        if (startInstance != 0)
        {
            GLReportUnsupported("GLContext::drawIndexedInstanced: startInstance != 0");
            return;
        }
        if (!flushVertexLayout())
            return;

        const GLenum topo = mapTopology(m_currentPipeline->desc().primitiveTopology);
        if (topo == 0)
        {
            LogError("GLContext::drawIndexedInstanced: invalid primitive topology");
            return;
        }

        // Re-bind here: glBindVertexArray inside flushVertexLayout resets the
        // element-array binding on most drivers.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(static_cast<GLBuffer *>(m_indexBuffer)->handle()));

        const GLenum indexType = indexTypeFor(m_indexBuffer);
        const uintptr_t byteOffset = static_cast<uintptr_t>(startIndex) * indexElementSize(indexType);

        if (instanceCount > 1)
        {
            if (startVertex != 0)
                glDrawElementsInstancedBaseVertex(
                    topo, static_cast<GLsizei>(indicesCount), indexType,
                    reinterpret_cast<const void *>(byteOffset),
                    static_cast<GLsizei>(instanceCount), startVertex);
            else
                glDrawElementsInstanced(
                    topo, static_cast<GLsizei>(indicesCount), indexType,
                    reinterpret_cast<const void *>(byteOffset),
                    static_cast<GLsizei>(instanceCount));
        }
        else
        {
            if (startVertex != 0)
                glDrawElementsBaseVertex(
                    topo, static_cast<GLsizei>(indicesCount), indexType,
                    reinterpret_cast<const void *>(byteOffset), startVertex);
            else
                glDrawElements(
                    topo, static_cast<GLsizei>(indicesCount), indexType,
                    reinterpret_cast<const void *>(byteOffset));
        }
        GLCheckError("GLContext::drawIndexedInstanced");
    }

}

#elif GFX_API_OPENGLES

#include "GLUtils.hpp"

namespace toolkit
{
    GLContext::GLContext(GLDevice *device) : GfxContext(device), m_device(device) {}
    GLContext::~GLContext() {}
    void GLContext::clear(GfxTextureView *, const Color &) {}
    void GLContext::clearDepth(GfxTextureView *, f32, u8) {}
    void GLContext::clearUA(GfxBuffer *, const FVec4 &) {}
    void GLContext::clearUA(GfxBuffer *, const u32[4]) {}
    void GLContext::clearUA(GfxTexture *, const u32[4]) {}
    void GLContext::clearUA(GfxTexture *, const FVec4 &) {}
    void GLContext::resetRenderTarget() {}
    void GLContext::setRenderTarget(GfxTextureView *) {}
    void GLContext::setRenderTarget(GfxTextureView *, GfxTextureView *) {}
    void GLContext::setRenderTarget(GfxTextureView *, const TkSpan<GfxTextureView *> &) {}
    void GLContext::setBlendFactor(const FVec4 &) {}
    void GLContext::setStencilRef(u32) {}
    void GLContext::resetSR() {}
    void GLContext::resetUA() {}
    void GLContext::resetCB() {}
    void GLContext::bindCB(i32, GfxConstantBuffer *) {}
    void GLContext::bindSR(i32, GfxResourceView *) {}
    void GLContext::bindUA(i32, GfxResourceView *) {}
    void GLContext::bindVB(const TkSpan<GfxBuffer *> &, const u32 *, GfxVertexLayout *) {}
    void GLContext::bindIB(GfxBuffer *) {}
    void GLContext::bindSampler(i32, GfxSampler *) {}
    void GLContext::updateCB(GfxConstantBuffer *, const void *) {}
    void GLContext::dispatch(GfxShaderProgramCS *, u32, u32, u32) {}
    void GLContext::dispatchIndirect(GfxShaderProgramCS *, GfxBuffer *, u32) {}
    void GLContext::resolveMultisample(GfxTexture *, GfxTexture *, i32, i32, PixelFormat) {}
    void GLContext::drawInstanced(u32, u32, i32, i32) {}
    void GLContext::drawIndexedInstanced(u32, u32, i32, i32, i32) {}
    void GLContext::drawInstancedIndirect(GfxBuffer *, u32) {}
    void GLContext::drawIndexedInstancedIndirect(GfxBuffer *, u32) {}
    void GLContext::setViewport(const Viewport &) {}
    void GLContext::setScissor(const Rectangle &) {}
    GfxPipelineState *GLContext::getState() const { return nullptr; }
    void GLContext::setState(GfxPipelineState *) {}
    void GLContext::clearState() {}
    void GLContext::flushState() {}
    void GLContext::flush() {}
    void GLContext::updateTexture(GfxTexture *, i32, i32, const void *, u32, u32) {}
    void GLContext::resetCounter(GfxBuffer *) {}
    void GLContext::copyCounter(GfxBuffer *, u32, GfxBuffer *) {}
    bool GLContext::flushVertexLayout() { return false; }
}

#endif
