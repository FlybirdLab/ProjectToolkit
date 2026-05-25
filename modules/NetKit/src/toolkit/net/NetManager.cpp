/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "NetManager.hpp"

#include <toolkit/debug/Logger.hpp>
#include <toolkit/net/NetClient.hpp>
#include <toolkit/net/NetEvent.hpp>
#include <toolkit/net/NetInternal.hpp>
#include <toolkit/net/NetMessage.hpp>
#include <toolkit/net/NetPeer.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/reflect/RkManager.hpp>

namespace toolkit
{
    TkAction<> NetManager::onStateChanged;
    TkAction<NetClientConnectionData &> NetManager::onClientConnecting;
    TkAction<NetClient *> NetManager::onClientConnected;
    TkAction<NetClient *> NetManager::onClientDisconnected;

    f32 NetManager::networkFPS = 60.0f;
    NetPeer *NetManager::peer = nullptr;
    NetManagerMode NetManager::mode = NetManagerMode::kOffline;
    NetConnectionState NetManager::state = NetConnectionState::kOffline;
    u32 NetManager::frame = 0;
    u32 NetManager::localClientId = NetManager::kServerClientId;
    NetClient *NetManager::localClient = nullptr;
    TkArray<NetClient *> NetManager::clients;

    namespace
    {
        u32 s_nextClientId = NetManager::kServerClientId + 1;
        double s_lastUpdateTime = 0.0;
        TkArray<NetConnection> s_activeConnections;

#pragma pack(push, 1)
        struct NetMessageHandshake
        {
            NetMessageId id = NetMessageId::kHandshake;
            u32 protocolVersion = kNetProtocolVersion;
            u16 payloadDataSize = 0;
            u8 platform = static_cast<u8>(PlatformType::kWindows);
            u8 architecture = static_cast<u8>(ArchitectureType::kX64);
        };

        struct NetMessageHandshakeReply
        {
            NetMessageId id = NetMessageId::kHandshakeReply;
            u32 clientId = 0;
            i32 result = 0;
        };
#pragma pack(pop)

        PlatformType currentPlatform()
        {
#if TK_WINDOWS
            return PlatformType::kWindows;
#elif TK_LINUX
            return PlatformType::kLinux;
#elif TK_MACOS
            return PlatformType::kMac;
#else
            return PlatformType::kWindows;
#endif
        }

        ArchitectureType currentArchitecture()
        {
#if TK_CPU_ARM && TK_PLATFORM_64BITS
            return ArchitectureType::kARM64;
#elif TK_CPU_ARM
            return ArchitectureType::kARM;
#elif TK_CPU_X86 && !TK_PLATFORM_64BITS
            return ArchitectureType::kX86;
#else
            return ArchitectureType::kX64;
#endif
        }

        NetClient *newClient(const u32 id, const NetConnection connection)
        {
            return TkMemory::newObject<NetClient>(RkManager::getAllocator(), id, connection);
        }

        void deleteClient(NetClient *client)
        {
            if (client != nullptr)
                TkMemory::deletePtr(RkManager::getAllocator(), client);
        }

        bool startPeer()
        {
            TK_ASSERT(NetManager::peer == nullptr);
            NetManager::state = NetConnectionState::kConnecting;
            NetManager::onStateChanged();

            NetConfig config;
            if (NetManager::mode == NetManagerMode::kClient)
            {
                config.address = TkString("127.0.0.1");
                config.connectionLimit = 1;
            }
            else
            {
                config.address = TkString("any");
                config.connectionLimit = 32;
            }

            NetManager::peer = NetPeer::createPeer(config);
            if (NetManager::peer == nullptr)
            {
                NetManager::state = NetConnectionState::kOffline;
                LogError("Failed to create NetPeer at %s:%u", config.address.getText(), static_cast<u32>(config.port));
                return false;
            }

            NetManager::frame = 0;
            return true;
        }

        void stopPeer()
        {
            if (NetManager::peer == nullptr)
                return;
            if (NetManager::mode == NetManagerMode::kClient)
                NetManager::peer->disconnect();
            NetPeer::shutdownPeer(NetManager::peer);
            NetManager::peer = nullptr;
        }

        void onNetMessageHandshake(NetEvent &event, NetClient *client, NetPeer *peer)
        {
            if (client == nullptr)
                return;

            NetMessageHandshake msgData;
            event.message.readStructure(msgData);

            NetClientConnectionData connectionData;
            connectionData.client = client;
            connectionData.result = 0;
            connectionData.platform = static_cast<PlatformType>(msgData.platform);
            connectionData.architecture = static_cast<ArchitectureType>(msgData.architecture);
            connectionData.payloadData.resize(msgData.payloadDataSize);
            if (msgData.payloadDataSize != 0)
                event.message.readBytes(connectionData.payloadData.get(), msgData.payloadDataSize);
            if (msgData.protocolVersion != kNetProtocolVersion)
                connectionData.result = 1;

            NetManager::onClientConnecting(connectionData);

            NetMessageHandshakeReply replyData;
            replyData.result = connectionData.result;
            replyData.clientId = client->clientId;
            NetMessage reply = peer->beginSendMessage();
            reply.writeStructure(replyData);
            peer->endSendMessage(NetChannelType::kReliableOrdered, reply, event.sender);

            if (connectionData.result != 0)
            {
                client->state = NetConnectionState::kDisconnecting;
                peer->disconnect(event.sender);
                client->state = NetConnectionState::kDisconnected;
                return;
            }

            client->state = NetConnectionState::kConnected;
            s_activeConnections.add(event.sender);
            NetManager::onClientConnected(client);
            NetInternal::netReplicatorClientConnected(client);
        }

        void onNetMessageHandshakeReply(NetEvent &event, NetClient *, NetPeer *)
        {
            if (!NetManager::isClient())
                return;

            NetMessageHandshakeReply msgData;
            event.message.readStructure(msgData);
            if (msgData.result != 0)
            {
                NetManager::stop();
                return;
            }

            NetManager::localClientId = msgData.clientId;
            NetManager::localClient->clientId = msgData.clientId;
            NetManager::localClient->state = NetConnectionState::kConnected;
            NetManager::state = NetConnectionState::kConnected;
            NetManager::onStateChanged();
        }

        void onNetMessageKey(NetEvent &, NetClient *, NetPeer *)
        {
            // Network key compression is intentionally replaced by direct id/name serialization in NetMessage.
        }

        using MessageHandler = void (*)(NetEvent &, NetClient *, NetPeer *);
        MessageHandler s_messageHandlers[static_cast<u32>(NetMessageId::kMax)] =
        {
            nullptr,
            onNetMessageHandshake,
            onNetMessageHandshakeReply,
            onNetMessageKey,
            NetInternal::onNetMessageObjectReplicate,
            NetInternal::onNetMessageObjectReplicatePart,
            NetInternal::onNetMessageObjectSpawn,
            NetInternal::onNetMessageObjectSpawnPart,
            NetInternal::onNetMessageObjectDespawn,
            NetInternal::onNetMessageObjectRole,
            NetInternal::onNetMessageObjectRpc,
        };
    }

    NetClient::NetClient(const u32 id, const NetConnection connection)
        : clientId(id), connection(connection), state(NetConnectionState::kConnecting)
    {
    }

    NetClient *NetManager::getClient(const NetConnection &connection)
    {
        if (connection.connectionId == 0 && localClient != nullptr && mode != NetManagerMode::kServer)
            return localClient;
        for (NetClient *client : clients)
        {
            if (client->connection == connection)
                return client;
        }
        return nullptr;
    }

    NetClient *NetManager::getClient(const u32 clientId)
    {
        for (NetClient *client : clients)
        {
            if (client->clientId == clientId)
                return client;
        }
        return nullptr;
    }

    bool NetManager::startServer()
    {
        stop();

        LogInfo("Starting NetManager as server");
        mode = NetManagerMode::kServer;
        if (!startPeer())
        {
            mode = NetManagerMode::kOffline;
            return false;
        }
        if (!peer->listen())
        {
            stop();
            return false;
        }

        localClientId = kServerClientId;
        s_nextClientId = kServerClientId + 1;
        state = NetConnectionState::kConnected;
        onStateChanged();
        return true;
    }

    bool NetManager::startClient()
    {
        stop();

        LogInfo("Starting NetManager as client");
        mode = NetManagerMode::kClient;
        if (!startPeer())
        {
            mode = NetManagerMode::kOffline;
            return false;
        }
        if (!peer->connect())
        {
            stop();
            return false;
        }

        localClientId = 0;
        s_nextClientId = 0;
        localClient = newClient(localClientId, NetConnection{0});
        return true;
    }

    bool NetManager::startHost()
    {
        stop();

        LogInfo("Starting NetManager as host");
        mode = NetManagerMode::kHost;
        if (!startPeer())
        {
            mode = NetManagerMode::kOffline;
            return false;
        }
        if (!peer->listen())
        {
            stop();
            return false;
        }

        localClientId = kServerClientId;
        s_nextClientId = kServerClientId + 1;
        localClient = newClient(localClientId, NetConnection{0});
        localClient->state = NetConnectionState::kConnected;
        state = NetConnectionState::kConnected;
        onStateChanged();
        onClientConnected(localClient);
        return true;
    }

    void NetManager::stop()
    {
        if (mode == NetManagerMode::kOffline && state == NetConnectionState::kOffline)
            return;

        LogInfo("Stopping NetManager");
        state = NetConnectionState::kDisconnecting;
        if (localClient != nullptr)
            localClient->state = NetConnectionState::kDisconnecting;
        for (NetClient *client : clients)
            client->state = NetConnectionState::kDisconnecting;
        onStateChanged();

        for (i32 i = clients.count() - 1; i >= 0; --i)
        {
            NetClient *client = clients[i];
            NetInternal::netReplicatorClientDisconnected(client);
            onClientDisconnected(client);
            client->state = NetConnectionState::kDisconnected;
            deleteClient(client);
            clients.removeAtKeepOrder(i);
        }
        if (mode == NetManagerMode::kHost && localClient != nullptr)
        {
            onClientDisconnected(localClient);
            localClient->state = NetConnectionState::kDisconnected;
        }

        NetInternal::netReplicatorClear();
        stopPeer();
        deleteClient(localClient);
        localClient = nullptr;

        s_nextClientId = kServerClientId + 1;
        s_lastUpdateTime = 0.0;
        s_activeConnections.clear();
        state = NetConnectionState::kDisconnected;
        mode = NetManagerMode::kOffline;
        onStateChanged();
    }

    void NetManager::update()
    {
        NetPeer *activePeer = peer;
        if (mode == NetManagerMode::kOffline || activePeer == nullptr)
            return;

        const double currentTime = System::getTimeSeconds();
        const f32 minDeltaTime = networkFPS > 0.0f ? 1.0f / networkFPS : 0.0f;
        if (currentTime - s_lastUpdateTime < minDeltaTime)
            return;
        s_lastUpdateTime = currentTime;

        frame++;
        NetInternal::netReplicatorPreUpdate();

        NetEvent event;
        while (activePeer->popEvent(event))
        {
            switch (event.type)
            {
            case NetEventType::kConnected:
                if (isClient())
                {
                    NetClientConnectionData connectionData;
                    connectionData.client = localClient;
                    connectionData.result = 0;
                    connectionData.platform = currentPlatform();
                    connectionData.architecture = currentArchitecture();
                    onClientConnecting(connectionData);
                    if (connectionData.result != 0)
                    {
                        stop();
                        return;
                    }

                    NetMessageHandshake msgData;
                    msgData.platform = static_cast<u8>(connectionData.platform);
                    msgData.architecture = static_cast<u8>(connectionData.architecture);
                    msgData.payloadDataSize = static_cast<u16>(connectionData.payloadData.count());
                    NetMessage message = activePeer->beginSendMessage();
                    message.writeStructure(msgData);
                    if (connectionData.payloadData.hasItems())
                        message.writeBytes(connectionData.payloadData.get(), connectionData.payloadData.count());
                    activePeer->endSendMessage(NetChannelType::kReliableOrdered, message);
                }
                else
                {
                    clients.add(newClient(s_nextClientId++, event.sender));
                }
                break;
            case NetEventType::kDisconnected:
            case NetEventType::kTimeout:
                if (isClient())
                {
                    stop();
                    return;
                }
                else
                {
                    NetClient *client = getClient(event.sender);
                    if (client == nullptr)
                        break;
                    client->state = NetConnectionState::kDisconnecting;
                    clients.removeKeepOrder(client);
                    NetInternal::netReplicatorClientDisconnected(client);
                    onClientDisconnected(client);
                    client->state = NetConnectionState::kDisconnected;
                    deleteClient(client);
                    s_activeConnections.remove(event.sender);
                }
                break;
            case NetEventType::kMessage:
            {
                NetClient *client = getClient(event.sender);
                if (client == nullptr && mode != NetManagerMode::kClient)
                {
                    activePeer->recycleMessage(event.message);
                    break;
                }

                const u8 id = event.message.buffer[0];
                if (id < static_cast<u8>(NetMessageId::kMax) && s_messageHandlers[id] != nullptr)
                    s_messageHandlers[id](event, client, activePeer);
                else
                    LogWarn("Unknown network message id=%u", static_cast<u32>(id));
                activePeer->recycleMessage(event.message);
                break;
            }
            default:
                break;
            }
        }

        NetInternal::netReplicatorUpdate();
    }
}
