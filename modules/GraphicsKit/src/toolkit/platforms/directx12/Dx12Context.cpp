/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx12Context.hpp"

#if GFX_API_DIRECTX12

#include "Dx12Device.hpp"
#include "Dx12Buffer.hpp"
#include "Dx12Texture.hpp"
#include "Dx12Sampler.hpp"
#include "Dx12PipelineState.hpp"
#include "Dx12Shader.hpp"
#include "Dx12ShaderProgram.hpp"
#include "Dx12CmdQueue.hpp"
#include "Dx12UploadBuffer.hpp"
#include "Dx12CmdSignature.hpp"
#include "Dx12DescriptorHeap.hpp"
#include "DX12ResOwner.hpp"

#include "toolkit/math/Vector4.hpp"
#include "toolkit/base/Math.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/graphics/GfxVertexLayout.hpp"
#include "toolkit/graphics/GfxShaderProgram.hpp"
#include "toolkit/platforms/directx/DxUtils.hpp"

#define DX12_ENABLE_RESOURCE_BARRIERS_BATCHING 1

namespace toolkit
{

    inline bool operator!=(const D3D12_VERTEX_BUFFER_VIEW &l, const D3D12_VERTEX_BUFFER_VIEW &r)
    {
        return l.SizeInBytes != r.SizeInBytes || l.StrideInBytes != r.StrideInBytes || l.BufferLocation != r.BufferLocation;
    }

    inline bool operator!=(const D3D12_INDEX_BUFFER_VIEW &l, const D3D12_INDEX_BUFFER_VIEW &r)
    {
        return l.SizeInBytes != r.SizeInBytes || l.Format != r.Format || l.BufferLocation != r.BufferLocation;
    }

    DX12Context::DX12Context(DX12Device *device, D3D12_COMMAND_LIST_TYPE type)
        : GfxContext(device), m_device(device), m_commandList(nullptr), m_currentAllocator(nullptr), m_currentState(nullptr), m_currentCompute(nullptr), m_swapChainsUsed(0), m_vbCount(0), m_rtCount(0), m_rbBufferSize(0), m_srMaskDirtyGraphics(0), m_srMaskDirtyCompute(0), m_stencilRef(0), m_primitiveTopology(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED), m_isCompute(0), m_rtDirtyFlag(0), m_psDirtyFlag(0), m_cbGraphicsDirtyFlag(0), m_cbComputeDirtyFlag(0), m_samplersDirtyFlag(0), m_rtDepth(nullptr), m_vertexLayout(nullptr), m_ibHandle(nullptr)
    {
        frameFenceValues[0] = 0;
        frameFenceValues[1] = 0;
        m_currentAllocator = m_device->getCommandQueue()->requestAllocator();
        VALIDATE_DIRECTX_CALL(device->getDevice()->CreateCommandList(0, type, m_currentAllocator, nullptr, IID_PPV_ARGS(&m_commandList)));
#if GFX_ENABLE_RESOURCE_NAMING
        m_commandList->SetName(L"DX12Context::CommandList");
#endif
    }

    DX12Context::~DX12Context()
    {
        DX_DEL(m_commandList, 0);
    }

    void DX12Context::addTransitionBarrier(DX12ResourceOwner *resource, const D3D12_RESOURCE_STATES before, const D3D12_RESOURCE_STATES after, const i32 subresourceIndex)
    {
        if (m_rbBufferSize == DX12_RB_BUFFER_SIZE)
            flushRBs();

        D3D12_RESOURCE_BARRIER barrier;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = resource->getResource();
        barrier.Transition.StateBefore = before;
        barrier.Transition.StateAfter = after;
        barrier.Transition.Subresource = subresourceIndex;
#if DX12_ENABLE_RESOURCE_BARRIERS_BATCHING
        m_rbBuffer[m_rbBufferSize++] = barrier;
#else
        m_commandList->ResourceBarrier(1, &barrier);
#endif
    }

    void DX12Context::addUAVBarrier()
    {
        if (m_rbBufferSize == DX12_RB_BUFFER_SIZE)
            flushRBs();

        D3D12_RESOURCE_BARRIER barrier;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.UAV.pResource = nullptr;
#if DX12_ENABLE_RESOURCE_BARRIERS_BATCHING
        m_rbBuffer[m_rbBufferSize++] = barrier;
#else
        m_commandList->ResourceBarrier(1, &barrier);
#endif
    }

    void DX12Context::setResourceState(DX12ResourceOwner *resource, D3D12_RESOURCE_STATES after, i32 subresourceIndex)
    {
        auto nativeResource = resource->getResource();
        if (nativeResource == nullptr)
            return;
        auto &st = resource->state;
        if (subresourceIndex == -1)
        {
            if (st.areAllSubresourcesSame())
            {
                // Transition entire resource at once
                const D3D12_RESOURCE_STATES before = st.getSubresourceState(-1);
                if (DX12ResourceState::isTransitionNeeded(before, after))
                {
                    addTransitionBarrier(resource, before, after, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
                    st.setResourceState(after);
                }
            }
            else
            {
                // Slow path to transition each subresource
                bool hadAnyTransition = false;
                for (i32 i = 0; i < (i32)st.getSubresourcesCount(); i++)
                {
                    const D3D12_RESOURCE_STATES before = st.getSubresourceState(i);
                    if (DX12ResourceState::isTransitionNeeded(before, after))
                    {
                        addTransitionBarrier(resource, before, after, i);
                        st.setSubresourceState(i, after);
                        hadAnyTransition = true;
                    }
                }
                if (hadAnyTransition)
                {
                    // Try merge all subresources states into a single state
                    after = st.getSubresourceState(0);
                    for (i32 i = 1; i < (i32)st.getSubresourcesCount(); i++)
                    {
                        if (st.getSubresourceState(i) != after)
                            return;
                    }
                    st.setResourceState(after);
                }
            }
        }
        else
        {
            const D3D12_RESOURCE_STATES before = st.getSubresourceState(subresourceIndex);
            if (DX12ResourceState::isTransitionNeeded(before, after))
            {
                // Transition a single subresource
                addTransitionBarrier(resource, before, after, subresourceIndex);
                st.setSubresourceState(subresourceIndex, after);
            }
        }
    }

    void DX12Context::reset()
    {
        // The command list persists, but we must request a new allocator
        TK_ASSERT(m_commandList != nullptr);
        if (m_currentAllocator == nullptr)
        {
            m_currentAllocator = m_device->getCommandQueue()->requestAllocator();
            m_commandList->Reset(m_currentAllocator, nullptr);
        }

        // Setup initial state
        m_currentState = nullptr;
        m_rtDirtyFlag = false;
        m_cbGraphicsDirtyFlag = false;
        m_cbComputeDirtyFlag = false;
        m_samplersDirtyFlag = false;
        m_rtCount = 0;
        m_rtDepth = nullptr;
        m_srMaskDirtyGraphics = 0;
        m_srMaskDirtyCompute = 0;
        m_stencilRef = 0;
        m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        m_psDirtyFlag = false;
        m_isCompute = false;
        m_currentCompute = nullptr;
        m_rbBufferSize = 0;
        m_vbCount = 0;
        TkMemory::memoryClear(m_rtHandles, sizeof(m_rtHandles));
        TkMemory::memoryClear(m_srHandles, sizeof(m_srHandles));
        TkMemory::memoryClear(m_uaHandles, sizeof(m_uaHandles));
        TkMemory::memoryClear(m_vbHandles, sizeof(m_vbHandles));
        m_vertexLayout = nullptr;
        m_ibHandle = nullptr;
        TkMemory::memoryClear(&m_cbHandles, sizeof(m_cbHandles));
        TkMemory::memoryClear(&m_samplers, sizeof(m_samplers));
        m_swapChainsUsed = 0;

        // Bind dummy vertex buffer (used by missing bindings)
        D3D12_VERTEX_BUFFER_VIEW dummyVBView;
        if (!m_device->dummyVB)
        {
            m_device->dummyVB = m_device->createBuffer(TkStringView("DummyVertexBuffer"));
            GfxVertexLayout::Elements elements;
            VertexElement ve;
            ve.type = VertexElementTypes::kAttribute3;
            ve.slot = 0;
            ve.offset = 0;
            ve.perInstance = 0;
            ve.format = PixelFormat::kR32G32B32A32_Float;
            elements.add(ve);
            auto *layout = GfxVertexLayout::get(elements);
            m_device->dummyVB->init(GfxBufferDescription::vertex(layout, sizeof(Color), 1, &Color::Transparent));
        }
        ((DX12Buffer *)m_device->dummyVB)->getVBView(dummyVBView);
        m_commandList->IASetVertexBuffers(GFX_MAX_VB_BINDED, 1, &dummyVBView);

        forceRebindDescriptors();
    }

    u64 DX12Context::execute(bool waitForCompletion)
    {
        TK_ASSERT(m_currentAllocator != nullptr);
        auto queue = m_device->getCommandQueue();

        // Flush remaining and buffered commands
        flushState();
        m_currentState = nullptr;

        // Execute commands
        const u64 fenceValue = queue->executeCommandList(m_commandList);

        // Cleanup used allocator
        queue->discardAllocator(fenceValue, m_currentAllocator);
        m_currentAllocator = nullptr;

        // Wait for GPU if need to
        if (waitForCompletion)
            queue->waitForFence(fenceValue);

        return fenceValue;
    }

    void DX12Context::onSwapChainFlush()
    {
        m_swapChainsUsed++;

        // Flush per-window (excluding the main window)
        if (m_swapChainsUsed > 1)
        {
            // Flush GPU commands
            flush();
        }
    }

    void DX12Context::getActiveHeapDescriptor(const D3D12_CPU_DESCRIPTOR_HANDLE &cpuHandle, Descriptor &descriptor)
    {
        // Allocate data for the table
        descriptor = m_device->ringHeapCvbSrvUav.allocateTable(1);

        // Copy descriptor
        m_device->getDevice()->CopyDescriptorsSimple(1, descriptor.cpu, cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    void DX12Context::flushSRVs()
    {
        u32 srMask;
        if (m_isCompute)
        {
            // Skip if no compute shader binded or it doesn't use shader resources
            if (m_srMaskDirtyCompute == 0 || m_currentCompute == nullptr || (srMask = m_currentCompute->getBindings().usedSRsMask) == 0)
                return;
        }
        else
        {
            // Skip if no state binded or it doesn't use shader resources
            if (m_srMaskDirtyGraphics == 0 || m_currentState == nullptr || (srMask = m_currentState->getUsedSRsMask()) == 0)
                return;
        }

        // Count SRVs required to be bind to the pipeline (the index of the most significant bit that's set)
        const u32 srCount = TkMath::floorLog2(srMask) + 1;
        TK_ASSERT(srCount <= GFX_MAX_SR_BINDED);

        // Fill table with source descriptors
        DxShaderHeader &header = m_currentCompute ? ((DX12ShaderProgramCS *)m_currentCompute)->header : m_currentState->header;
        D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptorRangeStarts[GFX_MAX_SR_BINDED];
        for (u32 i = 0; i < srCount; i++)
        {
            const auto handle = m_srHandles[i];
            const auto dimensions = (D3D12_SRV_DIMENSION)header.srDimensions[i];
            if ((srMask & (1u << i)) && handle != nullptr)
            {
                TK_ASSERT(handle->srvDimension == dimensions);
                srcDescriptorRangeStarts[i] = handle->srv();
                // TODO: for setup states based on binding mode
                D3D12_RESOURCE_STATES states = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
                if (handle->isDepthStencilResource())
                    states |= D3D12_RESOURCE_STATE_DEPTH_READ;
                setResourceState(handle->getResourceOwner(), states, handle->subResIdx);
            }
            else
            {
                srcDescriptorRangeStarts[i] = m_device->nullSRV(dimensions);
            }
        }

        // Allocate data for the table
        auto allocation = m_device->ringHeapCvbSrvUav.allocateTable(srCount);

        // Copy descriptors
        m_device->getDevice()->CopyDescriptors(1, &allocation.cpu, &srCount, srCount, srcDescriptorRangeStarts, nullptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        // Flush SRV descriptors table
        if (m_isCompute)
            m_commandList->SetComputeRootDescriptorTable(DX12_ROOT_SIGNATURE_SR, allocation.gpu);
        else
            m_commandList->SetGraphicsRootDescriptorTable(DX12_ROOT_SIGNATURE_SR, allocation.gpu);
    }

    void DX12Context::flushRTVs()
    {
        // Check if update render target bindings
        if (m_rtDirtyFlag)
        {
            // Clear flag
            m_rtDirtyFlag = false;

            // Get render targets CPU descriptors
            D3D12_CPU_DESCRIPTOR_HANDLE rtCPU[GFX_MAX_RT_BINDED];
            for (i32 i = 0; i < m_rtCount; i++)
            {
                const auto handle = m_rtHandles[i];
                setResourceState(handle->getResourceOwner(), D3D12_RESOURCE_STATE_RENDER_TARGET, handle->subResIdx);
                rtCPU[i] = handle->rtv();
            }

            // Get depth buffer handle
            D3D12_CPU_DESCRIPTOR_HANDLE depthBuffer;
            if (m_rtDepth)
            {
                depthBuffer = m_rtDepth->dsv();
                auto states = m_rtDepth->readOnlyDepthView ? D3D12_RESOURCE_STATE_DEPTH_READ : D3D12_RESOURCE_STATE_DEPTH_WRITE;
                if (m_currentState && m_rtDepth->readOnlyDepthView)
                {
                    // If read-only depth buffer is also binded as shader input then ensure it has proper state bit
                    const u32 srMask = m_currentState->getUsedSRsMask();
                    const u32 srCount = TkMath::floorLog2(srMask) + 1;
                    for (u32 i = 0; i < srCount; i++)
                    {
                        const auto handle = m_srHandles[i];
                        if ((srMask & (1u << i)) && handle == m_rtDepth)
                        {
                            states |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
                            break;
                        }
                    }
                }
                setResourceState(m_rtDepth->getResourceOwner(), states, m_rtDepth->subResIdx);
            }
            else
            {
                depthBuffer.ptr = 0;
            }

            // Flush output merger render targets
            m_commandList->OMSetRenderTargets(m_rtCount, rtCPU, false, depthBuffer.ptr != 0 ? &depthBuffer : nullptr);
        }
    }

    void DX12Context::flushUAVs()
    {
        u32 uaMask;
        if (m_isCompute)
        {
            // Skip if no compute shader binded or it doesn't use shader resources
            if ((uaMask = m_currentCompute->getBindings().usedUAsMask) == 0)
                return;
        }
        else
        {
            // Skip if no state binded or it doesn't use shader resources
            if (m_currentState == nullptr || (uaMask = m_currentState->getUsedUAsMask()) == 0)
                return;
        }

        // Count UAVs required to be bind to the pipeline (the index of the most significant bit that's set)
        const u32 uaCount = TkMath::floorLog2(uaMask) + 1;
        TK_ASSERT(uaCount <= GFX_MAX_UA_BINDED);

        // Fill table with source descriptors
        DxShaderHeader &header = m_currentCompute ? ((DX12ShaderProgramCS *)m_currentCompute)->header : m_currentState->header;
        D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptorRangeStarts[GFX_MAX_UA_BINDED];
        for (u32 i = 0; i < uaCount; i++)
        {
            const auto handle = m_uaHandles[i];
            const auto dimensions = (D3D12_UAV_DIMENSION)header.uaDimensions[i];
            if ((uaMask & (1u << i)) && handle != nullptr)
            {
                TK_ASSERT(handle->uavDimension == dimensions);
                srcDescriptorRangeStarts[i] = handle->uav();
                setResourceState(handle->getResourceOwner(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            }
            else
            {
                srcDescriptorRangeStarts[i] = m_device->nullUAV();
            }
        }

        // Allocate data for the table
        auto allocation = m_device->ringHeapCvbSrvUav.allocateTable(uaCount);

        // Copy descriptors
        m_device->getDevice()->CopyDescriptors(1, &allocation.cpu, &uaCount, uaCount, srcDescriptorRangeStarts, nullptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        // Flush UAV descriptors table
        if (m_isCompute)
            m_commandList->SetComputeRootDescriptorTable(DX12_ROOT_SIGNATURE_UA, allocation.gpu);
        else
            m_commandList->SetGraphicsRootDescriptorTable(DX12_ROOT_SIGNATURE_UA, allocation.gpu);
    }

    void DX12Context::flushCBs()
    {
        if (m_cbGraphicsDirtyFlag && !m_isCompute)
        {
            m_cbGraphicsDirtyFlag = false;
            for (u32 i = 0; i < (u32)(sizeof(m_cbHandles) / sizeof(m_cbHandles[0])); i++)
            {
                const auto cb = m_cbHandles[i];
                if (cb)
                {
                    if (cb->address != 0)
                        m_commandList->SetGraphicsRootConstantBufferView(DX12_ROOT_SIGNATURE_CB + i, cb->address);
                    else
                        m_cbGraphicsDirtyFlag = true; // CB was binded but not yet assigned so stay in dirty state
                }
            }
        }
        else if (m_cbComputeDirtyFlag && m_isCompute)
        {
            m_cbComputeDirtyFlag = false;
            for (u32 i = 0; i < (u32)(sizeof(m_cbHandles) / sizeof(m_cbHandles[0])); i++)
            {
                const auto cb = m_cbHandles[i];
                if (cb)
                {
                    if (cb->address != 0)
                        m_commandList->SetComputeRootConstantBufferView(DX12_ROOT_SIGNATURE_CB + i, cb->address);
                    else
                        m_cbComputeDirtyFlag = true; // CB was binded but not yet assigned so stay in dirty state
                }
            }
        }
    }

    void DX12Context::flushSamplers()
    {
        if (!m_samplersDirtyFlag)
            return;
        m_samplersDirtyFlag = false;
        const i32 sampleSlotCount = (i32)(sizeof(m_samplers) / sizeof(m_samplers[0]));
        i32 lastSlot = -1;
        for (i32 slotIndex = sampleSlotCount - 1; slotIndex >= 0; slotIndex--)
        {
            auto sampler = m_samplers[slotIndex];
            if (sampler)
            {
                lastSlot = slotIndex;
                break;
            }
        }
        if (lastSlot < 0)
            return;
        const u32 samplersCount = lastSlot + 1;
        D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptorRangeStarts[GFX_MAX_SAMPLER_BINDED - GFX_STATIC_SAMPLERS_COUNT];
        for (u32 i = 0; i < samplersCount; i++)
        {
            const auto handle = m_samplers[i];
            if (handle != nullptr)
            {
                srcDescriptorRangeStarts[i] = handle->handleCPU;
            }
        }
        auto allocation = m_device->ringHeapSampler.allocateTable(samplersCount);
        m_device->getDevice()->CopyDescriptors(1, &allocation.cpu, &samplersCount, samplersCount, srcDescriptorRangeStarts, nullptr, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        if (m_isCompute)
            m_commandList->SetComputeRootDescriptorTable(DX12_ROOT_SIGNATURE_SAMPLER, allocation.gpu);
        else
            m_commandList->SetGraphicsRootDescriptorTable(DX12_ROOT_SIGNATURE_SAMPLER, allocation.gpu);
    }

    void DX12Context::flushRBs()
    {
#if DX12_ENABLE_RESOURCE_BARRIERS_BATCHING
        // Check if flush buffer
        if (m_rbBufferSize > 0)
        {
            // Flush resource barriers
            m_commandList->ResourceBarrier(m_rbBufferSize, m_rbBuffer);

            // Clear size
            m_rbBufferSize = 0;
        }
#endif
    }

    void DX12Context::flushPS()
    {
        if (m_psDirtyFlag && m_currentState && (m_rtDepth || m_rtCount))
        {
            // Clear flag
            m_psDirtyFlag = false;

            // Change state
            TK_ASSERT(m_currentState->isValid());
            m_commandList->SetPipelineState(m_currentState->getState(m_rtDepth, m_rtCount, m_rtHandles, m_vertexLayout));
            if (m_primitiveTopology != m_currentState->primitiveTopology)
            {
                m_primitiveTopology = m_currentState->primitiveTopology;
                m_commandList->IASetPrimitiveTopology(m_primitiveTopology);
            }
        }
    }

    void DX12Context::onDrawCall()
    {
        // Ensure state of the vertex and index buffers
        for (i32 i = 0; i < m_vbCount; i++)
        {
            const auto vbDX12 = m_vbHandles[i];
            if (vbDX12)
            {
                setResourceState(vbDX12, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
            }
        }
        if (m_ibHandle)
        {
            setResourceState(m_ibHandle, D3D12_RESOURCE_STATE_INDEX_BUFFER);
        }

        if (m_currentState)
        {
            // If SRV resource is not binded to RTV then transition it to the whole state (GPU-BASED VALIDATION complains about it)
            const u32 srMask = m_currentState->getUsedSRsMask();
            const u32 srCount = TkMath::floorLog2(srMask) + 1;
            for (u32 i = 0; i < srCount; i++)
            {
                const auto handle = m_srHandles[i];
                if ((srMask & (1u << i)) && handle != nullptr && handle->getResourceOwner())
                {
                    const auto resourceOwner = handle->getResourceOwner();
                    bool isRtv = m_rtDepth == handle;
                    for (i32 j = 0; j < m_rtCount; j++)
                    {
                        if (m_rtHandles[j] && m_rtHandles[j]->getResourceOwner() == resourceOwner)
                        {
                            isRtv = true;
                            break;
                        }
                    }
                    if (!isRtv)
                    {
                        D3D12_RESOURCE_STATES states = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
                        if (handle->isDepthStencilResource())
                            states |= D3D12_RESOURCE_STATE_DEPTH_READ;
                        setResourceState(handle->getResourceOwner(), states);
                    }
                }
            }
        }

        // Flush
        flushSRVs();
        flushRTVs();
        flushUAVs();
        flushRBs();
        flushPS();
        flushCBs();
        flushSamplers();
    }

    void DX12Context::frameBegin()
    {
        // Base
        GfxContext::frameBegin();

        // Prepare command list
        reset();
    }

    void DX12Context::frameEnd()
    {
        // Base
        GfxContext::frameEnd();

        // Execute command (but don't wait for them)
        frameFenceValues[1] = frameFenceValues[0];
        frameFenceValues[0] = execute(false);
    }

#if GFX_ALLOW_PROFILE_EVENTS
    void DX12Context::eventBegin(const char *name)
    {
        // TODO: PIX integration
    }

    void DX12Context::eventEnd()
    {
        // TODO: PIX integration
    }
#endif

    void *DX12Context::getNativePtr() const
    {
        return (void *)m_commandList;
    }

    void DX12Context::clear(GfxTextureView *rt, const Color &color)
    {
        auto rtDX12 = static_cast<DX12TextureView *>(rt);
        if (rtDX12)
        {
            setResourceState(rtDX12->getResourceOwner(), D3D12_RESOURCE_STATE_RENDER_TARGET, rtDX12->subResIdx);
            flushRBs();

            m_commandList->ClearRenderTargetView(rtDX12->rtv(), color.raw, 0, nullptr);
        }
    }

    void DX12Context::clearDepth(GfxTextureView *depthBuffer, float depthValue, u8 stencilValue)
    {
        auto depthBufferDX12 = static_cast<DX12TextureView *>(depthBuffer);
        if (depthBufferDX12)
        {
            setResourceState(depthBufferDX12->getResourceOwner(), D3D12_RESOURCE_STATE_DEPTH_WRITE, depthBufferDX12->subResIdx);
            flushRBs();

            m_commandList->ClearDepthStencilView(depthBufferDX12->dsv(), D3D12_CLEAR_FLAG_DEPTH, depthValue, stencilValue, 0, nullptr);
        }
    }

    void DX12Context::clearUA(GfxBuffer *buf, const FVec4 &value)
    {
        TK_ASSERT(buf != nullptr && buf->isUnorderedAccess());
        auto bufDX12 = reinterpret_cast<DX12Buffer *>(buf);

        setResourceState(bufDX12, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        flushRBs();

        auto uav = ((DX12BufferView *)bufDX12->view())->uav();
        Descriptor desc;
        getActiveHeapDescriptor(uav, desc);
        m_commandList->ClearUnorderedAccessViewFloat(desc.gpu, uav, bufDX12->getResource(), value.raw, 0, nullptr);
    }

    void DX12Context::clearUA(GfxBuffer *buf, const u32 value[4])
    {
        TK_ASSERT(buf != nullptr && buf->isUnorderedAccess());
        auto bufDX12 = reinterpret_cast<DX12Buffer *>(buf);

        setResourceState(bufDX12, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        flushRBs();

        auto uav = ((DX12BufferView *)bufDX12->view())->uav();
        Descriptor desc;
        getActiveHeapDescriptor(uav, desc);
        m_commandList->ClearUnorderedAccessViewUint(desc.gpu, uav, bufDX12->getResource(), value, 0, nullptr);
    }

    void DX12Context::clearUA(GfxTexture *texture, const u32 value[4])
    {
        TK_ASSERT(texture != nullptr && texture->isUnorderedAccess());
        auto texDX12 = reinterpret_cast<DX12Texture *>(texture);

        setResourceState(texDX12, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        flushRBs();

        auto uav = ((DX12TextureView *)texDX12->view(0))->uav();
        Descriptor desc;
        getActiveHeapDescriptor(uav, desc);
        m_commandList->ClearUnorderedAccessViewUint(desc.gpu, uav, texDX12->getResource(), value, 0, nullptr);
    }

    void DX12Context::clearUA(GfxTexture *texture, const FVec4 &value)
    {
        TK_ASSERT(texture != nullptr && texture->isUnorderedAccess());
        auto texDX12 = reinterpret_cast<DX12Texture *>(texture);

        setResourceState(texDX12, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        flushRBs();

        auto uav = ((DX12TextureView *)(texDX12->isVolume() ? texDX12->viewVolume() : texDX12->view(0)))->uav();
        Descriptor desc;
        getActiveHeapDescriptor(uav, desc);
        m_commandList->ClearUnorderedAccessViewFloat(desc.gpu, uav, texDX12->getResource(), value.raw, 0, nullptr);
    }

    void DX12Context::resetRenderTarget()
    {
        if (m_rtDepth || m_rtCount != 0)
        {
            m_rtDirtyFlag = false;
            m_psDirtyFlag = true;
            m_rtCount = 0;
            m_rtDepth = nullptr;
            TkMemory::memoryClear(m_rtHandles, sizeof(m_rtHandles));

            D3D12_CPU_DESCRIPTOR_HANDLE rtCPU[GFX_MAX_RT_BINDED];
            TkMemory::memoryClear(rtCPU, sizeof(rtCPU));
            m_commandList->OMSetRenderTargets(0, rtCPU, false, nullptr);
        }
    }

    void DX12Context::setRenderTarget(GfxTextureView *rt)
    {
        DX12TextureView *rtDX12 = static_cast<DX12TextureView *>(rt);

        if (m_rtDepth != nullptr || m_rtCount != 1 || m_rtHandles[0] != rtDX12)
        {
            m_rtDirtyFlag = true;
            m_psDirtyFlag = true;
            m_rtCount = 1;
            m_rtDepth = nullptr;
            m_rtHandles[0] = rtDX12;
        }
    }

    void DX12Context::setRenderTarget(GfxTextureView *depthBuffer, GfxTextureView *rt)
    {
        auto rtDX12 = static_cast<DX12TextureView *>(rt);
        auto depthBufferDX12 = static_cast<DX12TextureView *>(depthBuffer);
        auto rtCount = rtDX12 ? 1 : 0;

        if (m_rtDepth != depthBufferDX12 || m_rtCount != rtCount || m_rtHandles[0] != rtDX12)
        {
            m_rtDirtyFlag = true;
            m_psDirtyFlag = true;
            m_rtCount = rtCount;
            m_rtDepth = depthBufferDX12;
            m_rtHandles[0] = rtDX12;
        }
    }

    void DX12Context::setRenderTarget(GfxTextureView *depthBuffer, const TkSpan<GfxTextureView *> &rts)
    {
        TK_ASSERT(rts.length() >= 1 && rts.length() <= GFX_MAX_RT_BINDED);

        const auto depthBufferDX12 = static_cast<DX12TextureView *>(depthBuffer);

        DX12TextureView *rtvs[GFX_MAX_RT_BINDED];
        for (i32 i = 0; i < rts.length(); i++)
        {
            rtvs[i] = static_cast<DX12TextureView *>(rts[i]);
        }
        const i32 rtvsSize = sizeof(DX12TextureView *) * rts.length();

        if (m_rtDepth != depthBufferDX12 || m_rtCount != rts.length() || TkMemory::memoryCompare(m_rtHandles, rtvs, rtvsSize) != 0)
        {
            m_rtDirtyFlag = true;
            m_psDirtyFlag = true;
            m_rtCount = rts.length();
            m_rtDepth = depthBufferDX12;
            TkMemory::memoryCopy(m_rtHandles, rtvs, rtvsSize);
        }
    }

    void DX12Context::setBlendFactor(const FVec4 &value)
    {
        m_commandList->OMSetBlendFactor(value.raw);
    }

    void DX12Context::setStencilRef(u32 value)
    {
        if (m_stencilRef != value)
        {
            m_stencilRef = value;
            m_commandList->OMSetStencilRef(value);
        }
    }

    void DX12Context::resetSR()
    {
        for (i32 slot = 0; slot < GFX_MAX_SR_BINDED; slot++)
        {
            if (m_srHandles[slot])
            {
                m_srMaskDirtyGraphics |= 1u << slot;
                m_srMaskDirtyCompute |= 1u << slot;
                m_srHandles[slot] = nullptr;
            }
        }
    }

    void DX12Context::resetUA()
    {
        const u32 uaSlotCount = (u32)(sizeof(m_uaHandles) / sizeof(m_uaHandles[0]));
        for (u32 slot = 0; slot < uaSlotCount; slot++)
        {
            if (m_uaHandles[slot])
            {
                m_srMaskDirtyGraphics |= 1u << slot;
                m_srMaskDirtyCompute |= 1u << slot;
                m_uaHandles[slot] = nullptr;
                break;
            }
        }
    }

    void DX12Context::resetCB()
    {
        m_cbGraphicsDirtyFlag = false;
        m_cbComputeDirtyFlag = false;
        TkMemory::memoryClear(m_cbHandles, sizeof(m_cbHandles));
    }

    void DX12Context::bindCB(i32 slot, GfxConstantBuffer *cb)
    {
        TK_ASSERT(slot >= 0 && slot < GFX_MAX_CB_BINDED);
        auto cbDX12 = static_cast<DX12ConstantBuffer *>(cb);
        if (m_cbHandles[slot] != cbDX12)
        {
            m_cbGraphicsDirtyFlag = true;
            m_cbComputeDirtyFlag = true;
            m_cbHandles[slot] = cbDX12;
        }
    }

    void DX12Context::bindSR(i32 slot, GfxResourceView *view)
    {
#if !BUILD_RELEASE
        TK_ASSERT(slot >= 0 && slot < GFX_MAX_SR_BINDED);
#if TK_DEBUG
        if (view && ((DX12IShaderResource *)view->getNativePtr())->srv().ptr == 0)
            logInvalidResourceUsage(slot, view, InvalidBindPoint::SRV);
#endif
#endif
        auto handle = view ? (DX12IShaderResource *)view->getNativePtr() : nullptr;
        if (m_srHandles[slot] != handle)
        {
            m_srMaskDirtyGraphics |= 1u << slot;
            m_srMaskDirtyCompute |= 1u << slot;
            m_srHandles[slot] = handle;
            if (view)
                *view->lastRenderTime = m_lastRenderTime;
        }
    }

    void DX12Context::bindUA(i32 slot, GfxResourceView *view)
    {
#if !BUILD_RELEASE
        TK_ASSERT(slot >= 0 && slot < GFX_MAX_UA_BINDED);
#if TK_DEBUG
        if (view && ((DX12IShaderResource *)view->getNativePtr())->uav().ptr == 0)
            logInvalidResourceUsage(slot, view, InvalidBindPoint::UAV);
#endif
#endif
        m_uaHandles[slot] = view ? (DX12IShaderResource *)view->getNativePtr() : nullptr;
        if (view)
            *view->lastRenderTime = m_lastRenderTime;
    }

    void DX12Context::bindVB(const TkSpan<GfxBuffer *> &vertexBuffers, const u32 *vertexBuffersOffsets, GfxVertexLayout *vertexLayout)
    {
        TK_ASSERT(vertexBuffers.length() >= 0 && vertexBuffers.length() <= GFX_MAX_VB_BINDED);
        bool vbEdited = m_vbCount != vertexBuffers.length();
        D3D12_VERTEX_BUFFER_VIEW views[GFX_MAX_VB_BINDED];
        for (i32 i = 0; i < vertexBuffers.length(); i++)
        {
            const auto vbDX12 = static_cast<DX12Buffer *>(vertexBuffers[i]);
            if (vbDX12)
            {
                vbDX12->getVBView(views[i]);
                if (vertexBuffersOffsets)
                {
                    views[i].BufferLocation += (D3D12_GPU_VIRTUAL_ADDRESS)vertexBuffersOffsets[i];
                    views[i].SizeInBytes -= vertexBuffersOffsets[i];
                }
                vbEdited |= views[i] != m_vbViews[i];
            }
            else
            {
                TkMemory::memoryClear(&views[i], sizeof(D3D12_VERTEX_BUFFER_VIEW));
            }
            vbEdited |= vbDX12 != m_vbHandles[i];
            m_vbHandles[i] = vbDX12;
        }
        if (vbEdited)
        {
            m_vbCount = vertexBuffers.length();
            TkMemory::memoryCopy(m_vbViews, views, sizeof(views));
            m_commandList->IASetVertexBuffers(0, vertexBuffers.length(), views);
        }
        if (!vertexLayout)
            vertexLayout = GfxVertexLayout::get(vertexBuffers);
        if (reinterpret_cast<GfxVertexLayout *>(m_vertexLayout) != vertexLayout)
        {
            m_vertexLayout = reinterpret_cast<DX12VertexLayout *>(vertexLayout);
            m_psDirtyFlag = true;
        }
    }

    void DX12Context::bindIB(GfxBuffer *indexBuffer)
    {
        const auto ibDX12 = static_cast<DX12Buffer *>(indexBuffer);
        D3D12_INDEX_BUFFER_VIEW view;
        ibDX12->getIBView(view);
        if (ibDX12 != m_ibHandle || m_ibView != view)
        {
            m_ibHandle = ibDX12;
            m_commandList->IASetIndexBuffer(&view);
            m_ibView = view;
        }
    }

    void DX12Context::bindSampler(i32 slot, GfxSampler *sampler)
    {
        TK_ASSERT(slot >= GFX_STATIC_SAMPLERS_COUNT && slot < GFX_MAX_SAMPLER_BINDED);
        const auto handle = sampler ? static_cast<Dx12Sampler *>(sampler) : nullptr;
        if (m_samplers[slot - GFX_STATIC_SAMPLERS_COUNT] != handle)
        {
            m_samplersDirtyFlag = true;
            m_samplers[slot - GFX_STATIC_SAMPLERS_COUNT] = handle;
        }
    }

    void DX12Context::updateCB(GfxConstantBuffer *cb, const void *data)
    {
        TK_ASSERT(data && cb);
        auto cbDX12 = static_cast<DX12ConstantBuffer *>(cb);
        const u32 size = cbDX12->getSize();
        if (size == 0)
            return;

        // Allocate bytes for the buffer
        DynamicAllocation allocation = m_device->uploadBuffer->allocate(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

        // Copy data
        TkMemory::memoryCopy(allocation.addressCPU, data, allocation.size);

        // Cache GPU address of the allocation
        cbDX12->address = allocation.addressGPU;

        // Mark CB slot as dirty if this CB is binded to the pipeline
        for (u32 i = 0; i < (u32)(sizeof(m_cbHandles) / sizeof(m_cbHandles[0])); i++)
        {
            if (m_cbHandles[i] == cbDX12)
            {
                m_cbGraphicsDirtyFlag = true;
                m_cbComputeDirtyFlag = true;
                break;
            }
        }
    }

    void DX12Context::dispatch(GfxShaderProgramCS *shader, u32 threadGroupCountX, u32 threadGroupCountY, u32 threadGroupCountZ)
    {
        m_isCompute = 1;
        m_currentCompute = shader;

        flushSRVs();
        flushUAVs();
        flushRBs();
        flushCBs();
        flushSamplers();

        auto shaderDX12 = (DX12ShaderProgramCS *)shader;
        auto computeState = shaderDX12->getOrCreateState();

        m_commandList->SetPipelineState(computeState);

        m_commandList->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);

        m_isCompute = 0;
        m_currentCompute = nullptr;

        // Restore previous state on next draw call
        m_psDirtyFlag = true;

        // Insert UAV barrier to ensure proper memory access for multiple sequential dispatches
        addUAVBarrier();
    }

    void DX12Context::dispatchIndirect(GfxShaderProgramCS *shader, GfxBuffer *bufferForArgs, u32 offsetForArgs)
    {
        m_isCompute = 1;
        m_currentCompute = shader;

        auto bufferForArgsDX12 = (DX12Buffer *)bufferForArgs;
        setResourceState(bufferForArgsDX12, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

        flushSRVs();
        flushUAVs();
        flushRBs();
        flushCBs();
        flushSamplers();

        auto shaderDX12 = (DX12ShaderProgramCS *)shader;
        auto computeState = shaderDX12->getOrCreateState();

        m_commandList->SetPipelineState(computeState);

        auto signature = m_device->dispatchIndirectCommandSignature->getSignature();
        m_commandList->ExecuteIndirect(signature, 1, bufferForArgsDX12->getResource(), (UINT64)offsetForArgs, nullptr, 0);

        m_isCompute = 0;
        m_currentCompute = nullptr;

        // Restore previous state on next draw call
        m_psDirtyFlag = true;

        // Insert UAV barrier to ensure proper memory access for multiple sequential dispatches
        addUAVBarrier();
    }

    void DX12Context::resolveMultisample(GfxTexture *sourceMultisampleTexture, GfxTexture *destTexture, i32 sourceSubResource, i32 destSubResource, PixelFormat format)
    {
        TK_ASSERT(sourceMultisampleTexture && sourceMultisampleTexture->isMultiSample());
        TK_ASSERT(destTexture && !destTexture->isMultiSample());

        auto dstResourceDX12 = static_cast<DX12Texture *>(destTexture);
        auto srcResourceDX12 = static_cast<DX12Texture *>(sourceMultisampleTexture);

        setResourceState(srcResourceDX12, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
        setResourceState(dstResourceDX12, D3D12_RESOURCE_STATE_RESOLVE_DEST);
        flushRBs();

        auto formatDXGI = DxUtils::toDxgiFormat(format == PixelFormat::kUnknown ? destTexture->format() : format);
        m_commandList->ResolveSubresource(dstResourceDX12->getResource(), destSubResource, srcResourceDX12->getResource(), destSubResource, formatDXGI);
    }

    void DX12Context::drawInstanced(u32 verticesCount, u32 instanceCount, i32 startInstance, i32 startVertex)
    {
        onDrawCall();
        m_commandList->DrawInstanced(verticesCount, instanceCount, startVertex, startInstance);
    }

    void DX12Context::drawIndexedInstanced(u32 indicesCount, u32 instanceCount, i32 startInstance, i32 startVertex, i32 startIndex)
    {
        onDrawCall();
        m_commandList->DrawIndexedInstanced(indicesCount, instanceCount, startIndex, startVertex, startInstance);
    }

    void DX12Context::drawInstancedIndirect(GfxBuffer *bufferForArgs, u32 offsetForArgs)
    {
        TK_ASSERT(bufferForArgs && (bufferForArgs->getFlags() & GfxBufferFlags::kArgument) != GfxBufferFlags::kNone);

        auto bufferForArgsDX12 = (DX12Buffer *)bufferForArgs;
        auto signature = m_device->drawIndirectCommandSignature->getSignature();
        setResourceState(bufferForArgsDX12, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

        onDrawCall();
        m_commandList->ExecuteIndirect(signature, 1, bufferForArgsDX12->getResource(), (UINT64)offsetForArgs, nullptr, 0);
    }

    void DX12Context::drawIndexedInstancedIndirect(GfxBuffer *bufferForArgs, u32 offsetForArgs)
    {
        TK_ASSERT(bufferForArgs && (bufferForArgs->getFlags() & GfxBufferFlags::kArgument) != GfxBufferFlags::kNone);

        auto bufferForArgsDX12 = (DX12Buffer *)bufferForArgs;
        auto signature = m_device->drawIndexedIndirectCommandSignature->getSignature();
        setResourceState(bufferForArgsDX12, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

        onDrawCall();
        m_commandList->ExecuteIndirect(signature, 1, bufferForArgsDX12->getResource(), (UINT64)offsetForArgs, nullptr, 0);
    }

    void DX12Context::setViewport(const Viewport &viewport)
    {
        m_commandList->RSSetViewports(1, (D3D12_VIEWPORT *)&viewport);
    }

    void DX12Context::setScissor(const Rectangle &scissorRect)
    {
        D3D12_RECT rect;
        rect.left = (LONG)scissorRect.getLeft();
        rect.right = (LONG)scissorRect.getRight();
        rect.top = (LONG)scissorRect.getTop();
        rect.bottom = (LONG)scissorRect.getBottom();
        m_commandList->RSSetScissorRects(1, &rect);
    }

    GfxPipelineState *DX12Context::getState() const
    {
        return m_currentState;
    }

    void DX12Context::setState(GfxPipelineState *state)
    {
        if (m_currentState != state)
        {
            m_currentState = static_cast<DX12PipelineState *>(state);
            m_psDirtyFlag = true;
        }
    }

    void DX12Context::clearState()
    {
        if (!m_commandList)
            return;

        resetRenderTarget();
        resetSR();
        resetUA();
        resetCB();
        setState(nullptr);

        flushState();
    }

    void DX12Context::flushState()
    {
        // Flush
        flushRBs();
    }

    void DX12Context::flush()
    {
        if (!m_currentAllocator)
            return;

        // Flush GPU commands
        execute(true);
        reset();
    }

    void DX12Context::updateBuffer(GfxBuffer *buffer, const void *data, u32 size, u32 offset)
    {
        TK_ASSERT(data);
        TK_ASSERT(buffer && buffer->getSize() >= size);

        auto bufferDX12 = (DX12Buffer *)buffer;

        setResourceState(bufferDX12, D3D12_RESOURCE_STATE_COPY_DEST);
        flushRBs();

        m_device->uploadBuffer->uploadBuffer(this, bufferDX12->getResource(), offset, data, size);
    }

    void DX12Context::copyBuffer(GfxBuffer *dstBuffer, GfxBuffer *srcBuffer, u32 size, u32 dstOffset, u32 srcOffset)
    {
        TK_ASSERT(dstBuffer && srcBuffer);

        auto dstBufferDX12 = static_cast<DX12Buffer *>(dstBuffer);
        auto srcBufferDX12 = static_cast<DX12Buffer *>(srcBuffer);

        setResourceState(dstBufferDX12, D3D12_RESOURCE_STATE_COPY_DEST);
        setResourceState(srcBufferDX12, D3D12_RESOURCE_STATE_COPY_SOURCE);
        flushRBs();

        m_commandList->CopyBufferRegion(dstBufferDX12->getResource(), dstOffset, srcBufferDX12->getResource(), srcOffset, size);
    }

    void DX12Context::updateTexture(GfxTexture *texture, i32 arrayIndex, i32 mipIndex, const void *data, u32 rowPitch, u32 slicePitch)
    {
        TK_ASSERT(texture && texture->isAllocated() && data);

        auto textureDX12 = static_cast<DX12Texture *>(texture);

        setResourceState(textureDX12, D3D12_RESOURCE_STATE_COPY_DEST);
        flushRBs();

        m_device->uploadBuffer->uploadTexture(this, textureDX12->getResource(), data, rowPitch, slicePitch, mipIndex, arrayIndex);
    }

    void DX12Context::copyTexture(GfxTexture *dstResource, u32 dstSubresource, u32 dstX, u32 dstY, u32 dstZ, GfxTexture *srcResource, u32 srcSubresource)
    {
        auto dstTextureDX12 = (DX12Texture *)dstResource;
        auto srcTextureDX12 = (DX12Texture *)srcResource;

        setResourceState(dstTextureDX12, D3D12_RESOURCE_STATE_COPY_DEST);
        setResourceState(srcTextureDX12, D3D12_RESOURCE_STATE_COPY_SOURCE);
        flushRBs();

        // Get destination copy location
        D3D12_TEXTURE_COPY_LOCATION dst;
        dst.pResource = dstTextureDX12->getResource();
        if (dstTextureDX12->isStaging())
        {
            const i32 mipLevel = (i32)dstSubresource % dstTextureDX12->mipLevels();
            const i32 copyOffset = dstTextureDX12->computeBufferOffset((i32)dstSubresource, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
            dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            dst.PlacedFootprint.Offset = copyOffset;
            dst.PlacedFootprint.Footprint.Width = dstTextureDX12->calculateMipSize(dstTextureDX12->width(), mipLevel);
            dst.PlacedFootprint.Footprint.Height = dstTextureDX12->calculateMipSize(dstTextureDX12->height(), mipLevel);
            dst.PlacedFootprint.Footprint.Depth = dstTextureDX12->calculateMipSize(dstTextureDX12->depth(), mipLevel);
            dst.PlacedFootprint.Footprint.Format = DxUtils::toDxgiFormat(dstTextureDX12->format());
            dst.PlacedFootprint.Footprint.RowPitch = dstTextureDX12->computeRowPitch(mipLevel, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
        }
        else
        {
            dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dst.SubresourceIndex = dstSubresource;
        }

        // Get source copy location
        D3D12_TEXTURE_COPY_LOCATION src;
        src.pResource = srcTextureDX12->getResource();
        src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        src.SubresourceIndex = srcSubresource;

        // Copy
        m_commandList->CopyTextureRegion(&dst, dstX, dstY, dstZ, &src, nullptr);
    }

    void DX12Context::resetCounter(GfxBuffer *buffer)
    {
        TK_ASSERT(buffer);

        auto bufferDX12 = static_cast<DX12Buffer *>(buffer);
        auto counter = bufferDX12->getCounter();
        TK_ASSERT(counter != nullptr);

        setResourceState(counter, D3D12_RESOURCE_STATE_COPY_DEST);
        flushRBs();

        u32 value = 0;
        m_device->uploadBuffer->uploadBuffer(this, counter->getResource(), 0, &value, 4);

        setResourceState(counter, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }

    void DX12Context::copyCounter(GfxBuffer *dstBuffer, u32 dstOffset, GfxBuffer *srcBuffer)
    {
        TK_ASSERT(dstBuffer && srcBuffer);

        auto dstBufferDX12 = static_cast<DX12Buffer *>(dstBuffer);
        auto srcBufferDX12 = static_cast<DX12Buffer *>(srcBuffer);
        auto counter = srcBufferDX12->getCounter();
        TK_ASSERT(counter != nullptr);

        setResourceState(dstBufferDX12, D3D12_RESOURCE_STATE_COPY_DEST);
        setResourceState(counter, D3D12_RESOURCE_STATE_COPY_SOURCE);
        flushRBs();

        m_commandList->CopyBufferRegion(dstBufferDX12->getResource(), dstOffset, counter->getResource(), 0, 4);
    }

    void DX12Context::copyResource(GfxResource *dstResource, GfxResource *srcResource)
    {
        TK_ASSERT(dstResource && srcResource);

        auto dstResourceDX12 = dynamic_cast<DX12ResourceOwner *>(dstResource);
        auto srcResourceDX12 = dynamic_cast<DX12ResourceOwner *>(srcResource);
        auto dstBufferDX12 = dynamic_cast<DX12Buffer *>(dstResource);
        auto srcBufferDX12 = dynamic_cast<DX12Buffer *>(srcResource);
        auto dstTextureDX12 = dynamic_cast<DX12Texture *>(dstResource);
        auto srcTextureDX12 = dynamic_cast<DX12Texture *>(srcResource);

        setResourceState(dstResourceDX12, D3D12_RESOURCE_STATE_COPY_DEST);
        setResourceState(srcResourceDX12, D3D12_RESOURCE_STATE_COPY_SOURCE);
        flushRBs();

        // Buffer -> Buffer
        if (srcBufferDX12 && dstBufferDX12)
        {
            m_commandList->CopyResource(dstResourceDX12->getResource(), srcResourceDX12->getResource());
        }
        // Texture -> Texture
        else if (srcTextureDX12 && dstTextureDX12)
        {
            if (dstTextureDX12->isStaging())
            {
                // Staging Texture -> Staging Texture
                if (srcTextureDX12->isStaging())
                {
                    const i32 size = dstTextureDX12->computeBufferTotalSize(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

                    // Map the staging resources
                    D3D12_RANGE range;
                    range.Begin = 0;
                    range.End = size;
                    void *srcMapped;
                    HRESULT mapResult = srcTextureDX12->getResource()->Map(0, &range, &srcMapped);
                    LOG_DIRECTX_RESULT(mapResult);
                    void *dstMapped;
                    mapResult = dstTextureDX12->getResource()->Map(0, &range, &dstMapped);
                    LOG_DIRECTX_RESULT(mapResult);

                    // Copy data
                    TkMemory::memoryCopy(dstMapped, srcMapped, size);

                    // Unmap buffers
                    srcTextureDX12->getResource()->Unmap(0, nullptr);
                    dstTextureDX12->getResource()->Unmap(0, nullptr);
                }
                else
                {
                    for (i32 arraySlice = 0; arraySlice < srcTextureDX12->arraySize(); arraySlice++)
                    {
                        for (i32 mipLevel = 0; mipLevel < srcTextureDX12->mipLevels(); mipLevel++)
                        {
                            const i32 subresource = (i32)DxUtils::calcSubresourceIndex(mipLevel, arraySlice, srcTextureDX12->mipLevels());
                            const i32 copyOffset = dstTextureDX12->computeBufferOffset(subresource, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

                            D3D12_TEXTURE_COPY_LOCATION dstLocation;
                            dstLocation.pResource = dstTextureDX12->getResource();
                            dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                            dstLocation.PlacedFootprint.Offset = copyOffset;
                            dstLocation.PlacedFootprint.Footprint.Width = dstTextureDX12->calculateMipSize(dstTextureDX12->width(), mipLevel);
                            dstLocation.PlacedFootprint.Footprint.Height = dstTextureDX12->calculateMipSize(dstTextureDX12->height(), mipLevel);
                            dstLocation.PlacedFootprint.Footprint.Depth = dstTextureDX12->calculateMipSize(dstTextureDX12->depth(), mipLevel);
                            dstLocation.PlacedFootprint.Footprint.Format = DxUtils::toDxgiFormat(dstTextureDX12->format());
                            dstLocation.PlacedFootprint.Footprint.RowPitch = dstTextureDX12->computeRowPitch(mipLevel, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

                            D3D12_TEXTURE_COPY_LOCATION srcLocation;
                            srcLocation.pResource = srcTextureDX12->getResource();
                            srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                            srcLocation.SubresourceIndex = subresource;

                            m_commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
                        }
                    }
                }
            }
            else
            {
                m_commandList->CopyResource(dstResourceDX12->getResource(), srcResourceDX12->getResource());
            }
        }
        else
        {
            // Not implemented
            TK_ASSERT(false);
        }
    }

    void DX12Context::copySubresource(GfxResource *dstResource, u32 dstSubresource, GfxResource *srcResource, u32 srcSubresource)
    {
        TK_ASSERT(dstResource && srcResource);

        auto dstResourceDX12 = dynamic_cast<DX12ResourceOwner *>(dstResource);
        auto srcResourceDX12 = dynamic_cast<DX12ResourceOwner *>(srcResource);
        auto dstBufferDX12 = dynamic_cast<DX12Buffer *>(dstResource);
        auto srcBufferDX12 = dynamic_cast<DX12Buffer *>(srcResource);
        auto dstTextureDX12 = dynamic_cast<DX12Texture *>(dstResource);
        auto srcTextureDX12 = dynamic_cast<DX12Texture *>(srcResource);

        setResourceState(dstResourceDX12, D3D12_RESOURCE_STATE_COPY_DEST);
        setResourceState(srcResourceDX12, D3D12_RESOURCE_STATE_COPY_SOURCE);
        flushRBs();

        // Buffer -> Buffer
        if (srcBufferDX12 && dstBufferDX12)
        {
            u64 bytesCount = srcResource->getMemoryUse();
            m_commandList->CopyBufferRegion(dstBufferDX12->getResource(), 0, srcBufferDX12->getResource(), 0, bytesCount);
        }
        // Texture -> Texture
        else if (srcTextureDX12 && dstTextureDX12)
        {
            if (srcTextureDX12->isStaging() || dstTextureDX12->isStaging())
            {
                // TODO: copy region of staging resources is not supported yet.
                return;
            }

            // Create copy locations structures
            D3D12_TEXTURE_COPY_LOCATION dstLocation;
            dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dstLocation.pResource = dstTextureDX12->getResource();
            dstLocation.SubresourceIndex = dstSubresource;
            D3D12_TEXTURE_COPY_LOCATION srcLocation;
            srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            srcLocation.pResource = srcTextureDX12->getResource();
            srcLocation.SubresourceIndex = srcSubresource;

            m_commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
        }
        else
        {
            // TODO: Cannot copy data between buffer and texture.
        }
    }

    void DX12Context::setResourceState(GfxResource *resource, u64 state, i32 subresource)
    {
        auto resourceDX12 = dynamic_cast<DX12ResourceOwner *>(resource);
        setResourceState(resourceDX12, (D3D12_RESOURCE_STATES)state, subresource);
    }

    void DX12Context::forceRebindDescriptors()
    {
        // Bind Root Signature
        m_commandList->SetGraphicsRootSignature(m_device->getRootSignature());
        m_commandList->SetComputeRootSignature(m_device->getRootSignature());

        // Bind heaps
        ID3D12DescriptorHeap *ppHeaps[] = {m_device->ringHeapCvbSrvUav.GetHeap(), m_device->ringHeapSampler.GetHeap()};
        m_commandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ppHeaps[0]), ppHeaps);
    }

}

#endif
