/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/directx12/common.hpp>

#if GFX_API_DIRECTX12

#include "toolkit/threading/Sync.hpp"
#include "Dx12CmdAllocatorPool.hpp"

namespace toolkit
{
    class DX12Device;
    class DX12Context;
    class DX12CmdQueue;

    // Wraps a fence object and provides functionality for common operations for GPU/CPU operations synchronization.
    class FenceDX12
    {
    public:
        FenceDX12(DX12Device *device);

    public:
        TK_FORCE_INLINE u64 getCurrentValue() const
        {
            return m_currentValue;
        }
        TK_FORCE_INLINE u64 getLastSignaledValue() const
        {
            return m_lastSignaledValue;
        }
        TK_FORCE_INLINE u64 getLastCompletedValue() const
        {
            return m_lastCompletedValue;
        }

    public:
        bool init();
        void release();
        u64 signal(DX12CmdQueue *queue);
        void waitGPU(DX12CmdQueue *queue, u64 value);
        void waitCPU(u64 value);
        bool isFenceComplete(u64 value);

    private:
        u64 m_currentValue;
        u64 m_lastSignaledValue;
        u64 m_lastCompletedValue;
        HANDLE m_event;
        ID3D12Fence *m_fence;
        DX12Device *m_device;
        TkMutex m_locker;
    };

    // GPU commands execution sync point for DirectX 12.
    struct DX12SyncPoint
    {
    public:
        DX12SyncPoint()
            : fence(nullptr), value(0)
        {
        }

        DX12SyncPoint(FenceDX12 *fence, u64 value)
            : fence(fence), value(value)
        {
        }

        DX12SyncPoint(const DX12SyncPoint &other)
            : fence(other.fence), value(other.value)
        {
        }

        DX12SyncPoint &operator=(const DX12SyncPoint &other)
        {
            fence = other.fence;
            value = other.value;
            return *this;
        }

    public:
        bool operator!() const
        {
            return fence == nullptr;
        }

        bool isValid() const
        {
            return fence != nullptr;
        }

        bool isOpen() const
        {
            return value == fence->getCurrentValue();
        }

        bool isComplete() const
        {
            return fence->isFenceComplete(value);
        }

        void WaitForCompletion() const
        {
            fence->waitCPU(value);
        }

    public:
        FenceDX12 *fence;
        u64 value;
    };

    class DX12CmdQueue
    {
        friend DX12Device;
        friend DX12Context;

    public:
        DX12CmdQueue(DX12Device *device, D3D12_COMMAND_LIST_TYPE type);
        ~DX12CmdQueue();

    public:
        TK_FORCE_INLINE bool isReady() const
        {
            return m_commandQueue != nullptr;
        }

        TK_FORCE_INLINE ID3D12CommandQueue *getCommandQueue() const
        {
            return m_commandQueue;
        }

        TK_FORCE_INLINE DX12CmdAllocatorPool &getAllocatorPool()
        {
            return m_allocatorPool;
        }

        TK_FORCE_INLINE DX12SyncPoint getSyncPoint()
        {
            return DX12SyncPoint(&m_fence, m_fence.getCurrentValue());
        }

    public:
        // Init resources
        bool init();
        // Cleanup all stuff
        void release();

    public:
        // Stalls the execution on current thread to wait for the GPU to step over given fence value.
        void waitForFence(u64 fenceValue);
        // Stalls the execution on current thread to wait for the GPU to finish it's job.
        void waitForGPU();

    public:
        // Execute a command list.
        u64 executeCommandList(ID3D12CommandList *list);
        // Request new clean allocator to use.
        ID3D12CommandAllocator *requestAllocator();
        // Discard used allocator.
        void discardAllocator(u64 fenceValueForReset, ID3D12CommandAllocator *allocator);

    private:
        DX12Device *m_device;
        ID3D12CommandQueue *m_commandQueue;
        const D3D12_COMMAND_LIST_TYPE m_type;
        DX12CmdAllocatorPool m_allocatorPool;
        FenceDX12 m_fence;
    };

}

#endif
