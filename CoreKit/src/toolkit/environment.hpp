#pragma once

#define _TK_STR_HELPER(x) #x
#define _TK_STR(x) _TK_STR_HELPER(x)

#define TK_SAFE_ALIGNMENT 16
#define TK_CACHE_LINE_SIZE 64

#if defined(__clang__)
#	define TK_COMPILER_CLANG 1
#	define TK_COMPILER_GCC 0
#	define TK_COMPILER_MSVC 0
#	define TK_COMPILER_GCC_COMPATIBLE 1
#	define TK_COMPILER_STR "clang " __VERSION__
#elif defined(__GNUC__) || defined(__GNUG__)
#	define TK_COMPILER_CLANG 0
#	define TK_COMPILER_GCC 1
#	define TK_COMPILER_MSVC 0
#	define TK_COMPILER_GCC_COMPATIBLE 1
#	define TK_COMPILER_STR "gcc " __VERSION__
#elif defined(_MSC_VER)
#	define TK_COMPILER_CLANG 0
#	define TK_COMPILER_GCC 0
#	define TK_COMPILER_MSVC 1
#	define TK_COMPILER_GCC_COMPATIBLE 0
#	define TK_COMPILER_STR "MSVC " _TK_STR(_MSC_FULL_VER)
#else
#	error Unknown compiler
#endif

// Operating system
#if defined(__WIN32__) || defined(_WIN32)
#	define TK_LINUX 0
#	define TK_ANDROID 0
#	define TK_MACOS 0
#	define TK_IOS 0
#	define TK_WINDOWS 1
#	define TK_STR "Windows"
#elif defined(__APPLE_CC__)
#	if (defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) && __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 40000) || (defined(__IPHONE_OS_VERSION_MIN_REQUIRED) && __IPHONE_OS_VERSION_MIN_REQUIRED >= 40000)
#		define TK_LINUX 0
#		define TK_ANDROID 0
#		define TK_MACOS 0
#		define TK_IOS 1
#		define TK_WINDOWS 0
#		define TK_STR "iOS"
#	else
#		define TK_LINUX 0
#		define TK_ANDROID 0
#		define TK_MACOS 1
#		define TK_IOS 0
#		define TK_WINDOWS 0
#		define TK_STR "MacOS"
#	endif
#elif defined(__ANDROID__)
#	define TK_LINUX 0
#	define TK_ANDROID 1
#	define TK_MACOS 0
#	define TK_IOS 0
#	define TK_WINDOWS 0
#	define TK_STR "Android"
#elif defined(__linux__)
#	define TK_LINUX 1
#	define TK_ANDROID 0
#	define TK_MACOS 0
#	define TK_IOS 0
#	define TK_WINDOWS 0
#	define TK_STR "Linux"
#else
#	error Unknown OS
#endif


#if TK_LINUX || TK_ANDROID || TK_MACOS || TK_IOS
#	define TK_POSIX 1
#else
#	define TK_POSIX 0
#endif


#if TK_COMPILER_GCC_COMPATIBLE
#	if defined(__arm__) || defined(__aarch64__)
#		define TK_CPU_ARCH_X86 0
#		define TK_CPU_ARCH_ARM 1
#	elif defined(__i386__) || defined(__amd64__)
#		define TK_CPU_ARCH_X86 1
#		define TK_CPU_ARCH_ARM 0
#	endif
#elif TK_COMPILER_MSVC
#	if defined(_M_ARM)
#		define TK_CPU_ARCH_X86 0
#		define TK_CPU_ARCH_ARM 1
#	elif defined(_M_X64) || defined(_M_IX86)
#		define TK_CPU_ARCH_X86 1
#		define TK_CPU_ARCH_ARM 0
#	endif
#endif


#if defined(TK_CPU_ARCH_X86) && TK_CPU_ARCH_X86
#	define TK_GPU_ARCH_STR "x86"
#elif defined(TK_CPU_ARCH_ARM) && TK_CPU_ARCH_ARM
#	define TK_GPU_ARCH_STR "arm"
#else
#	error Unknown CPU arch
#endif


#if !TK_ENABLE_SIMD
#	define TK_SIMD_NONE 1
#	define TK_SIMD_SSE 0
#	define TK_SIMD_NEON 0
#	define TK_SIMD_STR "None"
#elif TK_CPU_ARCH_X86
#	define TK_SIMD_NONE 0
#	define TK_SIMD_SSE 1
#	define TK_SIMD_NEON 0
#	define TK_SIMD_STR "SSE"
#else
#	define TK_SIMD_NONE 0
#	define TK_SIMD_SSE 0
#	define TK_SIMD_NEON 1
#	define TK_SIMD_STR "Neon"
#endif


#if TK_COMPILER_GCC_COMPATIBLE
#	define TK_RESTRICT __restrict
#	define TK_FORCE_INLINE inline __attribute__((always_inline))
#	define TK_DONT_INLINE __attribute__((noinline))
#	define TK_UNUSED __attribute__((__unused__))
#	define TK_COLD __attribute__((cold, optimize("Os")))
#	define TK_HOT __attribute__ ((hot))
#	define TK_UNREACHABLE() __builtin_unreachable()
#	define TK_PREFETCH_MEMORY(addr) __builtin_prefetch(addr)
#	define TK_CHECK_FORMAT(fmtArgIdx, firstArgIdx) __attribute__((format(printf, fmtArgIdx + 1, firstArgIdx + 1))) // On methods you need to include "this"
#	define TK_PURE __attribute__((pure))
#elif TK_COMPILER_MSVC
#	define TK_RESTRICT
#	define TK_FORCE_INLINE __forceinline
#	define TK_DONT_INLINE
#	define TK_UNUSED
#	define TK_COLD
#	define TK_HOT
#	define TK_UNREACHABLE() __assume(false)
#	define TK_PREFETCH_MEMORY(addr) (void)(addr)
#	define TK_CHECK_FORMAT(fmtArgIdx, firstArgIdx)
#	define TK_PURE
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