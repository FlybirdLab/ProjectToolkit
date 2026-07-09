/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>

#if TK_POSIX

// Platform description
#define PLATFORM_LINE_TERMINATOR "\n"
#define PLATFORM_TEXT_IS_CHAR16 1

#define LOG_UNIX_LAST_ERROR LogWarn("Unix::errno(): %s", strerror(errno))

#endif
