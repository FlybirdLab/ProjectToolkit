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
    class TK_API NetworkClient final
    {
        friend class NetworkManager;
        explicit NetworkClient(u32 id, NetConnection connection);
        
    public:
        // Unique client identifier.
        u32 clientId;
        // Local peer connection.
        NetConnection connection;
        // Client connection state.
        NetConnectionState state;

    };

}