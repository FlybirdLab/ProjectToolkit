/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx12QueryHeap.hpp"

#if GFX_API_DIRECTX12

#include "Dx12Device.hpp"
#include "Dx12Context.hpp"

namespace toolkit
{

    DX12QueryHeap::DX12QueryHeap(DX12Device *device, const D3D12_QUERY_HEAP_TYPE &queryHeapType, i32 queryHeapCount)
        : m_device(device), m_queryHeap(nullptr), m_resultBuffer(nullptr), m_queryHeapType(queryHeapType), m_currentIndex(0), m_queryHeapCount(queryHeapCount)
    {
        if (queryHeapType == D3D12_QUERY_HEAP_TYPE_OCCLUSION)
        {
            m_resultSize = sizeof(u64);
            m_queryType = D3D12_QUERY_TYPE_OCCLUSION;
        }
        else if (queryHeapType == D3D12_QUERY_HEAP_TYPE_TIMESTAMP)
        {
            m_resultSize = sizeof(u64);
            m_queryType = D3D12_QUERY_TYPE_TIMESTAMP;
        }
        else
        {
            TK_ASSERT(false && "Not support D3D12 query heap type.");
        }
    }

    bool DX12QueryHeap::init()
    {
        m_resultData.resize(m_resultSize * m_queryHeapCount);

        // Create the query heap
        D3D12_QUERY_HEAP_DESC heapDesc;
        heapDesc.Type = m_queryHeapType;
        heapDesc.Count = m_queryHeapCount;
        heapDesc.NodeMask = 0;
        HRESULT result = m_device->getDevice()->CreateQueryHeap(&heapDesc, IID_PPV_ARGS(&m_queryHeap));
        LOG_DIRECTX_RESULT_WITH_RETURN(result, false);
        DX_SET_DEBUG_NAME(m_queryHeap, L"Query Heap");

        // Create the result buffer
        D3D12_HEAP_PROPERTIES heapProperties;
        heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProperties.CreationNodeMask = 1;
        heapProperties.VisibleNodeMask = 1;
        D3D12_RESOURCE_DESC resourceDesc;
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Alignment = 0;
        resourceDesc.Width = m_resultData.count();
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        result = m_device->getDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_resultBuffer));
        LOG_DIRECTX_RESULT_WITH_RETURN(result, false);
        DX_SET_DEBUG_NAME(m_resultBuffer, "Query Heap Result Buffer");

        // Start out with an open query batch
        m_currentBatch.open = false;
        startQueryBatch();

        return true;
    }

    void DX12QueryHeap::destroy()
    {
        if (m_resultBuffer)
        {
            m_resultBuffer->Release();
            m_resultBuffer = nullptr;
        }
        if (m_queryHeap)
        {
            m_queryHeap->Release();
            m_queryHeap = nullptr;
        }
        m_currentBatch.clear();
        m_resultData.setCapacity(0);
    }

    void DX12QueryHeap::endQueryBatchAndResolveQueryData(DX12Context *context)
    {
        TK_ASSERT(m_currentBatch.open);
        if (m_currentBatch.count == 0)
            return;

        // Close the current batch
        m_currentBatch.open = false;

        // Resolve the batch
        const i32 offset = m_currentBatch.start * m_resultSize;
        context->getCommandList()->ResolveQueryData(m_queryHeap, m_queryType, m_currentBatch.start, m_currentBatch.count, m_resultBuffer, offset);
        m_currentBatch.sync = m_device->getCommandQueue()->getSyncPoint();

        // Begin a new query batch
        m_batches.add(m_currentBatch);
        startQueryBatch();
    }

    void DX12QueryHeap::allocQuery(DX12Context *context, ElementHandle &handle)
    {
        TK_ASSERT(m_currentBatch.open);

        // Check if need to start from the buffer head
        if (m_currentIndex >= getQueryHeapCount())
        {
            // We're in the middle of a batch, but we're at the end of the heap so split the batch in two
            endQueryBatchAndResolveQueryData(context);
        }

        // Allocate element into the current batch
        handle = m_currentIndex++;
        m_currentBatch.count++;
    }

    void DX12QueryHeap::beginQuery(DX12Context *context, ElementHandle &handle)
    {
        allocQuery(context, handle);

        context->getCommandList()->BeginQuery(m_queryHeap, m_queryType, handle);
    }

    void DX12QueryHeap::endQuery(DX12Context *context, ElementHandle &handle)
    {
        allocQuery(context, handle);

        context->getCommandList()->EndQuery(m_queryHeap, m_queryType, handle);
    }

    bool DX12QueryHeap::isReady(ElementHandle &handle)
    {
        // Current batch is not ready (not ended)
        if (m_currentBatch.containsElement(handle))
            return false;

        for (i32 i = 0; i < m_batches.count(); i++)
        {
            auto &batch = m_batches[i];
            if (batch.containsElement(handle))
            {
                TK_ASSERT(batch.sync.isValid());
                return batch.sync.isComplete();
            }
        }

        return true;
    }

    void *DX12QueryHeap::resolveQuery(ElementHandle &handle)
    {
        // Prevent queries from the current batch
        TK_ASSERT(!m_currentBatch.containsElement(handle));

        // Find the batch that contains this element to resolve it
        for (i32 i = 0; i < m_batches.count(); i++)
        {
            auto &batch = m_batches[i];
            if (batch.containsElement(handle))
            {
                TK_ASSERT(batch.sync.isValid());

                // Ensure that end point has been already executed
                if (!batch.sync.isComplete())
                {
                    if (batch.sync.isOpen())
                    {
                        // The query is on a command list that hasn't been submitted yet
                        LogWarn("Stalling the rendering and flushing GPU commands to wait for a query that hasn't been submitted to the GPU yet.");
                        m_device->waitForGPU();
                    }

                    batch.sync.WaitForCompletion();
                }

                // Map the query values readback buffer
                D3D12_RANGE range;
                range.Begin = batch.start * m_resultSize;
                range.End = range.Begin + batch.count * m_resultSize;
                void *mapped = nullptr;
                VALIDATE_DIRECTX_CALL(m_resultBuffer->Map(0, &range, &mapped));

                // Copy the results data
                TkMemory::memoryCopy(m_resultData.get() + range.Begin, (byte *)mapped + range.Begin, batch.count * m_resultSize);

                // Unmap with an empty range to indicate nothing was written by the CPU
                m_resultBuffer->Unmap(0, nullptr);

                // All elements got its results so we can remove this batch
                m_batches.removeAt(i);

                break;
            }
        }

        return m_resultData.get() + handle * m_resultSize;
    }

    void DX12QueryHeap::startQueryBatch()
    {
        TK_ASSERT(!m_currentBatch.open);

        // Clear the current batch
        m_currentBatch.clear();

        // Loop active index on overflow
        if (m_currentIndex >= getQueryHeapCount())
        {
            m_currentIndex = 0;
        }

        // Start a new batch
        m_currentBatch.start = m_currentIndex;
        m_currentBatch.open = true;
    }

}

#endif
