/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/base/Assert.hpp>
#include <toolkit/reflect/type/TypeHandle.hpp>

namespace toolkit
{
    // RkValue is a boundary-layer dynamic value helper. Core method invocation stays on
    // CallFrame + ArgValue + ReturnValue and must not depend on a Variant-like value container.
    struct TK_API RkValue
    {
        using DestroyFn = void (*)(void* data);

        TypeHandle type;
        void* data = nullptr;
        bool owns = false;
        DestroyFn destroy = nullptr;

        RkValue() = default;
        RkValue(const RkValue&) = delete;
        RkValue& operator=(const RkValue&) = delete;

        RkValue(RkValue&& other) noexcept;
        RkValue& operator=(RkValue&& other) noexcept;
        ~RkValue()
        {
            reset();
        }

        void reset();

        template <class T>
        T& as() const
        {
            TK_ASSERT(data != nullptr);
            return *reinterpret_cast<T*>(data);
        }
    };
}
