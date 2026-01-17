/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/debug/Logger.hpp"
#include "toolkit/utils/Convertor.hpp"
#include <toolkit/platforms/Network.hpp>

#if TK_WINDOWS

#include "toolkit/types/String.hpp"

#include <WinSock2.h>
#include <WS2ipdef.h>
#include <WS2tcpip.h>

namespace toolkit
{

#define SOCKGROUP_ITEMSIZE 16

    static_assert(sizeof(NetSocket::data) >= sizeof(SOCKET), "NetSocket::data is not big enough to contains SOCKET !");
    static_assert(sizeof(NetEndPoint::data) >= sizeof(sockaddr_in6), "NetEndPoint::data is not big enough to contains sockaddr_in6 !");
    static_assert(SOCKGROUP_ITEMSIZE >= sizeof(pollfd), "SOCKGROUP_ITEMSIZE macro is not big enough to contains pollfd !");

    static const IN6_ADDR v4MappedPrefix = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00}};

    static TkString getErrorMessage(int error)
    {
        wchar_t *s = nullptr;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&s), 0, nullptr);
        StringAsUTF8<> u8{s};
        TkString str(u8.get());
        LocalFree(s);
        return str;
    }

    TK_FORCE_INLINE static TkString getLastErrorMessage()
    {
        return getErrorMessage(WSAGetLastError());
    }

    TK_FORCE_INLINE static int getAddrSize(const sockaddr &addr)
    {
        return addr.sa_family == AF_INET6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in);
    }

    TK_FORCE_INLINE static int getAddrSizeFromEP(NetEndPoint &endPoint)
    {
        return endPoint.ipVer == NetIPVersion::kIPv6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in);
    }

    static bool CreateEndPointFromAddr(sockaddr *addr, NetEndPoint &endPoint)
    {
        u32 size = getAddrSize(*addr);
        u16 port;
        void *paddr;
        if (addr->sa_family == AF_INET6)
        {
            paddr = &((sockaddr_in6 *)addr)->sin6_addr;
            port = ntohs(((sockaddr_in6 *)addr)->sin6_port);
        }
        else if (addr->sa_family == AF_INET)
        {
            paddr = &((sockaddr_in *)addr)->sin_addr;
            port = ntohs(((sockaddr_in *)addr)->sin_port);
        }
        else
        {
            LogError("[WinNet] Unable to create endpoint, sockaddr must be INET or INET6! Family : %i", addr->sa_family);
            return true;
        }

        char ip[INET6_ADDRSTRLEN];
        if (inet_ntop(addr->sa_family, paddr, ip, INET6_ADDRSTRLEN) == nullptr)
        {
            LogError("[WinNet] Unable to extract address from sockaddr! Error : %s", getLastErrorMessage());
            return true;
        }
        char strPort[6];
        _itoa(port, strPort, 10);
        endPoint.ipVer = addr->sa_family == AF_INET6 ? NetIPVersion::kIPv6 : NetIPVersion::kIPv4;
        TkMemory::memoryCopy(endPoint.data, addr, size);
        return false;
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
            SOCKOPT(NetSocketOption::kUseLoopback, SOL_SOCKET, SO_USELOOPBACK)
            SOCKOPT(NetSocketOption::kLinger, SOL_SOCKET, SO_LINGER)
            SOCKOPT(NetSocketOption::kOOBInline, SOL_SOCKET, SO_OOBINLINE)
            SOCKOPT(NetSocketOption::kSendBuffer, SOL_SOCKET, SO_SNDBUF)
            SOCKOPT(NetSocketOption::kRecvBuffer, SOL_SOCKET, SO_RCVBUF)
            SOCKOPT(NetSocketOption::kSendTimeout, SOL_SOCKET, SO_SNDTIMEO)
            SOCKOPT(NetSocketOption::kRecvTimeout, SOL_SOCKET, SO_RCVTIMEO)
            SOCKOPT(NetSocketOption::kError, SOL_SOCKET, SO_ERROR)
            SOCKOPT(NetSocketOption::kNoDelay, IPPROTO_TCP, TCP_NODELAY)
            SOCKOPT(NetSocketOption::kIPv6Only, IPPROTO_IPV6, IPV6_V6ONLY)
            SOCKOPT(NetSocketOption::kMtu, IPPROTO_IP, IP_MTU)
            SOCKOPT(NetSocketOption::kType, SOL_SOCKET, SO_TYPE)
#undef SOCKOPT
        default:
            *level = 0;
            *name = 0;
            break;
        }
    }

    NetSocket* TkNetwork::newSocket(NetProtocol proto, NetIPVersion ipv)
    {
        auto* socket = TkMemory::newObject<NetSocket>(TkMemory::getDefault());
        socket->protocol = proto;
        socket->ipVer = ipv;
        const u8 family = socket->ipVer == NetIPVersion::kIPv6 ? AF_INET6 : AF_INET;
        const u8 stype = socket->protocol == NetProtocol::kTcp ? SOCK_STREAM : SOCK_DGRAM;
        const u8 prot = socket->protocol == NetProtocol::kTcp ? IPPROTO_TCP : IPPROTO_UDP;
        SOCKET sock;
        if ((sock = ::socket(family, stype, prot)) == INVALID_SOCKET)
        {
            LogError("[WinNet] Can't create native socket! Error : %s", getLastErrorMessage());
            TkMemory::deletePtr(socket);
            return nullptr;
        }
        TkMemory::memoryCopy(socket->data, &sock, sizeof(sock));
        unsigned long value = 1;
        if (ioctlsocket(sock, FIONBIO, &value) == SOCKET_ERROR)
        {
            LogError("[WinNet] Can't set socket to NON-BLOCKING type! Error : %s", getLastErrorMessage());
            TkMemory::deletePtr(socket);
            return nullptr;
        }
        return socket;
    }

    bool TkNetwork::destroySocket(NetSocket* socket)
    {
        const SOCKET sock = *(SOCKET *)socket->data;
        if (sock != INVALID_SOCKET)
        {
            closesocket(sock);
            TkMemory::deletePtr(socket);
            return true;
        }
        LogWarn("Unable to delete socket INVALID_SOCKET! Socket : %s", sock);
        return false;
    }

    bool TkNetwork::setOption(NetSocket* socket, NetSocketOption option, i32 value)
    {
        i32 optlvl = 0;
        i32 optnme = 0;
        convertSockOptToNative(option, &optlvl, &optnme);
        if (setsockopt(*(SOCKET *)socket->data, optlvl, optnme, (char *)&value, sizeof(value)) == SOCKET_ERROR)
        {
            LogWarn("Unable to set socket option ! Socket : %v Error : %s", *(SOCKET *)socket->data, getLastErrorMessage());
            return false;
        }
        return true;
    }

    bool TkNetwork::getOption(NetSocket* socket, NetSocketOption option, i32 &value)
    {
        i32 optlvl = 0;
        i32 optnme = 0;
        convertSockOptToNative(option, &optlvl, &optnme);
        i32 size;
        if (getsockopt(*(SOCKET *)socket->data, optlvl, optnme, (char *)&value, &size) == SOCKET_ERROR)
        {
            LogWarn("Unable to get socket option ! Socket : %v Error : %s", *(SOCKET *)socket->data, getLastErrorMessage());
            return false;
        }
        return true;
    }

    bool TkNetwork::connect(NetSocket* socket, NetEndPoint &endPoint)
    {
        const int size = getAddrSizeFromEP(endPoint);
        if (::connect(*(SOCKET *)socket->data, (const sockaddr *)endPoint.data, size) == SOCKET_ERROR)
        {
            const int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK)
                return true;
            LogError("[WinNet] Unable to connect socket to address! Socket : %v Error : %s", *(SOCKET *)socket->data, getErrorMessage(error));
            return false;
        }
        return true;
    }

    bool TkNetwork::bind(NetSocket* socket, NetEndPoint &endPoint)
    {
        if (socket->ipVer != endPoint.ipVer)
        {
            LogError("[WinNet] Can't bind socket to end point, Socket.ipVer != EndPoint.ipVer! Socket : %v", *(SOCKET *)socket->data);
            return false;
        }
        const int size = getAddrSizeFromEP(endPoint);
        if (::bind(*(SOCKET *)socket->data, (const sockaddr *)endPoint.data, size) == SOCKET_ERROR)
        {
            LogError("[WinNet] Unable to bind socket! Socket : %v Error : %s", *(SOCKET *)socket->data, getLastErrorMessage());
            return false;
        }
        return true;
    }

    bool TkNetwork::listen(NetSocket* socket, u16 queueSize)
    {
        if (::listen(*(SOCKET *)socket->data, (i32)queueSize) == SOCKET_ERROR)
        {
            LogError("[WinNet] Unable to listen ! Socket : %v Error : %s", *(SOCKET *)socket->data, getLastErrorMessage());
            return false;
        }
        return true;
    }

    bool TkNetwork::accept(NetSocket* serverSocket, NetSocket* newSocket, NetEndPoint &newEndPoint)
    {
        if (serverSocket->protocol != NetProtocol::kTcp)
        {
            LogWarn("Can't accept connection on UDP socket! Socket : %v", *(SOCKET *)serverSocket->data);
            return false;
        }
        SOCKET sock;
        sockaddr_in6 addr;
        i32 size = sizeof(sockaddr_in6);
        if ((sock = ::accept(*(SOCKET *)serverSocket->data, (sockaddr *)&addr, &size)) == INVALID_SOCKET)
        {
            const int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK)
                return true;
            LogWarn("[WinNet] Unable to accept incoming connection! Socket : %v Error : %s", *(SOCKET *)serverSocket->data, getErrorMessage(error));
            return false;
        }
        TkMemory::memoryCopy(newSocket->data, &sock, sizeof(sock));
        TkMemory::memoryCopy(newEndPoint.data, &addr, size);
        newSocket->protocol = serverSocket->protocol;
        newSocket->ipVer = serverSocket->ipVer;
        if (CreateEndPointFromAddr((sockaddr *)&addr, newEndPoint))
            return false;
        return true;
    }

    bool TkNetwork::isReadable(NetSocket* socket)
    {
        pollfd entry;
        entry.fd = *(SOCKET *)socket->data;
        entry.events = POLLRDNORM;
        if (WSAPoll(&entry, 1, 0) == SOCKET_ERROR)
        {
            const int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK)
                return true;
            LogError("[WinNet] Unable to poll socket! Socket : %v Error : %s", *(SOCKET *)socket->data, getErrorMessage(error));
            return false;
        }
        if (entry.revents & POLLRDNORM)
            return false;
        return true;
    }

    bool TkNetwork::isWritable(NetSocket* socket)
    {
        pollfd entry;
        entry.fd = *(SOCKET *)socket->data;
        entry.events = POLLWRNORM;
        if (WSAPoll(&entry, 1, 0) == SOCKET_ERROR)
        {
            const int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK)
                return true;
            LogError("[WinNet] Unable to poll socket! Socket %v :  Error : %s", *(SOCKET *)socket->data, getErrorMessage(error));
            return false;
        }
        if (entry.revents & POLLWRNORM)
            return false;
        return true;
    }

    bool TkNetwork::newSocketGroup(u32 capacity, NetSocketGroup* group)
    {
        group->data = (byte *)TkMemory::allocate(TkMemory::getDefault(), capacity * SOCKGROUP_ITEMSIZE, 16);
        if (!group->data)
        {
            LogError("[WinNet] Unable to malloc NetSocketGroup::data ! Size : %i", capacity * SOCKGROUP_ITEMSIZE);
            return false;
        }
        group->capacity = capacity;
        for (u32 i = 0; i < group->capacity; i++)
            ((pollfd *)&group->data[i * SOCKGROUP_ITEMSIZE])->fd = -1;
        return true;
    }

    bool TkNetwork::destroySocketGroup(NetSocketGroup* group)
    {
        if (!group->data)
            return false;
        TkMemory::free(TkMemory::getDefault(), group->data);
        return true;
    }

    i32 TkNetwork::poll(NetSocketGroup* group)
    {
        const int pollret = WSAPoll((pollfd *)group->data, group->count, 0);
        if (pollret == SOCKET_ERROR)
            LogError("[WinNet] Unable to poll socket group! Error : {0}", getLastErrorMessage());
        return pollret;
    }

    bool TkNetwork::getSocketState(NetSocketGroup* group, u32 index, NetSocketState &state)
    {
        if (index >= group->capacity)
            return false;
        pollfd *pollptr = (pollfd *)&group->data[index * SOCKGROUP_ITEMSIZE];
        state = NetSocketState::kNone;
        if (pollptr->revents & POLLERR)
            state |= NetSocketState::kError;
        if (pollptr->revents & POLLHUP)
            state |= NetSocketState::kDisconnected;
        if (pollptr->revents & POLLNVAL)
            state |= NetSocketState::kInvalid;
        if (pollptr->revents & POLLRDNORM)
            state |= NetSocketState::kReadable;
        if (pollptr->revents & POLLWRNORM)
            state |= NetSocketState::kWriteable;
        return true;
    }

    i32 TkNetwork::addSocket(NetSocketGroup* group, NetSocket* socket)
    {
        if (group->count >= group->capacity)
            return -1;
        pollfd pollinfo;
        pollinfo.fd = *(SOCKET *)socket->data;
        pollinfo.events = POLLRDNORM | POLLWRNORM;
        for (u32 i = 0; i < group->capacity; i++)
        {
            if (((pollfd *)&group->data[i * SOCKGROUP_ITEMSIZE])->fd == -1)
            {
                *(pollfd *)&group->data[i * SOCKGROUP_ITEMSIZE] = pollinfo;
                group->count++;
                return i;
            }
        }
        return -1;
    }

    bool TkNetwork::getSocket(NetSocketGroup* group, u32 index, NetSocket* socket)
    {
        if (index >= group->capacity)
            return false;
        SOCKET s = ((pollfd *)&group->data[index * SOCKGROUP_ITEMSIZE])->fd;
        TkMemory::memoryCopy(socket->data, &s, sizeof(s));
        i32 value;
        if (getOption(socket, NetSocketOption::kType, value))
            return false;
        if (value == SOCK_DGRAM)
            socket->protocol = NetProtocol::kUdp;
        else if (value == SOCK_STREAM)
            socket->protocol = NetProtocol::kTcp;
        else
            socket->protocol = NetProtocol::kUndefined;
        return true;
    }

    void TkNetwork::removeSocket(NetSocketGroup* group, u32 index)
    {
        if (((pollfd *)&group->data[index * SOCKGROUP_ITEMSIZE])->fd != -1)
        {
            ((pollfd *)&group->data[index * SOCKGROUP_ITEMSIZE])->fd = -1;
            group->count--;
        }
    }

    bool TkNetwork::removeSocket(NetSocketGroup* group, NetSocket* socket)
    {
        for (int i = 0; i < (int)group->capacity; i++)
        {
            if (((pollfd *)&group->data[i * SOCKGROUP_ITEMSIZE])->fd == *(SOCKET *)&socket->data)
            {
                ((pollfd *)&group->data[i * SOCKGROUP_ITEMSIZE])->fd = -1;
                group->count--;
                return true;
            }
        }
        return false;
    }

    void TkNetwork::clearGroup(NetSocketGroup* group)
    {
        for (u32 i = 0; i < group->capacity; i++)
            ((pollfd *)&group->data[i * SOCKGROUP_ITEMSIZE])->fd = -1;
        group->count = 0;
    }

    i32 TkNetwork::writeSocket(NetSocket* socket, byte *data, u32 length, NetEndPoint *endPoint)
    {
        if (endPoint != nullptr && socket->ipVer != endPoint->ipVer)
        {
            LogError("[WinNet] Unable to send data, Socket.ipVer != EndPoint.ipVer! Socket : %v", *(SOCKET *)socket->data);
            return -1;
        }
        u32 size;
        if (endPoint == nullptr && socket->protocol == NetProtocol::kTcp)
        {
            if ((size = send(*(SOCKET *)socket->data, (const char *)data, length, 0)) == SOCKET_ERROR)
            {
                LogError("[WinNet] Unable to send data! Socket : %v Data Length : %i Error : %s", *(SOCKET *)socket->data, length, getLastErrorMessage());
                return -1;
            }
        }
        else if (endPoint != nullptr && socket->protocol == NetProtocol::kUdp)
        {
            if ((size = sendto(*(SOCKET *)socket->data, (const char *)data, length, 0, (const sockaddr *)endPoint->data, getAddrSizeFromEP(*endPoint))) == SOCKET_ERROR)
            {
                LogError("[WinNet] Unable to send data! Socket : %v Data Length : %i Error : %s", *(SOCKET *)socket->data, length, getLastErrorMessage());
                return -1;
            }
        }
        else
        {
            // TODO: better explanation
            LogError("[WinNet] Unable to send data! Socket : %v Data Length : %i", *(SOCKET *)socket->data, length);
            return -1;
        }
        return size;
    }

    i32 TkNetwork::readSocket(NetSocket* socket, byte *buffer, u32 bufferSize, NetEndPoint *endPoint)
    {
        u32 size;
        if (endPoint == nullptr)
        {
            if ((size = recv(*(SOCKET *)socket->data, (char *)buffer, bufferSize, 0)) == SOCKET_ERROR)
            {
                const int error = WSAGetLastError();
                if (error == WSAEWOULDBLOCK)
                    return 0;
                LogError("[WinNet] Unable to read data! Socket : %v Buffer Size : %i Error : %s", *(SOCKET *)socket->data, bufferSize, getErrorMessage(error));
                return -1;
            }
        }
        else
        {
            i32 addrsize = sizeof(sockaddr_in6);
            sockaddr_in6 addr;
            if ((size = recvfrom(*(SOCKET *)socket->data, (char *)buffer, bufferSize, 0, (sockaddr *)&addr, &addrsize)) == SOCKET_ERROR)
            {
                LogError("[WinNet] Unable to read data! Socket : %v Buffer Size : %i Error : %s", *(SOCKET *)socket->data, bufferSize, getLastErrorMessage());
                return -1;
            }
            if (CreateEndPointFromAddr((sockaddr *)&addr, *endPoint))
                return true;
        }
        return size;
    }

    bool TkNetwork::createEndPoint(const TkString &address, const TkString &port, NetIPVersion ipv, NetEndPoint &endPoint, bool bindable)
    {
        INT status;
        addrinfoW hints;
        addrinfoW *info;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = ipv == NetIPVersion::kIPv6 ? AF_INET6 : ipv == NetIPVersion::kIPv4 ? AF_INET
                                                                                                   : AF_UNSPEC;
        hints.ai_flags |= AI_ADDRCONFIG;
        hints.ai_flags |= AI_V4MAPPED;
        if (bindable)
            hints.ai_flags = AI_PASSIVE;
        StringAsUTF16<MAX_PATH> addr{address.get()};
        StringAsUTF16<MAX_PATH> prt{port.get()};
        // Consider using NUMERICHOST/NUMERICSERV if address is a valid Ipv4 or IPv6 so we can skip some look up ( potentially slow when resolving host names )
        if ((status = ::GetAddrInfoW(address.isEmpty() ? nullptr : addr.get(), port.isEmpty() ? nullptr : prt.get(), &hints, &info)) != 0)
        {
            LogError("[WinNet] Unable to query info for address : %s::%s Error : %s", address, port, gai_strerror(status));
            return false;
        }
        if (info == nullptr)
        {
            LogError("[WinNet] Unable to resolve address! Address : %s::%s", address, port);
            return false;
        }
        if (CreateEndPointFromAddr(info->ai_addr, endPoint))
        {
            FreeAddrInfoW(info);
            return false;
        }
        FreeAddrInfoW(info);
        return true;
    }

    NetEndPoint TkNetwork::remapEndPointToIPv6(NetEndPoint &endPoint)
    {
        if (endPoint.ipVer == NetIPVersion::kIPv6)
        {
            LogWarn("[WinNet] Unable to remap EndPoint, already in IPv6 format!");
            return endPoint;
        }

        NetEndPoint pv6;
        sockaddr_in *addr4 = (sockaddr_in *)endPoint.data;
        sockaddr_in6 *addr6 = (sockaddr_in6 *)pv6.data;
        const SCOPE_ID scope = SCOPEID_UNSPECIFIED_INIT;

        // Can be replaced by windows built-in macro IN6ADDR_SETV4MAPPED()
        TkMemory::memorySet(addr6, 0, sizeof(sockaddr_in6));
        addr6->sin6_family = AF_INET6;
        addr6->sin6_scope_struct = scope;
        addr6->sin6_addr = v4MappedPrefix;
        addr6->sin6_port = addr4->sin_port;
        TkMemory::memoryCopy(&addr6->sin6_addr.u.Byte[12], &addr4->sin_addr, 4); // :::::FFFF:XXXX:XXXX    X=IPv4
        pv6.ipVer = NetIPVersion::kIPv6;

        return pv6;
    }
}

#endif