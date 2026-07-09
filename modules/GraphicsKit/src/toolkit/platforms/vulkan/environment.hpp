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
#define VK_RESOURCE_DELETE_SAFE_FRAMES_COUNT 3
// Keep reusable upload buffers around while streaming content. Video uploads
// can briefly pause between decoded frames; evicting after only the GPU safety
// delay causes large staging buffers to be destroyed and allocated again.
#define VK_STAGING_BUFFER_NOT_USED_FRAME_TIMEOUT 60

// ---------------------------------------------------------------------------
// VK_DEBUG — Vulkan-specific debug toggle, independent of TK_DEBUG.
// Set to 1 to enable validation layers, resource naming, debug data, and
// low-level assertions. Defaults to 0 so profiling builds are clean even
// when TK_DEBUG is on.
// ---------------------------------------------------------------------------
#ifndef VK_DEBUG
#define VK_DEBUG 0
#endif

// Redirect graphics-level debug flags to VK_DEBUG so they don't follow
// TK_DEBUG inside the Vulkan translation layer.
#undef GFX_ENABLE_ASSERTION_LOW_LAYERS
#define GFX_ENABLE_ASSERTION_LOW_LAYERS VK_DEBUG

#undef GFX_ENABLE_RESOURCE_NAMING
#define GFX_ENABLE_RESOURCE_NAMING VK_DEBUG

#undef GFX_ALLOW_PROFILE_EVENTS
#define GFX_ALLOW_PROFILE_EVENTS VK_DEBUG

#undef GFX_USE_SHADERS_DEBUG_LAYER
#define GFX_USE_SHADERS_DEBUG_LAYER VK_DEBUG

#define VK_ENABLE_API_DUMP 0
#define VK_RESET_QUERY_POOLS 1
#define VK_HASH_POOLS_WITH_LAYOUT_TYPES 1

// Validation layer and debug messenger: both gated on VK_DEBUG.
#define VK_USE_DEBUG_LAYER VK_DEBUG
#define VK_USE_DEBUG_DATA  VK_DEBUG

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
