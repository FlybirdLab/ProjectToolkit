/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/net/common.hpp>
#include "toolkit/collection/Array.hpp"
#include "toolkit/reflect/common.hpp"

namespace toolkit
{
    // The high-level network manage modes.
    rkenum class NetManagerMode
    {
        // Disabled.
        kOffline = 0,
        // Server-only without a client (host a game but not participate).
        kServer,
        // Client-only (connected to Server or Host).
        kClient,
        // Both server and client (other clients can connect).
        kHost,
    };

    // The high-level network client connection data. Can be used to accept/deny new connection.
    rkstruct NetClientConnectionData
    {
        R_STRUCT(NetClientConnectionData);
    public:
        // The incoming client.
        rkfield() NetClient *client;
        // 0 if accept new connection, error code otherwise. Send back to the client.
        rkfield() i32 result;
        // Client platform type (can be different that current one when using cross-play).
        rkfield() PlatformType platform;
        // Client platform architecture (can be different that current one when using cross-play).
        rkfield() ArchitectureType architecture;
        // Custom data to send from client to server as part of connection initialization and verification. Can contain game build info, platform data or certificate needed upon joining the game.
        rkfield() TkArray<byte> payloadData;
    };

    // High-level networking manager for multiplayer games.
    rkclass TK_API NetManager
    {
    public:
        // Server client identifier. Constant value of 0.
        static constexpr u32 kServerClientId = 0;

    public:
        // Event called when network manager state gets changed (eg. client connected to the server, or server shutdown started).
        static TkAction<> onStateChanged;
        // Event called when new client is connecting. Can be used to accept/deny connection. Called on client to fill PayloadData send and called on server/host to validate incoming client connection (eg. with PayloadData validation).
        static TkAction<NetClientConnectionData &> onClientConnecting;
        // Event called after new client successfully connected.
        static TkAction<NetClient *> onClientConnected;
        // Event called after new client successfully disconnected.
        static TkAction<NetClient *> onClientDisconnected;

    public:
        // Return true if network is a client.
        TK_FORCE_INLINE static bool isClient()
        {
            return mode == NetManagerMode::kClient;
        }

        // Return true if network is a server.
        TK_FORCE_INLINE static bool isServer()
        {
            return mode == NetManagerMode::kServer;
        }

        // Return true if network is a host (both client and server).
        TK_FORCE_INLINE static bool isHost()
        {
            return mode == NetManagerMode::kHost;
        }

        // Return true if network is connected and online.
        TK_FORCE_INLINE static bool isConnected()
        {
            return state == NetConnectionState::kConnected;
        }

        // Return true if network is online or disconnected.

        TK_FORCE_INLINE static bool isOffline()
        {
            return state == NetConnectionState::kOffline || state == NetConnectionState::kDisconnected;
        }

        // Get the network client for a given connection. Return null if failed to find it.
        static NetClient *getClient(const NetConnection &connection);

        // Get the network client with a given identifier. Return null if failed to find it.
        static NetClient *getClient(u32 clientId);

    public:
        // Start the network in server mode. Return true if started successfully.
        rkfunc static bool startServer();
        // Start the network in client mode. Return true if started successfully.
        rkfunc static bool startClient();
        // Start the network in host mode. Return true if started successfully.
        rkfunc static bool startHost();
        // Stop the network.
        rkfunc static void stop();
        // Poll network events and run replication. Call this once per frame from the host application.
        static void update();

    public:
        // The target amount of the network logic updates per second (frequency of replication, events sending and network ticking). Use 0 to run every game update.
        rkfield() static float networkFPS;
        // Current network peer (low-level).
        rkfield() static NetPeer *peer;
        // Current manager mode.
        rkfield() static NetManagerMode mode;
        // Current network connection state.
        rkfield() static NetConnectionState state;
        // Current network system frame number (incremented every tick). Can be used for frames counting in networking and replication.
        rkfield() static u32 frame;
        // Local client identifier. Valid even on server that doesn't have LocalClient.
        rkfield() static u32 localClientId;
        // Local client, valid only when Network Manager is running in client or host mode (server doesn't have a client).
        rkfield() static NetClient *localClient;
        // List of all clients: connecting, connected and disconnected. Empty on clients.
        rkfield() static TkArray<NetClient *> clients;
    };

}
