/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "../../common.hpp"

#if TK_MACOS

// Platform description
#define PLATFORM_TYPE ::toolkit::PlatformType::kMac
#define PLATFORM_64BITS TK_PLATFORM_64BITS
#if TK_CPU_ARM && TK_PLATFORM_64BITS
#define PLATFORM_ARCH_ARM64 1
#define PLATFORM_ARCH ::toolkit::ArchitectureType::kARM64
#elif TK_CPU_X86 && TK_PLATFORM_64BITS
#define PLATFORM_ARCH_X64 1
#define PLATFORM_ARCH ::toolkit::ArchitectureType::kX64
#elif TK_CPU_X86
#define PLATFORM_ARCH_X86 1
#define PLATFORM_ARCH ::toolkit::ArchitectureType::kX86
#else
#define PLATFORM_ARCH_ARM 1
#define PLATFORM_ARCH ::toolkit::ArchitectureType::kARM
#endif
#define PLATFORM_DESKTOP 1
#define PLATFORM_CACHE_LINE_SIZE TK_CACHE_LINE_SIZE
#define PLATFORM_HAS_HEADLESS_MODE 1
#define PLATFORM_DEBUG_BREAK TK_DEBUG_BREAK

// MoltenVK has artifacts when using tess so disable it
#define GFX_ALLOW_TESSELLATION_SHADERS 0

#endif
