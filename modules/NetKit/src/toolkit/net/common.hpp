/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/net/environment.hpp>
#include <toolkit/common.hpp>
#include <toolkit/types/String.hpp>

namespace toolkit
{

    enum class NetChannelType;
    enum class NetEventType;
    enum class NetConnectionState;

    class NetDriver;
    class INetSerializable;
    class NetPeer;
    class NetClient;
    class NetStream;
    class NetReplicationHierarchy;

    struct NetEvent;
    struct NetConnection;
    struct NetMessage;
    struct NetDriverStats;
    struct NetRpcParams;

        // The high-level network connection state.
    enum class NetConnectionState
    {
        // Not connected.
        kOffline = 0,
        // Connection process was started but not yet finished.
        kConnecting,
        // Connection has been made.
        kConnected,
        // Disconnection process was started but not yet finished.
        kDisconnecting,
        // Connection ended.
        kDisconnected,
    };

    // The low-level network channel type for message sending.
    enum class NetChannelType
    {
        // Invalid channel type.
        kNone = 0,
        // Unreliable channel type. Messages can be lost or arrive out-of-order.
        kUnreliable,
        // Unreliable-ordered channel type. Messages can be lost but always arrive in order.
        kUnreliableOrdered,
        // Reliable channel type. Messages won't be lost but may arrive out-of-order.
        kReliable,
        // Reliable-ordered channel type. Messages won't be lost and always arrive in order.
        kReliableOrdered,
    };

    // Low-level network configuration structure. Provides settings for the network driver and all internal components.
    struct TK_API NetConfig
    {
    public:
        NetConfig() = default;
        NetConfig(const NetConfig &other) = default;
        NetConfig(NetConfig &&other) = default;
        NetConfig &operator=(const NetConfig &other) = default;

    public:
        // The network driver instance (implements INetworkDriver) that will be used to create and manage the peer, send and receive messages.
        NetDriver *driver = nullptr;
        // The upper limit on how many peers can join when we're listening.
        u64 connectionLimit = 32;
        // Address used to connect to or listen at.
        TkString address = TkString("127.0.0.1");
        // The port to connect to or listen at.
        u64 port = 7777;
        // The size of a message buffer in bytes. Should be lower than the MTU (maximal transmission unit) - typically 1500 bytes.
        u64 messageSize = 1500;

        // The amount of pooled messages that can be used at once (receiving and sending!).
        u64 messagePoolSize = 2048;
    };

}