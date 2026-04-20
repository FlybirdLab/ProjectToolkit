/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "NetPeer.hpp"

#include "NetDriver.hpp"
#include "NetConnection.hpp"
#include "NetMessage.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/platforms/Network.hpp"
#include "toolkit/platforms/System.hpp"
#include "toolkit/platforms/Network.hpp"

namespace toolkit
{
    TkArray<NetPeer *> NetPeer::peers;
    u32 LastHostId = 0;

    bool NetPeer::init(const NetConfig &config)
    {
        if (driver)
            return true;

        this->config = config;
#if TK_NET_BACKEND_ENET
        this->config.driver = TkMemory::newObject<ENetDriver>();
#endif

        if (this->config.driver == nullptr)
        {
            LogError("Missing driver");
            return false;
        }
        if (this->config.connectionLimit <= 0)
        {
            LogError("Invalid ConnectionsLimit");
            return false;
        }
        if (this->config.messageSize <= 32) // TODO: Adjust this, not sure what the lowest limit should be.
        {
            LogError("Invalid MessageSize");
            return false;
        }
        if (this->config.messagePoolSize <= 128)
        {
            LogError("Invalid messagePoolSize");
            return false;
        }
        driver = toInterface<NetDriver>(this->config.driver);
        if (!driver)
        {
            LogError("driver doesn't implement Idriver interface");
            return false;
        }

        // TODO: Dynamic message pool allocation
        // Setup messages
        createMessageBuffers();
        messagePool.clear();

        // Warmup message pool
        for (u32 messageId = this->config.messagePoolSize; messageId > 0; messageId--)
            messagePool.push(messageId);

        // Setup network driver
        if (driver->init(this, this->config))
        {
            LogError("Failed to initialize driver");
            return false;
        }

        LogInfo("NetPeer initialized using driver = %s", driver->driverName());
        return true;
    }

    void NetPeer::shutdown()
    {
        driver->dispose();
        TkMemory::deletePtr(this->config.driver);
        disposeMessageBuffers();

        this->config.driver = nullptr;
        driver = nullptr;
    }

    void NetPeer::createMessageBuffers()
    {
        TK_ASSERT(messageBuffer == nullptr);

        const u32 pageSize = System::getCPUStats().pageSize;

        // Calculate total size in bytes
        const u64 totalSize = static_cast<u64>(this->config.messagePoolSize) * this->config.messageSize;

        // Calculate the amount of pages that we need
        const u32 numPages = totalSize > pageSize ? TkMath::ceilToInt(totalSize / static_cast<float>(pageSize)) : 1;

        messageBuffer = static_cast<u8 *>(System::allocatePages(numPages, pageSize));
        TkMemory::memorySet(messageBuffer, 0, numPages * pageSize);
    }

    void NetPeer::disposeMessageBuffers()
    {
        TK_ASSERT(messageBuffer != nullptr);

        System::releaseMemory(messageBuffer, 0);
        messageBuffer = nullptr;
    }

    bool NetPeer::listen()
    {
        LogInfo("Starting to listen on address = %i:%i", config.address, config.port);
        return driver->listen();
    }

    bool NetPeer::connect()
    {
        LogInfo("Connecting to %i:%i...", config.address, config.port);
        return driver->connect();
    }

    void NetPeer::disconnect()
    {
        LogInfo("Disconnecting...");
        driver->disconnect();
    }

    void NetPeer::disconnect(const NetConnection &connection)
    {
        LogInfo("Disconnecting connection with id = {0}...", connection.connectionId);
        driver->disconnect(connection);
    }

    bool NetPeer::popEvent(NetEvent &eventRef)
    {
        PROFILE();
        return driver->popEvent(eventRef);
    }

    NetMessage NetPeer::createMessage()
    {
        const u32 messageId = messagePool.pop();
        u8 *messageBuffer = getMessageBuffer(messageId);
        return NetMessage(messageBuffer, messageId, config.messageSize, 0, 0);
    }

    void NetPeer::recycleMessage(const NetMessage &message)
    {
        TK_ASSERT(message.isValid());
#ifdef TK_DEBUG
        TK_ASSERT(messagePool.contains(message.messageId) == false);
#endif

        // Return the message id
        messagePool.push(message.messageId);
    }

    NetMessage NetPeer::beginSendMessage()
    {
        return createMessage();
    }

    void NetPeer::abortSendMessage(const NetMessage &message)
    {
        TK_ASSERT(message.isValid());
        recycleMessage(message);
    }

    bool NetPeer::endSendMessage(const NetChannelType channelType, const NetMessage &message)
    {
        TK_ASSERT(message.isValid());

        driver->sendMessage(channelType, message);
        recycleMessage(message);

        return true;
    }

    bool NetPeer::endSendMessage(const NetChannelType channelType, const NetMessage &message, const NetConnection &target)
    {
        TK_ASSERT(message.isValid());

        driver->sendMessage(channelType, message, target);

        recycleMessage(message);
        return true;
    }

    bool NetPeer::endSendMessage(const NetChannelType channelType, const NetMessage &message, const TkArray<NetConnection> &targets)
    {
        TK_ASSERT(message.isValid());

        driver->sendMessage(channelType, message, targets);

        recycleMessage(message);
        return true;
    }

    NetPeer *NetPeer::createPeer(const NetConfig &config)
    {
        // Validate the address for listen/connect
        if (config.address != "any")
        {
            NetEndPoint endPoint;
            if (TkNetwork::createEndPoint(config.address, TkString::Empty, NetIPVersion::kIPv4, endPoint, false))
            {
                LogError("Invalid end point.");
                return nullptr;
            }
        }

        // Alloc new host
        NetPeer *host = TkMemory::newObject<NetPeer>();
        host->hostId = LastHostId++;

        // Initialize the host
        if (host->init(config))
        {
            TkMemory::deletePtr(host);
            return nullptr;
        }

        peers.add(host);
        return host;
    }

    void NetPeer::shutdownPeer(NetPeer *peer)
    {
        if (!peer)
            return;
        TK_CHECK(peer->isValid());
        peer->shutdown();
        peer->hostId = -1;
        peers.remove(peer);

        TkMemory::deletePtr(peer);
    }

}