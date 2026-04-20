/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/net/common.hpp>

namespace toolkit
{

    // The high-level network object role and authority. Used to define who owns the object and when it can be simulated or just replicated.
    enum class NetObjectRole : byte
    {
        // Not replicated object.
        None = 0,
        // Server/client owns the object and replicates it to others. Only owning client can simulate object and provides current state.
        OwnedAuthoritative,
        // Server/client gets replicated object from other server/client who owns it. Object cannot be simulated locally (any changes will be overriden by replication).
        Replicated,
        // Client gets replicated object from server but still can locally autonomously simulate it too. For example, client can control local pawn with real human input but will validate with server proper state (eg. to prevent cheats).
        ReplicatedSimulated,
    };

    // High-level networking replication system for game objects.
    class TK_API NetwReplicator
    {
        friend class NetwReplicatorInternal;
        typedef void (*SerializeFunc)(void *instance, NetStream *stream, void *tag);

    public:
#if TK_DEBUG
        // Enables verbose logging of the networking runtime. Can be used to debug problems of missing RPC invoke or object replication issues.
        static bool enableLog;
#endif

        // Get the network replication hierarchy.
        static NetReplicationHierarchy *getHierarchy();
        // Sets the network replication hierarchy.
        static void setHierarchy(NetReplicationHierarchy *value);

    public:
        // Add the network replication serializer for a given type.
        static void addSerializer(const ScriptingTypeHandle &typeHandle, SerializeFunc serialize, SerializeFunc deserialize, void *serializeTag = nullptr, void *deserializeTag = nullptr);
        // Invoke the network replication serializer for a given type.
        static bool invokeSerializer(const ScriptingTypeHandle &typeHandle, void *instance, NetStream *stream, bool serialize);
        // Add the object to the network replication system.
        static void addObject(ScriptingObject *obj, const ScriptingObject *parent = nullptr);
        // Remove the object from the network replication system.
        static void removeObject(ScriptingObject *obj);
        
        // Spawn the object to the other clients. Can be spawned by the owner who locally created it (eg. from prefab).
        static void spawnObject(ScriptingObject *obj);
        // Spawns the object to the other clients. Can be spawned by the owner who locally created it (eg. from prefab).
        static void spawnObject(ScriptingObject *obj, const TkDataCollection<u32> &clientIds);
        // Despawns the object from the other clients. Deletes object from remove clients.
        static void despawnObject(ScriptingObject *obj);
        // Check if the network object is spawned or added to the network replication system.
        static bool hasObject(const ScriptingObject *obj);

        // Map object ID into server or client ID (depending on the source ID). Leaves source value unchanged if that specific ID is unused.
        static void mapObjectId(TkUuid &objectId);
        // Add a new mapping for object identifier. Can be used to link locally-spawned object with across different clients.
        static void addObjectIdMapping(const ScriptingObject *obj, const TkUuid &objectId);
        // Resolve foreign Guid into a local ScriptingObject
        static ScriptingObject *resolveForeignObject(Guid objectId);
        // Get the Client Id of the network object owner.
        static u32 getObjectOwnerClientId(const ScriptingObject *obj);

        // Get the role of the network object used locally (eg. to check if can simulate object).
        static NetObjectRole getObjectRole(const ScriptingObject *obj);
        // Checks if the network object is owned locally (thus current client has authority to manage it).
        TK_FORCE_INLINE static bool isObjectOwned(const ScriptingObject *obj)
        {
            return getObjectRole(obj) == NetObjectRole::OwnedAuthoritative;
        }
        // Checks if the network object is simulated locally (thus current client has can modify it - changed might be overriden by other client who owns this object).
        TK_FORCE_INLINE static bool isObjectSimulated(const ScriptingObject *obj)
        {
            return getObjectRole(obj) != NetObjectRole::Replicated;
        }
        // Checks if the network object is replicated locally (any local changes might be overriden by other client who owns this object).
        TK_FORCE_INLINE static bool isObjectReplicated(const ScriptingObject *obj)
        {
            const NetObjectRole role = getObjectRole(obj);
            return role == NetObjectRole::Replicated || role == NetObjectRole::ReplicatedSimulated;
        }
        // Sets the network object ownership - owning client identifier and local role to use.
        static void setObjectOwnership(ScriptingObject *obj, u32 ownerClientId, NetObjectRole localRole = NetObjectRole::Replicated, bool hierarchical = true);
        // Marks the object dirty to perform immediate replication to the other clients.
        static void dirtyObject(ScriptingObject *obj);

    public:
        // Begins invoking the RPC and returns the Network Stream to serialize parameters to.
        static NetStream *beginInvokeRPC();
        // Ends invoking the RPC.
        static bool endInvokeRPC(ScriptingObject *obj, const ScriptingTypeHandle &type, const StringAnsiView &name, NetStream *argsStream, Span<u32> targetIds = Span<u32>());
    };

}