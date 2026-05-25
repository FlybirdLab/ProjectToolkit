/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx12CmdAllocatorPool.hpp"

#if GFX_API_DIRECTX12

#include "Dx12Device.hpp"

namespace toolkit
{

    DX12CmdAllocatorPool::DX12CmdAllocatorPool(DX12Device *device, D3D12_COMMAND_LIST_TYPE type)
        : m_type(type), m_device(device)
    {
    }

    DX12CmdAllocatorPool::~DX12CmdAllocatorPool()
    {
        release();
    }

    ID3D12CommandAllocator *DX12CmdAllocatorPool::requestAllocator(u64 completedFenceValue)
    {
        TkMutexGuard lock(m_locker);

        ID3D12CommandAllocator *allocator = nullptr;

        if (m_ready.hasItems())
        {
            PoolPair &firstPair = m_ready.first();

            if (firstPair.first <= completedFenceValue)
            {
                allocator = firstPair.second;
                VALIDATE_DIRECTX_CALL(allocator->Reset());
                m_ready.removeAtKeepOrder(0);
            }
        }

        // If no allocators were ready to be reused, create a new one
        if (allocator == nullptr)
        {
            VALIDATE_DIRECTX_CALL(m_device->getDevice()->CreateCommandAllocator(m_type, IID_PPV_ARGS(&allocator)));
#if GFX_ENABLE_RESOURCE_NAMING
            wchar name[32];
            swprintf(name, 32, L"CommandAllocator %u", m_pool.count());
            allocator->SetName(name);
#endif
            m_pool.add(allocator);
        }

        return allocator;
    }

    void DX12CmdAllocatorPool::discardAllocator(u64 fenceValue, ID3D12CommandAllocator *allocator)
    {
        TkMutexGuard lock(m_locker);

        // That fence value indicates we are free to reset the allocator
        m_ready.add(PoolPair(fenceValue, allocator));
    }

    void DX12CmdAllocatorPool::release()
    {
        for (i32 i = 0; i < m_pool.count(); i++)
        {
            DX_DEL(m_pool[i], 0);
        }

        m_pool.clear();
    }

}

#endif
