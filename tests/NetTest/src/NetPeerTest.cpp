#include "common.hpp"

#include <toolkit/net/NetDriver.hpp>
#include <toolkit/net/NetEvent.hpp>
#include <toolkit/net/NetMessage.hpp>
#include <toolkit/net/NetPeer.hpp>
#include <toolkit/net/NetStats.hpp>

namespace
{
    class FakeDriver final : public NetDriver
    {
    public:
        TkString driverName() override
        {
            return TkString("FakeDriver");
        }

        bool init(NetPeer *host, const NetConfig &config) override
        {
            initCalled = true;
            peer = host;
            this->config = config;
            return initResult;
        }

        void dispose() override
        {
            disposeCalled = true;
        }

        bool listen() override
        {
            listenCalled = true;
            return listenResult;
        }

        bool connect() override
        {
            connectCalled = true;
            return connectResult;
        }

        void disconnect() override
        {
            disconnectCalled = true;
        }

        void disconnect(const NetConnection &connection) override
        {
            disconnectedTarget = connection;
        }

        bool popEvent(NetEvent &eventRef) override
        {
            if (!hasEvent)
                return false;
            eventRef = event;
            hasEvent = false;
            return true;
        }

        void sendMessage(NetChannelType channelType, const NetMessage &message) override
        {
            lastChannel = channelType;
            lastLength = message.length;
            sendToServerCount++;
        }

        void sendMessage(NetChannelType channelType, const NetMessage &message, NetConnection target) override
        {
            lastChannel = channelType;
            lastLength = message.length;
            lastTarget = target;
            sendToTargetCount++;
        }

        void sendMessage(NetChannelType channelType, const NetMessage &message, const TkArray<NetConnection> &targets) override
        {
            lastChannel = channelType;
            lastLength = message.length;
            lastTargetsCount = targets.count();
            sendToTargetsCount++;
        }

        NetDriverStats getStats() override
        {
            return stats;
        }

        NetDriverStats getStats(NetConnection) override
        {
            return stats;
        }

        static inline bool initCalled = false;
        static inline bool disposeCalled = false;
        static inline bool listenCalled = false;
        static inline bool connectCalled = false;
        static inline bool disconnectCalled = false;
        static inline bool initResult = true;
        static inline bool listenResult = true;
        static inline bool connectResult = true;
        static inline bool hasEvent = false;
        static inline u32 lastLength = 0;
        static inline i32 lastTargetsCount = 0;
        static inline i32 sendToServerCount = 0;
        static inline i32 sendToTargetCount = 0;
        static inline i32 sendToTargetsCount = 0;
        static inline NetChannelType lastChannel = NetChannelType::kNone;
        static inline NetConnection lastTarget{};
        static inline NetConnection disconnectedTarget{};
        static inline NetEvent event{};
        static inline NetDriverStats stats{};
        static inline NetPeer *peer = nullptr;
        static inline NetConfig config{};
    };

    void resetFakeDriver()
    {
        FakeDriver::initCalled = false;
        FakeDriver::disposeCalled = false;
        FakeDriver::listenCalled = false;
        FakeDriver::connectCalled = false;
        FakeDriver::disconnectCalled = false;
        FakeDriver::initResult = true;
        FakeDriver::listenResult = true;
        FakeDriver::connectResult = true;
        FakeDriver::hasEvent = false;
        FakeDriver::lastLength = 0;
        FakeDriver::lastTargetsCount = 0;
        FakeDriver::sendToServerCount = 0;
        FakeDriver::sendToTargetCount = 0;
        FakeDriver::sendToTargetsCount = 0;
        FakeDriver::lastChannel = NetChannelType::kNone;
        FakeDriver::lastTarget = NetConnection{};
        FakeDriver::disconnectedTarget = NetConnection{};
        FakeDriver::event = NetEvent{};
        FakeDriver::stats = NetDriverStats{};
        FakeDriver::peer = nullptr;
        FakeDriver::config = NetConfig{};
    }

    NetConfig makePeerConfig()
    {
        NetConfig config;
        config.driver = TkMemory::newObject<FakeDriver>();
        config.address = TkString("any");
        config.connectionLimit = 4;
        config.messageSize = 256;
        config.messagePoolSize = 256;
        return config;
    }
}

TEST(NetPeerTest, CreatePeerInitializesDriverAndSendsMessages)
{
    resetFakeDriver();
    NetPeer *peer = NetPeer::createPeer(makePeerConfig());
    ASSERT_NE(peer, nullptr);
    EXPECT_TRUE(FakeDriver::initCalled);
    EXPECT_TRUE(peer->isValid());

    NetMessage message = peer->beginSendMessage();
    message.writeU8(7);
    EXPECT_TRUE(peer->endSendMessage(NetChannelType::kReliable, message));
    EXPECT_EQ(FakeDriver::sendToServerCount, 1);
    EXPECT_EQ(FakeDriver::lastLength, 1u);
    EXPECT_EQ(FakeDriver::lastChannel, NetChannelType::kReliable);

    NetMessage targetMessage = peer->beginSendMessage();
    targetMessage.writeU16(9);
    EXPECT_TRUE(peer->endSendMessage(NetChannelType::kReliableOrdered, targetMessage, NetConnection{3}));
    EXPECT_EQ(FakeDriver::sendToTargetCount, 1);
    EXPECT_EQ(FakeDriver::lastTarget.connectionId, 3u);

    TkArray<NetConnection> targets;
    targets.add(NetConnection{1});
    targets.add(NetConnection{2});
    NetMessage targetsMessage = peer->beginSendMessage();
    targetsMessage.writeU32(10);
    EXPECT_TRUE(peer->endSendMessage(NetChannelType::kUnreliable, targetsMessage, targets));
    EXPECT_EQ(FakeDriver::sendToTargetsCount, 1);
    EXPECT_EQ(FakeDriver::lastTargetsCount, 2);

    NetPeer::shutdownPeer(peer);
    EXPECT_TRUE(FakeDriver::disposeCalled);
}

TEST(NetPeerTest, CreatePeerFailsWhenDriverInitFails)
{
    resetFakeDriver();
    FakeDriver::initResult = false;

    NetPeer *peer = NetPeer::createPeer(makePeerConfig());

    EXPECT_EQ(peer, nullptr);
    EXPECT_TRUE(FakeDriver::initCalled);
}

TEST(NetPeerTest, ListenConnectAndDisconnectUseTrueOnSuccess)
{
    resetFakeDriver();
    NetPeer *peer = NetPeer::createPeer(makePeerConfig());
    ASSERT_NE(peer, nullptr);

    EXPECT_TRUE(peer->listen());
    EXPECT_TRUE(FakeDriver::listenCalled);
    EXPECT_TRUE(peer->connect());
    EXPECT_TRUE(FakeDriver::connectCalled);
    peer->disconnect();
    EXPECT_TRUE(FakeDriver::disconnectCalled);
    peer->disconnect(NetConnection{5});
    EXPECT_EQ(FakeDriver::disconnectedTarget.connectionId, 5u);

    NetPeer::shutdownPeer(peer);
}
