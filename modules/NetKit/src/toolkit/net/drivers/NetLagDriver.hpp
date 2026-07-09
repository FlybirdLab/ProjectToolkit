/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/collection/Array.hpp>
#include <toolkit/collection/DataCollection.hpp>
#include <toolkit/net/NetConnection.hpp>
#include <toolkit/net/NetDriver.hpp>
#include <toolkit/net/NetEvent.hpp>
#include <toolkit/net/NetStats.hpp>

namespace toolkit
{
    class TK_API NetLagDriver final : public NetDriver
    {
    public:
        ~NetLagDriver() override;

        TkString driverName() override;
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

        void setDriver(NetDriver *value);

    public:
        i32 lag = 100;

    private:
        struct SendItem
        {
            NetChannelType channelType = NetChannelType::kNone;
            NetConnection target;
            TkArray<NetConnection> targets;
            ByteCollection data;
            double sendTime = 0.0;
            u8 mode = 0;
        };

        struct ReceiveItem
        {
            NetEvent event;
            ByteCollection data;
            double receiveTime = 0.0;
        };

        void update();
        void queueSend(NetChannelType channelType, const NetMessage &message, NetConnection target, const TkArray<NetConnection> *targets, u8 mode);

    private:
        NetDriver *driver = nullptr;
        NetPeer *netHost = nullptr;
        TkArray<SendItem> sendQueue;
        TkArray<ReceiveItem> receiveQueue;
    };
}
