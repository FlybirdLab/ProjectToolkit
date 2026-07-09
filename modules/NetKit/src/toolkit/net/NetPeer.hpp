/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/net/common.hpp>
#include <toolkit/collection/Array.hpp>

namespace toolkit
{
    // Low-level network peer class. Provides server-client communication functions, message processing and sending.
    class TK_API NetPeer final
    {
        // List with all active peers.
        static TkArray<NetPeer *> peers;

    public:
        // Start listening for incoming connections.
        // Once this is called, this peer becomes a server.
        // Return true on success.
        bool listen();
        // Start connection handshake with the end point specified in the <seealso cref="NetworkConfig"/> structure.
        // Once this is called, this peer becomes a client.
        // Return true on success.
        bool connect();
        // Disconnect from the server.
        void disconnect();
        // Disconnect given connection from the server.
        void disconnect(const NetConnection &connection);
        // Try to pop an network event from the queue.
        bool popEvent(NetEvent &eventRef);

        // Acquire new message from the pool.
        // Cannot acquire more messages than the limit specified in the <seealso cref="NetworkConfig"/> structure.
        NetMessage createMessage();
        // Return given message to the pool.
        void recycleMessage(const NetMessage &message);

        // Acquire new message from the pool and setups it for sending.
        NetMessage beginSendMessage();
        // Abort given message send. This effectively deinitializes the message and returns it to the pool.
        void abortSendMessage(const NetMessage &message);

        // Send given message over specified channel to the server.
        bool endSendMessage(NetChannelType channelType, const NetMessage &message);
        // Send given message over specified channel to the given client connection (target).
        bool endSendMessage(NetChannelType channelType, const NetMessage &message, const NetConnection &target);
        // Send given message over specified channel to the given client connection (target).
        bool endSendMessage(NetChannelType channelType, const NetMessage &message, const TkArray<NetConnection> &targets);

        // Create new peer using given configuration.
        static NetPeer *createPeer(const NetConfig &config);
        // Shutdown and destroys given peer.
        static void shutdownPeer(NetPeer *peer);

    public:
        bool isValid() const
        {
            return driver != nullptr && hostId >= 0;
        }

        u8 *getMessageBuffer(const u32 messageId) const
        {
            // Calculate and return the buffer slice using previously calculated slice.
            return messageBuffer + config.messageSize * messageId;
        }

    public:
        TK_FORCE_INLINE bool operator==(const NetPeer &other) const
        {
            return hostId == other.hostId;
        }

        TK_FORCE_INLINE bool operator!=(const NetPeer &other) const
        {
            return hostId != other.hostId;
        }

    private:
        bool init(const NetConfig &config);
        void shutdown();
        void createMessageBuffers();
        void disposeMessageBuffers();

    public:
        int hostId = -1;
        NetConfig config;

        u8 *messageBuffer = nullptr;
        TkArray<u32> messagePool;

        // Low-level network transport driver used by this peer.
        NetDriver *driver = nullptr;
    };

}
