/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX11

#include "toolkit/graphics/GfxContext.hpp"
#include "Dx11Device.hpp"
#include "Dx11PipelineState.hpp"
#include "toolkit/math/Vector4.hpp"

namespace toolkit
{
    class DX11Buffer;
    class DX11VertexLayout;

    // GPU Context for DirectX 11 backend.
    class DX11Context : public GfxContext
    {
    public:
        DX11Context(DX11Device *device, ID3D11DeviceContext *context);
        ~DX11Context();

    public:
        // Gets DirectX 11 device context used by this context
        TK_FORCE_INLINE ID3D11DeviceContext *GetContext() const
        {
            return m_context;
        }

    private:
        void flushSRVs();
        void flushUAVs();
        void flushCBs();
        void flushOM();
        void flushIA();
        void onDrawCall();

    public:
        void frameBegin() override;
#if GFX_ALLOW_PROFILE_EVENTS
        void eventBegin(const char *name) override;
        void eventEnd() override;
#endif
        void *getNativePtr() const override;
        // bool isDepthBufferBinded() override;
        void clear(GfxTextureView *rt, const Color &color) override;
        void clearDepth(GfxTextureView *depthBuffer, float depthValue, u8 stencilValue) override;
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
        void updateBuffer(GfxBuffer *buffer, const void *data, u32 size, u32 offset) override;
        void copyBuffer(GfxBuffer *dstBuffer, GfxBuffer *srcBuffer, u32 size, u32 dstOffset, u32 srcOffset) override;
        void updateTexture(GfxTexture *texture, i32 arrayIndex, i32 mipIndex, const void *data, u32 rowPitch, u32 slicePitch) override;
        void copyTexture(GfxTexture *dstResource, u32 dstSubresource, u32 dstX, u32 dstY, u32 dstZ, GfxTexture *srcResource, u32 srcSubresource) override;
        void resetCounter(GfxBuffer *buffer) override;
        void copyCounter(GfxBuffer *dstBuffer, u32 dstOffset, GfxBuffer *srcBuffer) override;
        void copyResource(GfxResource *dstResource, GfxResource *srcResource) override;
        void copySubresource(GfxResource *dstResource, u32 dstSubresource, GfxResource *srcResource, u32 srcSubresource) override;

    private:
        DX11Device *m_device;
        ID3D11DeviceContext *m_context;
#if GFX_ALLOW_PROFILE_EVENTS
        ID3DUserDefinedAnnotation *m_userDefinedAnnotations;
#endif
        i32 m_maxUASlots;

        // Output Merger
        bool m_omDirtyFlag;
        i32 m_rtCount;
        ID3D11DepthStencilView *m_rtDepth;
        ID3D11RenderTargetView *m_rtHandles[GFX_MAX_RT_BINDED];

        // Shader Resources
        u32 m_srMaskDirtyGraphics;
        u32 m_srMaskDirtyCompute;
        ID3D11ShaderResourceView *m_srHandles[GFX_MAX_SR_BINDED];

        // Unordered Access
        bool m_uaDirtyFlag;
        ID3D11UnorderedAccessView *m_uaHandles[GFX_MAX_UA_BINDED];

        // Constant Buffers
        bool m_cbDirtyFlag;
        ID3D11Buffer *m_cbHandles[GFX_MAX_CB_BINDED];

        // Vertex Buffers
        DX11Buffer *m_ibHandle;
        ID3D11Buffer *m_vbHandles[GFX_MAX_VB_BINDED];
        UINT m_vbStrides[GFX_MAX_VB_BINDED];
        UINT m_vbOffsets[GFX_MAX_VB_BINDED];
        DX11VertexLayout *m_vertexLayout;
        bool m_iaInputLayoutDirtyFlag;

        // Pipeline State
        DX11PipelineState *m_currentState;
        ID3D11BlendState *currentBlendState;
        ID3D11RasterizerState *currentRasterizerState;
        ID3D11DepthStencilState *currentDepthStencilState;
        DX11ShaderProgramVS *currentVS;
#if GFX_ALLOW_TESSELLATION_SHADERS
        DX11ShaderProgramHS *currentHS;
        DX11ShaderProgramDS *currentDS;
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
        DX11ShaderProgramGS *currentGS;
#endif
        DX11ShaderProgramPS *currentPS;
        DX11ShaderProgramCS *currentCS;
        D3D11_PRIMITIVE_TOPOLOGY currentPrimitiveTopology;
        u32 currentStencilRef;
        FVec4 currentBlendFactor;
    };

}

#endif
