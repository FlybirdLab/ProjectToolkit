/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 * Reference: 
 *    https://github.com/GameTechDev/gpudetect/blob/master/GPUDetect.cpp
 *    https://gist.github.com/LxLasso/eccee4d71c2e49492f2cbf01a966fa73
 */

#include <toolkit/platforms/directx/DxAdapter.hpp>

#if GFX_API_DIRECTX

#define GFX_DRIVER_DETECTION_WIN32_REGISTRY (TK_WINDOWS)
#define GFX_DRIVER_DETECTION_WIN32_SETUPAPI (TK_WINDOWS)

#if GFX_DRIVER_DETECTION_WIN32_SETUPAPI
#define _SETUPAPI_VER WINVER
#if defined(LPCDLGTEMPLATE)
typedef void *LPCDLGTEMPLATE;
#endif
#include <SetupAPI.h>
#pragma comment(lib, "SetupAPI.lib")
#endif

namespace toolkit {

    void DirectXAdapter::getDriverVersion()
    {
#if GFX_DRIVER_DETECTION_WIN32_REGISTRY
        {
            // Fetch registry data
            HKEY dxKeyHandle = nullptr;
            DWORD numOfAdapters = 0;
            LSTATUS returnCode = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\DirectX", 0, KEY_READ, &dxKeyHandle);
            if (returnCode == S_OK)
            {
                // Find all sub keys
                DWORD subKeyMaxLength = 0;
                returnCode = RegQueryInfoKeyW(dxKeyHandle, 0, 0, 0, &numOfAdapters, &subKeyMaxLength, 0, 0, 0, 0, 0, 0);
                if (returnCode == S_OK && subKeyMaxLength < 100)
                {
                    subKeyMaxLength += 1;
                    uint64_t driverVersionRaw = 0;
                    wchar subKeyName[100];
                    for (DWORD i = 0; i < numOfAdapters; i++)
                    {
                        DWORD subKeyLength = subKeyMaxLength;
                        returnCode = RegEnumKeyExW(dxKeyHandle, i, subKeyName, &subKeyLength, 0, 0, 0, 0);
                        if (returnCode == S_OK)
                        {
                            LUID adapterLUID = {};
                            DWORD qwordSize = sizeof(uint64_t);
                            returnCode = RegGetValueW(dxKeyHandle, subKeyName, L"AdapterLuid", RRF_RT_QWORD, 0, &adapterLUID, &qwordSize);
                            if (returnCode == S_OK && adapterLUID.HighPart == description.AdapterLuid.HighPart && adapterLUID.LowPart == description.AdapterLuid.LowPart)
                            {
                                // Get driver version
                                returnCode = RegGetValueW(dxKeyHandle, subKeyName, L"DriverVersion", RRF_RT_QWORD, 0, &driverVersionRaw, &qwordSize);
                                if (returnCode == S_OK)
                                {
                                    TkVersion driverVersion(
                                        (i32)((driverVersionRaw & 0xFFFF000000000000) >> 16 * 3),
                                        (i32)((driverVersionRaw & 0x0000FFFF00000000) >> 16 * 2),
                                        (i32)((driverVersionRaw & 0x00000000FFFF0000) >> 16 * 1),
                                        (i32)((driverVersionRaw & 0x000000000000FFFF)));
                                    setDriverVersion(driverVersion);
                                }
                                break;
                            }
                        }
                    }
                }
                RegCloseKey(dxKeyHandle);
            }

            if (driverVersion != TkVersion(0, 0))
                return;
        }
#endif

#if GFX_DRIVER_DETECTION_WIN32_SETUPAPI
        {
            // Copied from devguid.h and devpkey.h
#define MAKE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) const GUID name = { l,  \
                                                                                       w1, \
                                                                                       w2, \
                                                                                       {b1, b2, b3, b4, b5, b6, b7, b8} }
#define MAKE_DEVPROPKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) const DEVPROPKEY name = {{l, w1, w2, {b1, b2, b3, b4, b5, b6, b7, b8}}, pid}
            MAKE_GUID(GUID_DEVCLASS_DISPLAY, 0x4d36e968L, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18);
            MAKE_DEVPROPKEY(DEVPKEY_Device_DriverDate, 0xa8b865dd, 0x2e3d, 0x4094, 0xad, 0x97, 0xe5, 0x93, 0xa7, 0xc, 0x75, 0xd6, 2);
            MAKE_DEVPROPKEY(DEVPKEY_Device_DriverVersion, 0xa8b865dd, 0x2e3d, 0x4094, 0xad, 0x97, 0xe5, 0x93, 0xa7, 0xc, 0x75, 0xd6, 3);
#undef MAKE_DEVPROPKEY
#undef MAKE_GUID

            HDEVINFO deviceInfoList = SetupDiGetClassDevs(&GUID_DEVCLASS_DISPLAY, NULL, NULL, DIGCF_PRESENT);
            if (deviceInfoList != INVALID_HANDLE_VALUE)
            {
                SP_DEVINFO_DATA deviceInfo;
                ZeroMemory(&deviceInfo, sizeof(deviceInfo));
                deviceInfo.cbSize = sizeof(SP_DEVINFO_DATA);

                wchar_t searchBuffer[128];
                swprintf(searchBuffer, sizeof(searchBuffer) / 2, L"PCI\\VEN_%04X&DEV_%04X&SUBSYS_%04X", description.VendorId, description.DeviceId, description.SubSysId);
                size_t searchBufferLen = wcslen(searchBuffer);

                DWORD deviceIndex = 0;
                DEVPROPTYPE propertyType;
                wchar_t buffer[300];
                while (SetupDiEnumDeviceInfo(deviceInfoList, deviceIndex, &deviceInfo))
                {
                    DWORD deviceIdSize;
                    if (SetupDiGetDeviceInstanceId(deviceInfoList, &deviceInfo, buffer, sizeof(buffer), &deviceIdSize) &&
                        wcsncmp(buffer, searchBuffer, searchBufferLen) == 0)
                    {
                        // Get driver version
                        if (SetupDiGetDeviceProperty(deviceInfoList, &deviceInfo, &DEVPKEY_Device_DriverVersion, &propertyType, (PBYTE)buffer, sizeof(buffer), NULL, 0) &&
                            propertyType == DEVPROP_TYPE_STRING)
                        {
                            // ParseDriverVersionBuffer(buffer);
                            TkVersion driverVersion;
                            TkString bufferStr(StringAsUTF8<>(buffer).get());
                            if (!TkVersion::parse(bufferStr, driverVersion))
                                setDriverVersion(driverVersion);
                        }

// #if 0
//                     // Get driver date
// 					DEVPROPTYPE propertyType;
// 					if (SetupDiGetDeviceProperty(deviceInfoList, &deviceInfo, &DEVPKEY_Device_DriverDate, &propertyType, (PBYTE)buffer, sizeof(FILETIME), NULL, 0) &&
//                         propertyType == DEVPROP_TYPE_FILETIME)
// 					{
// 						SYSTEMTIME deviceDriverSystemTime;
// 						FileTimeToSystemTime((FILETIME*)buffer, &deviceDriverSystemTime);
// 						DriverDate = DateTime(deviceDriverSystemTime.wYear, deviceDriverSystemTime.wMonth, deviceDriverSystemTime.wDay, deviceDriverSystemTime.wHour, deviceDriverSystemTime.wMinute, deviceDriverSystemTime.wSecond, deviceDriverSystemTime.wMilliseconds);
// 					}
// #endif
                    }
                    deviceIndex++;
                }

                SetupDiDestroyDeviceInfoList(deviceInfoList);
            }

            if (driverVersion != TkVersion(0, 0))
                return;
        }
#endif
    }

    void DirectXAdapter::setDriverVersion(TkVersion &ver)
    {
        if (isNVIDIA() && ver.build() > 0 && ver.revision() > 99)
        {
            // Convert NVIDIA version from 32.0.15.7247 into 572.47
            ver = TkVersion((ver.build() % 10) * 100 + ver.revision() / 100, ver.revision() % 100);
        }
        driverVersion = ver;
    }

}

#endif