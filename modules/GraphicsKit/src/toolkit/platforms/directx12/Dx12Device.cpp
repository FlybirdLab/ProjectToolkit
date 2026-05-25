/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "DX12Device.hpp"

#if GFX_API_DIRECTX12

#include "Dx12Buffer.hpp"
#include "DX12CmdSignature.hpp"
#include "Dx12CmdQueue.hpp"
#include "Dx12Context.hpp"
#include "Dx12PipelineState.hpp"
#include "Dx12Sampler.hpp"
#include "Dx12Shader.hpp"
#include "Dx12SwapChain.hpp"
#include "Dx12Texture.hpp"
#include "Dx12TimerQuery.hpp"
#include "Dx12UploadBuffer.hpp"
#include "Dx12VertexLayout.hpp"

#include "toolkit/graphics/utils/PixelFormat.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/platforms/directx/DxAdapter.hpp"
#include "toolkit/platforms/directx/DxUtils.hpp"
#include "toolkit/threading/Sync.hpp"

namespace toolkit
{

    static bool CheckDX12Support(IDXGIAdapter *adapter)
    {
        if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
        {
            return true;
        }
        return false;
    }

    static GfxMSAALevel GetMaximumMultisampleCount(ID3D12Device *device, DXGI_FORMAT dxgiFormat)
    {
        i32 maxCount = 1;
        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevels = {dxgiFormat};
        for (i32 i = 2; i <= 8; i *= 2)
        {
            qualityLevels.SampleCount = i;
            if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &qualityLevels, sizeof(qualityLevels))) && qualityLevels.NumQualityLevels > 0)
                maxCount = i;
        }
        return static_cast<GfxMSAALevel>(maxCount);
    }

    GfxDevice *DX12Device::create()
    {
        // Debug Layer
#if GFX_ENABLE_DIAGNOSTICS
        ComPtr<ID3D12Debug> debugLayer;
        D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
        if (debugLayer)
        {
            debugLayer->EnableDebugLayer();
        }
#ifdef __ID3D12DeviceRemovedExtendedDataSettings_FWD_DEFINED__
        ComPtr<ID3D12DeviceRemovedExtendedDataSettings> dredSettings;
        D3D12GetDebugInterface(IID_PPV_ARGS(&dredSettings));
        if (dredSettings)
        {
            dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
        }
#endif
#endif

        // Create DXGI factory
        IDXGIFactory4 *dxgiFactory = nullptr;
        IDXGIFactory6 *dxgiFactory6 = nullptr;
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory6));
        if (hr == S_OK)
            dxgiFactory = dxgiFactory6;
        else
        {
            dxgiFactory6 = nullptr;
            hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
        }
        if (hr != S_OK)
        {
            // TODO: log
            return nullptr;
        }

        // Enumerate the DXGIFactory's adapters
        i32 selectedAdapterIndex = -1;
        TkArray<DirectXAdapter> adapters;
        IDXGIAdapter *tempAdapter = nullptr;
        for (u32 index = 0; dxgiFactory->EnumAdapters(index, &tempAdapter) != DXGI_ERROR_NOT_FOUND; index++)
        {
            DirectXAdapter adapter;
            if (tempAdapter && CheckDX12Support(tempAdapter))
            {
                adapter.index = static_cast<i32>(index);
                adapter.maxFeatureLevel = D3D_FEATURE_LEVEL_12_0;
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
                if (tempAdapter && CheckDX12Support(tempAdapter))
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
            // TODO: log error - no valid DirectX adapter
            return nullptr;
        }
        DirectXAdapter selectedAdapter = adapters[selectedAdapterIndex];
        if (!selectedAdapter.isValid())
        {
            // TODO: log error - invalid adapter
            return nullptr;
        }
        if (selectedAdapter.maxFeatureLevel < D3D_FEATURE_LEVEL_12_0)
        {
            // TODO: log error - feature level too low
            return nullptr;
        }

        // Create device
        auto *adapterPtr = TkMemory::newObject<DirectXAdapter>(TkMemory::getDefault(), selectedAdapter);
        auto *device = TkMemory::newObject<DX12Device>(TkMemory::getDefault(), dxgiFactory, adapterPtr);
        if (!device->init())
        {
            LogError("DX12Device::init failed");
            TkMemory::deletePtr(device);
            return nullptr;
        }

        return device;
    }

    DX12Device::DX12Device(IDXGIFactory4 *dxgiFactory, DirectXAdapter *adapter)
        : DirectXDevice(GfxBackendType::kDirectX12, GfxShaderProfile::kDirectX_SM6, adapter)
        , m_device(nullptr)
        , m_factoryDXGI(dxgiFactory)
        , m_res2Dispose(256)
        , m_rootSignature(nullptr)
        , m_commandQueue(nullptr)
        , m_mainContext(nullptr)
        , uploadBuffer(nullptr)
        , timestampQueryHeap(this, D3D12_QUERY_HEAP_TYPE_TIMESTAMP, DX12_BACK_BUFFER_COUNT * 1024)
        , heapCvbSrvUav(this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4 * 1024, false)
        , heapRtv(this, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1 * 1024, false)
        , heapDsv(this, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 64, false)
        , heapSampler(this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 128, false)
        , ringHeapCvbSrvUav(this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 512 * 1024, true)
        , ringHeapSampler(this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1 * 1024, true)
    {
    }

    bool DX12Device::init()
    {
        // Get DXGI adapter
        IDXGIAdapter1 *adapter = nullptr;
        TK_ASSERT(m_factoryDXGI);
        if (m_factoryDXGI->EnumAdapters1(m_adapter->index, &adapter) == DXGI_ERROR_NOT_FOUND || adapter == nullptr)
        {
            LogError("DX12Device::init: cannot get DXGI adapter (index=%d)", m_adapter ? m_adapter->index : -1);
            return false;
        }
        updateOutputs(adapter);

        // Create DirectX device
        VALIDATE_DIRECTX_CALL(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));

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
                && !isDebuging // Disable tearing with RenderDoc (prevents crashing)
#endif
            )
                allowTearing = true;
        }

        // Debug Layer
#if GFX_ENABLE_DIAGNOSTICS
        ComPtr<ID3D12InfoQueue> infoQueue;
        VALIDATE_DIRECTX_CALL(m_device->QueryInterface(IID_PPV_ARGS(&infoQueue)));
        if (infoQueue)
        {
            D3D12_INFO_QUEUE_FILTER filter;
            TkMemory::memoryClear(&filter, sizeof(filter));

            D3D12_MESSAGE_SEVERITY denySeverity = D3D12_MESSAGE_SEVERITY_INFO;
            filter.DenyList.NumSeverities = 1;
            filter.DenyList.pSeverityList = &denySeverity;

            D3D12_MESSAGE_ID disabledMessages[] =
                {
                    D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                    D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,
                    D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_PS_OUTPUT_RT_OUTPUT_MISMATCH,
                    D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                    D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
                    D3D12_MESSAGE_ID_CREATEINPUTLAYOUT_EMPTY_LAYOUT,
                    D3D12_MESSAGE_ID_RESOURCE_BARRIER_DUPLICATE_SUBRESOURCE_TRANSITIONS,
                    D3D12_MESSAGE_ID_DRAW_EMPTY_SCISSOR_RECTANGLE,
                };

            filter.DenyList.NumIDs = sizeof(disabledMessages) / sizeof(disabledMessages[0]);
            filter.DenyList.pIDList = disabledMessages;

            infoQueue->AddStorageFilterEntries(&filter);

            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        }
#endif

        // Change state
        m_state = DeviceState::kCreated;

        // Query device features
        D3D12_FEATURE_DATA_D3D12_OPTIONS options;
        VALIDATE_DIRECTX_CALL(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)));

        // Init device limits
        {
            limits.hasCompute = true;
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
            limits.hasTypedUAVLoad = options.TypedUAVLoadAdditionalFormats != 0;
            limits.maxMipLevelsCount = D3D12_REQ_MIP_LEVELS;
            limits.maxTexture1DSize = D3D12_REQ_TEXTURE1D_U_DIMENSION;
            limits.maxTexture1DArraySize = D3D12_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION;
            limits.maxTexture2DSize = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            limits.maxTexture2DArraySize = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
            limits.maxTexture3DSize = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
            limits.maxTextureCubeSize = D3D12_REQ_TEXTURECUBE_DIMENSION;
            limits.maxSamplerAnisotropy = D3D12_DEFAULT_MAX_ANISOTROPY;

            for (i32 i = 0; i < static_cast<i32>(PixelFormat::MAX); i++)
            {
                const PixelFormat format = static_cast<PixelFormat>(i);
                const DXGI_FORMAT dxgiFormat = DxUtils::toDxgiFormat(format);
                D3D12_FEATURE_DATA_FORMAT_SUPPORT formatInfo = {dxgiFormat};
                if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatInfo, sizeof(formatInfo))))
                    formatInfo.Support1 = D3D12_FORMAT_SUPPORT1_NONE;
                const GfxMSAALevel maximumMultisampleCount = GetMaximumMultisampleCount(m_device, dxgiFormat);
                features[i] = FormatFeatures(format, maximumMultisampleCount, (FormatSupport)formatInfo.Support1);
            }
        }

        // Setup resources
        m_commandQueue = TkMemory::newObject<DX12CmdQueue>(TkMemory::getDefault(), this, D3D12_COMMAND_LIST_TYPE_DIRECT);
        if (!m_commandQueue->init())
            return false;
        m_mainContext = TkMemory::newObject<DX12Context>(TkMemory::getDefault(), this, D3D12_COMMAND_LIST_TYPE_DIRECT);
        if (!ringHeapCvbSrvUav.init())
            return false;
        if (!ringHeapSampler.init())
            return false;

        // Create empty views
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        const i32 nullSrvCount = (i32)(sizeof(m_nullSrv) / sizeof(m_nullSrv[0]));
        for (i32 i = 0; i < nullSrvCount; i++)
        {
            srvDesc.ViewDimension = (D3D12_SRV_DIMENSION)i;
            switch (srvDesc.ViewDimension)
            {
            case D3D12_SRV_DIMENSION_BUFFER:
                srvDesc.Buffer.FirstElement = 0;
                srvDesc.Buffer.NumElements = 0;
                srvDesc.Buffer.StructureByteStride = 0;
                srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
                break;
            case D3D12_SRV_DIMENSION_TEXTURE1D:
                srvDesc.Texture1D.MostDetailedMip = 0;
                srvDesc.Texture1D.MipLevels = 1;
                srvDesc.Texture1D.ResourceMinLODClamp = 0.0f;
                break;
            case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
                srvDesc.Texture1DArray.MostDetailedMip = 0;
                srvDesc.Texture1DArray.MipLevels = 1;
                srvDesc.Texture1DArray.FirstArraySlice = 0;
                srvDesc.Texture1DArray.ArraySize = 1;
                srvDesc.Texture1DArray.ResourceMinLODClamp = 0.0f;
                break;
            case D3D12_SRV_DIMENSION_UNKNOWN: // Map Unknown into Texture2D
            case D3D12_SRV_DIMENSION_TEXTURE2D:
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MostDetailedMip = 0;
                srvDesc.Texture2D.MipLevels = 1;
                srvDesc.Texture2D.PlaneSlice = 0;
                srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
                break;
            case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
                srvDesc.Texture2DArray.MostDetailedMip = 0;
                srvDesc.Texture2DArray.MipLevels = 1;
                srvDesc.Texture2DArray.FirstArraySlice = 0;
                srvDesc.Texture2DArray.ArraySize = 0;
                srvDesc.Texture2DArray.PlaneSlice = 0;
                srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
                break;
            case D3D12_SRV_DIMENSION_TEXTURE3D:
                srvDesc.Texture3D.MostDetailedMip = 0;
                srvDesc.Texture3D.MipLevels = 1;
                srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
                break;
            case D3D12_SRV_DIMENSION_TEXTURECUBE:
                srvDesc.TextureCube.MostDetailedMip = 0;
                srvDesc.TextureCube.MipLevels = 1;
                srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
                break;
            case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
                srvDesc.TextureCubeArray.MostDetailedMip = 0;
                srvDesc.TextureCubeArray.MipLevels = 1;
                srvDesc.TextureCubeArray.First2DArrayFace = 0;
                srvDesc.TextureCubeArray.NumCubes = 0;
                srvDesc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
                break;
            default:
                continue;
            }
            m_nullSrv[i].createSRV(this, nullptr, &srvDesc);
        }
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
            uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Texture2D.MipSlice = 0;
            uavDesc.Texture2D.PlaneSlice = 0;
            m_nullUav.createUAV(this, nullptr, &uavDesc);
        }

        // Create root signature
        {
            // Descriptor tables
            D3D12_DESCRIPTOR_RANGE r[3]; // SRV+UAV+Sampler
            {
                D3D12_DESCRIPTOR_RANGE &range = r[0];
                range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                range.NumDescriptors = GFX_MAX_SR_BINDED;
                range.BaseShaderRegister = 0;
                range.RegisterSpace = 0;
                range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            }
            {
                D3D12_DESCRIPTOR_RANGE &range = r[1];
                range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                range.NumDescriptors = GFX_MAX_UA_BINDED;
                range.BaseShaderRegister = 0;
                range.RegisterSpace = 0;
                range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            }
            {
                D3D12_DESCRIPTOR_RANGE &range = r[2];
                range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                range.NumDescriptors = GFX_MAX_SAMPLER_BINDED - GFX_STATIC_SAMPLERS_COUNT;
                range.BaseShaderRegister = GFX_STATIC_SAMPLERS_COUNT;
                range.RegisterSpace = 0;
                range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            }

            // Root parameters
            D3D12_ROOT_PARAMETER rootParameters[GFX_MAX_CB_BINDED + 3];
            for (i32 i = 0; i < GFX_MAX_CB_BINDED; i++)
            {
                // CB
                D3D12_ROOT_PARAMETER &rootParam = rootParameters[DX12_ROOT_SIGNATURE_CB + i];
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                rootParam.Descriptor.ShaderRegister = i;
                rootParam.Descriptor.RegisterSpace = 0;
            }
            {
                // SRVs
                D3D12_ROOT_PARAMETER &rootParam = rootParameters[DX12_ROOT_SIGNATURE_SR];
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                rootParam.DescriptorTable.NumDescriptorRanges = 1;
                rootParam.DescriptorTable.pDescriptorRanges = &r[0];
            }
            {
                // UAVs
                D3D12_ROOT_PARAMETER &rootParam = rootParameters[DX12_ROOT_SIGNATURE_UA];
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                rootParam.DescriptorTable.NumDescriptorRanges = 1;
                rootParam.DescriptorTable.pDescriptorRanges = &r[1];
            }
            {
                // Samplers
                D3D12_ROOT_PARAMETER &rootParam = rootParameters[DX12_ROOT_SIGNATURE_SAMPLER];
                rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                rootParam.DescriptorTable.NumDescriptorRanges = 1;
                rootParam.DescriptorTable.pDescriptorRanges = &r[2];
            }

            // Static samplers
            D3D12_STATIC_SAMPLER_DESC staticSamplers[6];
            static_assert(GFX_STATIC_SAMPLERS_COUNT == sizeof(staticSamplers) / sizeof(staticSamplers[0]), "Update static samplers setup.");
            // Linear Clamp
            staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            staticSamplers[0].MipLODBias = 0.0f;
            staticSamplers[0].MaxAnisotropy = 1;
            staticSamplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
            staticSamplers[0].MinLOD = 0;
            staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
            staticSamplers[0].ShaderRegister = 0;
            staticSamplers[0].RegisterSpace = 0;
            staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            // Point Clamp
            staticSamplers[1].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            staticSamplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            staticSamplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            staticSamplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            staticSamplers[1].MipLODBias = 0.0f;
            staticSamplers[1].MaxAnisotropy = 1;
            staticSamplers[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
            staticSamplers[1].MinLOD = 0;
            staticSamplers[1].MaxLOD = D3D12_FLOAT32_MAX;
            staticSamplers[1].ShaderRegister = 1;
            staticSamplers[1].RegisterSpace = 0;
            staticSamplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            // Linear Wrap
            staticSamplers[2].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            staticSamplers[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            staticSamplers[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            staticSamplers[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            staticSamplers[2].MipLODBias = 0.0f;
            staticSamplers[2].MaxAnisotropy = 1;
            staticSamplers[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
            staticSamplers[2].MinLOD = 0;
            staticSamplers[2].MaxLOD = D3D12_FLOAT32_MAX;
            staticSamplers[2].ShaderRegister = 2;
            staticSamplers[2].RegisterSpace = 0;
            staticSamplers[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            // Point Wrap
            staticSamplers[3].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            staticSamplers[3].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            staticSamplers[3].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            staticSamplers[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            staticSamplers[3].MipLODBias = 0.0f;
            staticSamplers[3].MaxAnisotropy = 1;
            staticSamplers[3].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
            staticSamplers[3].MinLOD = 0;
            staticSamplers[3].MaxLOD = D3D12_FLOAT32_MAX;
            staticSamplers[3].ShaderRegister = 3;
            staticSamplers[3].RegisterSpace = 0;
            staticSamplers[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            // Shadow
            staticSamplers[4].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
            staticSamplers[4].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            staticSamplers[4].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            staticSamplers[4].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            staticSamplers[4].MipLODBias = 0.0f;
            staticSamplers[4].MaxAnisotropy = 1;
            staticSamplers[4].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
            staticSamplers[4].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
            staticSamplers[4].MinLOD = 0;
            staticSamplers[4].MaxLOD = D3D12_FLOAT32_MAX;
            staticSamplers[4].ShaderRegister = 4;
            staticSamplers[4].RegisterSpace = 0;
            staticSamplers[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            // Shadow PCF
            staticSamplers[5].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
            staticSamplers[5].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            staticSamplers[5].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            staticSamplers[5].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            staticSamplers[5].MipLODBias = 0.0f;
            staticSamplers[5].MaxAnisotropy = 1;
            staticSamplers[5].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
            staticSamplers[5].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
            staticSamplers[5].MinLOD = 0;
            staticSamplers[5].MaxLOD = D3D12_FLOAT32_MAX;
            staticSamplers[5].ShaderRegister = 5;
            staticSamplers[5].RegisterSpace = 0;
            staticSamplers[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

            // Init
            D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
            rootSignatureDesc.NumParameters = sizeof(rootParameters) / sizeof(rootParameters[0]);
            rootSignatureDesc.pParameters = rootParameters;
            rootSignatureDesc.NumStaticSamplers = sizeof(staticSamplers) / sizeof(staticSamplers[0]);
            rootSignatureDesc.pStaticSamplers = staticSamplers;
            rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            // Serialize
            ComPtr<ID3DBlob> signature;
            ComPtr<ID3DBlob> error;
            VALIDATE_DIRECTX_CALL(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));

            // Create
            VALIDATE_DIRECTX_CALL(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
        }

        // Upload buffer
        uploadBuffer = TkMemory::newObject<DX12UploadBuffer>(TkMemory::getDefault(), this);

        if (!timestampQueryHeap.init())
            return false;

        // Cached command signatures
        {
            drawIndirectCommandSignature = TkMemory::newObject<DX12CmdSignature>(TkMemory::getDefault(), this, 1);
            drawIndirectCommandSignature->at(0).draw();
            drawIndirectCommandSignature->finalize();
        }
        {
            drawIndexedIndirectCommandSignature = TkMemory::newObject<DX12CmdSignature>(TkMemory::getDefault(), this, 1);
            drawIndexedIndirectCommandSignature->at(0).drawIndexed();
            drawIndexedIndirectCommandSignature->finalize();
        }
        {
            dispatchIndirectCommandSignature = TkMemory::newObject<DX12CmdSignature>(TkMemory::getDefault(), this, 1);
            dispatchIndirectCommandSignature->at(0).dispatch();
            dispatchIndirectCommandSignature->finalize();
        }

        m_state = DeviceState::kReady;
        return DirectXDevice::init();
    }

    void DX12Device::drawBegin()
    {
        // PROFILE_CPU_NAMED("Wait For GPU");
        m_commandQueue->waitForFence(m_mainContext->frameFenceValues[1]);

        // Base
        DirectXDevice::drawBegin();

        updateRes2Dispose();
        uploadBuffer->beginGeneration(m_frameIndex);
    }

    void DX12Device::renderEnd()
    {
        // Base
        DirectXDevice::renderEnd();

        // Resolve the timestamp queries
        timestampQueryHeap.endQueryBatchAndResolveQueryData(m_mainContext);
    }

    DX12Device::~DX12Device()
    {
        // Ensure to be disposed
        dispose();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DX12Device::nullSRV(D3D12_SRV_DIMENSION dimension) const
    {
        return m_nullSrv[dimension].cpu();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DX12Device::nullUAV() const
    {
        return m_nullUav.cpu();
    }

    ID3D12GraphicsCommandList *DX12Device::getCommandList() const
    {
        return m_mainContext->getCommandList();
    }

    ID3D12CommandQueue *DX12Device::getDX12CmdQueue() const
    {
        return m_commandQueue->getCommandQueue();
    }

    void DX12Device::dispose()
    {
        GfxDeviceLock lock(this);

        // Check if has been disposed already
        if (m_state == DeviceState::kDisposed)
            return;

        // Wait for rendering end
        waitForGPU();

        // Pre dispose
        preDispose();

        // Backend-owned releases below must also drain frame-delayed objects.
        m_state = DeviceState::kDisposing;

        // Release command-list references before destroying objects it can bind.
        if (m_mainContext)
        {
            TkMemory::deletePtr(m_mainContext);
            m_mainContext = nullptr;
        }

        // Clear pipeline objects
        GFX_DEL_RESOURCE(dummyVB);
        for (auto &srv : m_nullSrv)
            srv.release();
        m_nullUav.release();
        timestampQueryHeap.destroy();
        if (uploadBuffer)
        {
            TkMemory::deletePtr(uploadBuffer);
            uploadBuffer = nullptr;
        }
        if (drawIndirectCommandSignature)
        {
            TkMemory::deletePtr(drawIndirectCommandSignature);
            drawIndirectCommandSignature = nullptr;
        }
        if (drawIndexedIndirectCommandSignature)
        {
            TkMemory::deletePtr(drawIndexedIndirectCommandSignature);
            drawIndexedIndirectCommandSignature = nullptr;
        }
        if (dispatchIndirectCommandSignature)
        {
            TkMemory::deletePtr(dispatchIndirectCommandSignature);
            dispatchIndirectCommandSignature = nullptr;
        }

        // Release all GPU objects queued by resource disposal before core D3D12 state.
        updateRes2Dispose();
        if (m_rootSignature)
        {
            m_rootSignature->Release();
            m_rootSignature = nullptr;
        }
        heapCvbSrvUav.releaseGPU();
        heapRtv.releaseGPU();
        heapDsv.releaseGPU();
        heapSampler.releaseGPU();
        ringHeapCvbSrvUav.releaseData();
        ringHeapSampler.releaseData();
        if (m_commandQueue)
        {
            TkMemory::deletePtr(m_commandQueue);
            m_commandQueue = nullptr;
        }

        // Clear DirectX stuff
        if (m_adapter)
        {
            TkMemory::deletePtr(m_adapter);
            m_adapter = nullptr;
        }
        if (m_device)
        {
            m_device->Release();
            m_device = nullptr;
        }
        if (m_factoryDXGI)
        {
            m_factoryDXGI->Release();
            m_factoryDXGI = nullptr;
        }

        // Base
        DirectXDevice::dispose();

        // Set current state
        m_state = DeviceState::kDisposed;
    }

    void DX12Device::waitForGPU()
    {
        m_commandQueue->waitForGPU();
    }

    GfxTexture *DX12Device::createTexture(const TkStringView &name)
    {
        return TkMemory::newObject<DX12Texture>(TkMemory::getDefault(), this, name);
    }

    GfxShader *DX12Device::createShader(const TkStringView &name)
    {
        return TkMemory::newObject<DX12Shader>(TkMemory::getDefault(), this, name);
    }

    GfxPipelineState *DX12Device::createPipelineState()
    {
        return TkMemory::newObject<DX12PipelineState>(TkMemory::getDefault(), this);
    }

    GfxTimerQuery *DX12Device::createTimerQuery()
    {
        return TkMemory::newObject<DX12TimerQuery>(TkMemory::getDefault(), this);
    }

    GfxBuffer *DX12Device::createBuffer(const TkStringView &name)
    {
        return TkMemory::newObject<DX12Buffer>(TkMemory::getDefault(), this, name);
    }

    GfxSampler *DX12Device::createSampler()
    {
        return TkMemory::newObject<Dx12Sampler>(TkMemory::getDefault(), this);
    }

    GfxVertexLayout *DX12Device::createVertexLayout(const VertexElements &elements, bool explicitOffsets)
    {
        return TkMemory::newObject<GPUVertexLayoutDX12>(TkMemory::getDefault(), this, elements, explicitOffsets);
    }

    GfxSwapChain *DX12Device::createSwapChain(GfxWindow *window)
    {
        return TkMemory::newObject<DX12SwapChain>(TkMemory::getDefault(), this, window);
    }

    GfxConstantBuffer *DX12Device::createConstantBuffer(u32 size, const TkStringView &name)
    {
        return TkMemory::newObject<DX12ConstantBuffer>(TkMemory::getDefault(), this, size, name);
    }

    void DX12Device::addResourceToLateRelease(IGraphicsUnknown *resource, u32 safeFrameCount)
    {
        if (resource == nullptr)
            return;

        TkMutexGuard lock(m_res2DisposeLock);

        // Add to the list
        DisposeResourceEntry entry;
        entry.resource = resource;
        entry.targetFrame = m_frameIndex + safeFrameCount;
        m_res2Dispose.add(entry);
    }

    void DX12Device::updateRes2Dispose()
    {
        u64 currentFrame = m_frameIndex;
        if (m_state == DeviceState::kDisposing)
        {
            // During device disposing we want to remove all resources
            currentFrame = kMaxU32;
        }

        m_res2DisposeLock.lock();
        for (i32 i = m_res2Dispose.count() - 1; i >= 0 && i < m_res2Dispose.count(); i--)
        {
            const DisposeResourceEntry &entry = m_res2Dispose[i];
            if (entry.targetFrame <= currentFrame)
            {
                auto refs = entry.resource->Release();
                (void)refs;
                // TODO: log error if refs != 0 (late release resource has not been fully released)
                m_res2Dispose.removeAt(i);
            }
        }
        m_res2DisposeLock.unlock();
    }

    GPUVertexLayoutDX12::GPUVertexLayoutDX12(DX12Device *device, const Elements &elements, bool explicitOffsets)
        : DX12Resource<GfxVertexLayout>(device, TkStringView::Empty), inputElementsCount(elements.count())
    {
        setElements(elements, explicitOffsets);
        const auto &els = getElements();
        for (i32 i = 0; i < elements.count(); i++)
        {
            const VertexElement &src = els.get()[i];
            D3D12_INPUT_ELEMENT_DESC &dst = inputElements[i];
            UINT semanticIndex = 0;
            dst.SemanticName = DxUtils::getVertexInputSemantic(src.type, semanticIndex);
            dst.SemanticIndex = semanticIndex;
            dst.Format = DxUtils::toDxgiFormat(src.format);
            dst.InputSlot = src.slot;
            dst.AlignedByteOffset = src.offset;
            dst.InputSlotClass = src.perInstance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            dst.InstanceDataStepRate = src.perInstance ? 1 : 0;
        }
    }

    GfxDevice *createDeviceDX12()
    {
        return DX12Device::create();
    }

}

#endif
