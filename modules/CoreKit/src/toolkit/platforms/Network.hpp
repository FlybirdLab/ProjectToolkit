/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/Enum.hpp>

namespace toolkit
{
    enum class NetProtocol : u8
    {
        kUndefined,
        kUdp,
        kTcp,
    };

    enum class NetIPVersion : u8
    {
        kUndefined,
        kIPv4,
        kIPv6,
    };

    struct NetSocket
    {
        NetProtocol protocol = NetProtocol::kUndefined;
        NetIPVersion ipVer = NetIPVersion::kUndefined;
        byte data[8] = {};
    };

    struct NetEndPoint
    {
        NetIPVersion ipVer = NetIPVersion::kUndefined;
        byte data[28] = {};
    };

    enum class NetSocketOption : u8
    {
        kDebug,
        kReuseAddr,
        kKeepAlive,
        kDontRoute,
        kBroadcast,
        kUseLoopback,
        kLinger,
        kOOBInline,
        kSendBuffer,
        kRecvBuffer,
        kSendTimeout,
        kRecvTimeout,
        kError,
        kNoDelay,
        kIPv6Only,
        kMtu,
        kType,
    };

    enum class NetSocketState
    {
        kNone = 0,
        kError = 1 << 0,
        kInvalid = 1 << 1,
        kDisconnected = 1 << 2,
        kReadable = 1 << 3,
        kWriteable = 1 << 4,
    };
    TK_DECLARE_ENUM_OPERATORS(NetSocketState);

    struct NetSocketGroup
    {
        u32 count = 0;
        u32 capacity = 0;
        byte* data = nullptr;
    };

    class TkNetwork
    {
    public:
        // Creates a new native socket.
        static NetSocket* newSocket(NetProtocol proto, NetIPVersion ipv);
        // Close a native socket.
        static bool destroySocket(NetSocket *socket);
        // Set the specified socket option.
        static bool setOption(NetSocket *socket, NetSocketOption option, i32 value);
        // Get the specified socket option.
        static bool getOption(NetSocket *socket, NetSocketOption option, i32 &value);
        // Connect a socket to the specified end point.
        static bool connect(NetSocket *socket, NetEndPoint &endPoint);
        // Bind a socket to the specified end point.
        static bool bind(NetSocket *socket, NetEndPoint &endPoint);
        // Listen for incoming connection.
        static bool listen(NetSocket *socket, u16 queueSize);
        // Accept a pending connection.
        static bool accept(NetSocket *serverSocket, NetSocket *newSocket, NetEndPoint &newEndPoint);
        // Check for socket readability.
        static bool isReadable(NetSocket *socket);
        // Check for socket writeability.
        static bool isWritable(NetSocket *socket);

    public:
        // Create a socket group. It allocate memory based on the desired capacity.
        static bool newSocketGroup(u32 capacity, NetSocketGroup* group);
        // Destroy the socket group, and free the allocated memory.
        static bool destroySocketGroup(NetSocketGroup* group);
        // Update sockets states.
        static i32 poll(NetSocketGroup* group);
        // Retrier socket state.
        static bool getSocketState(NetSocketGroup* group, u32 index, NetSocketState &state);
        // Add a socket to a group.
        static i32 addSocket(NetSocketGroup* group, NetSocket *socket);
        // Get a socket by index. Some data like socket IPVersion might be undefined.
        static bool getSocket(NetSocketGroup* group, u32 index, NetSocket *socket);
        // Remove the socket at the specified index.
        static void removeSocket(NetSocketGroup* group, u32 index);
        // Remove the socket if present.
        static bool removeSocket(NetSocketGroup* group, NetSocket *socket);
        // Clear the socket group.
        static void clearGroup(NetSocketGroup* group);
        // Write data to the socket.
        static i32 writeSocket(NetSocket* socket, byte *data, u32 length, NetEndPoint *endPoint = nullptr);
        // Read data on the socket.
        static i32 readSocket(NetSocket* socket, byte *buffer, u32 bufferSize, NetEndPoint *endPoint = nullptr);
        // Creates an end point.
        static bool createEndPoint(const TkString &address, const TkString &port, NetIPVersion ipv, NetEndPoint &endPoint, bool bindable = true);
        // Remap an ipv4 end point to an ipv6 one.
        static NetEndPoint remapEndPointToIPv6(NetEndPoint &endPoint);
    };
}