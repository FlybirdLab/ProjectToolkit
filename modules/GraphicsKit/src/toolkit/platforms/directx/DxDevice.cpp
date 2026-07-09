/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/platforms/directx/DxDevice.hpp>

#if GFX_API_DIRECTX

#include "DxUtils.hpp"

namespace toolkit
{
    void DirectXDevice::updateOutputs(IDXGIAdapter *adapter)
    {
#if TK_WINDOWS
        // Collect output devices
        u32 outputIdx = 0;
        ComPtr<IDXGIOutput> output;
        DXGI_FORMAT defaultBackbufferFormat = DxUtils::toDxgiFormat(GFX_BACK_BUFFER_PIXEL_FORMAT);
        TkArray<DXGI_MODE_DESC> modeDesc;
        while (adapter->EnumOutputs(outputIdx, &output) != DXGI_ERROR_NOT_FOUND)
        {
            auto &outputDX11 = outputs.addOne();

            outputDX11.output = output;
            output->GetDesc(&outputDX11.desc);

            u32 numModes = 0;
            HRESULT hr = output->GetDisplayModeList(defaultBackbufferFormat, 0, &numModes, nullptr);
            if (FAILED(hr))
            {
                LogWarn("Error while enumerating adapter output video modes.");
                continue;
            }

            modeDesc.resize(numModes, false);
            hr = output->GetDisplayModeList(defaultBackbufferFormat, 0, &numModes, modeDesc.get());
            if (FAILED(hr))
            {
                LogWarn("Error while enumerating adapter output video modes.");
                continue;
            }

            for (auto &mode : modeDesc)
            {
                bool foundVideoMode = false;
                for (auto &videoMode : outputDX11.videoModes)
                {
                    if (videoMode.Width == mode.Width &&
                        videoMode.Height == mode.Height &&
                        videoMode.RefreshRate.Numerator == mode.RefreshRate.Numerator &&
                        videoMode.RefreshRate.Denominator == mode.RefreshRate.Denominator)
                    {
                        foundVideoMode = true;
                        break;
                    }
                }

                if (!foundVideoMode)
                {
                    outputDX11.videoModes.add(mode);

                    // Collect only from the main monitor
                    if (outputs.count() == 1)
                    {
                        GfxDevice::outputs.add({mode.Width, mode.Height, (u32)(mode.RefreshRate.Numerator / (f32)mode.RefreshRate.Denominator)});
                    }
                }
            }

            // Get desktop display mode
            HMONITOR hMonitor = outputDX11.desc.Monitor;
            MONITORINFOEX monitorInfo;
            monitorInfo.cbSize = sizeof(MONITORINFOEX);
            GetMonitorInfo(hMonitor, &monitorInfo);

            DEVMODEW devMode;
            devMode.dmSize = sizeof(DEVMODEW);
            devMode.dmDriverExtra = 0;
            EnumDisplaySettingsW(StringAsUTF16<>(monitorInfo.szDevice).get(), ((DWORD)-1), &devMode);

            DXGI_MODE_DESC currentMode;
            currentMode.Width = devMode.dmPelsWidth;
            currentMode.Height = devMode.dmPelsHeight;
            bool useDefaultRefreshRate = 1 == devMode.dmDisplayFrequency || 0 == devMode.dmDisplayFrequency;
            currentMode.RefreshRate.Numerator = useDefaultRefreshRate ? 0 : devMode.dmDisplayFrequency;
            currentMode.RefreshRate.Denominator = useDefaultRefreshRate ? 0 : 1;
            currentMode.Format = defaultBackbufferFormat;
            currentMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            currentMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

            if (output->FindClosestMatchingMode(&currentMode, &outputDX11.desktopViewMode, nullptr) != S_OK)
                outputDX11.desktopViewMode = currentMode;

            f32 refreshRate = outputDX11.desktopViewMode.RefreshRate.Numerator / (f32)outputDX11.desktopViewMode.RefreshRate.Denominator;
            LogInfo("Video output '%s' %ix%i %d Hz", StringAsUTF8<>(outputDX11.desc.DeviceName).get(), devMode.dmPelsWidth, devMode.dmPelsHeight, refreshRate);
            outputIdx++;
        }
#endif
    }

}

#endif