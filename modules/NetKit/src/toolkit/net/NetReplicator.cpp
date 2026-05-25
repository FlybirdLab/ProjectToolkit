/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "NetReplicator.hpp"

#include <toolkit/collection/HashMap.hpp>
#include <toolkit/collection/HashSet.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/net/INetObject.hpp>
#include <toolkit/net/INetSerializable.hpp>
#include <toolkit/net/NetClient.hpp>
#include <toolkit/net/NetEvent.hpp>
#include <toolkit/net/NetInternal.hpp>
#include <toolkit/net/NetManager.hpp>
#include <toolkit/net/NetMessage.hpp>
#include <toolkit/net/NetPeer.hpp>
#include <toolkit/net/NetReplicationHierarchy.hpp>
#include <toolkit/net/NetRpc.hpp>
#include <toolkit/net/NetStream.hpp>
#include <toolkit/reflect/RkManager.hpp>
#include <toolkit/threading/Sync.hpp>

namespace toolkit
{
#if TK_DEBUG
    bool NetReplicator::enableLog = false;
#endif

#pragma pack(push, 1)
    struct NetMessageObjectReplicate
    {
        NetMessageId id = NetMessageId::kObjectReplicate;
        u32 ownerFrame = 0;
    };

    struct NetMessageObjectReplicatePayload
    {
        u16 dataSize = 0;
        u16 partsCount = 0;
        u16 partSize = 0;
    };

    struct NetMessageObjectReplicatePart
    {
        NetMessageId id = NetMessageId::kObjectReplicatePart;
        u32 ownerFrame = 0;
        u16 dataSize = 0;
        u16 partsCount = 0;
        u16 partStart = 0;
        u16 partSize = 0;
    };

    struct NetMessageObjectSpawn
    {
        NetMessageId id = NetMessageId::kObjectSpawn;
        u32 ownerClientId = 0;
        u32 ownerSpawnId = 0;
        u16 itemsCount = 0;
        u8 useParts : 1;
    };

    struct NetMessageObjectSpawnPart
    {
        NetMessageId id = NetMessageId::kObjectSpawnPart;
        u32 ownerClientId = 0;
        u32 ownerSpawnId = 0;
    };

    struct NetMessageObjectSpawnItem
    {
        TkUuid objectId;
        TkUuid parentId;
        char objectTypeName[128] = {};
    };

    struct NetMessageObjectDespawn
    {
        NetMessageId id = NetMessageId::kObjectDespawn;
    };

    struct NetMessageObjectRole
    {
        NetMessageId id = NetMessageId::kObjectRole;
        u32 ownerClientId = 0;
    };

    struct NetMessageObjectRpc
    {
        NetMessageId id = NetMessageId::kObjectRpc;
        u16 argsSize = 0;
    };
#pragma pack(pop)

    struct Serializer
    {
        NetReplicator::SerializeFunc methods[2] = {nullptr, nullptr};
        void *tags[2] = {nullptr, nullptr};
    };

    struct ReplicatedObject
    {
        RkObject *object = nullptr;
        TkUuid objectId;
        TkUuid parentId;
        u32 ownerClientId = NetManager::kServerClientId;
        u32 lastOwnerFrame = 0;
        NetObjectRole role = NetObjectRole::Replicated;
        bool spawned = false;
        bool synced = false;
        bool dirty = true;
        TkDataCollection<u32> targetClientIds;
        INetworkObject *netObject = nullptr;

        NetClientsMask cacheMask;
        ByteCollection cacheData;

        void clearCache()
        {
            cacheMask = NetClientsMask();
            cacheData.release();
        }
    };

    struct ReplicateItem
    {
        RkObject *object = nullptr;
        TkUuid objectId;
        u16 partsLeft = 0;
        u32 ownerFrame = 0;
        u32 ownerClientId = 0;
        TkArray<byte> data;
    };

    struct SpawnItem
    {
        RkObject *object = nullptr;
        TkDataCollection<u32> targets;
        bool hasOwnership = false;
        bool hierarchicalOwnership = false;
        u32 ownerClientId = NetManager::kServerClientId;
        NetObjectRole role = NetObjectRole::OwnedAuthoritative;
    };

    struct SpawnItemParts
    {
        NetMessageObjectSpawn msgData;
        TkArray<NetMessageObjectSpawnItem> items;
    };

    struct DespawnItem
    {
        TkUuid id;
        TkDataCollection<u32> targets;
    };

    struct RpcItem
    {
        RkObject *object = nullptr;
        NetRpcName name;
        NetRpcInfo info;
        ByteCollection argsData;
        TkDataCollection<u32> targets;
    };

    namespace
    {
        TkMutex s_objectsLock;
        TkHashMap<TkUuid, ReplicatedObject> s_objects;
        TkArray<ReplicateItem> s_replicationParts;
        TkArray<SpawnItemParts> s_spawnParts;
        TkArray<SpawnItem> s_spawnQueue;
        TkArray<DespawnItem> s_despawnQueue;
        TkArray<RpcItem> s_rpcQueue;
        TkHashMap<TkUuid, TkUuid> s_idRemappingTable;
        TkHashMap<TypeHandle, Serializer> s_serializers;
        TkHashSet<TkUuid> s_despawnedObjects;
        TkArray<NetClient *> s_newClients;
        TkArray<NetConnection> s_cachedTargets;
        NetStream *s_cachedWriteStream = nullptr;
        NetStream *s_cachedReadStream = nullptr;
        NetReplicationHierarchyUpdateResult *s_cachedReplicationResult = nullptr;
        NetReplicationHierarchy *s_hierarchy = nullptr;
        u32 s_spawnId = 0;

        void logReplicator(const char *message)
        {
#if TK_DEBUG
            if (NetReplicator::enableLog)
                LogInfo("%s", message);
#else
            (void)message;
#endif
        }

        bool isLocalOwner(const ReplicatedObject &item)
        {
            return item.ownerClientId == NetManager::localClientId && item.role == NetObjectRole::OwnedAuthoritative;
        }

        TkUuid remapObjectId(TkUuid objectId)
        {
            const auto iterator = s_idRemappingTable.find(objectId);
            return iterator.isEnd() ? objectId : iterator->value;
        }

        ReplicatedObject *resolveObject(TkUuid objectId)
        {
            const auto direct = s_objects.find(objectId);
            if (direct.isNotEnd())
                return &direct->value;

            objectId = remapObjectId(objectId);
            const auto remapped = s_objects.find(objectId);
            return remapped.isNotEnd() ? &remapped->value : nullptr;
        }

        ReplicatedObject *resolveObject(const TkUuid objectId, TkUuid parentId, const TkStringView &objectTypeName)
        {
            if (ReplicatedObject *object = resolveObject(objectId))
                return object;

            parentId = remapObjectId(parentId);
            TypeHandle objectType = RkManager::findType(objectTypeName);
            if (!objectType.isValid())
                return nullptr;

            for (auto iterator = s_objects.Begin(); iterator.isNotEnd(); ++iterator)
            {
                ReplicatedObject &item = iterator->value;
                if (item.lastOwnerFrame == 0 &&
                    item.parentId == parentId &&
                    item.object != nullptr &&
                    item.object->getTypeHandle() == objectType &&
                    !s_idRemappingTable.containsValue(item.objectId))
                {
                    if (NetManager::isClient())
                        s_idRemappingTable[objectId] = item.objectId;
                    return &item;
                }
            }

            return nullptr;
        }

        ReplicatedObject *resolveObject(const RkObject *object)
        {
            return object != nullptr ? resolveObject(object->getId()) : nullptr;
        }

        void destroyNetworkObject(RkObject *object)
        {
            if (object == nullptr)
                return;
            const TkUuid id = object->getId();
            s_idRemappingTable.remove(id);
            s_idRemappingTable.removeValue(id);
            RkManager::destroyObject(object);
        }

        bool targetsContain(const TkDataCollection<u32> &targets, const u32 clientId)
        {
            if (!targets.isValid())
                return true;
            for (i32 i = 0; i < targets.length(); i++)
            {
                if (targets[i] == clientId)
                    return true;
            }
            return false;
        }

        void buildCachedTargets(const TkArray<NetClient *> &clients)
        {
            s_cachedTargets.clear();
            for (const NetClient *client : clients)
            {
                if (client != nullptr && client->state == NetConnectionState::kConnected)
                    s_cachedTargets.add(client->connection);
            }
        }

        void buildCachedTargets(const TkArray<NetClient *> &clients, const NetClient *excludedClient)
        {
            s_cachedTargets.clear();
            for (const NetClient *client : clients)
            {
                if (client != nullptr && client != excludedClient && client->state == NetConnectionState::kConnected)
                    s_cachedTargets.add(client->connection);
            }
        }

        void buildCachedTargets(const TkArray<NetClient *> &clients, const TkDataCollection<u32> &clientIds, const u32 excludedClientId = NetManager::kServerClientId, const NetClientsMask clientsMask = NetClientsMask::all)
        {
            s_cachedTargets.clear();
            for (i32 clientIndex = 0; clientIndex < clients.count(); clientIndex++)
            {
                const NetClient *client = clients[clientIndex];
                if (client != nullptr &&
                    client->state == NetConnectionState::kConnected &&
                    client->clientId != excludedClientId &&
                    clientsMask.hasBit(static_cast<u32>(clientIndex)) &&
                    targetsContain(clientIds, client->clientId))
                {
                    s_cachedTargets.add(client->connection);
                }
            }
        }

        void buildCachedTargets(const TkArray<NetClient *> &clients, const TkDataCollection<u32> &clientIds1, const TkSpan<u32> &clientIds2, const u32 excludedClientId = NetManager::kServerClientId)
        {
            s_cachedTargets.clear();
            for (const NetClient *client : clients)
            {
                if (client == nullptr || client->state != NetConnectionState::kConnected || client->clientId == excludedClientId)
                    continue;
                if (!targetsContain(clientIds1, client->clientId))
                    continue;
                if (clientIds2.isValid() && !spanContains(clientIds2, client->clientId))
                    continue;
                s_cachedTargets.add(client->connection);
            }
        }

        void buildCachedTargets(const ReplicatedObject &item, const NetClientsMask clientsMask = NetClientsMask::all)
        {
            buildCachedTargets(NetManager::clients, item.targetClientIds, item.ownerClientId, clientsMask);
        }

        void dirtyObjectImpl(ReplicatedObject &item, RkObject *object)
        {
            item.dirty = true;
            item.clearCache();
            if (s_hierarchy != nullptr)
                s_hierarchy->dirtyObject(object);
        }

        const Serializer *findSerializer(TypeHandle typeHandle)
        {
            while (typeHandle.isValid())
            {
                const auto iterator = s_serializers.find(typeHandle);
                if (iterator.isNotEnd())
                    return &iterator->value;

                const RkClass *klass = typeHandle.getType().asClass();
                typeHandle = klass != nullptr ? klass->baseType : TypeHandle();
            }
            return nullptr;
        }

        bool tryInvokeInterfaceSerializer(TypeHandle typeHandle, void *instance, NetStream *stream, const bool serialize)
        {
            if (!typeHandle.isValid() || typeHandle.getType().asClass() == nullptr)
                return false;

            auto *object = static_cast<RkObject *>(instance);
            auto *serializable = dynamic_cast<INetSerializable *>(object);
            if (serializable == nullptr)
                return false;

            if (serialize)
                serializable->serialize(stream);
            else
                serializable->deserialize(stream);
            return true;
        }

        ReplicatedObject &addObjectInternal(RkObject *object, const RkObject *parent)
        {
            TK_ASSERT(object != nullptr);

            if (!object->isRegistered())
                object->registerObject();

            ReplicatedObject item;
            item.object = object;
            item.objectId = object->getId();
            item.parentId = parent != nullptr ? parent->getId() : TkUuid::Empty;
            item.ownerClientId = NetManager::kServerClientId;
            item.role = NetManager::isClient() ? NetObjectRole::Replicated : NetObjectRole::OwnedAuthoritative;
            item.netObject = dynamic_cast<INetworkObject *>(object);
            s_objects[item.objectId] = item;
            s_despawnedObjects.remove(item.objectId);
            return s_objects[item.objectId];
        }

        void setupObjectSpawnMessageItem(SpawnItem &spawnItem, NetMessage &message)
        {
            RkObject *object = spawnItem.object;
            ReplicatedObject *item = resolveObject(object);
            if (object == nullptr || item == nullptr)
                return;

            NetMessageObjectSpawnItem msgDataItem;
            msgDataItem.objectId = item->objectId;
            msgDataItem.parentId = item->parentId;
            s_idRemappingTable.keyOf(msgDataItem.objectId, &msgDataItem.objectId);
            s_idRemappingTable.keyOf(msgDataItem.parentId, &msgDataItem.parentId);

            const TkStringView typeName = object->getClass().fullName;
            const i32 copyLength = typeName.length() < 127 ? typeName.length() : 127;
            if (copyLength > 0)
                TkMemory::memoryCopy(msgDataItem.objectTypeName, typeName.get(), copyLength);
            msgDataItem.objectTypeName[copyLength] = 0;
            message.writeStructure(msgDataItem);
        }

        void sendObjectSpawnMessage(SpawnItem &spawnItem, const TkArray<NetClient *> &clients)
        {
            RkObject *object = spawnItem.object;
            ReplicatedObject *item = resolveObject(object);
            if (object == nullptr || item == nullptr)
                return;

            NetPeer *peer = NetManager::peer;
            if (peer == nullptr)
                return;

            NetMessageObjectSpawn msgData;
            msgData.itemsCount = 1;
            msgData.ownerClientId = item->ownerClientId;
            msgData.ownerSpawnId = ++s_spawnId;
            msgData.useParts = 0;

            buildCachedTargets(clients, item->targetClientIds);
            NetMessage message = peer->beginSendMessage();
            message.writeStructure(msgData);
            setupObjectSpawnMessageItem(spawnItem, message);
            if (NetManager::isClient())
                peer->endSendMessage(NetChannelType::kReliable, message);
            else
                peer->endSendMessage(NetChannelType::kReliable, message, s_cachedTargets);
        }

        void sendObjectRoleMessage(const ReplicatedObject &item, const NetClient *excludedClient = nullptr)
        {
            NetPeer *peer = NetManager::peer;
            if (peer == nullptr)
                return;

            NetMessageObjectRole msgData;
            msgData.ownerClientId = item.ownerClientId;
            TkUuid objectId = item.objectId;
            s_idRemappingTable.keyOf(objectId, &objectId);

            NetMessage message = peer->beginSendMessage();
            message.writeStructure(msgData);
            message.writeNetId(objectId);
            if (NetManager::isClient())
            {
                peer->endSendMessage(NetChannelType::kReliableOrdered, message);
            }
            else
            {
                buildCachedTargets(NetManager::clients, excludedClient);
                peer->endSendMessage(NetChannelType::kReliableOrdered, message, s_cachedTargets);
            }
        }

        ReplicateItem *addObjectReplicateItem(NetEvent &event, const u32 ownerFrame, const u16 partsCount, const u16 dataSize, const TkUuid &objectId, const u16 partStart, const u16 partSize, const u32 senderClientId)
        {
            ReplicateItem *replicateItem = nullptr;
            for (ReplicateItem &item : s_replicationParts)
            {
                if (item.ownerFrame == ownerFrame && item.data.count() == dataSize && item.objectId == objectId)
                {
                    replicateItem = &item;
                    break;
                }
            }
            if (replicateItem == nullptr)
            {
                replicateItem = &s_replicationParts.addOne();
                replicateItem->objectId = objectId;
                replicateItem->partsLeft = partsCount;
                replicateItem->ownerFrame = ownerFrame;
                replicateItem->ownerClientId = senderClientId;
                replicateItem->data.resize(dataSize);
            }

            TK_ASSERT(replicateItem->partsLeft > 0);
            replicateItem->partsLeft--;
            TK_ASSERT(partStart + partSize <= replicateItem->data.count());
            const void *partData = event.message.skipBytes(partSize);
            TkMemory::memoryCopy(replicateItem->data.get() + partStart, partData, partSize);
            return replicateItem;
        }

        void invokeObjectReplication(ReplicatedObject &item, const u32 ownerFrame, byte *data, const u32 dataSize, const u32 senderClientId)
        {
            RkObject *object = item.object;
            if (object == nullptr || item.role == NetObjectRole::OwnedAuthoritative || item.lastOwnerFrame >= ownerFrame)
                return;
            item.lastOwnerFrame = ownerFrame;

            if (s_cachedReadStream == nullptr)
                s_cachedReadStream = TkMemory::newObject<NetStream>();
            NetStream *stream = s_cachedReadStream;
            stream->init(data, dataSize);
            stream->senderId = senderClientId;

            if (!NetReplicator::invokeSerializer(object->getTypeHandle(), object, stream, false))
                return;

            if (item.netObject != nullptr)
            {
                item.netObject->onNetworkDeserialize();
                if (!item.synced)
                {
                    item.synced = true;
                    item.netObject->onNetworkSync();
                }
            }

            if (NetManager::isServer())
                dirtyObjectImpl(item, object);
        }

        void invokeObjectSpawn(const NetMessageObjectSpawn &msgData, const NetMessageObjectSpawnItem *msgDataItems)
        {
            if (msgData.itemsCount == 0)
                return;

            ReplicatedObject *root = resolveObject(msgDataItems[0].objectId, msgDataItems[0].parentId, TkStringView(msgDataItems[0].objectTypeName));
            if (root != nullptr)
            {
                for (i32 i = 0; i < msgData.itemsCount; i++)
                {
                    ReplicatedObject *item = resolveObject(msgDataItems[i].objectId, msgDataItems[i].parentId, TkStringView(msgDataItems[i].objectTypeName));
                    if (item == nullptr)
                        continue;
                    item->spawned = true;
                    if (NetManager::isClient())
                    {
                        item->ownerClientId = msgData.ownerClientId;
                        item->clearCache();
                        if (item->role == NetObjectRole::OwnedAuthoritative && s_hierarchy != nullptr)
                            s_hierarchy->removeObject(item->object);
                        item->role = item->ownerClientId == NetManager::localClientId ? NetObjectRole::OwnedAuthoritative : NetObjectRole::Replicated;
                        if (item->role == NetObjectRole::OwnedAuthoritative && s_hierarchy != nullptr)
                            s_hierarchy->addObject(item->object);
                    }
                }
                return;
            }

            TkArray<RkObject *> objects;
            objects.resize(msgData.itemsCount);
            for (i32 i = 0; i < msgData.itemsCount; i++)
            {
                const NetMessageObjectSpawnItem &msgDataItem = msgDataItems[i];
                TypeHandle objectType = RkManager::findType(TkStringView(msgDataItem.objectTypeName));
                RkObject *object = objectType.isValid() ? RkManager::newObject(objectType) : nullptr;
                if (object == nullptr)
                {
                    for (RkObject *createdObject : objects)
                    {
                        if (createdObject != nullptr)
                            destroyNetworkObject(createdObject);
                    }
                    return;
                }
                objects[i] = object;
            }

            for (i32 i = 0; i < msgData.itemsCount; i++)
            {
                const NetMessageObjectSpawnItem &msgDataItem = msgDataItems[i];
                RkObject *object = objects[i];
                const ReplicatedObject *parent = resolveObject(msgDataItem.parentId);

                ReplicatedObject item;
                item.object = object;
                item.netObject = dynamic_cast<INetworkObject *>(object);
                item.objectId = object->getId();
                item.parentId = parent != nullptr ? parent->objectId : remapObjectId(msgDataItem.parentId);
                item.ownerClientId = msgData.ownerClientId;
                item.role = item.ownerClientId == NetManager::localClientId ? NetObjectRole::OwnedAuthoritative : NetObjectRole::Replicated;
                item.spawned = true;
                s_objects[item.objectId] = item;
                if (s_hierarchy != nullptr && item.role == NetObjectRole::OwnedAuthoritative)
                    s_hierarchy->addObject(object);
                s_idRemappingTable[msgDataItem.objectId] = item.objectId;
            }

            for (RkObject *object : objects)
            {
                ReplicatedObject *item = resolveObject(object);
                if (item != nullptr && item->netObject != nullptr)
                    item->netObject->onNetworkSpawn();
            }
        }
    }

    NetReplicationHierarchy *NetReplicator::getHierarchy()
    {
        return s_hierarchy;
    }

    void NetReplicator::setHierarchy(NetReplicationHierarchy *value)
    {
        TkMutexGuard lock(s_objectsLock);
        if (s_hierarchy == value)
            return;
        if (s_hierarchy != nullptr)
            TkMemory::deletePtr(s_hierarchy);
        s_hierarchy = value;
        if (s_hierarchy != nullptr)
        {
            for (auto iterator = s_objects.Begin(); iterator.isNotEnd(); ++iterator)
            {
                ReplicatedObject &item = iterator->value;
                if (item.object != nullptr && item.role == NetObjectRole::OwnedAuthoritative)
                    s_hierarchy->addObject(item.object);
            }
        }
    }

    void NetReplicator::addSerializer(const TypeHandle &typeHandle, SerializeFunc serialize, SerializeFunc deserialize, void *serializeTag, void *deserializeTag)
    {
        if (!typeHandle.isValid())
            return;
        Serializer serializer;
        serializer.methods[0] = serialize;
        serializer.methods[1] = deserialize;
        serializer.tags[0] = serializeTag;
        serializer.tags[1] = deserializeTag;
        s_serializers[typeHandle] = serializer;
    }

    bool NetReplicator::invokeSerializer(const TypeHandle &typeHandle, void *instance, NetStream *stream, const bool serialize)
    {
        if (!typeHandle.isValid() || instance == nullptr || stream == nullptr)
            return false;

        const Serializer *serializer = findSerializer(typeHandle);
        if (serializer != nullptr)
        {
            const u32 index = serialize ? 0 : 1;
            if (serializer->methods[index] == nullptr)
                return false;
            serializer->methods[index](instance, stream, serializer->tags[index]);
            return true;
        }

        return tryInvokeInterfaceSerializer(typeHandle, instance, stream, serialize);
    }

    void NetReplicator::addObject(RkObject *obj, const RkObject *parent)
    {
        if (obj == nullptr || NetManager::isOffline())
            return;
        TkMutexGuard lock(s_objectsLock);
        if (resolveObject(obj) != nullptr)
            return;

        ReplicatedObject &item = addObjectInternal(obj, parent);
        if (s_hierarchy != nullptr && item.role == NetObjectRole::OwnedAuthoritative)
            s_hierarchy->addObject(obj);
    }

    void NetReplicator::removeObject(RkObject *obj)
    {
        if (obj == nullptr || NetManager::isOffline())
            return;
        TkMutexGuard lock(s_objectsLock);
        ReplicatedObject *item = resolveObject(obj);
        if (item == nullptr)
            return;

        if (s_hierarchy != nullptr && item->role == NetObjectRole::OwnedAuthoritative)
            s_hierarchy->removeObject(obj);
        s_idRemappingTable.removeValue(item->objectId);
        s_objects.remove(item->objectId);
    }

    void NetReplicator::spawnObject(RkObject *obj)
    {
        spawnObject(obj, TkDataCollection<u32>());
    }

    void NetReplicator::spawnObject(RkObject *obj, const TkDataCollection<u32> &clientIds)
    {
        if (obj == nullptr || NetManager::isOffline())
            return;
        TkMutexGuard lock(s_objectsLock);
        ReplicatedObject *item = resolveObject(obj);
        if (item != nullptr && item->spawned)
            return;

        SpawnItem &spawn = s_spawnQueue.addOne();
        spawn.object = obj;
        spawn.ownerClientId = NetManager::localClientId;
        spawn.role = NetObjectRole::OwnedAuthoritative;
        if (clientIds.isValid())
            spawn.targets.copy(clientIds);
    }

    void NetReplicator::despawnObject(RkObject *obj)
    {
        if (obj == nullptr || NetManager::isOffline())
            return;
        TkMutexGuard lock(s_objectsLock);
        ReplicatedObject *item = resolveObject(obj);
        if (item == nullptr)
        {
            for (i32 i = 0; i < s_spawnQueue.count(); i++)
            {
                if (s_spawnQueue[i].object == obj)
                {
                    s_spawnQueue.removeAtKeepOrder(i);
                    destroyNetworkObject(obj);
                    break;
                }
            }
            return;
        }
        if (item->object != obj || !item->spawned || item->ownerClientId != NetManager::localClientId)
            return;

        DespawnItem &despawn = s_despawnQueue.addOne();
        despawn.id = obj->getId();
        despawn.targets = item->targetClientIds;

        s_despawnedObjects.add(item->objectId);
        if (item->netObject != nullptr)
            item->netObject->onNetworkDespawn();
        if (s_hierarchy != nullptr && item->role == NetObjectRole::OwnedAuthoritative)
            s_hierarchy->removeObject(obj);
        const TkUuid objectId = item->objectId;
        s_objects.remove(objectId);
        destroyNetworkObject(obj);
    }

    bool NetReplicator::hasObject(const RkObject *obj)
    {
        if (obj == nullptr)
            return false;
        TkMutexGuard lock(s_objectsLock);
        if (resolveObject(obj) != nullptr)
            return true;
        for (const SpawnItem &item : s_spawnQueue)
        {
            if (item.object == obj)
                return true;
        }
        return false;
    }

    void NetReplicator::mapObjectId(TkUuid &objectId)
    {
        TkMutexGuard lock(s_objectsLock);
        const auto iterator = s_idRemappingTable.find(objectId);
        if (iterator.isNotEnd())
            objectId = iterator->value;
        else
            s_idRemappingTable.keyOf(objectId, &objectId);
    }

    void NetReplicator::addObjectIdMapping(const RkObject *obj, const TkUuid &objectId)
    {
        if (obj == nullptr || !objectId.isValid())
            return;
        TkMutexGuard lock(s_objectsLock);
        s_idRemappingTable[objectId] = obj->getId();
    }

    RkObject *NetReplicator::resolveForeignObject(TkUuid objectId)
    {
        TkMutexGuard lock(s_objectsLock);
        if (ReplicatedObject *object = resolveObject(objectId))
            return object->object;
        objectId = remapObjectId(objectId);
        return RkManager::findObject(objectId);
    }

    u32 NetReplicator::getObjectOwnerClientId(const RkObject *obj)
    {
        if (obj == nullptr || !NetManager::isConnected())
            return NetManager::kServerClientId;
        TkMutexGuard lock(s_objectsLock);
        if (ReplicatedObject *item = resolveObject(obj))
            return item->ownerClientId;
        for (const SpawnItem &item : s_spawnQueue)
        {
            if (item.object == obj && item.hasOwnership)
                return item.ownerClientId;
        }
        return NetManager::kServerClientId;
    }

    NetObjectRole NetReplicator::getObjectRole(const RkObject *obj)
    {
        if (obj == nullptr || !NetManager::isConnected())
            return NetObjectRole::None;
        TkMutexGuard lock(s_objectsLock);
        if (ReplicatedObject *item = resolveObject(obj))
            return item->role;
        for (const SpawnItem &item : s_spawnQueue)
        {
            if (item.object == obj && item.hasOwnership)
                return item.role;
        }
        return NetObjectRole::None;
    }

    void NetReplicator::setObjectOwnership(RkObject *obj, const u32 ownerClientId, const NetObjectRole localRole, const bool hierarchical)
    {
        if (obj == nullptr || NetManager::isOffline())
            return;
        TkMutexGuard lock(s_objectsLock);
        ReplicatedObject *item = resolveObject(obj);
        if (item == nullptr)
        {
            for (SpawnItem &spawnItem : s_spawnQueue)
            {
                if (spawnItem.object == obj)
                {
                    spawnItem.hasOwnership = true;
                    spawnItem.hierarchicalOwnership = hierarchical;
                    spawnItem.ownerClientId = ownerClientId;
                    spawnItem.role = localRole;
                    return;
                }
            }
            return;
        }

        if (item->object != obj)
            return;
        if (s_hierarchy != nullptr && item->role == NetObjectRole::OwnedAuthoritative && localRole != NetObjectRole::OwnedAuthoritative)
            s_hierarchy->removeObject(obj);
        item->ownerClientId = ownerClientId;
        item->lastOwnerFrame = ownerClientId == NetManager::localClientId ? 0 : 1;
        item->role = localRole;
        item->clearCache();
        if (s_hierarchy != nullptr && item->role == NetObjectRole::OwnedAuthoritative)
            s_hierarchy->addObject(obj);
        if (item->spawned)
            sendObjectRoleMessage(*item);

        if (hierarchical)
        {
            const TkUuid objectId = obj->getId();
            for (auto iterator = s_objects.Begin(); iterator.isNotEnd(); ++iterator)
            {
                ReplicatedObject &child = iterator->value;
                if (child.parentId == objectId)
                {
                    child.ownerClientId = ownerClientId;
                    child.lastOwnerFrame = ownerClientId == NetManager::localClientId ? 0 : 1;
                    child.role = localRole;
                    child.clearCache();
                }
            }
        }
    }

    void NetReplicator::dirtyObject(RkObject *obj)
    {
        if (obj == nullptr)
            return;
        TkMutexGuard lock(s_objectsLock);
        ReplicatedObject *item = resolveObject(obj);
        if (item == nullptr || item->object != obj || item->role != NetObjectRole::OwnedAuthoritative)
            return;
        dirtyObjectImpl(*item, obj);
    }

    NetStream *NetReplicator::beginInvokeRPC()
    {
        if (s_cachedWriteStream == nullptr)
            s_cachedWriteStream = TkMemory::newObject<NetStream>();
        s_cachedWriteStream->init();
        s_cachedWriteStream->senderId = NetManager::localClientId;
        return s_cachedWriteStream;
    }

    bool NetReplicator::endInvokeRPC(RkObject *obj, const TypeHandle &type, const TkStringView &name, NetStream *argsStream, TkSpan<u32> targetIds)
    {
        if (targetIds.isValid() && targetIds.length() == 0)
            return true;
        if (obj == nullptr || !type.isValid() || argsStream == nullptr || NetManager::isOffline())
            return false;

        const NetRpcInfo *info = nullptr;
        {
            const auto infoIterator = NetRpcInfo::rpcsTable.find(NetRpcName(type, name));
            if (infoIterator.isNotEnd())
                info = &infoIterator->value;
        }
        if (info == nullptr)
            return false;

        TkMutexGuard lock(s_objectsLock);
        RpcItem &rpc = s_rpcQueue.addOne();
        rpc.object = obj;
        rpc.name = NetRpcName(type, name);
        rpc.info = *info;
        if (argsStream->pos() > 0)
            rpc.argsData.copy(argsStream->getBuffer(), argsStream->pos());
        if (targetIds.isValid())
            rpc.targets.copy(targetIds);

        return true;
    }

    NetRpcParams::NetRpcParams(const NetStream *stream)
    {
        if (stream != nullptr)
            senderId = stream->senderId;
    }

    TkHashMap<NetRpcName, NetRpcInfo> NetRpcInfo::rpcsTable;

    void NetInternal::netReplicatorClientConnected(NetClient *client)
    {
        TkMutexGuard lock(s_objectsLock);
        s_newClients.addUnique(client);
    }

    void NetInternal::netReplicatorClientDisconnected(NetClient *client)
    {
        if (client == nullptr)
            return;
        TkMutexGuard lock(s_objectsLock);
        s_newClients.remove(client);

        const u32 clientId = client->clientId;
        TkArray<TkUuid> objectsToRemove;
        for (auto iterator = s_objects.Begin(); iterator.isNotEnd(); ++iterator)
        {
            ReplicatedObject &item = iterator->value;
            if (item.object != nullptr && item.spawned && item.ownerClientId == clientId)
            {
                DespawnItem &despawn = s_despawnQueue.addOne();
                despawn.id = item.objectId;
                despawn.targets = item.targetClientIds;
                if (s_hierarchy != nullptr && item.role == NetObjectRole::OwnedAuthoritative)
                    s_hierarchy->removeObject(item.object);
                if (item.netObject != nullptr)
                    item.netObject->onNetworkDespawn();
                destroyNetworkObject(item.object);
                objectsToRemove.add(item.objectId);
            }
        }
        for (const TkUuid &id : objectsToRemove)
            s_objects.remove(id);
    }

    void NetInternal::netReplicatorClear()
    {
        TkMutexGuard lock(s_objectsLock);
        if (s_hierarchy != nullptr)
        {
            TkMemory::deletePtr(s_hierarchy);
            s_hierarchy = nullptr;
        }

        TkArray<TkUuid> objectsToRemove;
        for (auto iterator = s_objects.Begin(); iterator.isNotEnd(); ++iterator)
        {
            ReplicatedObject &item = iterator->value;
            if (item.object != nullptr && item.spawned)
            {
                if (item.netObject != nullptr)
                    item.netObject->onNetworkDespawn();
                destroyNetworkObject(item.object);
            }
            objectsToRemove.add(item.objectId);
        }
        for (const TkUuid &id : objectsToRemove)
            s_objects.remove(id);

        s_objects.clear();
        s_replicationParts.clear();
        s_spawnParts.clear();
        s_spawnQueue.clear();
        s_despawnQueue.clear();
        s_rpcQueue.clear();
        s_idRemappingTable.clear();
        s_newClients.clear();
        s_cachedTargets.clear();
        s_despawnedObjects.clear();
        if (s_cachedWriteStream != nullptr)
        {
            TkMemory::deletePtr(s_cachedWriteStream);
            s_cachedWriteStream = nullptr;
        }
        if (s_cachedReadStream != nullptr)
        {
            TkMemory::deletePtr(s_cachedReadStream);
            s_cachedReadStream = nullptr;
        }
        if (s_cachedReplicationResult != nullptr)
        {
            TkMemory::deletePtr(s_cachedReplicationResult);
            s_cachedReplicationResult = nullptr;
        }
    }

    void NetInternal::netReplicatorPreUpdate()
    {
    }

    void NetInternal::netReplicatorUpdate()
    {
        TkMutexGuard lock(s_objectsLock);
        NetPeer *peer = NetManager::peer;
        if (peer == nullptr)
            return;

        const bool isClient = NetManager::isClient();
        const bool isServer = NetManager::isServer();
        const bool isHost = NetManager::isHost();

        if (!isClient && s_newClients.hasItems())
        {
            for (auto iterator = s_objects.Begin(); iterator.isNotEnd(); ++iterator)
            {
                ReplicatedObject &item = iterator->value;
                if (item.object == nullptr || !item.spawned)
                    continue;
                SpawnItem spawnItem;
                spawnItem.object = item.object;
                spawnItem.ownerClientId = item.ownerClientId;
                spawnItem.role = item.role;
                spawnItem.targets.link(item.targetClientIds);
                sendObjectSpawnMessage(spawnItem, s_newClients);
            }
            s_newClients.clear();
        }

        for (DespawnItem &despawn : s_despawnQueue)
        {
            NetMessageObjectDespawn msgData;
            TkUuid objectId = despawn.id;
            s_idRemappingTable.keyOf(objectId, &objectId);
            NetMessage message = peer->beginSendMessage();
            message.writeStructure(msgData);
            message.writeNetId(objectId);
            buildCachedTargets(NetManager::clients, despawn.targets);
            if (isClient)
                peer->endSendMessage(NetChannelType::kReliableOrdered, message);
            else
                peer->endSendMessage(NetChannelType::kReliableOrdered, message, s_cachedTargets);
        }
        s_despawnQueue.clear();

        for (SpawnItem &spawn : s_spawnQueue)
        {
            RkObject *object = spawn.object;
            if (object == nullptr)
                continue;
            ReplicatedObject *item = resolveObject(object);
            if (item == nullptr)
                item = &addObjectInternal(object, nullptr);
            if (!isLocalOwner(*item))
                continue;

            if (spawn.hasOwnership)
            {
                item->ownerClientId = spawn.ownerClientId;
                item->role = spawn.role;
                item->clearCache();
            }
            if (spawn.targets.isValid())
                item->targetClientIds = spawn.targets;
            item->spawned = true;
            sendObjectSpawnMessage(spawn, NetManager::clients);
        }
        s_spawnQueue.clear();

        for (i32 i = s_replicationParts.count() - 1; i >= 0; --i)
        {
            ReplicateItem &part = s_replicationParts[i];
            if (part.partsLeft > 0)
                continue;
            ReplicatedObject *item = part.object != nullptr ? resolveObject(part.object) : resolveObject(part.objectId);
            if (item != nullptr)
                invokeObjectReplication(*item, part.ownerFrame, part.data.get(), static_cast<u32>(part.data.count()), part.ownerClientId);
            s_replicationParts.removeAtKeepOrder(i);
        }

        if (s_cachedReplicationResult == nullptr)
            s_cachedReplicationResult = TkMemory::newObject<NetReplicationHierarchyUpdateResult>();
        s_cachedReplicationResult->init();
        if ((!isClient && NetManager::clients.isEmpty()) || NetManager::networkFPS < 0.0f)
        {
        }
        else if (s_hierarchy != nullptr)
        {
            s_hierarchy->update(s_cachedReplicationResult);
        }
        else
        {
            for (auto iterator = s_objects.Begin(); iterator.isNotEnd(); ++iterator)
            {
                ReplicatedObject &item = iterator->value;
                if (item.object == nullptr)
                    continue;
                if (item.role == NetObjectRole::OwnedAuthoritative || (isServer && item.dirty))
                    s_cachedReplicationResult->addObject(item.object);
            }
        }

        if (s_cachedReplicationResult->entries.hasItems())
        {
            if (s_cachedWriteStream == nullptr)
                s_cachedWriteStream = TkMemory::newObject<NetStream>();
            NetStream *stream = s_cachedWriteStream;
            stream->senderId = NetManager::localClientId;

            for (const NetReplicationHierarchyUpdateResult::Entry &entry : s_cachedReplicationResult->entries)
            {
                RkObject *object = entry.object;
                ReplicatedObject *item = resolveObject(object);
                if (object == nullptr || item == nullptr)
                    continue;

                if (!isClient)
                {
                    buildCachedTargets(*item, entry.targetClients);
                    if (s_cachedTargets.isEmpty())
                        continue;
                }

                if (item->netObject != nullptr)
                    item->netObject->onNetworkSerialize();

                stream->init();
                if (!NetReplicator::invokeSerializer(object->getTypeHandle(), object, stream, true))
                    continue;
                const u32 size = stream->pos();
                if (size > 0xffffu)
                {
                    LogError("Too much data for object replication");
                    continue;
                }

                if (kUseNetReplicatorCache &&
                    item->cacheData.length() == static_cast<i32>(size) &&
                    item->cacheMask == entry.targetClients &&
                    TkMemory::memoryCompare(item->cacheData.get(), stream->getBuffer(), size) == 0)
                {
                    continue;
                }
                item->cacheMask = entry.targetClients;
                if (size > 0)
                    item->cacheData.copy(stream->getBuffer(), size);
                else
                    item->cacheData.release();
                item->dirty = false;

                constexpr NetChannelType repChannel = NetChannelType::kReliable;
                NetMessageObjectReplicate msgData;
                msgData.ownerFrame = NetManager::frame;
                TkUuid objectId = item->objectId;
                TkUuid parentId = item->parentId;
                s_idRemappingTable.keyOf(objectId, &objectId);
                s_idRemappingTable.keyOf(parentId, &parentId);

                NetMessage message = peer->beginSendMessage();
                message.writeStructure(msgData);
                message.writeNetId(objectId);
                message.writeNetId(parentId);
                message.writeNetName(object->getClass().fullName);

                NetMessageObjectReplicatePayload payload;
                payload.dataSize = static_cast<u16>(size);
                const u32 networkIdWorstCaseSize = sizeof(TkUuid);
                const u32 msgMaxData = peer->config.messageSize - message.position - sizeof(NetMessageObjectReplicatePayload);
                const u32 partMaxData = peer->config.messageSize - sizeof(NetMessageObjectReplicatePart) - networkIdWorstCaseSize;
                u32 partsCount = 1;
                u32 dataStart = 0;
                u32 msgDataSize = size;
                if (size > msgMaxData)
                {
                    msgDataSize = msgMaxData;
                    dataStart += msgMaxData;
                    partsCount += (size - dataStart + partMaxData - 1) / partMaxData;
                }
                else
                {
                    dataStart += size;
                }

                payload.partsCount = static_cast<u16>(partsCount);
                payload.partSize = static_cast<u16>(msgDataSize);
                message.writeStructure(payload);
                message.writeBytes(stream->getBuffer(), msgDataSize);
                if (isClient)
                    peer->endSendMessage(repChannel, message);
                else
                    peer->endSendMessage(repChannel, message, s_cachedTargets);

                for (u32 partIndex = 1; partIndex < partsCount; partIndex++)
                {
                    NetMessageObjectReplicatePart partMsg;
                    partMsg.ownerFrame = msgData.ownerFrame;
                    partMsg.dataSize = payload.dataSize;
                    partMsg.partsCount = payload.partsCount;
                    partMsg.partStart = static_cast<u16>(dataStart);
                    partMsg.partSize = static_cast<u16>((size - dataStart) < partMaxData ? (size - dataStart) : partMaxData);
                    message = peer->beginSendMessage();
                    message.writeStructure(partMsg);
                    message.writeNetId(objectId);
                    message.writeBytes(stream->getBuffer() + partMsg.partStart, partMsg.partSize);
                    dataStart += partMsg.partSize;
                    if (isClient)
                        peer->endSendMessage(repChannel, message);
                    else
                        peer->endSendMessage(repChannel, message, s_cachedTargets);
                }
            }
        }

        for (RpcItem &rpc : s_rpcQueue)
        {
            RkObject *object = rpc.object;
            ReplicatedObject *item = resolveObject(object);
            if (object == nullptr || item == nullptr)
                continue;

            NetMessageObjectRpc msgData;
            msgData.argsSize = static_cast<u16>(rpc.argsData.length());
            TkUuid objectId = item->objectId;
            TkUuid parentId = item->parentId;
            s_idRemappingTable.keyOf(objectId, &objectId);
            s_idRemappingTable.keyOf(parentId, &parentId);

            NetMessage message = peer->beginSendMessage();
            message.writeStructure(msgData);
            message.writeNetId(objectId);
            message.writeNetId(parentId);
            message.writeNetName(object->getClass().fullName);
            message.writeNetName(rpc.name.first.getType().fullName);
            message.writeNetName(rpc.name.second);
            if (rpc.argsData.length() > 0)
                message.writeBytes(rpc.argsData.get(), rpc.argsData.length());

            const NetChannelType channel = static_cast<NetChannelType>(rpc.info.channel);
            if (rpc.info.server && isClient)
            {
                peer->endSendMessage(channel, message);
            }
            else if (rpc.info.client && (isServer || isHost))
            {
                buildCachedTargets(NetManager::clients, item->targetClientIds, rpc.targets, NetManager::localClientId);
                peer->endSendMessage(channel, message, s_cachedTargets);
            }
            else
            {
                peer->abortSendMessage(message);
            }
        }
        s_rpcQueue.clear();
    }

    void NetInternal::onNetMessageObjectReplicate(NetEvent &event, NetClient *client, NetPeer *)
    {
        NetMessageObjectReplicate msgData;
        NetMessageObjectReplicatePayload payload;
        TkUuid objectId;
        TkUuid parentId;
        TkStringView objectTypeName;
        event.message.readStructure(msgData);
        event.message.readNetId(objectId);
        event.message.readNetId(parentId);
        event.message.readNetName(objectTypeName);
        event.message.readStructure(payload);

        TkMutexGuard lock(s_objectsLock);
        if (s_despawnedObjects.contains(objectId))
            return;
        ReplicatedObject *item = resolveObject(objectId, parentId, objectTypeName);
        if (item == nullptr)
            return;
        if (client != nullptr && item->ownerClientId != client->clientId)
            return;

        const u32 senderClientId = client != nullptr ? client->clientId : NetManager::kServerClientId;
        if (payload.partsCount == 1)
        {
            invokeObjectReplication(*item, msgData.ownerFrame, event.message.buffer + event.message.position, payload.dataSize, senderClientId);
        }
        else
        {
            ReplicateItem *replicateItem = addObjectReplicateItem(event, msgData.ownerFrame, payload.partsCount, payload.dataSize, objectId, 0, payload.partSize, senderClientId);
            replicateItem->object = item->object;
        }
    }

    void NetInternal::onNetMessageObjectReplicatePart(NetEvent &event, NetClient *client, NetPeer *)
    {
        NetMessageObjectReplicatePart msgData;
        TkUuid objectId;
        event.message.readStructure(msgData);
        event.message.readNetId(objectId);

        TkMutexGuard lock(s_objectsLock);
        if (s_despawnedObjects.contains(objectId))
            return;
        const u32 senderClientId = client != nullptr ? client->clientId : NetManager::kServerClientId;
        addObjectReplicateItem(event, msgData.ownerFrame, msgData.partsCount, msgData.dataSize, objectId, msgData.partStart, msgData.partSize, senderClientId);
    }

    void NetInternal::onNetMessageObjectSpawn(NetEvent &event, NetClient *, NetPeer *)
    {
        NetMessageObjectSpawn msgData;
        event.message.readStructure(msgData);
        if (msgData.itemsCount == 0)
            return;

        TkMutexGuard lock(s_objectsLock);
        if (msgData.useParts)
        {
            SpawnItemParts &parts = s_spawnParts.addOne();
            parts.msgData = msgData;
            parts.items.resize(msgData.itemsCount);
            for (NetMessageObjectSpawnItem &item : parts.items)
                item.objectId = TkUuid::Empty;
        }
        else
        {
            const auto *items = static_cast<NetMessageObjectSpawnItem *>(event.message.skipBytes(msgData.itemsCount * sizeof(NetMessageObjectSpawnItem)));
            invokeObjectSpawn(msgData, items);
        }
    }

    void NetInternal::onNetMessageObjectSpawnPart(NetEvent &event, NetClient *, NetPeer *)
    {
        NetMessageObjectSpawnPart msgData;
        event.message.readStructure(msgData);

        TkMutexGuard lock(s_objectsLock);
        i32 spawnPartsIndex = -1;
        for (i32 i = 0; i < s_spawnParts.count(); i++)
        {
            const SpawnItemParts &parts = s_spawnParts[i];
            if (parts.msgData.ownerClientId == msgData.ownerClientId && parts.msgData.ownerSpawnId == msgData.ownerSpawnId)
            {
                spawnPartsIndex = i;
                break;
            }
        }
        if (spawnPartsIndex == -1)
            return;

        SpawnItemParts &spawnParts = s_spawnParts[spawnPartsIndex];
        constexpr u32 spawnItemMaxSize = sizeof(u16) + sizeof(NetMessageObjectSpawnItem);
        while (event.message.position + spawnItemMaxSize <= event.message.length)
        {
            const u16 itemIndex = event.message.readU16();
            if (itemIndex < spawnParts.items.count())
                event.message.readStructure(spawnParts.items[itemIndex]);
        }

        for (const NetMessageObjectSpawnItem &item : spawnParts.items)
        {
            if (!item.objectId.isValid())
                return;
        }
        invokeObjectSpawn(spawnParts.msgData, spawnParts.items.get());
        s_spawnParts.removeAtKeepOrder(spawnPartsIndex);
    }

    void NetInternal::onNetMessageObjectDespawn(NetEvent &event, NetClient *client, NetPeer *)
    {
        NetMessageObjectDespawn msgData;
        TkUuid objectId;
        event.message.readStructure(msgData);
        event.message.readNetId(objectId);

        TkMutexGuard lock(s_objectsLock);
        ReplicatedObject *item = resolveObject(objectId);
        if (item == nullptr || item->object == nullptr || !item->spawned)
            return;
        if (client != nullptr && item->ownerClientId != client->clientId)
            return;

        RkObject *object = item->object;
        if (s_hierarchy != nullptr && item->role == NetObjectRole::OwnedAuthoritative)
            s_hierarchy->removeObject(object);
        s_despawnedObjects.add(objectId);
        if (item->netObject != nullptr)
            item->netObject->onNetworkDespawn();
        const TkUuid localObjectId = item->objectId;
        s_objects.remove(localObjectId);
        destroyNetworkObject(object);
    }

    void NetInternal::onNetMessageObjectRole(NetEvent &event, NetClient *client, NetPeer *)
    {
        NetMessageObjectRole msgData;
        TkUuid objectId;
        event.message.readStructure(msgData);
        event.message.readNetId(objectId);

        TkMutexGuard lock(s_objectsLock);
        ReplicatedObject *item = resolveObject(objectId);
        if (item == nullptr || item->object == nullptr)
            return;
        if (client != nullptr && item->ownerClientId != client->clientId)
            return;

        item->ownerClientId = msgData.ownerClientId;
        item->lastOwnerFrame = 1;
        if (item->ownerClientId == NetManager::localClientId)
        {
            if (s_hierarchy != nullptr && item->role != NetObjectRole::OwnedAuthoritative)
                s_hierarchy->addObject(item->object);
            item->role = NetObjectRole::OwnedAuthoritative;
            item->lastOwnerFrame = 0;
        }
        else if (item->role == NetObjectRole::OwnedAuthoritative)
        {
            if (s_hierarchy != nullptr)
                s_hierarchy->removeObject(item->object);
            item->role = NetObjectRole::Replicated;
        }

        if (!NetManager::isClient())
            sendObjectRoleMessage(*item, client);
    }

    void NetInternal::onNetMessageObjectRpc(NetEvent &event, NetClient *client, NetPeer *)
    {
        NetMessageObjectRpc msgData;
        TkUuid objectId;
        TkUuid parentId;
        TkStringView objectTypeName;
        TkStringView rpcTypeName;
        TkStringView rpcName;
        event.message.readStructure(msgData);
        event.message.readNetId(objectId);
        event.message.readNetId(parentId);
        event.message.readNetName(objectTypeName);
        event.message.readNetName(rpcTypeName);
        event.message.readNetName(rpcName);

        TkMutexGuard lock(s_objectsLock);
        NetRpcName name;
        name.first = RkManager::findType(rpcTypeName);
        name.second = rpcName;
        const auto infoIterator = NetRpcInfo::rpcsTable.find(name);
        if (infoIterator.isEnd())
            return;
        const NetRpcInfo &info = infoIterator->value;

        ReplicatedObject *item = resolveObject(objectId, parentId, objectTypeName);
        if (item == nullptr || item->object == nullptr)
        {
            if (info.channel != static_cast<u8>(NetChannelType::kUnreliable) && info.channel != static_cast<u8>(NetChannelType::kUnreliableOrdered))
                logReplicator("NetReplicator: unknown object RPC.");
            return;
        }

        if (info.server && NetManager::isClient())
            return;
        if (info.client && NetManager::isServer())
            return;

        if (s_cachedReadStream == nullptr)
            s_cachedReadStream = TkMemory::newObject<NetStream>();
        NetStream *stream = s_cachedReadStream;
        stream->senderId = client != nullptr ? client->clientId : NetManager::kServerClientId;
        stream->init(event.message.buffer + event.message.position, msgData.argsSize);

        if (info.execute != nullptr)
            info.execute(item->object, stream, info.tag);
    }
}
