/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: khfahqp khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/platforms/Network.hpp>

#if TK_POSIX

#include <toolkit/debug/Logger.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/types/String.hpp>

#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace toolkit
{
    static constexpr i32 kInvalidSocket = -1;
    static constexpr u32 kSockGroupItemSize = 16;

    static_assert(sizeof(NetSocket::data) >= sizeof(i32), "NetSocket::data is not big enough to contain a Unix socket fd");
    static_assert(sizeof(NetEndPoint::data) >= sizeof(sockaddr_in6), "NetEndPoint::data is not big enough to contain sockaddr_in6");
    static_assert(kSockGroupItemSize >= sizeof(pollfd), "kSockGroupItemSize is not big enough to contain pollfd");

    static i32 &socketFd(NetSocket *socket)
    {
        return *reinterpret_cast<i32 *>(socket->data);
    }

    static i32 getAddrSize(const sockaddr &addr)
    {
        return addr.sa_family == AF_INET6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in);
    }

    static i32 getAddrSizeFromEP(NetEndPoint &endPoint)
    {
        return endPoint.ipVer == NetIPVersion::kIPv6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in);
    }

    static bool isWouldBlock(i32 error)
    {
        return error == EAGAIN || error == EWOULDBLOCK || error == EINPROGRESS;
    }

    static bool createEndPointFromAddr(sockaddr *addr, NetEndPoint &endPoint)
    {
        if (addr->sa_family != AF_INET && addr->sa_family != AF_INET6)
        {
            LogError("[UnixNet] Unable to create endpoint, sockaddr family=%i", addr->sa_family);
            return false;
        }

        const i32 size = getAddrSize(*addr);
        endPoint.ipVer = addr->sa_family == AF_INET6 ? NetIPVersion::kIPv6 : NetIPVersion::kIPv4;
        TkMemory::memoryClear(endPoint.data, sizeof(endPoint.data));
        TkMemory::memoryCopy(endPoint.data, addr, size);
        return true;
    }

    static void convertSockOptToNative(NetSocketOption option, i32 *level, i32 *name)
    {
        switch (option)
        {
#define SOCKOPT(OPTENUM, OPTLEVEL, OPTNAME) \
    case OPTENUM:                           \
        *level = OPTLEVEL;                  \
        *name = OPTNAME;                    \
        break;
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
            break;
        }
    }

    NetSocket *TkNetwork::newSocket(NetProtocol proto, NetIPVersion ipv)
    {
        auto *socket = TkMemory::newObject<NetSocket>(TkMemory::getDefault());
        socket->protocol = proto;
        socket->ipVer = ipv;

        const i32 domain = ipv == NetIPVersion::kIPv6 ? AF_INET6 : AF_INET;
        const i32 type = proto == NetProtocol::kTcp ? SOCK_STREAM : SOCK_DGRAM;
        const i32 protocol = proto == NetProtocol::kTcp ? IPPROTO_TCP : IPPROTO_UDP;
        i32 fd = ::socket(domain, type, protocol);
        if (fd == kInvalidSocket)
        {
            LogError("[UnixNet] Can't create native socket. errno=%i", errno);
            TkMemory::deletePtr(socket);
            return nullptr;
        }

        const i32 flags = ::fcntl(fd, F_GETFL, 0);
        if (flags != -1)
            ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);

        TkMemory::memoryCopy(socket->data, &fd, sizeof(fd));
        return socket;
    }

    bool TkNetwork::destroySocket(NetSocket *socket)
    {
        if (!socket)
            return false;
        const i32 fd = socketFd(socket);
        if (fd != kInvalidSocket)
            ::close(fd);
        TkMemory::deletePtr(socket);
        return true;
    }

    bool TkNetwork::setOption(NetSocket *socket, NetSocketOption option, i32 value)
    {
        i32 level = 0, name = 0;
        convertSockOptToNative(option, &level, &name);
        if (::setsockopt(socketFd(socket), level, name, reinterpret_cast<char *>(&value), sizeof(value)) == -1)
        {
            LogWarn("[UnixNet] Unable to set socket option. fd=%i errno=%i", socketFd(socket), errno);
            return false;
        }
        return true;
    }

    bool TkNetwork::getOption(NetSocket *socket, NetSocketOption option, i32 &value)
    {
        i32 level = 0, name = 0;
        convertSockOptToNative(option, &level, &name);
        socklen_t size = sizeof(value);
        if (::getsockopt(socketFd(socket), level, name, reinterpret_cast<char *>(&value), &size) == -1)
        {
            LogWarn("[UnixNet] Unable to get socket option. fd=%i errno=%i", socketFd(socket), errno);
            return false;
        }
        return true;
    }

    bool TkNetwork::connect(NetSocket *socket, NetEndPoint &endPoint)
    {
        if (::connect(socketFd(socket), reinterpret_cast<const sockaddr *>(endPoint.data), getAddrSizeFromEP(endPoint)) == -1)
        {
            if (isWouldBlock(errno))
                return true;
            LogError("[UnixNet] Unable to connect socket. fd=%i errno=%i", socketFd(socket), errno);
            return false;
        }
        return true;
    }

    bool TkNetwork::bind(NetSocket *socket, NetEndPoint &endPoint)
    {
        if (socket->ipVer != endPoint.ipVer)
        {
            LogError("[UnixNet] Socket ip version does not match endpoint");
            return false;
        }
        if (::bind(socketFd(socket), reinterpret_cast<const sockaddr *>(endPoint.data), getAddrSizeFromEP(endPoint)) == -1)
        {
            LogError("[UnixNet] Unable to bind socket. fd=%i errno=%i", socketFd(socket), errno);
            return false;
        }
        return true;
    }

    bool TkNetwork::listen(NetSocket *socket, u16 queueSize)
    {
        if (::listen(socketFd(socket), static_cast<i32>(queueSize)) == -1)
        {
            LogError("[UnixNet] Unable to listen. fd=%i errno=%i", socketFd(socket), errno);
            return false;
        }
        return true;
    }

    bool TkNetwork::accept(NetSocket *serverSocket, NetSocket *newSocket, NetEndPoint &newEndPoint)
    {
        if (serverSocket->protocol != NetProtocol::kTcp)
            return false;

        sockaddr_in6 addr{};
        socklen_t size = sizeof(addr);
        const i32 fd = ::accept(socketFd(serverSocket), reinterpret_cast<sockaddr *>(&addr), &size);
        if (fd == kInvalidSocket)
        {
            if (isWouldBlock(errno))
                return true;
            LogWarn("[UnixNet] Unable to accept incoming connection. errno=%i", errno);
            return false;
        }

        TkMemory::memoryCopy(newSocket->data, &fd, sizeof(fd));
        newSocket->protocol = serverSocket->protocol;
        newSocket->ipVer = serverSocket->ipVer;
        return createEndPointFromAddr(reinterpret_cast<sockaddr *>(&addr), newEndPoint);
    }

    bool TkNetwork::isReadable(NetSocket *socket)
    {
        pollfd entry{socketFd(socket), POLLIN, 0};
        return ::poll(&entry, 1, 0) > 0 && (entry.revents & POLLIN) != 0;
    }

    bool TkNetwork::isWritable(NetSocket *socket)
    {
        pollfd entry{socketFd(socket), POLLOUT, 0};
        return ::poll(&entry, 1, 0) > 0 && (entry.revents & POLLOUT) != 0;
    }

    bool TkNetwork::newSocketGroup(u32 capacity, NetSocketGroup *group)
    {
        group->data = static_cast<byte *>(TkMemory::allocate(TkMemory::getDefault(), capacity * kSockGroupItemSize, 16));
        if (!group->data)
            return false;
        group->capacity = capacity;
        group->count = 0;
        for (u32 i = 0; i < capacity; ++i)
            reinterpret_cast<pollfd *>(&group->data[i * kSockGroupItemSize])->fd = kInvalidSocket;
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
        pollfd *entry = reinterpret_cast<pollfd *>(&group->data[index * kSockGroupItemSize]);
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
            pollfd *entry = reinterpret_cast<pollfd *>(&group->data[i * kSockGroupItemSize]);
            if (entry->fd == kInvalidSocket)
            {
                entry->fd = socketFd(socket);
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
        pollfd *entry = reinterpret_cast<pollfd *>(&group->data[index * kSockGroupItemSize]);
        if (entry->fd == kInvalidSocket)
            return false;
        TkMemory::memoryCopy(socket->data, &entry->fd, sizeof(entry->fd));
        i32 value = 0;
        if (!getOption(socket, NetSocketOption::kType, value))
            return false;
        socket->protocol = value == SOCK_DGRAM ? NetProtocol::kUdp : value == SOCK_STREAM ? NetProtocol::kTcp : NetProtocol::kUndefined;
        return true;
    }

    void TkNetwork::removeSocket(NetSocketGroup *group, u32 index)
    {
        pollfd *entry = reinterpret_cast<pollfd *>(&group->data[index * kSockGroupItemSize]);
        if (entry->fd != kInvalidSocket)
        {
            entry->fd = kInvalidSocket;
            --group->count;
        }
    }

    bool TkNetwork::removeSocket(NetSocketGroup *group, NetSocket *socket)
    {
        for (u32 i = 0; i < group->capacity; ++i)
        {
            pollfd *entry = reinterpret_cast<pollfd *>(&group->data[i * kSockGroupItemSize]);
            if (entry->fd == socketFd(socket))
            {
                entry->fd = kInvalidSocket;
                --group->count;
                return true;
            }
        }
        return false;
    }

    void TkNetwork::clearGroup(NetSocketGroup *group)
    {
        for (u32 i = 0; i < group->capacity; ++i)
            reinterpret_cast<pollfd *>(&group->data[i * kSockGroupItemSize])->fd = kInvalidSocket;
        group->count = 0;
    }

    i32 TkNetwork::writeSocket(NetSocket *socket, byte *data, u32 length, NetEndPoint *endPoint)
    {
        ssize_t size = -1;
        if (!endPoint && socket->protocol == NetProtocol::kTcp)
            size = ::send(socketFd(socket), reinterpret_cast<const char *>(data), length, 0);
        else if (endPoint && socket->protocol == NetProtocol::kUdp)
            size = ::sendto(socketFd(socket), reinterpret_cast<const char *>(data), length, 0, reinterpret_cast<const sockaddr *>(endPoint->data), getAddrSizeFromEP(*endPoint));
        if (size < 0)
        {
            if (isWouldBlock(errno))
                return 0;
            LogError("[UnixNet] Unable to send data. fd=%i errno=%i", socketFd(socket), errno);
            return -1;
        }
        return static_cast<i32>(size);
    }

    i32 TkNetwork::readSocket(NetSocket *socket, byte *buffer, u32 bufferSize, NetEndPoint *endPoint)
    {
        ssize_t size = -1;
        if (!endPoint)
            size = ::recv(socketFd(socket), reinterpret_cast<char *>(buffer), bufferSize, 0);
        else
        {
            sockaddr_in6 addr{};
            socklen_t addrSize = sizeof(addr);
            size = ::recvfrom(socketFd(socket), reinterpret_cast<char *>(buffer), bufferSize, 0, reinterpret_cast<sockaddr *>(&addr), &addrSize);
            if (size >= 0 && !createEndPointFromAddr(reinterpret_cast<sockaddr *>(&addr), *endPoint))
                return -1;
        }
        if (size < 0)
        {
            if (isWouldBlock(errno))
                return 0;
            LogError("[UnixNet] Unable to read data. fd=%i errno=%i", socketFd(socket), errno);
            return -1;
        }
        return static_cast<i32>(size);
    }

    bool TkNetwork::createEndPoint(const TkString &address, const TkString &port, NetIPVersion ipv, NetEndPoint &endPoint, bool bindable)
    {
        addrinfo hints{};
        hints.ai_family = ipv == NetIPVersion::kIPv6 ? AF_INET6 : ipv == NetIPVersion::kIPv4 ? AF_INET : AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_ADDRCONFIG | AI_V4MAPPED;
        if (bindable)
            hints.ai_flags |= AI_PASSIVE;

        addrinfo *info = nullptr;
        const i32 status = ::getaddrinfo(address.isEmpty() ? nullptr : address.get(), port.isEmpty() ? nullptr : port.get(), &hints, &info);
        if (status != 0 || !info)
        {
            LogError("[UnixNet] Unable to resolve address %s:%s error=%s", address.get(), port.get(), gai_strerror(status));
            return false;
        }

        const bool result = createEndPointFromAddr(info->ai_addr, endPoint);
        ::freeaddrinfo(info);
        return result;
    }

    NetEndPoint TkNetwork::remapEndPointToIPv6(NetEndPoint &endPoint)
    {
        if (endPoint.ipVer == NetIPVersion::kIPv6)
            return endPoint;

        NetEndPoint result{};
        auto *addr4 = reinterpret_cast<sockaddr_in *>(endPoint.data);
        auto *addr6 = reinterpret_cast<sockaddr_in6 *>(result.data);
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = addr4->sin_port;
        addr6->sin6_addr.s6_addr[10] = 0xff;
        addr6->sin6_addr.s6_addr[11] = 0xff;
        TkMemory::memoryCopy(&addr6->sin6_addr.s6_addr[12], &addr4->sin_addr, 4);
        result.ipVer = NetIPVersion::kIPv6;
        return result;
    }
}

#endif
