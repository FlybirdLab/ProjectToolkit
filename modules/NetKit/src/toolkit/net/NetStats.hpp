/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/net/common.hpp>

namespace toolkit
{
    // The network transport driver statistics container. Contains information about INetworkDriver usage and performance.
    struct TK_API NetDriverStats
    {
        // The mean round trip time (RTT), in milliseconds, between sending a reliable packet and receiving its acknowledgement. Also known as ping time.
        float rtt = 0.0f;
        // Total amount of data bytes sent by this client.
        u32 totalDataSent = 0;
        // Total amount of data bytes received by this client.
        u32 totalDataReceived = 0;
    };

    template <>
    struct TIsPODType<NetDriverStats>
    {
        enum
        {
            Value = true
        };
    };

}