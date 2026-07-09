/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/directx11/common.hpp>

#if GFX_API_DIRECTX11

#include "Dx11Device.hpp"
#include "toolkit/platforms/headless/NullTimeQuery.hpp"

namespace toolkit
{
    // GPU timer query object for DirectX 11 backend.
    class DX11TimerQuery : public DX11Resource<GfxTimerQuery>
    {
    public:
        DX11TimerQuery(DX11Device *device);
        ~DX11TimerQuery();

    public:
        ID3D11Resource *getResource() final override;

        void begin() override;
        void end() override;
        bool hasResult() override;
        f32 getResult() override;

    protected:
        void onReleaseData() override;

    private:
        bool m_finalized = false;
        bool m_endCalled = false;
        f32 m_timeDelta = 0.0f;

        ID3D11Query *m_beginQuery = nullptr;
        ID3D11Query *m_endQuery = nullptr;
        ID3D11Query *m_disjointQuery = nullptr;
    };

}

#endif
