/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/platforms/Network.hpp>

#if TK_POSIX

#include <toolkit/debug/Logger.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/types/String.hpp>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace toolkit
{
    static constexpr i32 kInvalidSocket = -1;
    static constexpr i32 kSockGroupItemSize = sizeof(pollfd);
    static const in6_addr v4MappedPrefix = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00}};

    static_assert(sizeof(NetSocket::data) >= sizeof(i32), "NetSocket::data is not big enough to contain a native socket.");
    static_assert(sizeof(NetEndPoint::data) >= sizeof(sockaddr_in6), "NetEndPoint::data is not big enough to contain sockaddr_in6.");

    static i32 &nativeSocket(NetSocket *socket)
    {
        return *reinterpret_cast<i32 *>(socket->data);
    }

    static i32 nativeSocket(const NetSocket *socket)
    {
        return *reinterpret_cast<const i32 *>(socket->data);
    }

    static i32 getAddrSize(const sockaddr &addr)
    {
        return addr.sa_family == AF_INET6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in);
    }

    static i32 getAddrSizeFromEP(const NetEndPoint &endPoint)
    {
        return endPoint.ipVer == NetIPVersion::kIPv6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in);
    }

    static bool isWouldBlock()
    {
        return errno == EWOULDBLOCK || errno == EAGAIN || errno == EINPROGRESS;
    }

    static bool setNonBlocking(i32 sock)
    {
        const i32 flags = fcntl(sock, F_GETFL, 0);
        return flags >= 0 && fcntl(sock, F_SETFL, flags | O_NONBLOCK) == 0;
    }

    static bool createEndPointFromAddr(sockaddr *addr, NetEndPoint &endPoint)
    {
        if (addr->sa_family != AF_INET && addr->sa_family != AF_INET6)
        {
            LogError("[UnixNet] Unable to create endpoint, sockaddr must be INET or INET6. Family: %i", addr->sa_family);
            return false;
        }

        endPoint.ipVer = addr->sa_family == AF_INET6 ? NetIPVersion::kIPv6 : NetIPVersion::kIPv4;
        TkMemory::memoryCopy(endPoint.data, addr, getAddrSize(*addr));
        return true;
    }

    static bool convertSockOptToNative(NetSocketOption option, i32 *level, i32 *name)
    {
        switch (option)
        {
#define SOCKOPT(OPTENUM, OPTLEVEL, OPTNAME) \
    case OPTENUM:                           \
        *level = OPTLEVEL;                  \
        *name = OPTNAME;                    \
        return true;
            SOCKOPT(NetSocketOption::kDebug, SOL_SOCKET, SO_DEBUG)
            SOCKOPT(NetSocketOption::kReuseAddr, SOL_SOCKET, SO_REUSEADDR)
            SOCKOPT(NetSocketOption::kKeepAlive, SOL_SOCKET, SO_KEEPALIVE)
            SOCKOPT(NetSocketOption::kDontRoute, SOL_SOCKET, SO_DONTROUTE)
            SOCKOPT(NetSocketOption::kBroadcast, SOL_SOCKET, SO_BROADCAST)
#ifdef SO_USELOOPBACK
            SOCKOPT(NetSocketOption::kUseLoopback, SOL_SOCKET, SO_USELOOPBACK)
#endif
            SOCKOPT(NetSocketOption::kLinger, SOL_SOCKET, SO_LINGER)
            SOCKOPT(NetSocketOption::kOOBInline, SOL_SOCKET, SO_OOBINLINE)
            SOCKOPT(NetSocketOption::kSendBuffer, SOL_SOCKET, SO_SNDBUF)
            SOCKOPT(NetSocketOption::kRecvBuffer, SOL_SOCKET, SO_RCVBUF)
            SOCKOPT(NetSocketOption::kSendTimeout, SOL_SOCKET, SO_SNDTIMEO)
            SOCKOPT(NetSocketOption::kRecvTimeout, SOL_SOCKET, SO_RCVTIMEO)
            SOCKOPT(NetSocketOption::kError, SOL_SOCKET, SO_ERROR)
#ifdef TCP_NODELAY
            SOCKOPT(NetSocketOption::kNoDelay, IPPROTO_TCP, TCP_NODELAY)
#endif
            SOCKOPT(NetSocketOption::kIPv6Only, IPPROTO_IPV6, IPV6_V6ONLY)
#ifdef IP_MTU
            SOCKOPT(NetSocketOption::kMtu, IPPROTO_IP, IP_MTU)
#endif
            SOCKOPT(NetSocketOption::kType, SOL_SOCKET, SO_TYPE)
#undef SOCKOPT
        default:
            *level = 0;
            *name = 0;
            return false;
        }
    }

    NetSocket *TkNetwork::newSocket(NetProtocol proto, NetIPVersion ipv)
    {
        auto *socket = TkMemory::newObject<NetSocket>(TkMemory::getDefault());
        socket->protocol = proto;
        socket->ipVer = ipv;

        const i32 family = ipv == NetIPVersion::kIPv6 ? AF_INET6 : AF_INET;
        const i32 type = proto == NetProtocol::kTcp ? SOCK_STREAM : SOCK_DGRAM;
        const i32 protocol = proto == NetProtocol::kTcp ? IPPROTO_TCP : IPPROTO_UDP;
        const i32 sock = ::socket(family, type, protocol);
        if (sock < 0)
        {
            LogError("[UnixNet] Can't create native socket. Error: %s", strerror(errno));
            TkMemory::deletePtr(socket);
            return nullptr;
        }

        if (!setNonBlocking(sock))
        {
            LogError("[UnixNet] Can't set socket to non-blocking mode. Error: %s", strerror(errno));
            close(sock);
            TkMemory::deletePtr(socket);
            return nullptr;
        }

        nativeSocket(socket) = sock;
        return socket;
    }

    bool TkNetwork::destroySocket(NetSocket *socket)
    {
        if (!socket)
            return false;
        const i32 sock = nativeSocket(socket);
        if (sock != kInvalidSocket)
            close(sock);
        TkMemory::deletePtr(socket);
        return true;
    }

    bool TkNetwork::setOption(NetSocket *socket, NetSocketOption option, i32 value)
    {
        i32 level = 0;
        i32 name = 0;
        if (!convertSockOptToNative(option, &level, &name))
            return false;
        if (setsockopt(nativeSocket(socket), level, name, reinterpret_cast<char *>(&value), sizeof(value)) != 0)
        {
            LogWarn("[UnixNet] Unable to set socket option. Socket: %i Error: %s", nativeSocket(socket), strerror(errno));
            return false;
        }
        return true;
    }

    bool TkNetwork::getOption(NetSocket *socket, NetSocketOption option, i32 &value)
    {
        i32 level = 0;
        i32 name = 0;
        if (!convertSockOptToNative(option, &level, &name))
            return false;
        socklen_t size = sizeof(value);
        if (getsockopt(nativeSocket(socket), level, name, reinterpret_cast<char *>(&value), &size) != 0)
        {
            LogWarn("[UnixNet] Unable to get socket option. Socket: %i Error: %s", nativeSocket(socket), strerror(errno));
            return false;
        }
        return true;
    }

    bool TkNetwork::connect(NetSocket *socket, NetEndPoint &endPoint)
    {
        if (::connect(nativeSocket(socket), reinterpret_cast<const sockaddr *>(endPoint.data), getAddrSizeFromEP(endPoint)) != 0)
        {
            if (isWouldBlock())
                return true;
            LogError("[UnixNet] Unable to connect socket. Socket: %i Error: %s", nativeSocket(socket), strerror(errno));
            return false;
        }
        return true;
    }

    bool TkNetwork::bind(NetSocket *socket, NetEndPoint &endPoint)
    {
        if (socket->ipVer != endPoint.ipVer)
        {
            LogError("[UnixNet] Can't bind socket to endpoint, socket ip version differs. Socket: %i", nativeSocket(socket));
            return false;
        }
        if (::bind(nativeSocket(socket), reinterpret_cast<const sockaddr *>(endPoint.data), getAddrSizeFromEP(endPoint)) != 0)
        {
            LogError("[UnixNet] Unable to bind socket. Socket: %i Error: %s", nativeSocket(socket), strerror(errno));
            return false;
        }
        return true;
    }

    bool TkNetwork::listen(NetSocket *socket, u16 queueSize)
    {
        if (::listen(nativeSocket(socket), static_cast<i32>(queueSize)) != 0)
        {
            LogError("[UnixNet] Unable to listen. Socket: %i Error: %s", nativeSocket(socket), strerror(errno));
            return false;
        }
        return true;
    }

    bool TkNetwork::accept(NetSocket *serverSocket, NetSocket *newSocket, NetEndPoint &newEndPoint)
    {
        if (serverSocket->protocol != NetProtocol::kTcp)
        {
            LogWarn("[UnixNet] Can't accept connection on UDP socket. Socket: %i", nativeSocket(serverSocket));
            return false;
        }

        sockaddr_in6 addr {};
        socklen_t size = sizeof(addr);
        const i32 sock = ::accept(nativeSocket(serverSocket), reinterpret_cast<sockaddr *>(&addr), &size);
        if (sock < 0)
        {
            if (isWouldBlock())
                return true;
            LogWarn("[UnixNet] Unable to accept incoming connection. Socket: %i Error: %s", nativeSocket(serverSocket), strerror(errno));
            return false;
        }

        if (!setNonBlocking(sock))
        {
            close(sock);
            return false;
        }

        nativeSocket(newSocket) = sock;
        newSocket->protocol = serverSocket->protocol;
        newSocket->ipVer = serverSocket->ipVer;
        return createEndPointFromAddr(reinterpret_cast<sockaddr *>(&addr), newEndPoint);
    }

    bool TkNetwork::isReadable(NetSocket *socket)
    {
        pollfd entry {};
        entry.fd = nativeSocket(socket);
        entry.events = POLLIN;
        if (::poll(&entry, 1, 0) < 0)
            return false;
        return (entry.revents & POLLIN) != 0;
    }

    bool TkNetwork::isWritable(NetSocket *socket)
    {
        pollfd entry {};
        entry.fd = nativeSocket(socket);
        entry.events = POLLOUT;
        if (::poll(&entry, 1, 0) < 0)
            return false;
        return (entry.revents & POLLOUT) != 0;
    }

    bool TkNetwork::newSocketGroup(u32 capacity, NetSocketGroup *group)
    {
        group->data = static_cast<byte *>(TkMemory::allocate(TkMemory::getDefault(), capacity * kSockGroupItemSize, alignof(pollfd)));
        if (!group->data)
            return false;
        group->count = 0;
        group->capacity = capacity;
        for (u32 i = 0; i < group->capacity; ++i)
            reinterpret_cast<pollfd *>(group->data + i * kSockGroupItemSize)->fd = kInvalidSocket;
        return true;
    }

    bool TkNetwork::destroySocketGroup(NetSocketGroup *group)
    {
        if (!group || !group->data)
            return false;
        TkMemory::free(TkMemory::getDefault(), group->data);
        group->data = nullptr;
        group->count = 0;
        group->capacity = 0;
        return true;
    }

    i32 TkNetwork::poll(NetSocketGroup *group)
    {
        return ::poll(reinterpret_cast<pollfd *>(group->data), group->capacity, 0);
    }

    bool TkNetwork::getSocketState(NetSocketGroup *group, u32 index, NetSocketState &state)
    {
        if (index >= group->capacity)
            return false;
        pollfd *entry = reinterpret_cast<pollfd *>(group->data + index * kSockGroupItemSize);
        state = NetSocketState::kNone;
        if (entry->revents & POLLERR)
            state |= NetSocketState::kError;
        if (entry->revents & POLLHUP)
            state |= NetSocketState::kDisconnected;
        if (entry->revents & POLLNVAL)
            state |= NetSocketState::kInvalid;
        if (entry->revents & POLLIN)
            state |= NetSocketState::kReadable;
        if (entry->revents & POLLOUT)
            state |= NetSocketState::kWriteable;
        return true;
    }

    i32 TkNetwork::addSocket(NetSocketGroup *group, NetSocket *socket)
    {
        if (group->count >= group->capacity)
            return -1;
        for (u32 i = 0; i < group->capacity; ++i)
        {
            pollfd *entry = reinterpret_cast<pollfd *>(group->data + i * kSockGroupItemSize);
            if (entry->fd == kInvalidSocket)
            {
                entry->fd = nativeSocket(socket);
                entry->events = POLLIN | POLLOUT;
                entry->revents = 0;
                ++group->count;
                return static_cast<i32>(i);
            }
        }
        return -1;
    }

    bool TkNetwork::getSocket(NetSocketGroup *group, u32 index, NetSocket *socket)
    {
        if (index >= group->capacity)
            return false;
        pollfd *entry = reinterpret_cast<pollfd *>(group->data + index * kSockGroupItemSize);
        if (entry->fd == kInvalidSocket)
            return false;
        nativeSocket(socket) = entry->fd;
        i32 type = 0;
        if (!getOption(socket, NetSocketOption::kType, type))
            return false;
        socket->protocol = type == SOCK_DGRAM ? NetProtocol::kUdp : type == SOCK_STREAM ? NetProtocol::kTcp : NetProtocol::kUndefined;
        return true;
    }

    void TkNetwork::removeSocket(NetSocketGroup *group, u32 index)
    {
        if (index >= group->capacity)
            return;
        pollfd *entry = reinterpret_cast<pollfd *>(group->data + index * kSockGroupItemSize);
        if (entry->fd != kInvalidSocket)
        {
            entry->fd = kInvalidSocket;
            entry->revents = 0;
            --group->count;
        }
    }

    bool TkNetwork::removeSocket(NetSocketGroup *group, NetSocket *socket)
    {
        const i32 sock = nativeSocket(socket);
        for (u32 i = 0; i < group->capacity; ++i)
        {
            pollfd *entry = reinterpret_cast<pollfd *>(group->data + i * kSockGroupItemSize);
            if (entry->fd == sock)
            {
                removeSocket(group, i);
                return true;
            }
        }
        return false;
    }

    void TkNetwork::clearGroup(NetSocketGroup *group)
    {
        for (u32 i = 0; i < group->capacity; ++i)
        {
            pollfd *entry = reinterpret_cast<pollfd *>(group->data + i * kSockGroupItemSize);
            entry->fd = kInvalidSocket;
            entry->revents = 0;
        }
        group->count = 0;
    }

    i32 TkNetwork::writeSocket(NetSocket *socket, byte *data, u32 length, NetEndPoint *endPoint)
    {
        if (endPoint && socket->ipVer != endPoint->ipVer)
            return -1;
        ssize_t result = 0;
        if (endPoint && socket->protocol == NetProtocol::kUdp)
            result = sendto(nativeSocket(socket), data, length, 0, reinterpret_cast<const sockaddr *>(endPoint->data), getAddrSizeFromEP(*endPoint));
        else
            result = send(nativeSocket(socket), data, length, 0);
        if (result < 0)
            return isWouldBlock() ? 0 : -1;
        return static_cast<i32>(result);
    }

    i32 TkNetwork::readSocket(NetSocket *socket, byte *buffer, u32 bufferSize, NetEndPoint *endPoint)
    {
        ssize_t result = 0;
        if (endPoint)
        {
            sockaddr_in6 addr {};
            socklen_t addrSize = sizeof(addr);
            result = recvfrom(nativeSocket(socket), buffer, bufferSize, 0, reinterpret_cast<sockaddr *>(&addr), &addrSize);
            if (result >= 0 && !createEndPointFromAddr(reinterpret_cast<sockaddr *>(&addr), *endPoint))
                return -1;
        }
        else
        {
            result = recv(nativeSocket(socket), buffer, bufferSize, 0);
        }
        if (result < 0)
            return isWouldBlock() ? 0 : -1;
        return static_cast<i32>(result);
    }

    bool TkNetwork::createEndPoint(const TkString &address, const TkString &port, NetIPVersion ipv, NetEndPoint &endPoint, bool bindable)
    {
        addrinfo hints {};
        hints.ai_family = ipv == NetIPVersion::kIPv6 ? AF_INET6 : ipv == NetIPVersion::kIPv4 ? AF_INET : AF_UNSPEC;
        hints.ai_flags = AI_ADDRCONFIG | AI_V4MAPPED;
        if (bindable)
            hints.ai_flags |= AI_PASSIVE;

        addrinfo *info = nullptr;
        const i32 status = getaddrinfo(address.isEmpty() ? nullptr : address.getText(), port.isEmpty() ? nullptr : port.getText(), &hints, &info);
        if (status != 0 || !info)
        {
            LogError("[UnixNet] Unable to resolve address: %s::%s Error: %s", address.getText(), port.getText(), gai_strerror(status));
            return false;
        }

        const bool result = createEndPointFromAddr(info->ai_addr, endPoint);
        freeaddrinfo(info);
        return result;
    }

    NetEndPoint TkNetwork::remapEndPointToIPv6(NetEndPoint &endPoint)
    {
        if (endPoint.ipVer == NetIPVersion::kIPv6)
            return endPoint;

        NetEndPoint ipv6;
        auto *addr4 = reinterpret_cast<sockaddr_in *>(endPoint.data);
        auto *addr6 = reinterpret_cast<sockaddr_in6 *>(ipv6.data);
        TkMemory::memorySet(addr6, sizeof(sockaddr_in6), 0);
        addr6->sin6_family = AF_INET6;
        addr6->sin6_addr = v4MappedPrefix;
        addr6->sin6_port = addr4->sin_port;
        TkMemory::memoryCopy(&addr6->sin6_addr.s6_addr[12], &addr4->sin_addr, 4);
        ipv6.ipVer = NetIPVersion::kIPv6;
        return ipv6;
    }
}

#endif
