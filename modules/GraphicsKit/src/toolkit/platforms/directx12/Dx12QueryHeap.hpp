/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX12

#include "toolkit/collection/Array.hpp"
#include "Dx12CmdQueue.hpp"

namespace toolkit
{
    class DX12TimerQuery;
    class DX12Device;
    class DX12Context;
    class GfxBuffer;

    struct _QueryBatch
    {
    public:
        // The synchronization point when query has been submitted to be executed.
        DX12SyncPoint sync;
        // The first element in the batch (inclusive).
        i32 start = 0;
        // The amount of elements added to this batch.
        i32 count = 0;
        // Is the batch still open for more begin/end queries.
        bool open = false;

    public:
        // Clear this batch.
        inline void clear()
        {
            sync = DX12SyncPoint();
            start = 0;
            count = 0;
            open = false;
        }

        // Check if this query batch contains a given element contains the element.
        bool containsElement(i32 elementIndex) const
        {
            return elementIndex >= start && elementIndex < start + count;
        }
    };

    // GPU queries heap for DirectX 12 backend.
    class DX12QueryHeap
    {
    public:
        // The query element handle.
        typedef i32 ElementHandle;

    private:
        DX12Device *m_device;
        ID3D12QueryHeap *m_queryHeap;
        ID3D12Resource *m_resultBuffer;
        D3D12_QUERY_TYPE m_queryType;
        D3D12_QUERY_HEAP_TYPE m_queryHeapType;
        i32 m_currentIndex;
        i32 m_resultSize;
        i32 m_queryHeapCount;
        _QueryBatch m_currentBatch;
        TkArray<_QueryBatch> m_batches;
        TkArray<byte> m_resultData;

    public:
        DX12QueryHeap(DX12Device *device, const D3D12_QUERY_HEAP_TYPE &queryHeapType, i32 queryHeapCount);

    public:
        // Init this instance.
        bool init();
        // Destroy this instance.
        void destroy();

    public:
        // Get the query heap capacity.
        TK_FORCE_INLINE i32 getQueryHeapCount() const
        {
            return m_queryHeapCount;
        }
        // Get the size of the result value (in bytes).
        TK_FORCE_INLINE i32 getResultSize() const
        {
            return m_resultSize;
        }
        // Get the result buffer (CPU readable via Map/Unmap).
        TK_FORCE_INLINE ID3D12Resource *getResultBuffer() const
        {
            return m_resultBuffer;
        }

    public:
        // Stops tracking the current batch of begin/end query calls that will be resolved together. This implicitly starts a new batch.
        void endQueryBatchAndResolveQueryData(DX12Context *context);

        // Allocates the query heap element.
        void allocQuery(DX12Context *context, ElementHandle &handle);
        // Calls BeginQuery on command list for the given query heap slot.
        void beginQuery(DX12Context *context, ElementHandle &handle);
        // Calls EndQuery on command list for the given query heap slot.
        void endQuery(DX12Context *context, ElementHandle &handle);

        // Determines whether the specified query handle is ready to read data (command list has been executed by the GPU).
        bool isReady(ElementHandle &handle);
        // Resolves the query (or skips if already resolved).
        void *resolveQuery(ElementHandle &handle);

    private:
        // Starts tracking a new batch of begin/end query calls that will be resolved together
        void startQueryBatch();
    };

}

#endif
