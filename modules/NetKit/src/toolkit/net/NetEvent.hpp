/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/net/common.hpp>
#include <toolkit/net/NetMessage.hpp>
#include <toolkit/net/NetConnection.hpp>

namespace toolkit
{

    // Network event type enum contains all possible events that can be returned by PopEvent function.
    enum class NetEventType
    {
        // Invalid network event type.
        kUndefined = 0,
        // Event "connected" - client connected to our server or we've connected to the server.
        kConnected,
        // Event "disconnected" - client disconnected from our server or we've been kicked from the server.
        kDisconnected,
        // Event "disconnected" - client got a timeout from our server or we've list the connection to the server.
        kTimeout,
        // Event "message" - message received from some client or the server.
        kMessage
    };

    // Network event structure that wraps all data needed to identify and process it.
    struct TK_API NetworkEvent
    {
    public:
        // The type of the received event.
        NetEventType type;
        // The message when this event is an "message" event - not valid in any other cases.
        // If this is an message-event, make sure to return the message using RecycleMessage function of the peer after processing it!
        NetMessage message;
        // The connected of the client that has sent message, connected, disconnected or got a timeout.
        NetConnection sender;
    };

    template <>
    struct TIsPODType<NetworkEvent>
    {
        enum
        {
            Value = true
        };
    };

}