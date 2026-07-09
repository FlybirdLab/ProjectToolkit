/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/directx12/common.hpp>

#if GFX_API_DIRECTX12

#include "toolkit/graphics/GfxContext.hpp"
#include "Dx12IShaderRes.hpp"
#include "Dx12DescriptorHeap.hpp"

namespace toolkit
{
    class DX12Device;
    class DX12PipelineState;
    class DX12Buffer;
    class Dx12Sampler;
    class DX12ConstantBuffer;
    class DX12TextureView;
    class DX12VertexLayout;

// Size of the resource barriers buffer size (will be flushed on overflow)
#define DX12_RB_BUFFER_SIZE 16

    // GPU Commands Context implementation for DirectX 12
    class DX12Context : public GfxContext
    {
        friend DX12Device;

    public:
        typedef DX12DescriptorHeapRingBuffer::Allocation Descriptor;

    private:
        DX12Device *m_device;
        ID3D12GraphicsCommandList *m_commandList;
        ID3D12CommandAllocator *m_currentAllocator;
        DX12PipelineState *m_currentState;
        GfxShaderProgramCS *m_currentCompute;

        i32 m_swapChainsUsed;
        i32 m_vbCount;
        i32 m_rtCount;
        i32 m_rbBufferSize;

        u32 m_srMaskDirtyGraphics;
        u32 m_srMaskDirtyCompute;
        u32 m_stencilRef;
        D3D_PRIMITIVE_TOPOLOGY m_primitiveTopology;

        u32 m_isCompute : 1;
        u32 m_rtDirtyFlag : 1;
        u32 m_psDirtyFlag : 1;
        u32 m_cbGraphicsDirtyFlag : 1;
        u32 m_cbComputeDirtyFlag : 1;
        u32 m_samplersDirtyFlag : 1;

        DX12TextureView *m_rtDepth;
        DX12TextureView *m_rtHandles[GFX_MAX_RT_BINDED];
        DX12IShaderResource *m_srHandles[GFX_MAX_SR_BINDED];
        DX12IShaderResource *m_uaHandles[GFX_MAX_UA_BINDED];
        DX12VertexLayout *m_vertexLayout;
        DX12Buffer *m_ibHandle;
        DX12Buffer *m_vbHandles[GFX_MAX_VB_BINDED];
        D3D12_INDEX_BUFFER_VIEW m_ibView;
        D3D12_VERTEX_BUFFER_VIEW m_vbViews[GFX_MAX_VB_BINDED];
        D3D12_RESOURCE_BARRIER m_rbBuffer[DX12_RB_BUFFER_SIZE];
        DX12ConstantBuffer *m_cbHandles[GFX_MAX_CB_BINDED];
        Dx12Sampler *m_samplers[GFX_MAX_SAMPLER_BINDED - GFX_STATIC_SAMPLERS_COUNT];

    public:
        DX12Context(DX12Device *device, D3D12_COMMAND_LIST_TYPE type);
        ~DX12Context();

    public:
        TK_FORCE_INLINE ID3D12GraphicsCommandList *getCommandList() const
        {
            return m_commandList;
        }

        u64 frameFenceValues[2];

    public:
        // Add the transition barrier for the given resource (or subresource). Supports batching barriers.
        void addTransitionBarrier(DX12ResourceOwner *resource, const D3D12_RESOURCE_STATES before, const D3D12_RESOURCE_STATES after, const i32 subresourceIndex);
        // Add the UAV barrier. Supports batching barriers.
        void addUAVBarrier();

        // Set DirectX 12 resource state using resource barrier
        void setResourceState(DX12ResourceOwner *resource, D3D12_RESOURCE_STATES after, i32 subresourceIndex = -1);
        // Reset commands list and request new allocator for commands
        void reset();

        // Flush existing commands to the GPU
        u64 execute(bool waitForCompletion = false);

        // External event called by swap chain after using context end
        void onSwapChainFlush();
        // Flush pending resource barriers
        TK_FORCE_INLINE void flushResourceBarriers()
        {
            flushRBs();
        }

    protected:
        void getActiveHeapDescriptor(const D3D12_CPU_DESCRIPTOR_HANDLE &cpuHandle, Descriptor &descriptor);

    private:
        void flushSRVs();
        void flushRTVs();
        void flushUAVs();
        void flushCBs();
        void flushSamplers();
        void flushRBs();
        void flushPS();
        void onDrawCall();

    public:
        void frameBegin() override;
        void frameEnd() override;
#if GFX_ALLOW_PROFILE_EVENTS
        void eventBegin(const char *name) override;
        void eventEnd() override;
#endif
        void *getNativePtr() const override;
        // TODO bool isDepthBufferBinded() override;
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
        void setResourceState(GfxResource *resource, u64 state, i32 subresource) override;
        void forceRebindDescriptors() override;
    };

}

#endif
