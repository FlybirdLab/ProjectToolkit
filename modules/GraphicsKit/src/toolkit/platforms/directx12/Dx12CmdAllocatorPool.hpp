/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/directx12/common.hpp>

#if GFX_API_DIRECTX12

#include "toolkit/types/Pair.hpp"
#include "toolkit/collection/Array.hpp"
#include "toolkit/threading/Sync.hpp"

namespace toolkit
{
    class DX12Device;

    class DX12CmdAllocatorPool
    {
    public:
        DX12CmdAllocatorPool(DX12Device *device, D3D12_COMMAND_LIST_TYPE type);
        ~DX12CmdAllocatorPool();

    public:
        TK_FORCE_INLINE u32 size() const
        {
            return m_pool.count();
        }

        ID3D12CommandAllocator *requestAllocator(u64 completedFenceValue);

        void discardAllocator(u64 fenceValue, ID3D12CommandAllocator *allocator);
        void release();

    private:
        typedef TkPair<u64, ID3D12CommandAllocator *> PoolPair;

        const D3D12_COMMAND_LIST_TYPE m_type;
        DX12Device *m_device;
        TkArray<ID3D12CommandAllocator *> m_pool;
        TkArray<PoolPair> m_ready;
        TkMutex m_locker;
    };

}

#endif
