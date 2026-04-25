/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/reflect/typedef.hpp>

namespace toolkit
{
    // Generated wrappers may assume RkMethod::invokeRaw has validated args before invocation.
    // Value/ref args point at T, pointer args point at a T* cell. Value returns write context.result.
    // Ref returns write ReturnValue::data = &ref. Pointer returns write ReturnValue::data to stable T* storage.
    // Never point a pointer return at a wrapper stack local T*; use CallFrame/member/static storage instead.
    struct InvokeContext
    {
        void* instance = nullptr;
        ArgValue* args = nullptr;
        u32 argCount = 0;
        void* result = nullptr;
        ReturnValue* returnValue = nullptr;
    };
}
