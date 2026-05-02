/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/graphics/common.hpp>

#if GFX_API_VULKAN

// Vulkan API version to target
#ifndef VK_API_VERSION
#define VK_API_VERSION VK_API_VERSION_1_3
#endif

// Amount of back buffers to use
#ifndef VK_BACK_BUFFERS_COUNT
#define VK_BACK_BUFFERS_COUNT 2
#endif
#ifndef VK_BACK_BUFFERS_COUNT_MAX
#define VK_BACK_BUFFERS_COUNT_MAX 4
#endif

// Default amount of frames to wait until resource delete.
#define VK_RESOURCE_DELETE_SAFE_FRAMES_COUNT 20

#define VK_ENABLE_API_DUMP 0
#define VK_RESET_QUERY_POOLS 1
#define VK_HASH_POOLS_WITH_LAYOUT_TYPES 1
#define VK_USE_DEBUG_LAYER GFX_ENABLE_DIAGNOSTICS
#define VK_USE_DEBUG_DATA (GFX_ENABLE_DIAGNOSTICS && COMPILE_WITH_DEV_ENV)

#ifndef VK_USE_VALIDATION_CACHE
#ifdef VK_EXT_validation_cache
#define VK_USE_VALIDATION_CACHE VK_EXT_validation_cache
#else
#define VK_USE_VALIDATION_CACHE 0
#endif
#endif

#ifndef VK_USE_QUERIES
#define VK_USE_QUERIES 1
#endif

#endif
