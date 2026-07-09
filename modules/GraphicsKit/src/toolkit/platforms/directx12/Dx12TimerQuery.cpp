/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx12TimerQuery.hpp"

#if GFX_API_DIRECTX12

#include "Dx12Context.hpp"

namespace toolkit
{
    DX12TimerQuery::DX12TimerQuery(DX12Device *device)
        : DX12Resource<GfxTimerQuery>(device, TkString::Empty)
    {
    }

    void DX12TimerQuery::onReleaseData()
    {
        m_hasResult = false;
        m_endCalled = false;
        m_timeDelta = 0.0f;
    }

    void DX12TimerQuery::begin()
    {
        const auto context = m_device->getMainContextDX12();
        auto &heap = m_device->timestampQueryHeap;
        heap.endQuery(context, m_begin);

        m_hasResult = false;
        m_endCalled = false;
    }

    void DX12TimerQuery::end()
    {
        if (m_endCalled)
            return;

        const auto context = m_device->getMainContextDX12();
        auto &heap = m_device->timestampQueryHeap;
        heap.endQuery(context, m_end);

        const auto queue = m_device->getCommandQueue()->getCommandQueue();
        VALIDATE_DIRECTX_CALL(queue->GetTimestampFrequency(&m_gpuFrequency));

        m_endCalled = true;
    }

    bool DX12TimerQuery::hasResult()
    {
        if (!m_endCalled)
            return false;
        if (m_hasResult)
            return true;

        auto &heap = m_device->timestampQueryHeap;
        return heap.isReady(m_end) && heap.isReady(m_begin);
    }

    float DX12TimerQuery::getResult()
    {
        if (m_hasResult)
        {
            return m_timeDelta;
        }

        const u64 timeBegin = *(u64 *)m_device->timestampQueryHeap.resolveQuery(m_begin);
        const u64 timeEnd = *(u64 *)m_device->timestampQueryHeap.resolveQuery(m_end);

        // Calculate event duration in milliseconds
        if (timeEnd > timeBegin)
        {
            const u64 delta = timeEnd - timeBegin;
            const double frequency = double(m_gpuFrequency);
            m_timeDelta = static_cast<float>((delta / frequency) * 1000.0);
        }
        else
        {
            m_timeDelta = 0.0f;
        }

        m_hasResult = true;
        return m_timeDelta;
    }

}

#endif
