/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/directx12/common.hpp>

#if GFX_API_DIRECTX12

#include "toolkit/platforms/directx/DxDevice.hpp"
#include "toolkit/graphics/GfxResource.hpp"
#include "DX12ResOwner.hpp"
#include "DX12QueryHeap.hpp"
#include "DX12DescriptorHeap.hpp"

#if TK_WINDOWS
#define DX12_BACK_BUFFER_COUNT 3
#else
#define DX12_BACK_BUFFER_COUNT 2
#endif

#define DX12_ROOT_SIGNATURE_CB 0
#define DX12_ROOT_SIGNATURE_SR (GFX_MAX_CB_BINDED + 0)
#define DX12_ROOT_SIGNATURE_UA (GFX_MAX_CB_BINDED + 1)
#define DX12_ROOT_SIGNATURE_SAMPLER (GFX_MAX_CB_BINDED + 2)

namespace toolkit
{
    class DX12Context;
    class DX12SwapChain;
    class DX12UploadBuffer;
    class DX12CmdQueue;
    class DX12CmdSignature;

    // Implementation of Graphics Device for DirectX 12 RHI
    class DX12Device : public DirectXDevice
    {
        friend DX12Context;
        friend DX12SwapChain;

    private:
        struct DisposeResourceEntry
        {
            IGraphicsUnknown *resource;
            u64 targetFrame;
        };

    private:
        // Private Stuff
        ID3D12Device *m_device;
        IDXGIFactory4 *m_factoryDXGI;
        TkMutex m_res2DisposeLock;
        TkArray<DisposeResourceEntry> m_res2Dispose;

        // Pipeline
        ID3D12RootSignature *m_rootSignature;
        DX12CmdQueue *m_commandQueue;
        DX12Context *m_mainContext;

        // Heaps
        DX12DescriptorHeapWithSlots::Slot m_nullSrv[D3D12_SRV_DIMENSION_TEXTURECUBEARRAY + 1];
        DX12DescriptorHeapWithSlots::Slot m_nullUav;

    public:
        static GfxDevice *create();
        DX12Device(IDXGIFactory4 *dxgiFactory, DirectXAdapter *adapter);
        ~DX12Device();

    public:
        // Upload buffer for general purpose
        DX12UploadBuffer *uploadBuffer;
        // The timestamp queries heap.
        DX12QueryHeap timestampQueryHeap;

        bool allowTearing = false;
        DX12CmdSignature *dispatchIndirectCommandSignature = nullptr;
        DX12CmdSignature *drawIndexedIndirectCommandSignature = nullptr;
        DX12CmdSignature *drawIndirectCommandSignature = nullptr;
        GfxBuffer *dummyVB = nullptr;

        D3D12_CPU_DESCRIPTOR_HANDLE nullSRV(D3D12_SRV_DIMENSION dimension) const;
        D3D12_CPU_DESCRIPTOR_HANDLE nullUAV() const;

    public:
        // Get DX12 device.
        TK_FORCE_INLINE ID3D12Device *getDevice() const
        {
            return m_device;
        }
        // Get DXGI factory.
        TK_FORCE_INLINE IDXGIFactory4 *getDXGIFactory() const
        {
            return m_factoryDXGI;
        }

        // Get DirectX 12 command list object
        ID3D12GraphicsCommandList *getCommandList() const;

        // Get command queue.
        TK_FORCE_INLINE DX12CmdQueue *getCommandQueue() const
        {
            return m_commandQueue;
        }

        // Get DirectX 12 command queue object.
        ID3D12CommandQueue *getDX12CmdQueue() const;

        // Get root signature of the graphics pipeline.
        TK_FORCE_INLINE ID3D12RootSignature *getRootSignature() const
        {
            return m_rootSignature;
        }

        // Get main commands context (for DirectX 12)
        TK_FORCE_INLINE DX12Context *getMainContextDX12() const
        {
            return m_mainContext;
        }

    public:
        DX12DescriptorHeapPool heapCvbSrvUav;
        DX12DescriptorHeapPool heapRtv;
        DX12DescriptorHeapPool heapDsv;
        DX12DescriptorHeapPool heapSampler;
        DX12DescriptorHeapRingBuffer ringHeapCvbSrvUav;
        DX12DescriptorHeapRingBuffer ringHeapSampler;

    public:
        // Add resource to late release service (will be released after 'safeFrameCount' frames)
        void addResourceToLateRelease(IGraphicsUnknown *resource, u32 safeFrameCount = DX12_RESOURCE_DELETE_SAFE_FRAMES_COUNT);

        static TK_FORCE_INLINE u32 getMaxMSAAQuality(u32 sampleCount)
        {
            if (sampleCount <= 8)
                return 0;
            return 0xffffffff;
        }

    private:
        void updateRes2Dispose();

    public:
        GfxContext *getMainContext() override
        {
            return reinterpret_cast<GfxContext *>(m_mainContext);
        }
        void *getNativePtr() const override
        {
            return m_device;
        }
        bool init() override;
        void beginFrame() override;
        void renderEnd() override;
        void dispose() final override;
        void waitForGPU() override;
        GfxTexture *createTexture(const TkStringView &name) override;
        GfxShader *createShader(const TkStringView &name) override;
        GfxPipelineState *createPipelineState() override;
        GfxTimerQuery *createTimerQuery() override;
        GfxBuffer *createBuffer(const TkStringView &name) override;
        GfxSampler *createSampler() override;
        GfxVertexLayout *createVertexLayout(const VertexElements &elements, bool explicitOffsets) override;
        GfxSwapChain *createSwapChain(GfxWindow *window) override;
        GfxConstantBuffer *createConstantBuffer(u32 size, const TkStringView &name) override;
    };

    // GPU resource implementation for DirectX 12 backend
    template <class BaseType>
    class DX12Resource : public GfxResourceBase<DX12Device, BaseType>
    {
    public:
        DX12Resource(DX12Device *device, const TkStringView &name)
            : GfxResourceBase<DX12Device, BaseType>(device, name)
        {
        }
    };

    extern GfxDevice *createDeviceDX12();
}

#endif
