/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <stdint.h>

#if TK_ANDROID

typedef struct android_app android_app;
typedef struct AAssetManager AAssetManager;

namespace toolkit
{
    android_app* getAndroidApp();
}

extern "C" TK_API void TkAndroidSetApp(android_app* app);
extern "C" TK_API void TkAndroidSetPlatformInfo(
    const char* appPackageName,
    const char* deviceManufacturer,
    const char* deviceModel,
    const char* deviceBuildNumber,
    const char* systemVersion,
    const char* systemLanguage,
    int32_t screenWidth,
    int32_t screenHeight,
    const char* cacheDir,
    const char* executablePath,
    const char* internalDataPath,
    const char* externalDataPath);

#endif
