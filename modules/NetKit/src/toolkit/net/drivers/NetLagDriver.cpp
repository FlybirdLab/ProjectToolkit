/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "NetLagDriver.hpp"

#include "ENetDriver.hpp"
#include <toolkit/debug/Logger.hpp>
#include <toolkit/net/NetEvent.hpp>
#include <toolkit/net/NetMessage.hpp>
#include <toolkit/net/NetPeer.hpp>
#include <toolkit/net/NetStats.hpp>
#include <toolkit/platforms/System.hpp>

namespace toolkit
{
    NetLagDriver::~NetLagDriver()
    {
        dispose();
    }

    void NetLagDriver::setDriver(NetDriver *value)
    {
        if (driver == value)
            return;
        if (driver != nullptr)
        {
            driver->dispose();
            TkMemory::deletePtr(driver);
        }
        driver = value;
    }

    TkString NetLagDriver::driverName()
    {
        return TkString("NetLagDriver");
    }

    bool NetLagDriver::init(NetPeer *host, const NetConfig &config)
    {
        netHost = host;
        if (driver == nullptr)
            driver = TkMemory::newObject<ENetDriver>();
        if (driver == nullptr)
        {
            LogError("Missing nested driver for NetLagDriver");
            return false;
        }
        return driver->init(host, config);
    }

    void NetLagDriver::dispose()
    {
        if (driver != nullptr)
        {
            driver->dispose();
            TkMemory::deletePtr(driver);
            driver = nullptr;
        }
        sendQueue.clear();
        receiveQueue.clear();
        netHost = nullptr;
    }

    bool NetLagDriver::listen()
    {
        return driver != nullptr && driver->listen();
    }

    bool NetLagDriver::connect()
    {
        return driver != nullptr && driver->connect();
    }

    void NetLagDriver::disconnect()
    {
        if (driver != nullptr)
            driver->disconnect();
    }

    void NetLagDriver::disconnect(const NetConnection &connection)
    {
        if (driver != nullptr)
            driver->disconnect(connection);
    }

    bool NetLagDriver::popEvent(NetEvent &eventRef)
    {
        update();
        const double now = System::getTimeSeconds();
        for (i32 i = 0; i < receiveQueue.count(); i++)
        {
            ReceiveItem &item = receiveQueue[i];
            if (item.receiveTime > now)
                continue;

            eventRef = item.event;
            if (item.data.length() > 0)
            {
                eventRef.message = netHost->createMessage();
                eventRef.message.length = static_cast<u32>(item.data.length());
                eventRef.message.writeBytes(item.data.get(), item.data.length());
                eventRef.message.position = 0;
            }
            receiveQueue.removeAtKeepOrder(i);
            return true;
        }
        return false;
    }

    void NetLagDriver::sendMessage(const NetChannelType channelType, const NetMessage &message)
    {
        if (lag <= 0)
            driver->sendMessage(channelType, message);
        else
            queueSend(channelType, message, NetConnection{0}, nullptr, 0);
    }

    void NetLagDriver::sendMessage(const NetChannelType channelType, const NetMessage &message, const NetConnection target)
    {
        if (lag <= 0)
            driver->sendMessage(channelType, message, target);
        else
            queueSend(channelType, message, target, nullptr, 1);
    }

    void NetLagDriver::sendMessage(const NetChannelType channelType, const NetMessage &message, const TkArray<NetConnection> &targets)
    {
        if (lag <= 0)
            driver->sendMessage(channelType, message, targets);
        else
            queueSend(channelType, message, NetConnection{0}, &targets, 2);
    }

    NetDriverStats NetLagDriver::getStats()
    {
        return driver != nullptr ? driver->getStats() : NetDriverStats();
    }

    NetDriverStats NetLagDriver::getStats(NetConnection target)
    {
        return driver != nullptr ? driver->getStats(target) : NetDriverStats();
    }

    void NetLagDriver::queueSend(const NetChannelType channelType, const NetMessage &message, const NetConnection target, const TkArray<NetConnection> *targets, const u8 mode)
    {
        SendItem &item = sendQueue.addOne();
        item.channelType = channelType;
        item.target = target;
        item.mode = mode;
        item.sendTime = System::getTimeSeconds() + static_cast<double>(lag) / 1000.0;
        if (message.length > 0)
            item.data.copy(message.buffer, message.length);
        if (targets != nullptr)
            item.targets = *targets;
    }

    void NetLagDriver::update()
    {
        if (driver == nullptr || netHost == nullptr)
            return;

        const double now = System::getTimeSeconds();
        for (i32 i = sendQueue.count() - 1; i >= 0; --i)
        {
            SendItem &item = sendQueue[i];
            if (item.sendTime > now)
                continue;

            NetMessage message(item.data.get(), 0, static_cast<u32>(item.data.length()), static_cast<u32>(item.data.length()), 0);
            if (item.mode == 0)
                driver->sendMessage(item.channelType, message);
            else if (item.mode == 1)
                driver->sendMessage(item.channelType, message, item.target);
            else
                driver->sendMessage(item.channelType, message, item.targets);
            sendQueue.removeAtKeepOrder(i);
        }

        NetEvent event;
        while (driver->popEvent(event))
        {
            ReceiveItem &item = receiveQueue.addOne();
            item.event = event;
            item.receiveTime = now + static_cast<double>(lag) / 1000.0;
            if (event.type == NetEventType::kMessage && event.message.isValid())
            {
                item.data.copy(event.message.buffer, event.message.length);
                netHost->recycleMessage(event.message);
                item.event.message = NetMessage();
            }
        }
    }
}
