/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/collection/Array.hpp>
#include <toolkit/net/common.hpp>

namespace toolkit
{
    struct TK_API NetClientsMask
    {
        static NetClientsMask all;

        u64 bits[2] = {0, 0};

        bool hasBit(u32 index) const;
        void setBit(u32 index);
        void unsetBit(u32 index);
        bool isEmpty() const;

        bool operator==(const NetClientsMask &other) const
        {
            return bits[0] == other.bits[0] && bits[1] == other.bits[1];
        }

        bool operator!=(const NetClientsMask &other) const
        {
            return !(*this == other);
        }
    };

    struct TK_API NetReplicationHierarchyObject
    {
        RkObject *object = nullptr;
        f32 replicationFPS = 0.0f;
        i32 replicationUpdatesLeft = 0;
        bool dirty = true;

        explicit NetReplicationHierarchyObject(RkObject *inObject = nullptr)
            : object(inObject)
        {
        }

        bool operator==(const NetReplicationHierarchyObject &other) const
        {
            return object == other.object;
        }

        bool operator==(const RkObject *other) const
        {
            return object == other;
        }
    };

    class TK_API NetReplicationHierarchyUpdateResult
    {
    public:
        struct Entry
        {
            RkObject *object = nullptr;
            NetClientsMask targetClients;
        };

        void init();
        void addObject(RkObject *object);
        void addObject(RkObject *object, NetClientsMask targetClients);

        u32 getClientsCount() const
        {
            return clientsCount;
        }

        NetClientsMask getClientsMask() const
        {
            return clientsMask;
        }

    public:
        TkArray<Entry> entries;
        f32 replicationScale = 1.0f;

    private:
        u32 clientsCount = 0;
        NetClientsMask clientsMask;
    };

    class TK_API NetReplicationHierarchy
    {
    public:
        virtual ~NetReplicationHierarchy() = default;

        virtual void addObject(NetReplicationHierarchyObject object);
        virtual void addObject(RkObject *object);
        virtual bool removeObject(RkObject *object);
        virtual bool getObject(RkObject *object, NetReplicationHierarchyObject &result);
        virtual bool setObject(const NetReplicationHierarchyObject &value);
        virtual bool dirtyObject(RkObject *object);
        virtual void update(NetReplicationHierarchyUpdateResult *result);

        virtual TkString toString() const
        {
            return TkString("NetReplicationHierarchy");
        }

    public:
        TkArray<NetReplicationHierarchyObject> objects;
    };
}
