/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/GfxContext.hpp>

#if GFX_API_NULL

namespace toolkit
{

    // GPU Context for Null backend.
    class NullContext : public GfxContext
    {
    public:
        NullContext(GfxDevice *device)
            : GfxContext(device)
        {
        }

    public:
#if GFX_ALLOW_PROFILE_EVENTS
        void eventBegin(const char *name) override
        {
        }

        void eventEnd() override
        {
        }
#endif
        void *getNativePtr() const override
        {
            return nullptr;
        }

        // bool isDepthBufferBinded() override
        // {
        //     return false;
        // }

        void clear(GfxTextureView *rt, const Color &color) override
        {
        }

        void clearDepth(GfxTextureView *depthBuffer, f32 depthValue, u8 stencilValue) override
        {
        }

        void clearUA(GfxBuffer *buf, const FVec4 &value) override
        {
        }

        void clearUA(GfxBuffer *buf, const u32 value[4]) override
        {
        }

        void clearUA(GfxTexture *texture, const u32 value[4]) override
        {
        }

        void clearUA(GfxTexture *texture, const FVec4 &value) override
        {
        }

        void resetRenderTarget() override
        {
        }

        void setRenderTarget(GfxTextureView *rt) override
        {
        }

        void setRenderTarget(GfxTextureView *depthBuffer, GfxTextureView *rt) override
        {
        }

        void setRenderTarget(GfxTextureView *depthBuffer, const TkSpan<GfxTextureView *> &rts) override
        {
        }

        void setBlendFactor(const FVec4 &value) override
        {
        }

        void setStencilRef(u32 value) override
        {
        }

        void resetSR() override
        {
        }

        void resetUA() override
        {
        }

        void resetCB() override
        {
        }

        void bindCB(i32 slot, GfxConstantBuffer *cb) override
        {
        }

        void bindSR(i32 slot, GfxResourceView *view) override
        {
        }

        void bindUA(i32 slot, GfxResourceView *view) override
        {
        }

        void bindVB(const TkSpan<GfxBuffer *> &vertexBuffers, const u32 *vertexBuffersOffsets = nullptr, GfxVertexLayout *vertexLayout = nullptr) override
        {
        }

        void bindIB(GfxBuffer *indexBuffer) override
        {
        }

        void bindSampler(i32 slot, GfxSampler *sampler) override
        {
        }

        void updateCB(GfxConstantBuffer *cb, const void *data) override
        {
        }

        void dispatch(GfxShaderProgramCS *shader, u32 threadGroupCountX, u32 threadGroupCountY, u32 threadGroupCountZ) override
        {
        }

        void dispatchIndirect(GfxShaderProgramCS *shader, GfxBuffer *bufferForArgs, u32 offsetForArgs) override
        {
        }

        void resolveMultisample(GfxTexture *sourceMultisampleTexture, GfxTexture *destTexture, i32 sourceSubResource, i32 destSubResource, PixelFormat format) override
        {
        }

        void drawInstanced(u32 verticesCount, u32 instanceCount, i32 startInstance, i32 startVertex) override
        {
        }

        void drawIndexedInstanced(u32 indicesCount, u32 instanceCount, i32 startInstance, i32 startVertex, i32 startIndex) override
        {
        }

        void drawInstancedIndirect(GfxBuffer *bufferForArgs, u32 offsetForArgs) override
        {
        }

        void drawIndexedInstancedIndirect(GfxBuffer *bufferForArgs, u32 offsetForArgs) override
        {
        }

        void setViewport(const Viewport &viewport) override
        {
        }

        void setScissor(const Rectangle &scissorRect) override
        {
        }

        GfxPipelineState *getState() const override
        {
            return nullptr;
        }

        void setState(GfxPipelineState *state) override
        {
        }

        void clearState() override
        {
        }

        void flushState() override
        {
        }

        void flush() override
        {
        }

        void updateBuffer(GfxBuffer *buffer, const void *data, u32 size, u32 offset) override
        {
        }

        void copyBuffer(GfxBuffer *dstBuffer, GfxBuffer *srcBuffer, u32 size, u32 dstOffset, u32 srcOffset) override
        {
        }

        void updateTexture(GfxTexture *texture, i32 arrayIndex, i32 mipIndex, const void *data, u32 rowPitch, u32 slicePitch) override
        {
        }

        void copyTexture(GfxTexture *dstResource, u32 dstSubresource, u32 dstX, u32 dstY, u32 dstZ, GfxTexture *srcResource, u32 srcSubresource) override
        {
        }

        void resetCounter(GfxBuffer *buffer) override
        {
        }

        void copyCounter(GfxBuffer *dstBuffer, u32 dstOffset, GfxBuffer *srcBuffer) override
        {
        }

        void copyResource(GfxResource *dstResource, GfxResource *srcResource) override
        {
        }

        void copySubresource(GfxResource *dstResource, u32 dstSubresource, GfxResource *srcResource, u32 srcSubresource) override
        {
        }
    };

}

#endif