/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "NetworkSandbox.hpp"

#include <toolkit/debug/Logger.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/net/NetDriver.hpp>
#include <toolkit/net/NetEvent.hpp>
#include <toolkit/net/NetMessage.hpp>
#include <toolkit/net/NetPeer.hpp>
#include <toolkit/net/NetReplicationHierarchy.hpp>
#include <toolkit/net/NetStats.hpp>
#include <toolkit/net/NetStream.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/reflect/RkObject.hpp>
#include <toolkit/threading/Thread.hpp>
#include <toolkit/types/StringView.hpp>
#include <toolkit/utils/CommandLine.hpp>
#include <toolkit/utils/StringUtils.hpp>

namespace toolkit
{
    namespace
    {
        constexpr u8 kPingMessage = 1;
        constexpr u8 kPongMessage = 2;
        constexpr i32 kCommunicationTextLimit = 509;

        struct CommunicationOptions
        {
            TkString address = TkString("127.0.0.1");
            TkString message = TkString("Hello from Sandbox client");
            u16 port = 7777;
            f32 timeoutSeconds = 10.0f;
        };

        class SandboxNetDriver final : public NetDriver
        {
        public:
            TkString driverName() override
            {
                return TkString("SandboxNetDriver");
            }

            bool init(NetPeer *host, const NetConfig &config) override
            {
                peer = host;
                this->config = config;
                initCalled = true;
                return true;
            }

            void dispose() override
            {
                disposeCalled = true;
            }

            bool listen() override
            {
                listenCalled = true;
                return true;
            }

            bool connect() override
            {
                connectCalled = true;
                return true;
            }

            void disconnect() override
            {
                disconnectCalled = true;
            }

            void disconnect(const NetConnection &connection) override
            {
                disconnectedTarget = connection;
            }

            bool popEvent(NetEvent &) override
            {
                return false;
            }

            void sendMessage(NetChannelType channelType, const NetMessage &message) override
            {
                lastChannel = channelType;
                lastLength = message.length;
                sendCount++;
            }

            void sendMessage(NetChannelType channelType, const NetMessage &message, NetConnection target) override
            {
                lastChannel = channelType;
                lastLength = message.length;
                lastTarget = target;
                sendCount++;
            }

            void sendMessage(NetChannelType channelType, const NetMessage &message, const TkArray<NetConnection> &targets) override
            {
                lastChannel = channelType;
                lastLength = message.length;
                lastTargetCount = targets.count();
                sendCount++;
            }

            NetDriverStats getStats() override
            {
                return stats;
            }

            NetDriverStats getStats(NetConnection) override
            {
                return stats;
            }

            NetPeer *peer = nullptr;
            NetConfig config;
            NetDriverStats stats;
            NetChannelType lastChannel = NetChannelType::kNone;
            NetConnection lastTarget{};
            NetConnection disconnectedTarget{};
            u32 lastLength = 0;
            i32 lastTargetCount = 0;
            i32 sendCount = 0;
            bool initCalled = false;
            bool disposeCalled = false;
            bool listenCalled = false;
            bool connectCalled = false;
            bool disconnectCalled = false;
        };

        bool check(bool value, const char *message)
        {
            if (!value)
                LogError("[NetKitSandbox] %s", message);
            return value;
        }

        bool parseOptions(const TkCommandLine &commandLine, CommunicationOptions &options)
        {
            TkString value;
            if (commandLine.get("address", value))
                options.address = value;
            if (commandLine.get("message", value))
                options.message = value;
            if (options.message.length() > kCommunicationTextLimit)
            {
                LogError("[NetKitSandbox] --message is too long. Maximum UTF-8 byte length is %d.", kCommunicationTextLimit);
                return false;
            }
            if (commandLine.get("port", value) &&
                (!TkStringUtils::fromString(TkStringView(value.getText(), value.length()), options.port) || options.port == 0))
            {
                LogError("[NetKitSandbox] Invalid --port value: %s", value.getText());
                return false;
            }
            if (commandLine.get("timeout", value) &&
                (!TkStringUtils::fromString(TkStringView(value.getText(), value.length()), options.timeoutSeconds) ||
                 options.timeoutSeconds <= 0.0f))
            {
                LogError("[NetKitSandbox] Invalid --timeout value: %s", value.getText());
                return false;
            }
            return true;
        }

        NetPeer *createCommunicationPeer(const TkString &address, const u16 port)
        {
            NetConfig config;
            config.address = address;
            config.port = port;
            config.connectionLimit = 4;
            config.messageSize = 512;
            config.messagePoolSize = 256;
            return NetPeer::createPeer(config);
        }

        void writeTextMessage(NetMessage &message, const u8 type, const TkStringView &text)
        {
            message.writeU8(type);
            message.writeString(text);
        }

        bool readTextMessage(NetMessage &message, u8 &type, TkString &text)
        {
            if (message.length < sizeof(u8) + sizeof(u16))
                return false;

            message.position = 0;
            type = message.readU8();
            const u16 textLength = message.readU16();
            if (message.position + textLength > message.length)
                return false;

            text = TkString(reinterpret_cast<const char *>(message.buffer + message.position), textLength);
            return true;
        }

        int runCommunicationServer(const CommunicationOptions &options)
        {
            NetPeer *peer = createCommunicationPeer(TkString("any"), options.port);
            if (peer == nullptr)
            {
                LogError("[NetKitSandbox] Failed to create server peer.");
                return 1;
            }
            if (!peer->listen())
            {
                LogError("[NetKitSandbox] Failed to start server.");
                NetPeer::shutdownPeer(peer);
                return 1;
            }

            LogInfo("[NetKitSandbox] Server listening at 0.0.0.0:%u for %.1f seconds.", static_cast<u32>(options.port), options.timeoutSeconds);
            const double deadline = System::getTimeSeconds() + options.timeoutSeconds;
            bool replied = false;
            while (System::getTimeSeconds() < deadline && !replied)
            {
                NetEvent event{};
                while (peer->popEvent(event))
                {
                    if (event.type == NetEventType::kConnected)
                    {
                        LogInfo("[NetKitSandbox] Client connected, connectionId=%u.", event.sender.connectionId);
                    }
                    else if (event.type == NetEventType::kMessage)
                    {
                        u8 type = 0;
                        TkString text;
                        if (readTextMessage(event.message, type, text) && type == kPingMessage)
                        {
                            LogInfo("[NetKitSandbox] Server received ping: %s", text.getText());
                            NetMessage response = peer->beginSendMessage();
                            writeTextMessage(response, kPongMessage, TkStringView(text.getText(), text.length()));
                            peer->endSendMessage(NetChannelType::kReliableOrdered, response, event.sender);
                            LogInfo("[NetKitSandbox] Server sent pong: %s", text.getText());
                            replied = true;
                        }
                        else
                        {
                            LogError("[NetKitSandbox] Server received an invalid ping message.");
                        }
                        peer->recycleMessage(event.message);
                    }
                }
                if (!replied)
                    TkThread::sleep(1);
            }

            if (replied)
            {
                const double flushDeadline = System::getTimeSeconds() + 0.25;
                while (System::getTimeSeconds() < flushDeadline)
                {
                    NetEvent event{};
                    while (peer->popEvent(event))
                    {
                        if (event.type == NetEventType::kMessage)
                            peer->recycleMessage(event.message);
                    }
                    TkThread::sleep(1);
                }
            }

            NetPeer::shutdownPeer(peer);
            if (!replied)
            {
                LogError("[NetKitSandbox] Server timed out before receiving a ping.");
                return 1;
            }
            LogInfo("[NetKitSandbox] Server communication case passed.");
            return 0;
        }

        int runCommunicationClient(const CommunicationOptions &options)
        {
            NetPeer *peer = createCommunicationPeer(options.address, options.port);
            if (peer == nullptr)
            {
                LogError("[NetKitSandbox] Failed to create client peer.");
                return 1;
            }
            if (!peer->connect())
            {
                LogError("[NetKitSandbox] Failed to start client connection.");
                NetPeer::shutdownPeer(peer);
                return 1;
            }

            LogInfo("[NetKitSandbox] Client connecting to %s:%u.", options.address.getText(), static_cast<u32>(options.port));
            const double deadline = System::getTimeSeconds() + options.timeoutSeconds;
            bool sent = false;
            bool received = false;
            bool disconnected = false;
            while (System::getTimeSeconds() < deadline && !received && !disconnected)
            {
                NetEvent event{};
                while (peer->popEvent(event))
                {
                    if (event.type == NetEventType::kConnected && !sent)
                    {
                        NetMessage request = peer->beginSendMessage();
                        writeTextMessage(request, kPingMessage, TkStringView(options.message.getText(), options.message.length()));
                        peer->endSendMessage(NetChannelType::kReliableOrdered, request);
                        sent = true;
                        LogInfo("[NetKitSandbox] Client sent ping: %s", options.message.getText());
                    }
                    else if (event.type == NetEventType::kMessage)
                    {
                        u8 type = 0;
                        TkString text;
                        const bool valid = readTextMessage(event.message, type, text);
                        received = valid && type == kPongMessage && text == options.message;
                        if (received)
                            LogInfo("[NetKitSandbox] Client received matching pong: %s", text.getText());
                        else
                            LogError("[NetKitSandbox] Client received an invalid pong response.");
                        peer->recycleMessage(event.message);
                    }
                    else if (event.type == NetEventType::kDisconnected || event.type == NetEventType::kTimeout)
                    {
                        LogError("[NetKitSandbox] Client disconnected before receiving pong.");
                        disconnected = true;
                    }
                }
                if (!received)
                    TkThread::sleep(1);
            }

            peer->disconnect();
            NetPeer::shutdownPeer(peer);
            if (!received)
            {
                LogError("[NetKitSandbox] Client timed out before receiving matching pong.");
                return 1;
            }
            LogInfo("[NetKitSandbox] Client communication case passed.");
            return 0;
        }

        int runLocalChecks()
        {
            bool ok = true;

            byte buffer[256] = {};
            NetMessage message(buffer, 1, sizeof(buffer), 0, 0);
            const TkUuid id(1, 2, 3, 4);
            message.writeU32(123);
            message.writeNetId(id);
            message.writeNetName(TkStringView("SandboxName"));
            message.position = 0;
            TkUuid readId;
            TkStringView readName;
            ok &= check(message.readU32() == 123, "NetMessage u32 roundtrip failed");
            message.readNetId(readId);
            message.readNetName(readName);
            ok &= check(readId == id, "NetMessage id roundtrip failed");
            ok &= check(readName == TkStringView("SandboxName"), "NetMessage name roundtrip failed");

            NetStream stream;
            stream.init(4);
            const u64 streamValue = 0x1020304050607080ull;
            stream.writeBytes(&streamValue, sizeof(streamValue));
            stream.setPos(0);
            u64 readStreamValue = 0;
            stream.readBytes(&readStreamValue, sizeof(readStreamValue));
            ok &= check(readStreamValue == streamValue, "NetStream roundtrip failed");

            NetConfig config;
            config.driver = TkMemory::newObject<SandboxNetDriver>();
            config.address = TkString("any");
            config.connectionLimit = 2;
            config.messageSize = 256;
            config.messagePoolSize = 256;
            NetPeer *peer = NetPeer::createPeer(config);
            ok &= check(peer != nullptr, "NetPeer create failed");
            if (peer != nullptr)
            {
                auto *driver = static_cast<SandboxNetDriver *>(peer->driver);
                ok &= check(driver->initCalled, "NetPeer did not initialize driver");
                ok &= check(peer->listen(), "NetPeer listen should succeed");
                NetMessage outgoing = peer->beginSendMessage();
                outgoing.writeU8(77);
                ok &= check(peer->endSendMessage(NetChannelType::kReliable, outgoing), "NetPeer send should succeed");
                ok &= check(driver->sendCount == 1 && driver->lastLength == 1, "NetPeer did not dispatch message");
                NetPeer::shutdownPeer(peer);
            }

            RkObject object(ObjectSpawnParams(TkUuid(10, 11, 12, 13), RkObject::staticTypeHandle()));
            NetReplicationHierarchy hierarchy;
            NetReplicationHierarchyUpdateResult result;
            hierarchy.addObject(&object);
            result.init();
            hierarchy.update(&result);
            ok &= check(result.entries.count() == 1 && result.entries[0].object == &object, "NetReplicationHierarchy update failed");

            LogInfo("[NetKitSandbox] %s", ok ? "all local checks passed" : "local checks failed");
            return ok ? 0 : 1;
        }
    }

    int NetworkSandbox::run(const TkCommandLine &commandLine)
    {
        const TkArray<TkString> &positionals = commandLine.getPositionals();
        const TkString action = positionals.count() > 1 ? positionals[1] : TkString("check");
        if (action == "check")
            return runLocalChecks();

        CommunicationOptions options;
        if (action == "server")
            options.timeoutSeconds = 30.0f;
        if (!parseOptions(commandLine, options))
            return 1;
        if (action == "server")
            return runCommunicationServer(options);
        if (action == "client")
            return runCommunicationClient(options);

        LogError("[NetKitSandbox] Unknown net command: %s", action.getText());
        LogInfo("Use: Sandbox net [check|server|client] [options]");
        return 1;
    }
}
