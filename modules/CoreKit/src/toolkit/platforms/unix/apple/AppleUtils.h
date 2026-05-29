/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

#if TK_APPLE

#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>
#include <toolkit/types/String.hpp>
#include <toolkit/types/StringView.hpp>

namespace toolkit::AppleUtils
{
    TkString toString(CFStringRef value);
    CFStringRef toCFString(TkStringView value);
    NSString* toNSString(TkStringView value);
    NSString* toNSString(const char* value);
    NSArray* parseArguments(NSString* arguments);
}

#endif
