/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX12

#include "DX12Device.hpp"
#include "DX12ResOwner.hpp"

#define DX12_DEFAULT_UPLOAD_PAGE_SIZE (4 * 1014 * 1024) // 4 MB
// Upload buffer generations timeout to dispose
#define DX12_UPLOAD_PAGE_GEN_TIMEOUT DX12_BACK_BUFFER_COUNT
// Upload buffer pages that are not used for a few frames are disposed
#define DX12_UPLOAD_PAGE_NOT_USED_FRAME_TIMEOUT 60

namespace toolkit
{
    class DX12Texture;

    // Single page for the upload buffer
    class DX12UploadBufferPage : public DX12Resource<GfxResource>, public DX12ResourceOwner
    {
    public:
        DX12UploadBufferPage(DX12Device *device, u64 size);

    public:
        // Last generation that has been using that page
        u64 lastGen;
        // CPU memory address of the page
        void *addressCPU;
        // GPU memory address of the page
        D3D12_GPU_VIRTUAL_ADDRESS addressGPU;
        // Page size in bytes
        u64 size;

    public:
        GfxResType getType() const final override
        {
            return GfxResType::kBuffer;
        }
        GfxResource *asResource() const override
        {
            return (GfxResource *)this;
        }

    protected:
        void onReleaseData() final override;
    };

    // Upload buffer allocatio
    struct DynamicAllocation
    {
    public:
        // Allocation offset in bytes (from the start of the heap buffer).
        u64 offset;
        // Allocation size in bytes
        u64 size;
        // CPU memory address of the allocation start.
        void *addressCPU;
        // GPU virtual memory address of the allocation start.
        D3D12_GPU_VIRTUAL_ADDRESS addressGPU;
        // Upload buffer page that owns that allocation
        DX12UploadBufferPage *page;
        // Generation number of that allocation (generally allocation is invalid after one or two generations)
        u64 generation;

    public:
        DynamicAllocation()
            : addressCPU(nullptr), offset(0), size(0), addressGPU(0), page(nullptr), generation(0)
        {
        }

        DynamicAllocation(void *address, u64 offset, u64 size, D3D12_GPU_VIRTUAL_ADDRESS gpuAddress, DX12UploadBufferPage *page, u64 generation)
            : addressCPU(address), offset(offset), size(size), addressGPU(gpuAddress), page(page), generation(generation)
        {
        }

        // Returns true if allocation is invalid.
        bool isInvalid() const
        {
            return addressCPU == nullptr || size == 0 || page == nullptr;
        }
    };

    // Uploading data to GPU buffer utilit
    class DX12UploadBuffer
    {
    public:
        DX12UploadBuffer(DX12Device *device);
        ~DX12UploadBuffer();

    public:
        // Get the current generation number.
        TK_FORCE_INLINE u64 getCurrentGeneration() const
        {
            return m_currentGeneration;
        }

    public:
        // Allocate memory for custom data in the buffer.
        DynamicAllocation allocate(u64 size, u64 align);
        // Upload data to the buffer.
        bool uploadBuffer(DX12Context *context, ID3D12Resource *buffer, u32 bufferOffset, const void *data, u64 size);
        // Upload data to the texture.
        bool uploadTexture(DX12Context *context, ID3D12Resource *texture, const void *srcData, u32 srcRowPitch, u32 srcSlicePitch, i32 mipIndex, i32 arrayIndex);

    public:
        // Begin new generation.
        void beginGeneration(u64 generation);

    private:
        DX12UploadBufferPage *requestPage(u64 size);

    private:
        DX12Device *m_device;
        DX12UploadBufferPage *m_currentPage;
        u64 m_currentOffset;
        u64 m_currentGeneration;

        TkStaticArray<DX12UploadBufferPage *, 64> m_freePages;
        TkStaticArray<DX12UploadBufferPage *, 64> m_usedPages;
    };

}

#endif
