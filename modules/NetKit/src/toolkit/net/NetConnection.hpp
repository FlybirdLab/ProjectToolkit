/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/net/common.hpp>

namespace toolkit
{
    // Network connection structure - used to identify connected peers when we're listening.
    struct TK_API NetConnection
    {
    public:
        // The identifier of the connection.
        u32 connectionId;
    };

    template <>
    struct TIsPODType<NetConnection>
    {
        enum
        {
            Value = true
        };
    };

    inline bool operator==(const NetConnection &a, const NetConnection &b)
    {
        return a.connectionId == b.connectionId;
    }

}