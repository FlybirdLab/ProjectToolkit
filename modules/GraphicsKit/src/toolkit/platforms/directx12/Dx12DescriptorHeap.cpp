/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx12DescriptorHeap.hpp"

#if GFX_API_DIRECTX12

#include "Dx12Device.hpp"

namespace toolkit
{

    void DX12DescriptorHeapWithSlots::Slot::createSRV(DX12Device *device, ID3D12Resource *resource, D3D12_SHADER_RESOURCE_VIEW_DESC *desc)
    {
        if (heap == nullptr)
            device->heapCvbSrvUav.allocateSlot(heap, index);
        device->getDevice()->CreateShaderResourceView(resource, desc, cpu());
    }

    void DX12DescriptorHeapWithSlots::Slot::createRTV(DX12Device *device, ID3D12Resource *resource, D3D12_RENDER_TARGET_VIEW_DESC *desc)
    {
        if (heap == nullptr)
            device->heapRtv.allocateSlot(heap, index);
        device->getDevice()->CreateRenderTargetView(resource, desc, cpu());
    }

    void DX12DescriptorHeapWithSlots::Slot::createDSV(DX12Device *device, ID3D12Resource *resource, D3D12_DEPTH_STENCIL_VIEW_DESC *desc)
    {
        if (heap == nullptr)
            device->heapDsv.allocateSlot(heap, index);
        device->getDevice()->CreateDepthStencilView(resource, desc, cpu());
    }

    void DX12DescriptorHeapWithSlots::Slot::createUAV(DX12Device *device, ID3D12Resource *resource, D3D12_UNORDERED_ACCESS_VIEW_DESC *desc, ID3D12Resource *counterResource)
    {
        if (heap == nullptr)
            device->heapCvbSrvUav.allocateSlot(heap, index);
        device->getDevice()->CreateUnorderedAccessView(resource, counterResource, desc, cpu());
    }

    void DX12DescriptorHeapWithSlots::Slot::release()
    {
        if (heap)
        {
            heap->releaseSlot(index);
            heap = nullptr;
        }
    }

    DX12DescriptorHeapWithSlots::DX12DescriptorHeapWithSlots(DX12Device *device)
        : m_device(device), m_heap(nullptr), m_descriptorsCount(0)
    {
    }

    bool DX12DescriptorHeapWithSlots::create(D3D12_DESCRIPTOR_HEAP_TYPE type, u32 descriptorsCount, bool shaderVisible)
    {
        // Create description
        D3D12_DESCRIPTOR_HEAP_DESC desc;
        desc.Type = type;
        desc.NumDescriptors = descriptorsCount;
        desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 0;

        // Create heap
        const HRESULT result = m_device->getDevice()->CreateDescriptorHeap(&desc, __uuidof(ID3D12DescriptorHeap), reinterpret_cast<void **>(&m_heap));
        LOG_DIRECTX_RESULT_WITH_RETURN(result, false);

        // Setup
        m_type = type;
        m_shaderVisible = shaderVisible;
        m_descriptorsCount = descriptorsCount;
        m_beginCPU = m_heap->GetCPUDescriptorHandleForHeapStart();
        if (shaderVisible)
            m_beginGPU = m_heap->GetGPUDescriptorHandleForHeapStart();
        else
            m_beginGPU.ptr = 0;
        m_incrementSize = m_device->getDevice()->GetDescriptorHandleIncrementSize(desc.Type);

        // Setup usage cache
        m_usage.resize(static_cast<i32>(m_descriptorsCount / 32), false);
        TkMemory::memorySet(m_usage.get(), m_usage.count() * sizeof(u32), 0);

        m_memoryUsage = 1;

        return true;
    }

    bool DX12DescriptorHeapWithSlots::tryToGetUnusedSlot(u32 &index)
    {
        for (i32 i = 0; i < m_usage.count(); i++)
        {
            u32 &value = m_usage[i];
            if (value != kMaxU32)
            {
                // TODO: make it better?
                for (i32 bit = 0; bit < 32; bit++)
                {
                    const u32 mask = 1 << bit;
                    if ((value & mask) == 0)
                    {
                        // Found
                        index = i * 32 + bit;
                        value |= mask;
                        return true;
                    }
                }
            }
        }

        return false;
    }

    void DX12DescriptorHeapWithSlots::releaseSlot(u32 index)
    {
        u32 &value = m_usage[index / 32];
        const u32 mask = 1 << (index & 31);
        // ASSERT_LOW_LAYER((value & mask) == mask);
        value &= ~mask;
    }

    GfxResType DX12DescriptorHeapWithSlots::getType() const
    {
        return GfxResType::kDescriptor;
    }

    DX12DescriptorHeapPool::DX12DescriptorHeapPool(DX12Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 descriptorsCountPerHeap, bool shaderVisible)
        : m_device(device), m_type(type), m_descriptorsCountPerHeap(descriptorsCountPerHeap), m_shaderVisible(shaderVisible)
    {
    }

    void DX12DescriptorHeapPool::allocateSlot(DX12DescriptorHeapWithSlots *&heap, u32 &slot)
    {
        for (i32 i = 0; i < m_heaps.count(); i++)
        {
            if (m_heaps[i]->tryToGetUnusedSlot(slot))
            {
                heap = m_heaps[i];
                return;
            }
        }

        heap = TkMemory::newObject<DX12DescriptorHeapWithSlots>(TkMemory::getDefault(), m_device);
        if (!heap->create(m_type, m_descriptorsCountPerHeap, m_shaderVisible))
        {
            TK_ASSERT(false && "Failed to allocate descriptor heap.");
        }
        m_heaps.add(heap);
        heap->tryToGetUnusedSlot(slot);
    }

    void DX12DescriptorHeapPool::releaseGPU()
    {
        for (auto heap : m_heaps)
        {
            heap->releaseData();
            TkMemory::deletePtr(heap);
        }
        m_heaps.clear();
    }

    void DX12DescriptorHeapWithSlots::onReleaseData()
    {
        m_usage.setCapacity(0, false);
        DX_DEL(m_heap, 0);
        m_descriptorsCount = 0;
    }

    DX12DescriptorHeapRingBuffer::DX12DescriptorHeapRingBuffer(DX12Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 descriptorsCount, bool shaderVisible)
        : m_device(device), m_heap(nullptr), m_type(type), m_descriptorsCount(descriptorsCount), m_shaderVisible(shaderVisible)
    {
    }

    bool DX12DescriptorHeapRingBuffer::init()
    {
        // Create heap
        D3D12_DESCRIPTOR_HEAP_DESC desc;
        desc.Type = m_type;
        desc.NumDescriptors = m_descriptorsCount;
        desc.Flags = m_shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 0;
        const HRESULT result = m_device->getDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap));
        LOG_DIRECTX_RESULT_WITH_RETURN(result, false);

        // Setup
        m_firstFree = 0;
        m_beginCPU = m_heap->GetCPUDescriptorHandleForHeapStart();
        if (m_shaderVisible)
            m_beginGPU = m_heap->GetGPUDescriptorHandleForHeapStart();
        else
            m_beginGPU.ptr = 0;
        m_incrementSize = m_device->getDevice()->GetDescriptorHandleIncrementSize(desc.Type);
        m_memoryUsage = 1;

        return true;
    }

    DX12DescriptorHeapRingBuffer::Allocation DX12DescriptorHeapRingBuffer::allocateTable(u32 numDesc)
    {
        Allocation result;

        // Move the ring buffer pointer
        u32 index = m_firstFree;
        m_firstFree += numDesc;

        // Check for overflow
        if (m_firstFree >= m_descriptorsCount)
        {
            // Move to the begin
            index = 0;
            m_firstFree = numDesc;
        }

        // Set pointers
        result.cpu.ptr = m_beginCPU.ptr + static_cast<SIZE_T>(index * m_incrementSize);
        result.gpu.ptr = m_shaderVisible ? m_beginGPU.ptr + index * m_incrementSize : 0;

        return result;
    }

    void DX12DescriptorHeapRingBuffer::onReleaseData()
    {
        DX_DEL(m_heap, 0);
        m_firstFree = 0;
    }

}

#endif
