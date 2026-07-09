/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/net/common.hpp>

namespace toolkit
{
    // Interface for objects for network replication to receive additional events.
    class TK_API INetworkObject
    {
    public:
        // Event called when network object gets spawned.
        virtual void onNetworkSpawn() {};
        // Event called when network object gets despawned.
        virtual void onNetworkDespawn() {};
        // Event called before network object gets replicated (before reading data).
        virtual void onNetworkSerialize() {};
        // Event called when network object gets replicated (after reading data).
        virtual void onNetworkDeserialize() {};
        // Event called when network object gets synced (called only once upon initial sync).
        virtual void onNetworkSync() {};
    };

}