/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/net/common.hpp>

namespace toolkit
{
    static constexpr u32 kNetProtocolVersion = 4;
    static constexpr bool kUseNetReplicatorCache = true;

    enum class NetMessageId : u8
    {
        kNone = 0,
        kHandshake,
        kHandshakeReply,
        kKey,
        kObjectReplicate,
        kObjectReplicatePart,
        kObjectSpawn,
        kObjectSpawnPart,
        kObjectDespawn,
        kObjectRole,
        kObjectRpc,

        kMax,
    };

    class NetInternal
    {
    public:
        static void netReplicatorClientConnected(NetClient *client);
        static void netReplicatorClientDisconnected(NetClient *client);
        static void netReplicatorClear();
        static void netReplicatorPreUpdate();
        static void netReplicatorUpdate();
        static void onNetMessageObjectReplicate(NetEvent &event, NetClient *client, NetPeer *peer);
        static void onNetMessageObjectReplicatePart(NetEvent &event, NetClient *client, NetPeer *peer);
        static void onNetMessageObjectSpawn(NetEvent &event, NetClient *client, NetPeer *peer);
        static void onNetMessageObjectSpawnPart(NetEvent &event, NetClient *client, NetPeer *peer);
        static void onNetMessageObjectDespawn(NetEvent &event, NetClient *client, NetPeer *peer);
        static void onNetMessageObjectRole(NetEvent &event, NetClient *client, NetPeer *peer);
        static void onNetMessageObjectRpc(NetEvent &event, NetClient *client, NetPeer *peer);
    };
}
