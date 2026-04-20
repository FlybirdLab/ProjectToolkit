/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/collection/StaticArray.hpp"
#include <toolkit/net/common.hpp>
#include <toolkit/types/Span.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/types/Pair.hpp>

namespace toolkit
{
    class NetStream;

    // Additional context parameters for Network RPC execution (eg. to identify who sends the data).
    struct TK_API NetRpcParams
    {
        NetRpcParams() = default;
        NetRpcParams(const NetStream *stream);

        // The ClientId of the network client that is a data sender. Can be used to detect who send the incoming RPC or replication data. Ignored when sending data.
        u32 senderId = 0;
        // The list of ClientId of the network clients that should receive RPC. Can be used to send RPC to a specific client(s). Ignored when receiving data.
        TkSpan<u32> targetIds;
    };

    // Network RPC identifier name (pair of type and function name)
    typedef TkPair<ScriptingTypeHandle, TkStringView> NetRpcName;

    // Network RPC descriptor
    struct TK_API NetRpcInfo
    {
        u8 server : 1;
        u8 client : 1;
        u8 channel : 4;
        void (*execute)(ScriptingObject *obj, NetStream *stream, void *tag);
        bool (*invoke)(ScriptingObject *obj, void **args);
        void* tag;

        // Global table for registered RPCs. Key: pair of type, RPC name. Value: RPC descriptor.
        static TkHashMap<NetRpcName, NetRpcInfo> RPCsTable;
    };

    // Gets the pointer to the RPC argument into the args buffer
    template <typename T>
    TK_FORCE_INLINE void netRpcInitArg(TkStaticArray<void *, 16> &args, const T &v)
    {
        args.add((void *)&v);
    }

    // Gets the pointers to the RPC arguments into the args buffer
    template <typename T, typename... Params>
    TK_FORCE_INLINE void netRpcInitArg(TkStaticArray<void *, 16> &args, const T &first, Params &&...params)
    {
        netRpcInitArg(args, first);
        netRpcInitArg(args, ::toolkit::forward<Params>(params)...);
    }

// Network RPC implementation (placed in the beginning of the method body)
#define NETWORK_RPC_IMPL(type, name, ...)                                                                                                                       \
    {                                                                                                                                                           \
        const NetRpcInfo *rpcInfoPtr = NetRpcInfo::RPCsTable.TryGet(NetRpcName(type::TypeInitializer, StringAnsiView(#name)));                      \
        if (rpcInfoPtr == nullptr)                                                                                                                              \
        {                                                                                                                                                       \
            LogError("Invalid RPC {0}::{1}. Ensure to use proper type name and method name (and 'Network' tag on a code module).", TEXT(#type), TEXT(#name)); \
            if (System::isDebuggerPresent())                                                                                                                  \
                PLATFORM_DEBUG_BREAK;                                                                                                                           \
            TK_ASSERT("Invalid RPC.", __FILE__, __LINE__);                                                                                               \
            return;                                                                                                                                             \
        }                                                                                                                                                       \
        const NetRpcInfo &rpcInfo = *rpcInfoPtr;                                                                                                            \
        const NetManagerMode networkMode = NetworkManager::Mode;                                                                                            \
        if ((rpcInfo.server && networkMode == NetManagerMode::kClient) || (rpcInfo.client && networkMode != NetManagerMode::kClient))                     \
        {                                                                                                                                                       \
            TkStaticArray<void *, 16> args;                                                                                                            \
            netRpcInitArg(args, __VA_ARGS__);                                                                                                               \
            if (rpcInfo.invoke(this, args.get()))                                                                                                               \
                return;                                                                                                                                         \
        }                                                                                                                                                       \
    }

// Network RPC override implementation (placed in the beginning of the overriden method body - after call to the base class method)
#define NETWORK_RPC_OVERRIDE_IMPL(type, name, ...)                                                                                          \
    {                                                                                                                                       \
        const NetRpcInfo &rpcInfo = NetRpcInfo::RPCsTable[NetRpcName(type::TypeInitializer, StringAnsiView(#name))];            \
        const NetManagerMode networkMode = NetworkManager::Mode;                                                                        \
        if ((rpcInfo.server && networkMode == NetManagerMode::kClient) || (rpcInfo.client && networkMode != NetManagerMode::kClient)) \
        {                                                                                                                                   \
            if (rpcInfo.server && networkMode == NetManagerMode::kClient)                                                                \
                return;                                                                                                                     \
            if (rpcInfo.client && networkMode == NetManagerMode::kServer)                                                                \
                return;                                                                                                                     \
        }                                                                                                                                   \
    }

}