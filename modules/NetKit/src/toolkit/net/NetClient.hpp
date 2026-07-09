/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/net/common.hpp>
#include <toolkit/net/NetConnection.hpp>

namespace toolkit
{
    // High-level network client object (local or connected to the server).
    rkclass TK_API NetClient final : public RkObject
    {
        friend class NetManager;

    public:
        rkctor explicit NetClient(u32 id, NetConnection connection);

    public:
        // Unique client identifier.
        rkfield() u32 clientId;
        // Local peer connection.
        rkfield() NetConnection connection;
        // Client connection state.
        rkfield() NetConnectionState state;

    public:
        TkString toString() const override
        {
            return TkString::format("NetworkClient Id=%i, ConnectionId=%i", clientId, connection.connectionId);
        }
    };

}