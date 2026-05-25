/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "ENetDriver.hpp"

#include <toolkit/debug/Logger.hpp>
#include <toolkit/net/NetEvent.hpp>
#include <toolkit/net/NetMessage.hpp>
#include <toolkit/net/NetPeer.hpp>
#include <toolkit/net/NetStats.hpp>

#define ENET_IMPLEMENTATION
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <enet/enet.h>
#undef _WINSOCK_DEPRECATED_NO_WARNINGS
#undef SendMessage

namespace toolkit
{
    namespace
    {
        ENetPacketFlag channelTypeToPacketFlag(const NetChannelType channel)
        {
            int flag = 0;
            if (channel == NetChannelType::kReliable || channel == NetChannelType::kReliableOrdered)
                flag |= ENET_PACKET_FLAG_RELIABLE;
            if (channel == NetChannelType::kUnreliable)
                flag |= ENET_PACKET_FLAG_UNSEQUENCED;
            return static_cast<ENetPacketFlag>(flag);
        }

        void sendPacketToPeer(ENetPeer *peer, const NetChannelType channelType, const NetMessage &message)
        {
            if (peer == nullptr || !message.isValid())
                return;
            ENetPacket *packet = enet_packet_create(message.buffer, message.length, channelTypeToPacketFlag(channelType));
            enet_peer_send(peer, 0, packet);
        }

        u32 getPeerId(const ENetPeer *peer)
        {
            if (peer == nullptr || peer->host == nullptr || peer->host->peers == nullptr)
                return 0;
            return static_cast<u32>(peer - peer->host->peers);
        }
    }

    bool ENetDriver::init(NetPeer *host, const NetConfig &config)
    {
        netHost = host;
        this->config = config;
        peerMap.clear();

        if (enet_initialize() != 0)
        {
            LogError("Failed to initialize ENet driver");
            return false;
        }

        LogInfo("Initialized ENet driver");
        return true;
    }

    void ENetDriver::dispose()
    {
        if (peer != nullptr)
            enet_peer_disconnect_now(peer, 0);
        if (host != nullptr)
            enet_host_destroy(host);

        enet_deinitialize();
        peerMap.clear();
        peer = nullptr;
        host = nullptr;
        netHost = nullptr;
        LogInfo("ENet driver stopped");
    }

    bool ENetDriver::listen()
    {
        ENetAddress address{};
        address.port = static_cast<enet_uint16>(config.port);
        address.host = ENET_HOST_ANY;
        if (config.address != "any")
            enet_address_set_host(&address, config.address.getText());

        host = enet_host_create(&address, static_cast<size_t>(config.connectionLimit), 1, 0, 0);
        if (host == nullptr)
        {
            LogError("Failed to initialize ENet host");
            return false;
        }

        LogInfo("Created ENet server");
        return true;
    }

    bool ENetDriver::connect()
    {
        ENetAddress address{};
        address.port = static_cast<enet_uint16>(config.port);
        enet_address_set_host(&address, config.address.getText());

        host = enet_host_create(nullptr, 1, 1, 0, 0);
        if (host == nullptr)
        {
            LogError("Failed to initialize ENet client host");
            return false;
        }

        peer = enet_host_connect(host, &address, 1, 0);
        if (peer == nullptr)
        {
            LogError("Failed to create ENet peer");
            enet_host_destroy(host);
            host = nullptr;
            return false;
        }

        return true;
    }

    void ENetDriver::disconnect()
    {
        if (peer != nullptr)
        {
            enet_peer_disconnect_now(peer, 0);
            peer = nullptr;
        }
    }

    void ENetDriver::disconnect(const NetConnection &connection)
    {
        const auto iterator = peerMap.find(connection.connectionId);
        if (iterator.isEnd())
            return;
        enet_peer_disconnect_now(iterator->value, 0);
        peerMap.remove(iterator);
    }

    bool ENetDriver::popEvent(NetEvent &eventRef)
    {
        TK_ASSERT(host != nullptr);
        ENetEvent event{};
        const int result = enet_host_service(host, &event, 0);
        if (result < 0)
        {
            LogError("Failed to poll ENet events");
            return false;
        }
        if (result == 0)
            return false;

        eventRef.sender.connectionId = getPeerId(event.peer);
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            eventRef.type = NetEventType::kConnected;
            if (isServer())
                peerMap[eventRef.sender.connectionId] = event.peer;
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            eventRef.type = NetEventType::kDisconnected;
            if (isServer())
                peerMap.remove(eventRef.sender.connectionId);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            eventRef.type = NetEventType::kMessage;
            eventRef.message = netHost->createMessage();
            eventRef.message.length = static_cast<u32>(event.packet->dataLength);
            TkMemory::memoryCopy(eventRef.message.buffer, event.packet->data, event.packet->dataLength);
            enet_packet_destroy(event.packet);
            break;
        default:
            return false;
        }

        return true;
    }

    void ENetDriver::sendMessage(const NetChannelType channelType, const NetMessage &message)
    {
        TK_ASSERT(!isServer());
        sendPacketToPeer(peer, channelType, message);
    }

    void ENetDriver::sendMessage(const NetChannelType channelType, const NetMessage &message, const NetConnection target)
    {
        TK_ASSERT(isServer());
        const auto iterator = peerMap.find(target.connectionId);
        if (iterator.isNotEnd() && iterator->value != nullptr && iterator->value->state == ENET_PEER_STATE_CONNECTED)
            sendPacketToPeer(iterator->value, channelType, message);
    }

    void ENetDriver::sendMessage(const NetChannelType channelType, const NetMessage &message, const TkArray<NetConnection> &targets)
    {
        TK_ASSERT(isServer());
        for (const NetConnection &target : targets)
            sendMessage(channelType, message, target);
    }

    NetDriverStats ENetDriver::getStats()
    {
        return getStats(NetConnection{0});
    }

    NetDriverStats ENetDriver::getStats(const NetConnection target)
    {
        NetDriverStats stats;
        ENetPeer *statsPeer = peer;
        if (statsPeer == nullptr)
        {
            const auto iterator = peerMap.find(target.connectionId);
            if (iterator.isNotEnd())
                statsPeer = iterator->value;
        }
        if (statsPeer == nullptr && host != nullptr && host->peerCount > 0)
            statsPeer = host->peers;
        if (statsPeer != nullptr)
        {
            stats.rtt = static_cast<f32>(statsPeer->roundTripTime);
            stats.totalDataSent = statsPeer->outgoingDataTotal;
            stats.totalDataReceived = statsPeer->incomingDataTotal;
        }
        else if (host != nullptr)
        {
            stats.totalDataSent = host->totalSentData;
            stats.totalDataReceived = host->totalReceivedData;
        }
        return stats;
    }
}
