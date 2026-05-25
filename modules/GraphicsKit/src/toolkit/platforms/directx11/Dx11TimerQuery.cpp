/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Dx11TimerQuery.hpp"

#if GFX_API_DIRECTX11

#include "toolkit/debug/Logger.hpp"

namespace toolkit
{

    DX11TimerQuery::DX11TimerQuery(DX11Device *device)
        : DX11Resource<GfxTimerQuery>(device, TkStringView::Empty)
    {
        D3D11_QUERY_DESC queryDesc;
        queryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        queryDesc.MiscFlags = 0;

        HRESULT hr = device->getDevice()->CreateQuery(&queryDesc, &m_disjointQuery);
        if (hr != S_OK)
        {
            LogError("Failed to create a timer query.");
        }

        queryDesc.Query = D3D11_QUERY_TIMESTAMP;

        hr = device->getDevice()->CreateQuery(&queryDesc, &m_beginQuery);
        if (hr != S_OK)
        {
            LogError("Failed to create a timer query.");
        }

        hr = device->getDevice()->CreateQuery(&queryDesc, &m_endQuery);
        if (hr != S_OK)
        {
            LogError("Failed to create a timer query.");
        }

        // Set non-zero mem usage (fake)
        m_memoryUsage = sizeof(D3D11_QUERY_DESC) * 3;
    }

    DX11TimerQuery::~DX11TimerQuery()
    {
        if (m_beginQuery)
            m_beginQuery->Release();
        if (m_endQuery)
            m_endQuery->Release();
        if (m_disjointQuery)
            m_disjointQuery->Release();
    }

    void DX11TimerQuery::onReleaseData()
    {
        DX_DEL(m_beginQuery, 0);
        DX_DEL(m_endQuery, 0);
        DX_DEL(m_disjointQuery, 0);
    }

    ID3D11Resource *DX11TimerQuery::getResource()
    {
        return nullptr;
    }

    void DX11TimerQuery::begin()
    {
        auto context = m_device->getIM();
        context->Begin(m_disjointQuery);
        context->End(m_beginQuery);

        m_endCalled = false;
    }

    void DX11TimerQuery::end()
    {
        if (m_endCalled)
            return;

        auto context = m_device->getIM();
        context->End(m_endQuery);
        context->End(m_disjointQuery);

        m_endCalled = true;
        m_finalized = false;
    }

    bool DX11TimerQuery::hasResult()
    {
        if (!m_endCalled)
            return false;

        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
        return m_device->getIM()->GetData(m_disjointQuery, &disjointData, sizeof(disjointData), 0) == S_OK;
    }

    f32 DX11TimerQuery::getResult()
    {
        if (!m_finalized)
        {
#if TK_DEBUG
            TK_ASSERT(hasResult());
#endif

            UINT64 timeStart, timeEnd;
            auto context = m_device->getIM();

            context->GetData(m_beginQuery, &timeStart, sizeof(timeStart), 0);
            context->GetData(m_endQuery, &timeEnd, sizeof(timeEnd), 0);

            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
            context->GetData(m_disjointQuery, &disjointData, sizeof(disjointData), 0);

            if (disjointData.Disjoint == FALSE)
            {
                const UINT64 delta = timeEnd - timeStart;
                m_timeDelta = (f32)(((f64)delta / (f64)disjointData.Frequency) * 1000.0);
            }
            else
            {
                m_timeDelta = 0.0f;
#if !TK_RELEASE
                static bool SingleShotLog = false;
                if (!SingleShotLog)
                {
                    SingleShotLog = true;
                    LogWarn("Unreliable GPU timer query detected.");
                }
#endif
            }

            m_finalized = true;
        }
        return m_timeDelta;
    }

}

#endif
