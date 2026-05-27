/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx12UploadBuffer.hpp"

#if GFX_API_DIRECTX12

#include "DX12Texture.hpp"
#include "Dx12Context.hpp"
#include "toolkit/platforms/directx/DxUtils.hpp"

namespace toolkit
{

    DX12UploadBuffer::DX12UploadBuffer(DX12Device *device)
        : m_device(device), m_currentPage(nullptr), m_currentOffset(0), m_currentGeneration(0)
    {
    }

    DX12UploadBuffer::~DX12UploadBuffer()
    {
        m_freePages.add(m_usedPages);
        for (auto page : m_freePages)
        {
            page->releaseData();
            TkMemory::deletePtr(page);
        }
    }

    DynamicAllocation DX12UploadBuffer::allocate(u64 size, u64 align)
    {
        const u64 alignmentMask = align - 1;
        TK_ASSERT((alignmentMask & align) == 0);

        // Check if use default or bigger page
        const bool useDefaultSize = size <= DX12_DEFAULT_UPLOAD_PAGE_SIZE;
        const u64 pageSize = useDefaultSize ? DX12_DEFAULT_UPLOAD_PAGE_SIZE : size;
        const u64 alignedSize = TkMath::alignUpWithMask(size, alignmentMask);

        // Align the allocation
        m_currentOffset = TkMath::alignUpWithMask(m_currentOffset, alignmentMask);

        // Check if there is enough space for that chunk of the data in the current page
        if (m_currentPage && m_currentOffset + alignedSize > m_currentPage->size)
        {
            m_currentPage = nullptr;
        }

        // Check if need to get new page
        if (m_currentPage == nullptr)
        {
            m_currentPage = requestPage(pageSize);
            m_currentOffset = 0;
        }

        // Mark page as used in this generation
        m_currentPage->lastGen = m_currentGeneration;

        // Create allocation result
        const DynamicAllocation result(static_cast<byte *>(m_currentPage->addressCPU) + m_currentOffset, m_currentOffset, size, m_currentPage->addressGPU + m_currentOffset, m_currentPage, m_currentGeneration);

        // Move in the page
        m_currentOffset += size;

        TK_ASSERT(m_currentPage->getResource());
        return result;
    }

    bool DX12UploadBuffer::uploadBuffer(DX12Context *context, ID3D12Resource *buffer, u32 bufferOffset, const void *data, u64 size)
    {
        // Allocate data
        const DynamicAllocation allocation = allocate(size, 4);
        if (allocation.isInvalid())
            return false;

        // Copy data
        TkMemory::memoryCopy(allocation.addressCPU, data, static_cast<size_t>(size));

        // Copy buffer region
        context->getCommandList()->CopyBufferRegion(buffer, bufferOffset, allocation.page->getResource(), allocation.offset, size);

        return true;
    }

    bool DX12UploadBuffer::uploadTexture(DX12Context *context, ID3D12Resource *texture, const void *srcData, u32 srcRowPitch, u32 srcSlicePitch, i32 mipIndex, i32 arrayIndex)
    {
        D3D12_RESOURCE_DESC resourceDesc = texture->GetDesc();
        const UINT subresourceIndex = DxUtils::calcSubresourceIndex(mipIndex, arrayIndex, resourceDesc.MipLevels);
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
        u32 numRows;
        u64 rowPitchAligned, mipSizeAligned;
        m_device->getDevice()->GetCopyableFootprints(&resourceDesc, subresourceIndex, 1, 0, &footprint, &numRows, &rowPitchAligned, &mipSizeAligned);
        rowPitchAligned = footprint.Footprint.RowPitch;
        mipSizeAligned = rowPitchAligned * footprint.Footprint.Height;
        const u32 numSlices = resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? TkMath::max(1, resourceDesc.DepthOrArraySize >> mipIndex) : 1;
        const u64 sliceSizeAligned = numSlices * mipSizeAligned;

        // Allocate data
        const DynamicAllocation allocation = allocate(sliceSizeAligned, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
        if (allocation.size != sliceSizeAligned)
            return false;

        byte *ptr = (byte *)srcData;
        TK_ASSERT(srcSlicePitch <= sliceSizeAligned);
        if (srcSlicePitch == sliceSizeAligned)
        {
            // Copy data at once
            TkMemory::memoryCopy(allocation.addressCPU, ptr, srcSlicePitch);
        }
        else
        {
            // Copy data per-row
            byte *dst = static_cast<byte *>(allocation.addressCPU);
            TK_ASSERT(srcRowPitch <= rowPitchAligned);
            const u32 numCopies = numSlices * numRows;
            for (u32 i = 0; i < numCopies; i++)
            {
                TkMemory::memoryCopy(dst, ptr, srcRowPitch);
                dst += rowPitchAligned;
                ptr += srcRowPitch;
            }
        }

        // Destination texture copy location description
        D3D12_TEXTURE_COPY_LOCATION dstLocation;
        dstLocation.pResource = texture;
        dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLocation.SubresourceIndex = subresourceIndex;

        // Source buffer copy location description
        D3D12_TEXTURE_COPY_LOCATION srcLocation;
        srcLocation.pResource = allocation.page->getResource();
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        srcLocation.PlacedFootprint.Offset = allocation.offset;
        srcLocation.PlacedFootprint.Footprint = footprint.Footprint;

        // Copy texture region
        context->getCommandList()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

        return true;
    }

    void DX12UploadBuffer::beginGeneration(u64 generation)
    {
        // Upload allocations recorded in an earlier frame may still be read
        // by the GPU. Reuse a page only after it has passed through the
        // generation timeout below; never retain the prior frame's cursor.
        m_currentPage = nullptr;
        m_currentOffset = 0;

        // Restore ready pages to be reused
        for (i32 i = 0; m_usedPages.hasItems() && i < m_usedPages.count(); i++)
        {
            auto page = m_usedPages[i];
            if (page->lastGen + DX12_UPLOAD_PAGE_GEN_TIMEOUT < generation)
            {
                m_usedPages.removeAt(i);
                i--;
                m_freePages.add(page);
            }
        }

        // Remove old pages
        for (i32 i = m_freePages.count() - 1; i >= 0 && m_freePages.hasItems(); i--)
        {
            auto page = m_freePages[i];
            if (page->lastGen + DX12_UPLOAD_PAGE_GEN_TIMEOUT + DX12_UPLOAD_PAGE_NOT_USED_FRAME_TIMEOUT < generation)
            {
                m_freePages.removeAt(i);
                i--;
                page->releaseData();
                TkMemory::deletePtr(page);
            }
        }

        // Set new generation
        m_currentGeneration = generation;
    }

    DX12UploadBufferPage *DX12UploadBuffer::requestPage(u64 size)
    {
        // Try to find valid page
        i32 freePageIndex = -1;
        for (i32 i = 0; i < m_freePages.count(); i++)
        {
            if (m_freePages[i]->size == size)
            {
                freePageIndex = i;
                break;
            }
        }

        // Check if create a new page
        DX12UploadBufferPage *page;
        if (freePageIndex == -1)
        {
            // Get a new page to use
            page = TkMemory::newObject<DX12UploadBufferPage>(TkMemory::getDefault(), m_device, size);
        }
        else
        {
            // Remove from free pages
            page = m_freePages[freePageIndex];
            m_freePages.removeAt(freePageIndex);
        }

        // Mark page as used
        m_usedPages.add(page);

        return page;
    }

    DX12UploadBufferPage::DX12UploadBufferPage(DX12Device *device, u64 size)
        : DX12Resource(device, "Upload Buffer Page"), lastGen(0), addressCPU(nullptr), addressGPU(0), size(size)
    {
        // Create page buffer
        D3D12_HEAP_PROPERTIES heapProperties;
        heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProperties.CreationNodeMask = 1;
        heapProperties.VisibleNodeMask = 1;
        D3D12_RESOURCE_DESC resourceDesc;
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Alignment = 0;
        resourceDesc.Width = size;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        ID3D12Resource *resource;
        VALIDATE_DIRECTX_CALL(m_device->getDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource)));

        // Set state
        initResource(resource, D3D12_RESOURCE_STATE_GENERIC_READ, 1);
        DX_SET_DEBUG_NAME(m_resource, DX12Resource::getName());
        m_memoryUsage = size;
        addressGPU = m_resource->GetGPUVirtualAddress();

        // Map buffer
        VALIDATE_DIRECTX_CALL(m_resource->Map(0, nullptr, &addressCPU));
    }

    void DX12UploadBufferPage::onReleaseData()
    {
        // Unmap
        if (m_resource && addressCPU)
        {
            m_resource->Unmap(0, nullptr);
        }
        addressGPU = 0;
        addressCPU = nullptr;

        // Release
        releaseResource();
    }

}

#endif
