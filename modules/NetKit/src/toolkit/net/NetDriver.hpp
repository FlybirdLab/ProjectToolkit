/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/net/common.hpp>

namespace toolkit
{

    // Basic interface for the low-level network transport/driver.
    class TK_API NetDriver
    {
    public:
        virtual ~NetDriver() = default;
        NetDriver() = default;

    public:
        // Return name of this network driver implementation.
        virtual TkString driverName() = 0;

        // Initialize the instance of this network driver using given configuration. Return true on success.
        virtual bool init(NetPeer *host, const NetConfig &config) = 0;
        // Dispose this driver making it no longer usable. Reserved for resource deallocation etc.
        virtual void dispose() = 0;
        // Start listening for incoming connections. Once this is called, this driver becomes a server. Return true on success.
        virtual bool listen() = 0;
        // Starts connection handshake with the end point specified in the <seealso cref="NetworkConfig"/> structure. Return true on success.
        // Once this is called, this driver becomes a client.
        virtual bool connect() = 0;

        // Disconnect from the server.
        virtual void disconnect() = 0;
        // Disconnect given connection from the server.
        virtual void disconnect(const NetConnection &connection) = 0;
        // Try to pop an network event from the queue.
        virtual bool popEvent(NetEvent &eventPtr) = 0;

        // Send given message over specified channel to the server.
        virtual void sendMessage(NetChannelType channelType, const NetMessage &message) = 0;
        // Send given message over specified channel to the given client connection (target).
        virtual void sendMessage(NetChannelType channelType, const NetMessage &message, NetConnection target) = 0;
        // Send given message over specified channel to the given client connection (target).
        virtual void sendMessage(NetChannelType channelType, const NetMessage &message, const TkArray<NetConnection> &targets) = 0;

        // Get the network transport layer stats.
        virtual NetDriverStats getStats() = 0;
        // Get the network transport layer stats for a given connection.
        virtual NetDriverStats getStats(NetConnection target) = 0;
    };

}
