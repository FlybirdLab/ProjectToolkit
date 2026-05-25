/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "NetReplicationHierarchy.hpp"

#include <toolkit/net/NetManager.hpp>

namespace toolkit
{
    namespace
    {
        u16 s_objectCounter = 0;

        i32 replicationPeriodFrames(f32 replicationFPS)
        {
            if (replicationFPS <= 0.0f || NetManager::networkFPS <= 0.0f)
                return 0;
            i32 frames = static_cast<i32>(NetManager::networkFPS / replicationFPS + 0.5f);
            if (frames < 1)
                frames = 1;
            if (frames > 60)
                frames = 60;
            return frames;
        }
    }

    NetClientsMask NetClientsMask::all = {{~0ull, ~0ull}};

    bool NetClientsMask::hasBit(const u32 index) const
    {
        const u32 word = index / 64;
        return word < 2 && (bits[word] & (1ull << (index % 64))) != 0;
    }

    void NetClientsMask::setBit(const u32 index)
    {
        const u32 word = index / 64;
        if (word < 2)
            bits[word] |= 1ull << (index % 64);
    }

    void NetClientsMask::unsetBit(const u32 index)
    {
        const u32 word = index / 64;
        if (word < 2)
            bits[word] &= ~(1ull << (index % 64));
    }

    bool NetClientsMask::isEmpty() const
    {
        return (bits[0] | bits[1]) == 0;
    }

    void NetReplicationHierarchyUpdateResult::init()
    {
        entries.clear();
        replicationScale = 1.0f;
        clientsCount = static_cast<u32>(NetManager::clients.count());
        clientsMask = NetManager::isClient() ? NetClientsMask::all : NetClientsMask();
        for (u32 i = 0; i < clientsCount; i++)
            clientsMask.setBit(i);
    }

    void NetReplicationHierarchyUpdateResult::addObject(RkObject *object)
    {
        addObject(object, NetClientsMask::all);
    }

    void NetReplicationHierarchyUpdateResult::addObject(RkObject *object, NetClientsMask targetClients)
    {
        if (object == nullptr)
            return;
        Entry &entry = entries.addOne();
        entry.object = object;
        entry.targetClients = targetClients;
    }

    void NetReplicationHierarchy::addObject(NetReplicationHierarchyObject object)
    {
        if (object.object == nullptr || objects.contains(object.object))
            return;

        const i32 period = replicationPeriodFrames(object.replicationFPS);
        object.replicationUpdatesLeft = period > 0 ? s_objectCounter++ % period : 0;
        objects.add(object);
    }

    void NetReplicationHierarchy::addObject(RkObject *object)
    {
        addObject(NetReplicationHierarchyObject(object));
    }

    bool NetReplicationHierarchy::removeObject(RkObject *object)
    {
        const i32 index = objects.find(object);
        if (index == -1)
            return false;
        objects.removeAt(index);
        return true;
    }

    bool NetReplicationHierarchy::getObject(RkObject *object, NetReplicationHierarchyObject &result)
    {
        const i32 index = objects.find(object);
        if (index == -1)
            return false;
        result = objects[index];
        return true;
    }

    bool NetReplicationHierarchy::setObject(const NetReplicationHierarchyObject &value)
    {
        const i32 index = objects.find(value.object);
        if (index == -1)
            return false;
        objects[index] = value;
        return true;
    }

    bool NetReplicationHierarchy::dirtyObject(RkObject *object)
    {
        const i32 index = objects.find(object);
        if (index == -1)
            return false;
        NetReplicationHierarchyObject &entry = objects[index];
        entry.dirty = true;
        if (entry.replicationFPS < 0.0f)
            entry.replicationUpdatesLeft = 1;
        return true;
    }

    void NetReplicationHierarchy::update(NetReplicationHierarchyUpdateResult *result)
    {
        if (result == nullptr)
            return;

        const f32 networkFPS = NetManager::networkFPS / result->replicationScale;
        for (NetReplicationHierarchyObject &entry : objects)
        {
            if (entry.object == nullptr)
                continue;

            bool shouldReplicate = entry.dirty;
            if (!shouldReplicate)
            {
                if (entry.replicationFPS == 0.0f)
                    shouldReplicate = true;
                else if (entry.replicationFPS > 0.0f && --entry.replicationUpdatesLeft <= 0)
                    shouldReplicate = true;
            }

            if (!shouldReplicate)
                continue;

            entry.dirty = false;
            if (entry.replicationFPS > 0.0f && networkFPS > 0.0f)
                entry.replicationUpdatesLeft = replicationPeriodFrames(entry.replicationFPS);
            result->addObject(entry.object, result->getClientsMask());
        }
    }
}
