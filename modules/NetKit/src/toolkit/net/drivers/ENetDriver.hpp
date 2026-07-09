/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/collection/HashMap.hpp>
#include <toolkit/net/NetDriver.hpp>

struct _ENetHost;
struct _ENetPeer;

namespace toolkit
{
    class TK_API ENetDriver final : public NetDriver
    {
    public:
        TkString driverName() override
        {
            return TkString("ENetDriver");
        }

        bool init(NetPeer *host, const NetConfig &config) override;
        void dispose() override;
        bool listen() override;
        bool connect() override;
        void disconnect() override;
        void disconnect(const NetConnection &connection) override;
        bool popEvent(NetEvent &eventRef) override;
        void sendMessage(NetChannelType channelType, const NetMessage &message) override;
        void sendMessage(NetChannelType channelType, const NetMessage &message, NetConnection target) override;
        void sendMessage(NetChannelType channelType, const NetMessage &message, const TkArray<NetConnection> &targets) override;
        NetDriverStats getStats() override;
        NetDriverStats getStats(NetConnection target) override;

    private:
        bool isServer() const
        {
            return host != nullptr && peer == nullptr;
        }

    private:
        NetConfig config;
        NetPeer *netHost = nullptr;
        _ENetHost *host = nullptr;
        _ENetPeer *peer = nullptr;
        TkHashMap<u32, _ENetPeer *> peerMap;
    };
}
