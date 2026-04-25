/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "RkValue.hpp"

namespace toolkit
{
    RkValue::RkValue(RkValue &&other) noexcept
        : type(other.type), data(other.data), owns(other.owns), destroy(other.destroy)
    {
        other.data = nullptr;
        other.owns = false;
        other.destroy = nullptr;
    }

    RkValue &RkValue::operator=(RkValue &&other) noexcept
    {
        if (this != &other)
        {
            reset();
            type = other.type;
            data = other.data;
            owns = other.owns;
            destroy = other.destroy;
            other.data = nullptr;
            other.owns = false;
            other.destroy = nullptr;
        }
        return *this;
    }
    
    void RkValue::reset()
    {
        if (owns && data != nullptr && destroy != nullptr)
            destroy(data);
        type = TypeHandle();
        data = nullptr;
        owns = false;
        destroy = nullptr;
    }
}