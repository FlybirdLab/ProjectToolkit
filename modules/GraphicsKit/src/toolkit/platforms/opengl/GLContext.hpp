/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxContext.hpp>
#include <toolkit/math/Vector4.hpp>
#include <toolkit/platforms/opengl/common.hpp>
#include <toolkit/types/Span.hpp>

#if GFX_API_OPENGL || GFX_API_OPENGLES

namespace toolkit
{

    // GPU Context for the OpenGL fallback backend.
    //
    // Phase 3: full present-path + draw path for VS+PS pipelines with vertex
    // buffers, index buffer, constant buffers, single-backbuffer RT.
    //
    // Unsupported in this phase (and they call GLReportUnsupported):
    //   - SRV / sampler binding
    //   - UAV / SSBO / dispatch / indirect
    //   - texture render targets, MRT, depth/stencil
    //   - resolveMultisample / copyBackbuffer
    class GLContext : public GfxContext
    {
    public:
        GLContext(class GLDevice *device);
        ~GLContext();

    public:
#if GFX_ALLOW_PROFILE_EVENTS
        void eventBegin(const char *name) override {}
        void eventEnd() override {}
#endif
        void *getNativePtr() const override { return nullptr; }

        void clear(GfxTextureView *rt, const Color &color) override;
        void clearDepth(GfxTextureView *depthBuffer, f32 depthValue, u8 stencilValue) override;

        void clearUA(GfxBuffer *buf, const FVec4 &value) override;
        void clearUA(GfxBuffer *buf, const u32 value[4]) override;
        void clearUA(GfxTexture *texture, const u32 value[4]) override;
        void clearUA(GfxTexture *texture, const FVec4 &value) override;

        void resetRenderTarget() override;
        void setRenderTarget(GfxTextureView *rt) override;
        void setRenderTarget(GfxTextureView *depthBuffer, GfxTextureView *rt) override;
        void setRenderTarget(GfxTextureView *depthBuffer, const TkSpan<GfxTextureView *> &rts) override;

        void setBlendFactor(const FVec4 &value) override;
        void setStencilRef(u32 value) override;

        void resetSR() override;
        void resetUA() override;
        void resetCB() override;

        void bindCB(i32 slot, GfxConstantBuffer *cb) override;
        void bindSR(i32 slot, GfxResourceView *view) override;
        void bindUA(i32 slot, GfxResourceView *view) override;
        void bindVB(const TkSpan<GfxBuffer *> &vertexBuffers, const u32 *vertexBuffersOffsets = nullptr, GfxVertexLayout *vertexLayout = nullptr) override;
        void bindIB(GfxBuffer *indexBuffer) override;
        void bindSampler(i32 slot, GfxSampler *sampler) override;

        void updateCB(GfxConstantBuffer *cb, const void *data) override;

        void dispatch(GfxShaderProgramCS *shader, u32 threadGroupCountX, u32 threadGroupCountY, u32 threadGroupCountZ) override;
        void dispatchIndirect(GfxShaderProgramCS *shader, GfxBuffer *bufferForArgs, u32 offsetForArgs) override;

        void resolveMultisample(GfxTexture *sourceMultisampleTexture, GfxTexture *destTexture, i32 sourceSubResource, i32 destSubResource, PixelFormat format) override;

        void drawInstanced(u32 verticesCount, u32 instanceCount, i32 startInstance, i32 startVertex) override;
        void drawIndexedInstanced(u32 indicesCount, u32 instanceCount, i32 startInstance, i32 startVertex, i32 startIndex) override;
        void drawInstancedIndirect(GfxBuffer *bufferForArgs, u32 offsetForArgs) override;
        void drawIndexedInstancedIndirect(GfxBuffer *bufferForArgs, u32 offsetForArgs) override;

        void setViewport(const Viewport &viewport) override;
        void setScissor(const Rectangle &scissorRect) override;

        GfxPipelineState *getState() const override;
        void setState(GfxPipelineState *state) override;
        void clearState() override;
        void flushState() override;
        void flush() override;

        void updateBuffer(GfxBuffer *buffer, const void *data, u32 size, u32 offset) override {}
        void copyBuffer(GfxBuffer *dstBuffer, GfxBuffer *srcBuffer, u32 size, u32 dstOffset, u32 srcOffset) override {}

        void updateTexture(GfxTexture *texture, i32 arrayIndex, i32 mipIndex, const void *data, u32 rowPitch, u32 slicePitch) override;
        void copyTexture(GfxTexture *dstResource, u32 dstSubresource, u32 dstX, u32 dstY, u32 dstZ, GfxTexture *srcResource, u32 srcSubresource) override {}

        void resetCounter(GfxBuffer *buffer) override;
        void copyCounter(GfxBuffer *dstBuffer, u32 dstOffset, GfxBuffer *srcBuffer) override;

        void copyResource(GfxResource *dstResource, GfxResource *srcResource) override {}
        void copySubresource(GfxResource *dstResource, u32 dstSubresource, GfxResource *srcResource, u32 srcSubresource) override {}

    private:
        // Apply the currently-bound pipeline's vertex layout to m_vao using
        // the cached vertex buffers. Must be called right before any glDraw*.
        // Returns false (and logs) if state is missing or inconsistent.
        bool flushVertexLayout();

        // Pick the right VAO to bind for the current draw. Swap-chain-owned
        // VAOs take precedence because VAOs are not shared between GL
        // contexts; falls back to the device VAO for offscreen flows.
        u32 activeVAO() const;

    private:
        class GLDevice *m_device = nullptr;
        class GLTextureView *m_currentRenderTarget = nullptr;
        class GLTextureView *m_currentDepthTarget = nullptr;
        // The swap chain the context most recently bound a backbuffer view
        // from. Used to keep the right window GL context current when we
        // switch to a texture RT (FBOs are not shared across GL contexts).
        class GLSwapChain *m_lastSwapChain = nullptr;
        class GLPipelineState *m_currentPipeline = nullptr;
        GfxVertexLayout *m_currentVertexLayout = nullptr;

        GfxBuffer *m_vertexBuffers[GFX_MAX_VB_BINDED] = {};
        u32 m_vertexBufferOffsets[GFX_MAX_VB_BINDED] = {};

        GfxBuffer *m_indexBuffer = nullptr;
        GfxConstantBuffer *m_constantBuffers[GFX_MAX_CB_BINDED] = {};

        // Cached SR / sampler bindings. Phase 4 only writes/clears these on
        // the corresponding bind* calls; no lazy flush.
        GfxResourceView *m_shaderResources[GFX_MAX_SR_BINDED] = {};
        GfxSampler *m_samplers[GFX_MAX_SAMPLER_BINDED] = {};

        // VAO owned by the context; we re-program its state every draw rather
        // than caching per-layout VAOs (Phase 3 keeps lifecycle simple).
        u32 m_vao = 0;

        // GL_MAX_VERTEX_ATTRIBS for the GPU we initialised against. Cached at
        // ctor time so flushVertexLayout doesn't disable attribute slots
        // beyond the driver's limit (which would log GL_INVALID_VALUE every
        // frame on cards like NVIDIA where the limit is 16).
        i32 m_maxVertexAttribs = 16;

        FVec4 m_blendFactor = FVec4::kZero;
        u32 m_stencilRef = 0;
    };

}

#endif
