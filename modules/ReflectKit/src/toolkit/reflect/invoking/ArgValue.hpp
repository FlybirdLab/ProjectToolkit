/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/reflect/common.hpp>
#include <toolkit/reflect/type/TypeHandle.hpp>
#include <toolkit/types/Span.hpp>

namespace toolkit
{
    // ArgValue is the lightweight runtime argument cell used by method wrappers.
    // Value/ref data points at a T object. Pointer data points at a T* variable, while type stores the pointee type.
    struct ArgValue
    {
        TypeHandle type;
        void* data = nullptr;
        ArgPassing passing = ArgPassing::kValue;
        bool owns = false;
    };

    using ArgSpan = TkSpan<ArgValue>;
}
