/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/directx12/common.hpp>

#if GFX_API_DIRECTX12

#include "toolkit/collection/Array.hpp"
#include "toolkit/graphics/GfxResource.hpp"

namespace toolkit
{

    class DX12DescriptorHeapPool;
    class DX12Device;

    // Descriptors heap for DirectX 12 that bit array concept to implement descriptor slots allocation.
    class DX12DescriptorHeapWithSlots : public GfxResource
    {
    public:
        struct Slot
        {
        public:
            DX12DescriptorHeapWithSlots *heap = nullptr;
            u32 index;

        public:
            TK_FORCE_INLINE bool isValid() const
            {
                return heap != nullptr;
            }

#if TK_DEBUG
            ~Slot()
            {
                TK_ASSERT(heap == nullptr);
            }
#endif

            TK_FORCE_INLINE D3D12_CPU_DESCRIPTOR_HANDLE cpu() const
            {
                return heap ? heap->cpu(index) : D3D12_CPU_DESCRIPTOR_HANDLE{};
            }

            TK_FORCE_INLINE D3D12_GPU_DESCRIPTOR_HANDLE gpu() const
            {
                return heap ? heap->gpu(index) : D3D12_GPU_DESCRIPTOR_HANDLE{};
            }

            void createSRV(DX12Device *device, ID3D12Resource *resource, D3D12_SHADER_RESOURCE_VIEW_DESC *desc = nullptr);
            void createRTV(DX12Device *device, ID3D12Resource *resource, D3D12_RENDER_TARGET_VIEW_DESC *desc = nullptr);
            void createDSV(DX12Device *device, ID3D12Resource *resource, D3D12_DEPTH_STENCIL_VIEW_DESC *desc = nullptr);
            void createUAV(DX12Device *device, ID3D12Resource *resource, D3D12_UNORDERED_ACCESS_VIEW_DESC *desc = nullptr, ID3D12Resource *counterResource = nullptr);

            void release();
        };

    private:
        DX12Device *m_device;
        ID3D12DescriptorHeap *m_heap;
        D3D12_CPU_DESCRIPTOR_HANDLE m_beginCPU;
        D3D12_GPU_DESCRIPTOR_HANDLE m_beginGPU;
        D3D12_DESCRIPTOR_HEAP_TYPE m_type;
        u32 m_incrementSize;
        u32 m_descriptorsCount;
        bool m_shaderVisible;
        TkArray<u32> m_usage;

    public:
        DX12DescriptorHeapWithSlots(DX12Device *device);

    public:
        TK_FORCE_INLINE D3D12_CPU_DESCRIPTOR_HANDLE cpu(u32 index)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle;
            handle.ptr = m_beginCPU.ptr + (SIZE_T)(index * m_incrementSize);
            return handle;
        }

        TK_FORCE_INLINE D3D12_GPU_DESCRIPTOR_HANDLE gpu(u32 index)
        {
            D3D12_GPU_DESCRIPTOR_HANDLE handle;
            handle.ptr = m_beginGPU.ptr + index * m_incrementSize;
            return handle;
        }

    public:
        bool create(D3D12_DESCRIPTOR_HEAP_TYPE type, u32 descriptorsCount, bool shaderVisible = false);
        bool tryToGetUnusedSlot(u32 &index);
        void releaseSlot(u32 index);

        GfxResType getType() const final override;

    protected:
        void onReleaseData() override;
    };

    // Descriptors heap pool for DirectX 12.
    class DX12DescriptorHeapPool
    {
    public:
        DX12DescriptorHeapPool(DX12Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 descriptorsCountPerHeap, bool shaderVisible);

    public:
        void allocateSlot(DX12DescriptorHeapWithSlots *&heap, u32 &slot);
        void releaseGPU();

    private:
        DX12Device *m_device;
        D3D12_DESCRIPTOR_HEAP_TYPE m_type;
        u32 m_descriptorsCountPerHeap;
        bool m_shaderVisible;
        TkStaticArray<DX12DescriptorHeapWithSlots *, 32> m_heaps;
    };

    // Descriptors heap for DirectX 12 that uses a ring buffer concept to implement descriptor tables allocation.
    class DX12DescriptorHeapRingBuffer : public GfxResource
    {
    public:
        // Heap allocation info
        struct Allocation
        {
            // Handle in CPU memory
            D3D12_CPU_DESCRIPTOR_HANDLE cpu;
            // Handle in GPU memory
            D3D12_GPU_DESCRIPTOR_HANDLE gpu;
        };

    public:
        DX12DescriptorHeapRingBuffer(DX12Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 descriptorsCount, bool shaderVisible);

    public:
        TK_FORCE_INLINE ID3D12DescriptorHeap *GetHeap() const
        {
            return m_heap;
        }

        bool init();
        Allocation allocateTable(u32 numDesc);

        GfxResType getType() const final override
        {
            return GfxResType::kDescriptor;
        }

    protected:
        void onReleaseData() override;

    private:
        DX12Device *m_device;
        ID3D12DescriptorHeap *m_heap;
        D3D12_CPU_DESCRIPTOR_HANDLE m_beginCPU;
        D3D12_GPU_DESCRIPTOR_HANDLE m_beginGPU;
        D3D12_DESCRIPTOR_HEAP_TYPE m_type;
        u32 m_incrementSize;
        u32 m_descriptorsCount;
        u32 m_firstFree;
        bool m_shaderVisible;
    };

}

#endif
