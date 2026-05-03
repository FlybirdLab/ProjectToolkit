/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

// #pragma execution_character_set("utf-8")

// =======================================================
// Stringify helpers
// =======================================================
#define TK_STR_HELPER(x) #x
#define TK_STR(x) TK_STR_HELPER(x)
#define TK_OFFSET_OF(X, Y) __builtin_offsetof(X, Y)

// =======================================================
// Constants
// =======================================================
#define TK_SAFE_ALIGNMENT     16
#define TK_CACHE_LINE_SIZE    64

// =======================================================
// Compiler detection
// =======================================================
#if defined(__clang__)
    #define TK_COMPILER_CLANG 1
    #define TK_COMPILER_GCC   0
    #define TK_COMPILER_MSVC  0
    #define TK_COMPILER_GCC_COMPATIBLE 1
    #define TK_COMPILER_STR "clang " __VERSION__
#elif defined(__GNUC__)
    #define TK_COMPILER_CLANG 0
    #define TK_COMPILER_GCC   1
    #define TK_COMPILER_MSVC  0
    #define TK_COMPILER_GCC_COMPATIBLE 1
    #define TK_COMPILER_STR "gcc " __VERSION__
#elif defined(_MSC_VER)
    #define TK_COMPILER_CLANG 0
    #define TK_COMPILER_GCC   0
    #define TK_COMPILER_MSVC  1
    #define TK_COMPILER_GCC_COMPATIBLE 0
    #define TK_COMPILER_STR "MSVC " TK_STR(_MSC_FULL_VER)
#else
    #error Unknown compiler
#endif

// =======================================================
// OS detection
// =======================================================
#if defined(_WIN32)
    #define TK_WINDOWS 1
    #define TK_LINUX   0
    #define TK_ANDROID 0
    #define TK_MACOS   0
    #define TK_IOS     0
    #define TK_APPLE   0
    #define TK_OS_STR "Windows"
#elif defined(__ANDROID__)
    #define TK_WINDOWS 0
    #define TK_LINUX   0
    #define TK_ANDROID 1
    #define TK_MACOS   0
    #define TK_IOS     0
    #define TK_APPLE   0
    #define TK_OS_STR "Android"
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #define TK_WINDOWS 0
    #define TK_LINUX   0
    #define TK_ANDROID 0
    #define TK_APPLE   1
    #if TARGET_OS_IPHONE
        #define TK_IOS   1
        #define TK_MACOS 0
        #define TK_OS_STR "iOS"
    #else
        #define TK_IOS   0
        #define TK_MACOS 1
        #define TK_OS_STR "macOS"
    #endif
#elif defined(__linux__)
    #define TK_WINDOWS 0
    #define TK_LINUX   1
    #define TK_ANDROID 0
    #define TK_MACOS   0
    #define TK_IOS     0
    #define TK_APPLE   0
    #define TK_OS_STR "Linux"
#else
    #error Unknown OS
#endif

#if TK_LINUX || TK_ANDROID || TK_MACOS || TK_IOS
    #define TK_POSIX 1
    #define TK_LINE_TERMINATOR "\n"
#else
    #define TK_POSIX 0
    #define TK_LINE_TERMINATOR "\r\n"
#endif

// =======================================================
// CPU architecture
// =======================================================
#if TK_COMPILER_GCC_COMPATIBLE
    #if defined(__aarch64__) || defined(__arm__)
        #define TK_CPU_ARM 1
        #define TK_CPU_X86 0
    #elif defined(__x86_64__) || defined(__i386__)
        #define TK_CPU_ARM 0
        #define TK_CPU_X86 1
    #endif
#elif TK_COMPILER_MSVC
    #if defined(_M_ARM64) || defined(_M_ARM)
        #define TK_CPU_ARM 1
        #define TK_CPU_X86 0
    #else
        #define TK_CPU_ARM 0
        #define TK_CPU_X86 1
    #endif
#endif

#if TK_CPU_X86
    #define TK_CPU_ARCH_STR "x86"
#elif TK_CPU_ARM
    #define TK_CPU_ARCH_STR "arm"
#else
    #error Unknown CPU architecture
#endif

// =======================================================
// Pointer width
// =======================================================
#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
    #define TK_PLATFORM_64BITS 1
#else
    #define TK_PLATFORM_64BITS 0
#endif

// =======================================================
// SIMD
// =======================================================
#ifndef TK_ENABLE_SIMD
    #define TK_ENABLE_SIMD 1
#endif

#if !TK_ENABLE_SIMD
    #define TK_SIMD_NONE 1
    #define TK_SIMD_SSE  0
    #define TK_SIMD_NEON 0
#elif TK_CPU_X86
    #define TK_SIMD_NONE 0
    #define TK_SIMD_SSE  1
    #define TK_SIMD_NEON 0
#else
    #define TK_SIMD_NONE 0
    #define TK_SIMD_SSE  0
    #define TK_SIMD_NEON 1
#endif

// =======================================================
// Attributes / keywords
// =======================================================
#if TK_COMPILER_GCC_COMPATIBLE
#	define TK_RESTRICT __restrict
#   define TK_THREAD_LOCAL __thread
#	define TK_FORCE_INLINE inline __attribute__((always_inline))
#	define TK_DONT_INLINE __attribute__((noinline))
#	define TK_UNUSED __attribute__((__unused__))
#	define TK_COLD __attribute__((cold, optimize("Os")))
#	define TK_HOT __attribute__ ((hot))
#	define TK_UNREACHABLE() __builtin_unreachable()
#	define TK_PREFETCH_MEMORY(addr) __builtin_prefetch(addr)
#	define TK_CHECK_FORMAT(fmtArgIdx, firstArgIdx) __attribute__((format(printf, fmtArgIdx + 1, firstArgIdx + 1))) // On methods you need to include "this"
#	define TK_PURE __attribute__((pure))
#   define TK_PACK_BEGIN()
#   define TK_PACK_END() __attribute__((__packed__))
#elif TK_COMPILER_MSVC
#	define TK_RESTRICT
#   define TK_THREAD_LOCAL __declspec(thread)
#	define TK_FORCE_INLINE __forceinline
#	define TK_DONT_INLINE
#	define TK_UNUSED
#	define TK_COLD
#	define TK_HOT
#	define TK_UNREACHABLE() __assume(false)
#	define TK_PREFETCH_MEMORY(addr) (void)(addr)
#	define TK_CHECK_FORMAT(fmtArgIdx, firstArgIdx)
#	define TK_PURE
#   define TK_PACK_BEGIN() __pragma(pack(push, 1))
#   define TK_PACK_END() ; __pragma(pack(pop))
#else
#	define TK_RESTRICT
#	define TK_FORCE_INLINE 
#	define TK_DONT_INLINE
#	define TK_UNUSED
#	define TK_COLD
#	define TK_HOT
#	define TK_UNREACHABLE() 
#	define TK_PREFETCH_MEMORY(addr) (void)(addr)
#	define TK_CHECK_FORMAT(fmtArgIdx, firstArgIdx)
#	define TK_PURE
#endif

#if TK_MACOS
#define TK_DEBUG_BREAK __builtin_trap()
#define TK_CACHE_LINE_SIZE 128
#endif

// =======================================================
// DLL export
// =======================================================
#if TK_WINDOWS
    #define TK_LIBRARY_EXPORT __declspec(dllexport)
    #define TK_LIBRARY_IMPORT __declspec(dllimport)
    #define TK_NO_SANITIZE_THREAD
    #define TK_NO_SANITIZE_ADDRESS
#else
    #define TK_LIBRARY_EXPORT __attribute__((visibility("default")))
    #define TK_LIBRARY_IMPORT
    #define TK_NO_SANITIZE_THREAD __attribute__((no_sanitize_thread))
    #define TK_NO_SANITIZE_ADDRESS __attribute__((no_sanitize_address))
#endif

#ifdef TK_STATIC_PLUGINS
    #define TK_API
#elif defined(BUILDING)
    #define TK_API TK_LIBRARY_EXPORT
#else
    #define TK_API TK_LIBRARY_IMPORT
#endif

#include <toolkit/definations.hpp>