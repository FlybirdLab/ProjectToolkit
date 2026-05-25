/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX11

#include "toolkit/graphics/GfxDevice.hpp"
#include "toolkit/graphics/GfxResource.hpp"
#include "toolkit/collection/HashMap.hpp"
#include "toolkit/platforms/directx/DxDevice.hpp"

namespace toolkit
{
    enum class StencilOperation : byte;
    class DX11Context;
    class DX11SwapChain;

    // Implementation of Graphics Device for DirectX 11 backend.
    class DX11Device : public DirectXDevice
    {
        friend DX11Context;
        friend DX11SwapChain;

    private:
        struct DepthStencilMode
        {
        public:
            i8 depthEnable : 1;
            i8 depthWriteEnable : 1;
            i8 depthClipEnable : 1;
            i8 stencilEnable : 1;
            u8 stencilReadMask;
            u8 stencilWriteMask;
            GfxComparisonFunc depthFunc;
            GfxComparisonFunc stencilFunc;
            StencilOperation stencilFailOp;
            StencilOperation stencilDepthFailOp;
            StencilOperation stencilPassOp;

        public:
            bool operator==(const DepthStencilMode &other) const
            {
                return TkMemory::memoryCompare(this, &other, sizeof(DepthStencilMode)) == 0;
            }

            friend u32 getHash(const DepthStencilMode &key)
            {
                u32 hash = 0;
                for (i32 i = 0; i < sizeof(DepthStencilMode) / 4; i++)
                    combineHash(hash, ((u32 *)&key)[i]);
                return hash;
            }
        };

    private:
        // Private Stuff
        ID3D11Device *m_device = nullptr;
        ID3D11DeviceContext *m_imContext = nullptr;
        IDXGIFactory *m_factoryDXGI;

        DX11Context *m_mainContext = nullptr;
        bool m_allowTearing = false;
        GfxBuffer *m_dummyVB = nullptr;

        // Static Samplers
        ID3D11SamplerState *m_samplerLinearClamp = nullptr;
        ID3D11SamplerState *m_samplerPointClamp = nullptr;
        ID3D11SamplerState *m_samplerLinearWrap = nullptr;
        ID3D11SamplerState *m_samplerPointWrap = nullptr;
        ID3D11SamplerState *m_samplerShadow = nullptr;
        ID3D11SamplerState *m_samplerShadowLinear = nullptr;

    public:
        // Shared data for pipeline states
        TkMutex StatesWriteLocker;
        TkHashMap<GfxBlendingMode, ID3D11BlendState *> blendStates;
        TkHashMap<DepthStencilMode, ID3D11DepthStencilState *> depthStencilStates;
        ID3D11RasterizerState *rasterizerStates[3 * 2 * 2]; // Index =  CullMode[0-2] + Wireframe[0?3] + DepthClipEnable[0?6]

    public:
        static GfxDevice *create();
        DX11Device(IDXGIFactory *dxgiFactory, DirectXAdapter *adapter);
        ~DX11Device();

    public:
        // Gets DX11 device
        ID3D11Device *getDevice() const
        {
            return m_device;
        }

        // Gets DXGI factory
        IDXGIFactory *getDXGIFactory() const
        {
            return m_factoryDXGI;
        }

        // Gets immediate context
        ID3D11DeviceContext *getIM() const
        {
            return m_imContext;
        }

        DX11Context *getMainContextDX11()
        {
            return m_mainContext;
        }

        ID3D11DepthStencilState *getDepthStencilState(const void *descriptionPtr);
        ID3D11BlendState *getBlendState(const GfxBlendingMode &blending);
        GfxBuffer *getDummyVB();

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
        void dispose() override;
        void waitForGPU() override;
        void drawEnd() override;
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

    // Base interface for GPU resources on DirectX 11
    class IDX11Resource
    {
    public:
        // Gets DirectX 11 resource object handle.
        virtual ID3D11Resource *getResource() = 0;
    };

    // GPU resource implementation for DirectX 11 backend.
    template <class BaseType>
    class DX11Resource : public GfxResourceBase<DX11Device, BaseType>, public IDX11Resource
    {
    public:
        DX11Resource(DX11Device *device, const TkStringView &name) noexcept
            : GfxResourceBase<DX11Device, BaseType>(device, name)
        {
        }
    };

    extern GfxDevice *createDeviceDX11();

}

#endif
