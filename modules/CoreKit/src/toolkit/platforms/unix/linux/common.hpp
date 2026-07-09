/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "../common.hpp"

#if TK_LINUX

// Platform description
#define PLATFORM_64BITS TK_PLATFORM_64BITS
#if TK_PLATFORM_64BITS && TK_CPU_X86
#define PLATFORM_ARCH_X64 1
#define PLATFORM_ARCH ::toolkit::ArchitectureType::kX64
#elif TK_CPU_X86
#define PLATFORM_ARCH_X86 1
#define PLATFORM_ARCH ::toolkit::ArchitectureType::kX86
#elif TK_PLATFORM_64BITS && TK_CPU_ARM
#define PLATFORM_ARCH_ARM64 1
#define PLATFORM_ARCH ::toolkit::ArchitectureType::kARM64
#else
#define PLATFORM_ARCH_ARM 1
#define PLATFORM_ARCH ::toolkit::ArchitectureType::kARM
#endif
#define PLATFORM_TYPE ::toolkit::PlatformType::kLinux
#define PLATFORM_DESKTOP 1
#define PLATFORM_CACHE_LINE_SIZE TK_CACHE_LINE_SIZE
#define PLATFORM_HAS_HEADLESS_MODE 1
#define PLATFORM_DEBUG_BREAK __asm__ volatile("int $0x03")

#endif
