/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#if TK_DEBUG
namespace toolkit
{
    // Message mustn't be out of ASCII charset.
    void assert(const char *msg, const char *func, const char *file, int line);
    // List stack frames and print to console without any heap allocation.
    void printStackTrace();
}
#define TK_ASSERT(x)                                           \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            toolkit::assert(#x, __func__, __FILE__, __LINE__); \
        }                                                      \
    } while (0)
#define TK_ASSERT_ENABLED 1

#define TK_CHECK(expression) \
    if (!(expression)) \
    { \
        TK_ASSERT(false && #expression); \
        return; \
    }
// Performs a soft check of the expression. Logs the expression failure and returns from the function call using the given return value.
#define TK_CHECK_RETURN(expression, returnValue) \
    if (!(expression)) \
    { \
        TK_ASSERT(false && #expression); \
        return returnValue; \
    }

// Performs a soft check of the expression. Logs the expression failure and returns from the function call.
#define TK_CHECK_DEBUG(expression) TK_CHECK(expression)
// Performs a soft check of the expression. Logs the expression failure and returns from the function call using the given return value.
#define TK_CHECK_RETURN_DEBUG(expression, returnValue) TK_CHECK_RETURN(expression, returnValue)

#else
#define TK_ASSERT(x) ((void)(x))
#define TK_ASSERT_ENABLED 0
#endif