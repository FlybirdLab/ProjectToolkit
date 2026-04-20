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
        ~NetDriver() = default;
        NetDriver() = default;

    public:
        // Return name of this network driver implementation.
        TkString driverName();

        // Initialize the instance of this network driver using given configuration.
        bool init(NetPeer *host, const NetConfig &config);
        // Dispose this driver making it no longer usable. Reserved for resource deallocation etc.
        void dispose();
        // Start listening for incoming connections. Once this is called, this driver becomes a server.
        bool listen();
        // Starts connection handshake with the end point specified in the <seealso cref="NetworkConfig"/> structure.
        // Once this is called, this driver becomes a client.
        bool connect();

        // Disconnect from the server.
        void disconnect();
        // Disconnect given connection from the server.
        void disconnect(const NetConnection &connection);
        // Try to pop an network event from the queue.
        bool popEvent(NetEvent &eventPtr);

        // Send given message over specified channel to the server.
        void sendMessage(NetChannelType channelType, const NetMessage &message);
        // Send given message over specified channel to the given client connection (target).
        void sendMessage(NetChannelType channelType, const NetMessage &message, NetConnection target);
        // Send given message over specified channel to the given client connection (target).
        void sendMessage(NetChannelType channelType, const NetMessage &message, const TkArray<NetConnection> &targets);

        // Get the network transport layer stats.
        NetDriverStats getStats();
        // Get the network transport layer stats for a given connection.
        NetDriverStats getStats(NetConnection target);
    };

}