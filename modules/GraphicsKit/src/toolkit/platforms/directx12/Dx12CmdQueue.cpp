/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx12CmdQueue.hpp"

#if GFX_API_DIRECTX12

#include "DX12Device.hpp"

namespace toolkit
{

    FenceDX12::FenceDX12(DX12Device *device)
        : m_currentValue(1), m_lastSignaledValue(0), m_lastCompletedValue(0), m_device(device)
    {
    }

    bool FenceDX12::init()
    {
        LOG_DIRECTX_RESULT(m_device->getDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
#if GFX_ENABLE_RESOURCE_NAMING
        m_fence->SetName(L"Fence");
#endif

        m_event = CreateEvent(nullptr, false, false, nullptr);
        TK_ASSERT(m_event != INVALID_HANDLE_VALUE);
        return true;
    }

    void FenceDX12::release()
    {
        CloseHandle(m_event);
        m_event = nullptr;

        m_fence->Release();
        m_fence = nullptr;
    }

    u64 FenceDX12::signal(DX12CmdQueue *queue)
    {
        TkMutexGuard lock(m_locker);
        TK_ASSERT(m_lastSignaledValue != m_currentValue);

        // Insert signal into command queue
        LOG_DIRECTX_RESULT(queue->getCommandQueue()->Signal(m_fence, m_currentValue));

        // Update state
        m_lastSignaledValue = m_currentValue;
        m_lastCompletedValue = m_fence->GetCompletedValue();

        // Increment the current value
        m_currentValue++;

        return m_lastSignaledValue;
    }

    void FenceDX12::waitGPU(DX12CmdQueue *queue, u64 value)
    {
        // Insert wait into command queue
        LOG_DIRECTX_RESULT(queue->getCommandQueue()->Wait(m_fence, value));
    }

    void FenceDX12::waitCPU(u64 value)
    {
        if (isFenceComplete(value))
            return;

        TkMutexGuard lock(m_locker);

        m_fence->SetEventOnCompletion(value, m_event);
        WaitForSingleObject(m_event, INFINITE);

        m_lastCompletedValue = m_fence->GetCompletedValue();
    }

    bool FenceDX12::isFenceComplete(u64 value)
    {
        TK_ASSERT(value <= m_currentValue);

        if (value > m_lastCompletedValue)
        {
            m_lastCompletedValue = m_fence->GetCompletedValue();
        }

        return value <= m_lastCompletedValue;
    }

    DX12CmdQueue::DX12CmdQueue(DX12Device *device, D3D12_COMMAND_LIST_TYPE type)
        : m_device(device), m_commandQueue(nullptr), m_type(type), m_allocatorPool(device, type), m_fence(device)
    {
    }

    DX12CmdQueue::~DX12CmdQueue()
    {
        release();
    }

    bool DX12CmdQueue::init()
    {
        TK_ASSERT(m_device != nullptr);
        TK_ASSERT(!isReady());
        TK_ASSERT(m_allocatorPool.size() == 0);

        D3D12_COMMAND_QUEUE_DESC desc;
        desc.Type = m_type;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;
        HRESULT result = m_device->getDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue));
        LOG_DIRECTX_RESULT_WITH_RETURN(result, false);
#if GFX_ENABLE_RESOURCE_NAMING
        m_commandQueue->SetName(L"DX12CmdQueue::CommandQueue");
#endif

        if (!m_fence.init())
            return false;

        TK_ASSERT(isReady());
        return true;
    }

    void DX12CmdQueue::release()
    {
        if (m_commandQueue == nullptr)
            return;

        m_allocatorPool.release();
        m_fence.release();

        m_commandQueue->Release();
        m_commandQueue = nullptr;
    }

    void DX12CmdQueue::waitForFence(u64 fenceValue)
    {
        m_fence.waitCPU(fenceValue);
    }

    void DX12CmdQueue::waitForGPU()
    {
        const u64 value = m_fence.signal(this);
        m_fence.waitCPU(value);
    }

    u64 DX12CmdQueue::executeCommandList(ID3D12CommandList *list)
    {
        VALIDATE_DIRECTX_CALL((static_cast<ID3D12GraphicsCommandList *>(list))->Close());

        m_commandQueue->ExecuteCommandLists(1, &list);

        return m_fence.signal(this);
    }

    ID3D12CommandAllocator *DX12CmdQueue::requestAllocator()
    {
        const u64 completedFence = m_fence.getLastCompletedValue();
        return m_allocatorPool.requestAllocator(completedFence);
    }

    void DX12CmdQueue::discardAllocator(u64 fenceValue, ID3D12CommandAllocator *allocator)
    {
        m_allocatorPool.discardAllocator(fenceValue, allocator);
    }

}

#endif
