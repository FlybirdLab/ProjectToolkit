/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "PlatformTestCommon.hpp"

#include <toolkit/threading/Thread.hpp>

namespace
{
    struct BoundUdpSocket
    {
        NetSocket *socket = nullptr;
        NetEndPoint endPoint{};
        TkString port;
    };

    BoundUdpSocket bindUdpSocket()
    {
        BoundUdpSocket result{};
        const u32 basePort = 30000u + static_cast<u32>(System::getCurrentProcessId() % 10000u);
        for (u32 offset = 0; offset < 256; ++offset)
        {
            result.port = TkString::format("%u", basePort + offset);
            if (!TkNetwork::createEndPoint("127.0.0.1", result.port, NetIPVersion::kIPv4, result.endPoint, false))
                continue;

            result.socket = TkNetwork::newSocket(NetProtocol::kUdp, NetIPVersion::kIPv4);
            if (result.socket == nullptr)
                continue;

            if (TkNetwork::bind(result.socket, result.endPoint))
                return result;

            TkNetwork::destroySocket(result.socket);
            result.socket = nullptr;
        }
        return result;
    }
}

class PlatformNetworkApiTest : public PlatformTestBase
{
};

TEST_F(PlatformNetworkApiTest, SocketOptionsAndEndpointApis)
{
    NetEndPoint bindEndPoint;
    ASSERT_TRUE(TkNetwork::createEndPoint("", "0", NetIPVersion::kIPv4, bindEndPoint));

    NetSocket *socket = TkNetwork::newSocket(NetProtocol::kUdp, NetIPVersion::kIPv4);
    ASSERT_NE(socket, nullptr);

    ASSERT_TRUE(TkNetwork::setOption(socket, NetSocketOption::kBroadcast, 1));
    i32 optionValue = 0;
    ASSERT_TRUE(TkNetwork::getOption(socket, NetSocketOption::kBroadcast, optionValue));
    EXPECT_EQ(optionValue, 1);

    EXPECT_TRUE(TkNetwork::bind(socket, bindEndPoint));
    EXPECT_TRUE(TkNetwork::destroySocket(socket));

    NetEndPoint ipv4EndPoint;
    ASSERT_TRUE(TkNetwork::createEndPoint("127.0.0.1", "34567", NetIPVersion::kIPv4, ipv4EndPoint, false));
    EXPECT_EQ(ipv4EndPoint.ipVer, NetIPVersion::kIPv4);

    const NetEndPoint ipv6EndPoint = TkNetwork::remapEndPointToIPv6(ipv4EndPoint);
    EXPECT_EQ(ipv6EndPoint.ipVer, NetIPVersion::kIPv6);

    NetEndPoint invalidEndPoint;
    EXPECT_FALSE(TkNetwork::createEndPoint("256.256.256.256", "99999", NetIPVersion::kIPv4, invalidEndPoint, false));
}

TEST_F(PlatformNetworkApiTest, SocketGroupAndUdpReadWriteApis)
{
    BoundUdpSocket receiver = bindUdpSocket();
    ASSERT_NE(receiver.socket, nullptr);

    NetSocket *sender = TkNetwork::newSocket(NetProtocol::kUdp, NetIPVersion::kIPv4);
    ASSERT_NE(sender, nullptr);

    const char payload[] = "network payload";
    ASSERT_EQ(TkNetwork::writeSocket(sender, (byte *)payload, sizeof(payload) - 1, &receiver.endPoint), sizeof(payload) - 1);

    TkArray<byte> buffer;
    buffer.resize(64);
    NetEndPoint senderEndPoint{};
    i32 readBytes = 0;
    for (i32 i = 0; i < 50 && readBytes == 0; ++i)
    {
        readBytes = TkNetwork::readSocket(receiver.socket, buffer.get(), buffer.count(), &senderEndPoint);
        if (readBytes == 0)
            TkThread::sleep(10);
    }

    ASSERT_GT(readBytes, 0);
    EXPECT_EQ(TkString(reinterpret_cast<char *>(buffer.get()), readBytes), "network payload");
    EXPECT_EQ(senderEndPoint.ipVer, NetIPVersion::kIPv4);

    NetSocketGroup group{};
    ASSERT_TRUE(TkNetwork::newSocketGroup(4, &group));

    const i32 index = TkNetwork::addSocket(&group, receiver.socket);
    ASSERT_GE(index, 0);
    EXPECT_EQ(group.count, 1u);

    EXPECT_GE(TkNetwork::poll(&group), 0);

    NetSocket socketCopy{};
    ASSERT_TRUE(TkNetwork::getSocket(&group, index, &socketCopy));
    EXPECT_EQ(socketCopy.protocol, NetProtocol::kUdp);

    NetSocketState state = NetSocketState::kNone;
    ASSERT_TRUE(TkNetwork::getSocketState(&group, index, state));

    TkNetwork::removeSocket(&group, static_cast<u32>(index));
    EXPECT_EQ(group.count, 0u);

    TkNetwork::clearGroup(&group);
    EXPECT_TRUE(TkNetwork::destroySocketGroup(&group));
    EXPECT_TRUE(TkNetwork::destroySocket(sender));
    EXPECT_TRUE(TkNetwork::destroySocket(receiver.socket));
}
