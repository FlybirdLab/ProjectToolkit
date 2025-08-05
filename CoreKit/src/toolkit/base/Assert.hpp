// Created by cubevlmu on 2025/8/4.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#if TK_DEBUG
namespace toolkit {
    void assert(const char* msg, const char* func, const char* file, int line);
}
#	define TK_ASSERT(x) do { if(!(x)) { toolkit::assert(#x, __func__, __FILE__, __LINE__); } } while(0)
#	define TK_ASSERT_ENABLED 1
#else
#   define TK_ASSERT(x) ((void)(x))
#   define TK_ASSERT_ENABLED 0
#endif