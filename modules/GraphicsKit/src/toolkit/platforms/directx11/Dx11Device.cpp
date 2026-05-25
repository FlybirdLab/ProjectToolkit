/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx11Device.hpp"

#if GFX_API_DIRECTX11

#include "Dx11Buffer.hpp"
#include "Dx11Context.hpp"
#include "Dx11PipelineState.hpp"
#include "Dx11Sampler.hpp"
#include "Dx11Shader.hpp"
#include "Dx11SwapChain.hpp"
#include "Dx11Texture.hpp"
#include "Dx11TimerQuery.hpp"
#include "Dx11VertexLayout.hpp"

#include "toolkit/base/Math.hpp"
#include "toolkit/graphics/GfxPipelineState.hpp"
#include "toolkit/graphics/utils/PixelFormat.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/platforms/directx/DxAdapter.hpp"
#include "toolkit/platforms/directx/DxUtils.hpp"
#include "toolkit/utils/Color.hpp"

namespace toolkit
{

    static D3D11_COMPARISON_FUNC toDX11(GfxComparisonFunc value)
    {
        switch (value)
        {
        case GfxComparisonFunc::kNever:
            return D3D11_COMPARISON_NEVER;
        case GfxComparisonFunc::kLess:
            return D3D11_COMPARISON_LESS;
        case GfxComparisonFunc::kEqual:
            return D3D11_COMPARISON_EQUAL;
        case GfxComparisonFunc::kLessEqual:
            return D3D11_COMPARISON_LESS_EQUAL;
        case GfxComparisonFunc::kGreater:
            return D3D11_COMPARISON_GREATER;
        case GfxComparisonFunc::kNotEqual:
            return D3D11_COMPARISON_NOT_EQUAL;
        case GfxComparisonFunc::kGreaterEqual:
            return D3D11_COMPARISON_GREATER_EQUAL;
        case GfxComparisonFunc::kAlways:
            return D3D11_COMPARISON_ALWAYS;
        default:
            return (D3D11_COMPARISON_FUNC)-1;
        }
    }

    static D3D11_STENCIL_OP toDX11(StencilOperation value)
    {
        switch (value)
        {
        case StencilOperation::kKeep:
            return D3D11_STENCIL_OP_KEEP;
        case StencilOperation::kZero:
            return D3D11_STENCIL_OP_ZERO;
        case StencilOperation::kReplace:
            return D3D11_STENCIL_OP_REPLACE;
        case StencilOperation::kIncrementSaturated:
            return D3D11_STENCIL_OP_INCR_SAT;
        case StencilOperation::kDecrementSaturated:
            return D3D11_STENCIL_OP_DECR_SAT;
        case StencilOperation::kInvert:
            return D3D11_STENCIL_OP_INVERT;
        case StencilOperation::kIncrement:
            return D3D11_STENCIL_OP_INCR;
        case StencilOperation::kDecrement:
            return D3D11_STENCIL_OP_DECR;
        default:
            return (D3D11_STENCIL_OP)-1;
        }
    }

    static bool tryCreateDevice(IDXGIAdapter *adapter, D3D_FEATURE_LEVEL maxFeatureLevel, D3D_FEATURE_LEVEL *featureLevel)
    {
        ID3D11Device *device = nullptr;
        ID3D11DeviceContext *context = nullptr;
        u32 deviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if GFX_ENABLE_DIAGNOSTICS
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        const i32 levelsCount = sizeof(featureLevels) / sizeof(featureLevels[0]);
        i32 levelIndex = 0;
        while (levelIndex < levelsCount)
        {
            if (featureLevels[levelIndex] == maxFeatureLevel)
                break;
            levelIndex++;
        }
        if (levelIndex >= levelsCount)
            return false;

        if (SUCCEEDED(D3D11CreateDevice(
                adapter,
                D3D_DRIVER_TYPE_UNKNOWN,
                NULL,
                deviceFlags,
                &featureLevels[levelIndex],
                levelsCount - levelIndex,
                D3D11_SDK_VERSION,
                &device,
                featureLevel,
                &context)))
        {
            device->Release();
            context->Release();
            return true;
        }
#if GFX_ENABLE_DIAGNOSTICS
        deviceFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
        if (SUCCEEDED(D3D11CreateDevice(
                adapter,
                D3D_DRIVER_TYPE_UNKNOWN,
                NULL,
                deviceFlags,
                &featureLevels[levelIndex],
                levelsCount - levelIndex,
                D3D11_SDK_VERSION,
                &device,
                featureLevel,
                &context)))
        {
            LogWarn("Direct3D SDK debug layers were requested, but not available.");
            device->Release();
            context->Release();
            return true;
        }
#endif

        return false;
    }

    GfxDevice *DX11Device::create()
    {
        D3D_FEATURE_LEVEL maxAllowedFeatureLevel = D3D_FEATURE_LEVEL_11_0;

        // Create DXGI factory
        IDXGIFactory *dxgiFactory = nullptr;
        IDXGIFactory6 *dxgiFactory6 = nullptr;
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory6));
        if (hr == S_OK)
            dxgiFactory = dxgiFactory6;
        else
        {
            dxgiFactory6 = nullptr;
            IDXGIFactory1 *factory1 = nullptr;
            hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory1));
            dxgiFactory = factory1;
        }
        if (hr != S_OK || dxgiFactory == nullptr)
        {
            LogError("Cannot create DXGI factory. Error code: %x", (u32)hr);
            return nullptr;
        }

        // Enumerate the DXGIFactory's adapters
        i32 selectedAdapterIndex = -1;
        TkArray<DirectXAdapter> adapters;
        IDXGIAdapter *tempAdapter = nullptr;
        for (u32 index = 0; dxgiFactory->EnumAdapters(index, &tempAdapter) != DXGI_ERROR_NOT_FOUND; index++)
        {
            DirectXAdapter adapter;
            if (tempAdapter && tryCreateDevice(tempAdapter, maxAllowedFeatureLevel, &adapter.maxFeatureLevel))
            {
                adapter.index = static_cast<i32>(index);
                VALIDATE_DIRECTX_CALL(tempAdapter->GetDesc(&adapter.description));
                adapters.add(adapter);
            }
        }

        // Find the best performing adapter and prefer using it instead of the first device
        const auto gpuPreference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
        if (dxgiFactory6 != nullptr && selectedAdapterIndex == -1)
        {
            if (dxgiFactory6->EnumAdapterByGpuPreference(0, gpuPreference, IID_PPV_ARGS(&tempAdapter)) != DXGI_ERROR_NOT_FOUND)
            {
                DirectXAdapter adapter;
                if (tempAdapter && tryCreateDevice(tempAdapter, maxAllowedFeatureLevel, &adapter.maxFeatureLevel))
                {
                    DXGI_ADAPTER_DESC desc;
                    VALIDATE_DIRECTX_CALL(tempAdapter->GetDesc(&desc));
                    for (i32 i = 0; i < adapters.count(); i++)
                    {
                        if (adapters[i].description.AdapterLuid.LowPart == desc.AdapterLuid.LowPart &&
                            adapters[i].description.AdapterLuid.HighPart == desc.AdapterLuid.HighPart)
                        {
                            selectedAdapterIndex = i;
                            break;
                        }
                    }
                }
            }
        }

        if (selectedAdapterIndex < 0)
            selectedAdapterIndex = 0;
        if (adapters.count() == 0 || selectedAdapterIndex >= adapters.count())
        {
            LogError("Failed to find valid DirectX adapter!");
            dxgiFactory->Release();
            return nullptr;
        }

        DirectXAdapter selectedAdapter = adapters[selectedAdapterIndex];
        if (!selectedAdapter.isValid())
        {
            LogError("Failed to choose valid DirectX adapter!");
            dxgiFactory->Release();
            return nullptr;
        }

        // Create device
        auto *adapterPtr = TkMemory::newObject<DirectXAdapter>(TkMemory::getDefault(), selectedAdapter);
        auto *device = TkMemory::newObject<DX11Device>(TkMemory::getDefault(), dxgiFactory, adapterPtr);
        if (!device->init())
        {
            LogWarn("Graphics Device init failed");
            TkMemory::deletePtr(device);
            return nullptr;
        }

        return device;
    }

    DX11Device::DX11Device(IDXGIFactory *dxgiFactory, DirectXAdapter *adapter)
        : DirectXDevice(getRendererType(adapter), getGfxShaderProfile(adapter), adapter), m_factoryDXGI(dxgiFactory)
    {
        TkMemory::memoryClear(rasterizerStates, sizeof(rasterizerStates));
    }

    DX11Device::~DX11Device()
    {
        DX11Device::dispose();
    }

    ID3D11DepthStencilState *DX11Device::getDepthStencilState(const void *descriptionPtr)
    {
        const GfxPipelineState::Description &description = *(const GfxPipelineState::Description *)descriptionPtr;
        DepthStencilMode key;
        TkMemory::memoryClear(&key, sizeof(key));
        key.depthEnable = description.depthEnable ? 1 : 0;
        key.depthWriteEnable = description.depthWriteEnable ? 1 : 0;
        key.depthClipEnable = description.depthClipEnable ? 1 : 0;
        key.stencilEnable = description.stencilEnable ? 1 : 0;
        key.stencilReadMask = description.stencilReadMask;
        key.stencilWriteMask = description.stencilWriteMask;
        key.depthFunc = description.depthFunc;
        key.stencilFunc = description.stencilFunc;
        key.stencilFailOp = description.stencilFailOp;
        key.stencilDepthFailOp = description.stencilDepthFailOp;
        key.stencilPassOp = description.stencilPassOp;

        if (depthStencilStates.containsKey(key))
            return depthStencilStates[key];

        StatesWriteLocker.lock();
        if (depthStencilStates.containsKey(key))
        {
            ID3D11DepthStencilState *cached = depthStencilStates[key];
            StatesWriteLocker.unlock();
            return cached;
        }

        D3D11_DEPTH_STENCIL_DESC desc;
        desc.DepthEnable = !!description.depthEnable;
        desc.DepthWriteMask = description.depthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc = toDX11(description.depthFunc);
        desc.StencilEnable = !!description.stencilEnable;
        desc.StencilReadMask = description.stencilReadMask;
        desc.StencilWriteMask = description.stencilWriteMask;
        desc.FrontFace.StencilFailOp = toDX11(description.stencilFailOp);
        desc.FrontFace.StencilDepthFailOp = toDX11(description.stencilDepthFailOp);
        desc.FrontFace.StencilPassOp = toDX11(description.stencilPassOp);
        desc.FrontFace.StencilFunc = toDX11(description.stencilFunc);
        desc.BackFace = desc.FrontFace;

        ID3D11DepthStencilState *state = nullptr;
        VALIDATE_DIRECTX_CALL(m_device->CreateDepthStencilState(&desc, &state));
        depthStencilStates.add(key, state);
        StatesWriteLocker.unlock();
        return state;
    }

    ID3D11BlendState *DX11Device::getBlendState(const GfxBlendingMode &blending)
    {
        if (blendStates.containsKey(blending))
            return blendStates[blending];

        StatesWriteLocker.lock();
        if (blendStates.containsKey(blending))
        {
            ID3D11BlendState *cached = blendStates[blending];
            StatesWriteLocker.unlock();
            return cached;
        }

        D3D11_BLEND_DESC desc;
        desc.AlphaToCoverageEnable = blending.alphaToCoverageEnable ? TRUE : FALSE;
        desc.IndependentBlendEnable = FALSE;
        desc.RenderTarget[0].BlendEnable = blending.blendEnable ? TRUE : FALSE;
        desc.RenderTarget[0].SrcBlend = (D3D11_BLEND)blending.srcBlend;
        desc.RenderTarget[0].DestBlend = (D3D11_BLEND)blending.destBlend;
        desc.RenderTarget[0].BlendOp = (D3D11_BLEND_OP)blending.blendOp;
        desc.RenderTarget[0].SrcBlendAlpha = (D3D11_BLEND)blending.srcBlendAlpha;
        desc.RenderTarget[0].DestBlendAlpha = (D3D11_BLEND)blending.destBlendAlpha;
        desc.RenderTarget[0].BlendOpAlpha = (D3D11_BLEND_OP)blending.blendOpAlpha;
        desc.RenderTarget[0].RenderTargetWriteMask = (UINT8)blending.renderTargetWriteMask;
#if TK_DEBUG
        for (byte i = 1; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
            desc.RenderTarget[i] = desc.RenderTarget[0];
#endif

        ID3D11BlendState *state = nullptr;
        VALIDATE_DIRECTX_CALL(m_device->CreateBlendState(&desc, &state));
        blendStates.add(blending, state);
        StatesWriteLocker.unlock();
        return state;
    }

    GfxBuffer *DX11Device::getDummyVB()
    {
        if (!m_dummyVB)
        {
            m_dummyVB = createBuffer(TkStringView("DummyVertexBuffer"));
            VertexElement element;
            element.type = VertexElementTypes::kAttribute3;
            element.slot = 0;
            element.offset = 0;
            element.perInstance = 0;
            element.format = PixelFormat::kR32G32B32A32_Float;
            VertexElements elements;
            elements.add(element);
            GfxVertexLayout *layout = GfxVertexLayout::get(elements);
            m_dummyVB->init(GfxBufferDescription::vertex(layout, sizeof(Color), 1, &Color::Transparent));
        }
        return m_dummyVB;
    }

    bool DX11Device::init()
    {
        HRESULT result;

        // Get DXGI adapter
        IDXGIAdapter *adapter = nullptr;
        if (m_factoryDXGI->EnumAdapters(m_adapter->index, &adapter) == DXGI_ERROR_NOT_FOUND || adapter == nullptr)
        {
            LogWarn("Cannot get the adapter.");
            return false;
        }
        updateOutputs(adapter);

        // Get flags and device type base on current configuration
        u32 flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if GFX_ENABLE_DIAGNOSTICS
        flags |= D3D11_CREATE_DEVICE_DEBUG;
        LogInfo("DirectX debugging layer enabled");
#endif

        // Create DirectX device
        D3D_FEATURE_LEVEL createdFeatureLevel = static_cast<D3D_FEATURE_LEVEL>(0);
        D3D_FEATURE_LEVEL targetFeatureLevel = m_adapter->maxFeatureLevel;
        const HRESULT createDeviceResult = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, flags, &targetFeatureLevel, 1, D3D11_SDK_VERSION, &m_device, &createdFeatureLevel, &m_imContext);
        adapter->Release();
        if (FAILED(createDeviceResult))
        {
            LOG_DIRECTX_RESULT(createDeviceResult);
            return false;
        }
        TK_ASSERT(m_device);
        TK_ASSERT(m_imContext);
        TK_ASSERT(createdFeatureLevel == targetFeatureLevel);
        m_state = DeviceState::kCreated;

#if TK_WINDOWS
        // Detect RenderDoc usage (UUID {A7AA6116-9C8D-4BBA-9083-B4D816B71B78})
        IUnknown *unknown = nullptr;
        const GUID uuidRenderDoc = {0xa7aa6116, 0x9c8d, 0x4bba, {0x90, 0x83, 0xb4, 0xd8, 0x16, 0xb7, 0x1b, 0x78}};
        HRESULT hrRD = m_device->QueryInterface(uuidRenderDoc, (void **)&unknown);
        if (SUCCEEDED(hrRD) && unknown)
        {
            isDebuging = true;
            unknown->Release();
        }
        if (!isDebuging && GetModuleHandleA("renderdoc.dll") != nullptr)
        {
            isDebuging = true;
        }
#endif

        // Check if can use screen tearing on a swapchain
        ComPtr<IDXGIFactory5> factory5;
        m_factoryDXGI->QueryInterface(IID_PPV_ARGS(&factory5));
        if (factory5)
        {
            BOOL tearing = FALSE;
            if (SUCCEEDED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearing, sizeof(tearing))) && tearing
#if TK_WINDOWS
                && !isDebuging
#endif
            )
            {
                m_allowTearing = true;
            }
        }

        // Init device limits
        {
            if (createdFeatureLevel >= D3D_FEATURE_LEVEL_11_0)
            {
                D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS d3D10XHardwareOptions = {};
                m_device->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &d3D10XHardwareOptions, sizeof(d3D10XHardwareOptions));
                D3D11_FEATURE_DATA_D3D11_OPTIONS2 featureDataD3D11Options2 = {};
                m_device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &featureDataD3D11Options2, sizeof(featureDataD3D11Options2));
                limits.hasCompute = d3D10XHardwareOptions.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x != 0;
                limits.hasTessellation = GFX_ALLOW_TESSELLATION_SHADERS;
                limits.hasGeometryShaders = GFX_ALLOW_GEOMETRY_SHADERS;
                limits.hasInstancing = true;
                limits.hasVolumeTextureRendering = true;
                limits.hasDrawIndirect = true;
                limits.hasAppendConsumeBuffers = true;
                limits.hasSeparateRenderTargetBlendState = true;
                limits.hasDepthAsSRV = true;
                limits.hasDepthClip = true;
                limits.hasReadOnlyDepth = true;
                limits.hasMultisampleDepthAsSRV = true;
                limits.hasTypedUAVLoad = featureDataD3D11Options2.TypedUAVLoadAdditionalFormats != 0;
                limits.maxMipLevelsCount = D3D11_REQ_MIP_LEVELS;
                limits.maxTexture1DSize = D3D11_REQ_TEXTURE1D_U_DIMENSION;
                limits.maxTexture1DArraySize = D3D11_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION;
                limits.maxTexture2DSize = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
                limits.maxTexture2DArraySize = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
                limits.maxTexture3DSize = D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
                limits.maxTextureCubeSize = D3D11_REQ_TEXTURECUBE_DIMENSION;
                limits.maxSamplerAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY;
            }
            else
            {
                limits.hasCompute = false;
                limits.hasTessellation = false;
                limits.hasGeometryShaders = GFX_ALLOW_GEOMETRY_SHADERS;
                limits.hasInstancing = true;
                limits.hasVolumeTextureRendering = false;
                limits.hasDrawIndirect = false;
                limits.hasAppendConsumeBuffers = false;
                limits.hasSeparateRenderTargetBlendState = false;
                limits.hasDepthAsSRV = false;
                limits.hasDepthClip = true;
                limits.hasReadOnlyDepth = createdFeatureLevel == D3D_FEATURE_LEVEL_10_1;
                limits.hasMultisampleDepthAsSRV = false;
                limits.hasTypedUAVLoad = false;
                limits.maxMipLevelsCount = D3D10_REQ_MIP_LEVELS;
                limits.maxTexture1DSize = D3D10_REQ_TEXTURE1D_U_DIMENSION;
                limits.maxTexture1DArraySize = D3D10_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION;
                limits.maxTexture2DSize = D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;
                limits.maxTexture2DArraySize = D3D10_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
                limits.maxTexture3DSize = D3D10_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
                limits.maxTextureCubeSize = D3D10_REQ_TEXTURECUBE_DIMENSION;
                limits.maxSamplerAnisotropy = D3D10_DEFAULT_MAX_ANISOTROPY;
            }

            for (i32 i = 0; i < static_cast<i32>(PixelFormat::MAX); i++)
            {
                const PixelFormat format = static_cast<PixelFormat>(i);
                const DXGI_FORMAT dxgiFormat = DxUtils::toDxgiFormat(format);
                i32 maxCount = 1;
                UINT numQualityLevels;
                for (i32 c = 2; c <= 8; c *= 2)
                {
                    if (SUCCEEDED(m_device->CheckMultisampleQualityLevels(dxgiFormat, c, &numQualityLevels)) && numQualityLevels > 0)
                        maxCount = c;
                }
                UINT formatSupport = 0;
                m_device->CheckFormatSupport(dxgiFormat, &formatSupport);
                features[i] = FormatFeatures(format, static_cast<GfxMSAALevel>(maxCount), (FormatSupport)formatSupport);
            }
        }

        // Init debug layer
#if GFX_ENABLE_DIAGNOSTICS
        ComPtr<ID3D11InfoQueue> infoQueue;
        VALIDATE_DIRECTX_CALL(m_device->QueryInterface(IID_PPV_ARGS(&infoQueue)));
        if (infoQueue)
        {
            D3D11_INFO_QUEUE_FILTER filter;
            TkMemory::memoryClear(&filter, sizeof(filter));

            D3D11_MESSAGE_SEVERITY denySeverity = D3D11_MESSAGE_SEVERITY_INFO;
            filter.DenyList.NumSeverities = 1;
            filter.DenyList.pSeverityList = &denySeverity;

            D3D11_MESSAGE_ID disabledMessages[] = {
                D3D11_MESSAGE_ID_OMSETRENDERTARGETS_INVALIDVIEW,
                D3D11_MESSAGE_ID_QUERY_BEGIN_ABANDONING_PREVIOUS_RESULTS,
                D3D11_MESSAGE_ID_QUERY_END_ABANDONING_PREVIOUS_RESULTS,
                D3D11_MESSAGE_ID_CREATEINPUTLAYOUT_EMPTY_LAYOUT,
                D3D11_MESSAGE_ID_DEVICE_DRAW_INDEX_BUFFER_TOO_SMALL,
                D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET,
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                D3D11_MESSAGE_ID_DEVICE_DRAW_VERTEX_BUFFER_TOO_SMALL,
            };

            filter.DenyList.NumIDs = sizeof(disabledMessages) / sizeof(disabledMessages[0]);
            filter.DenyList.pIDList = disabledMessages;

            infoQueue->PushStorageFilter(&filter);

            infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
        }
#endif

        // Create main context from Immediate Context
        m_mainContext = TkMemory::newObject<DX11Context>(TkMemory::getDefault(), this, m_imContext);

        // Static Samplers
        {
            D3D11_SAMPLER_DESC samplerDesc;
            TkMemory::memoryClear(&samplerDesc, sizeof(samplerDesc));
            samplerDesc.MinLOD = 0;
            samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
            samplerDesc.MipLODBias = 0.0f;
            samplerDesc.MaxAnisotropy = 1;
            samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

            // Linear Clamp
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            result = m_device->CreateSamplerState(&samplerDesc, &m_samplerLinearClamp);
            LOG_DIRECTX_RESULT_WITH_RETURN(result, false);

            // Point Clamp
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            result = m_device->CreateSamplerState(&samplerDesc, &m_samplerPointClamp);
            LOG_DIRECTX_RESULT_WITH_RETURN(result, false);

            // Linear Wrap
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            result = m_device->CreateSamplerState(&samplerDesc, &m_samplerLinearWrap);
            LOG_DIRECTX_RESULT_WITH_RETURN(result, false);

            // Point Wrap
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            result = m_device->CreateSamplerState(&samplerDesc, &m_samplerPointWrap);
            LOG_DIRECTX_RESULT_WITH_RETURN(result, false);

            // Shadow
            samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            result = m_device->CreateSamplerState(&samplerDesc, &m_samplerShadow);
            LOG_DIRECTX_RESULT_WITH_RETURN(result, false);

            // Shadow Linear
            samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            result = m_device->CreateSamplerState(&samplerDesc, &m_samplerShadowLinear);
            LOG_DIRECTX_RESULT_WITH_RETURN(result, false);
        }

        // Rasterizer States
        {
            D3D11_RASTERIZER_DESC rDesc;
            rDesc.ScissorEnable = TRUE;
            rDesc.MultisampleEnable = TRUE;
            rDesc.FrontCounterClockwise = FALSE;
            rDesc.DepthBias = 0;
            rDesc.DepthBiasClamp = 0.0f;
            rDesc.SlopeScaledDepthBias = 0.0f;
            i32 index;
#define CREATE_RASTERIZER_STATE(cullMode, dxCullMode, wireframe, depthClip)                  \
    index = (i32)cullMode + (wireframe ? 0 : 3) + (depthClip ? 0 : 6);                       \
    rDesc.CullMode = dxCullMode;                                                             \
    rDesc.FillMode = wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;                    \
    rDesc.AntialiasedLineEnable = !!wireframe;                                               \
    rDesc.DepthClipEnable = !!depthClip;                                                     \
    result = m_device->CreateRasterizerState(&rDesc, &rasterizerStates[index]);              \
    LOG_DIRECTX_RESULT_WITH_RETURN(result, false)
            CREATE_RASTERIZER_STATE(GfxCullMode::kNormal, D3D11_CULL_BACK, false, false);
            CREATE_RASTERIZER_STATE(GfxCullMode::kInverted, D3D11_CULL_FRONT, false, false);
            CREATE_RASTERIZER_STATE(GfxCullMode::kTwoSided, D3D11_CULL_NONE, false, false);
            CREATE_RASTERIZER_STATE(GfxCullMode::kNormal, D3D11_CULL_BACK, true, false);
            CREATE_RASTERIZER_STATE(GfxCullMode::kInverted, D3D11_CULL_FRONT, true, false);
            CREATE_RASTERIZER_STATE(GfxCullMode::kTwoSided, D3D11_CULL_NONE, true, false);
            //
            CREATE_RASTERIZER_STATE(GfxCullMode::kNormal, D3D11_CULL_BACK, false, true);
            CREATE_RASTERIZER_STATE(GfxCullMode::kInverted, D3D11_CULL_FRONT, false, true);
            CREATE_RASTERIZER_STATE(GfxCullMode::kTwoSided, D3D11_CULL_NONE, false, true);
            CREATE_RASTERIZER_STATE(GfxCullMode::kNormal, D3D11_CULL_BACK, true, true);
            CREATE_RASTERIZER_STATE(GfxCullMode::kInverted, D3D11_CULL_FRONT, true, true);
            CREATE_RASTERIZER_STATE(GfxCullMode::kTwoSided, D3D11_CULL_NONE, true, true);
#undef CREATE_RASTERIZER_STATE
        }

        m_state = DeviceState::kReady;
        return DirectXDevice::init();
    }

    void DX11Device::dispose()
    {
        GfxDeviceLock lock(this);

        if (m_state == DeviceState::kDisposed)
            return;

        m_state = DeviceState::kDisposing;

        waitForGPU();
        preDispose();

        // Static samplers
        DX_DEL(m_samplerLinearClamp, 0);
        DX_DEL(m_samplerPointClamp, 0);
        DX_DEL(m_samplerLinearWrap, 0);
        DX_DEL(m_samplerPointWrap, 0);
        DX_DEL(m_samplerShadow, 0);
        DX_DEL(m_samplerShadowLinear, 0);

        // Pipeline state caches
        for (auto it = blendStates.begin(); it.isNotEnd(); ++it)
        {
            if (it->value)
                it->value->Release();
        }
        blendStates.clear();
        for (auto it = depthStencilStates.begin(); it.isNotEnd(); ++it)
        {
            if (it->value)
                it->value->Release();
        }
        depthStencilStates.clear();
        for (u32 i = 0; i < sizeof(rasterizerStates) / sizeof(rasterizerStates[0]); i++)
        {
            DX_DEL(rasterizerStates[i], 0);
        }

        // Clear DirectX objects
        if (m_mainContext)
        {
            TkMemory::deletePtr(m_mainContext);
            m_mainContext = nullptr;
        }
        if (m_adapter)
        {
            TkMemory::deletePtr(m_adapter);
            m_adapter = nullptr;
        }
        DX_DEL(m_imContext, 0);
        DX_DEL(m_device, 0);
        DX_DEL(m_factoryDXGI, 0);

        DirectXDevice::dispose();
        m_state = DeviceState::kDisposed;
    }

    void DX11Device::waitForGPU()
    {
        // In DirectX 11 driver manages CPU/GPU work synchronization and work submission
    }

    void DX11Device::drawEnd()
    {
        DirectXDevice::drawEnd();

#if GFX_ENABLE_DIAGNOSTICS
        ComPtr<ID3D11InfoQueue> infoQueue;
        VALIDATE_DIRECTX_CALL(m_device->QueryInterface(IID_PPV_ARGS(&infoQueue)));
        if (infoQueue)
        {
            TkArray<u8> data;
            const u64 messagesCount = infoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();
            for (u64 i = 0; i < messagesCount; i++)
            {
                SIZE_T length = 0;
                if (SUCCEEDED(infoQueue->GetMessage(i, nullptr, &length)))
                {
                    data.resize((i32)length);
                    auto messageData = (D3D11_MESSAGE *)data.get();
                    if (SUCCEEDED(infoQueue->GetMessage(i, messageData, &length)))
                    {
                        switch (messageData->Severity)
                        {
                        case D3D11_MESSAGE_SEVERITY_CORRUPTION:
                        case D3D11_MESSAGE_SEVERITY_ERROR:
                            LogError("%s", messageData->pDescription);
                            break;
                        case D3D11_MESSAGE_SEVERITY_WARNING:
                            LogWarn("%s", messageData->pDescription);
                            break;
                        default:
                            LogInfo("%s", messageData->pDescription);
                            break;
                        }
                    }
                }
            }
            infoQueue->ClearStoredMessages();
        }
#endif
    }

    GfxTexture *DX11Device::createTexture(const TkStringView &name)
    {
        return TkMemory::newObject<DX11Texture>(TkMemory::getDefault(), this, name);
    }

    GfxShader *DX11Device::createShader(const TkStringView &name)
    {
        return TkMemory::newObject<DX11Shader>(TkMemory::getDefault(), this, name);
    }

    GfxPipelineState *DX11Device::createPipelineState()
    {
        return TkMemory::newObject<DX11PipelineState>(TkMemory::getDefault(), this);
    }

    GfxTimerQuery *DX11Device::createTimerQuery()
    {
        return TkMemory::newObject<DX11TimerQuery>(TkMemory::getDefault(), this);
    }

    GfxBuffer *DX11Device::createBuffer(const TkStringView &name)
    {
        return TkMemory::newObject<DX11Buffer>(TkMemory::getDefault(), this, name);
    }

    GfxSampler *DX11Device::createSampler()
    {
        return TkMemory::newObject<DX11Sampler>(TkMemory::getDefault(), this);
    }

    GfxVertexLayout *DX11Device::createVertexLayout(const VertexElements &elements, bool explicitOffsets)
    {
        return TkMemory::newObject<DX11VertexLayout>(TkMemory::getDefault(), this, elements, explicitOffsets);
    }

    GfxSwapChain *DX11Device::createSwapChain(GfxWindow *window)
    {
        return TkMemory::newObject<DX11SwapChain>(TkMemory::getDefault(), this, window);
    }

    GfxConstantBuffer *DX11Device::createConstantBuffer(u32 size, const TkStringView &name)
    {
        ID3D11Buffer *buffer = nullptr;
        u32 memorySize = 0;
        if (size)
        {
            D3D11_BUFFER_DESC cbDesc;
            cbDesc.ByteWidth = TkMath::alignUp<u32>(size, 16);
            cbDesc.Usage = D3D11_USAGE_DEFAULT;
            cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbDesc.CPUAccessFlags = 0;
            cbDesc.MiscFlags = 0;
            cbDesc.StructureByteStride = 0;
            const HRESULT result = m_device->CreateBuffer(&cbDesc, nullptr, &buffer);
            if (FAILED(result))
            {
                LOG_DIRECTX_RESULT(result);
                return nullptr;
            }
            memorySize = cbDesc.ByteWidth;
        }
        return TkMemory::newObject<DX11ConstantBuffer>(TkMemory::getDefault(), this, size, memorySize, buffer, name);
    }

    GfxDevice *createDeviceDX11()
    {
        return DX11Device::create();
    }

}

#endif
