/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "common.hpp"

#if GFX_API_DIRECTX12

#include "toolkit/graphics/GfxTimerQuery.hpp"
#include "DX12Device.hpp"

namespace toolkit
{
    // GPU timer query object for DirectX 12 backend.
    class DX12TimerQuery : public DX12Resource<GfxTimerQuery>
    {
    private:
        bool m_hasResult = false;
        bool m_endCalled = false;
        f32 m_timeDelta = 0.0f;
        u64 m_gpuFrequency = 0;
        DX12QueryHeap::ElementHandle m_begin;
        DX12QueryHeap::ElementHandle m_end;

    public:
        DX12TimerQuery(DX12Device *device);

    public:
        void begin() override;
        void end() override;
        bool hasResult() override;
        f32 getResult() override;

    protected:
        void onReleaseData() override;
    };

}

#endif
