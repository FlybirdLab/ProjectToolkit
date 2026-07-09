#include "common.hpp"

#include <toolkit/net/INetSerializable.hpp>
#include <toolkit/net/NetInternal.hpp>
#include <toolkit/net/NetManager.hpp>
#include <toolkit/net/NetReplicator.hpp>
#include <toolkit/net/NetRpc.hpp>
#include <toolkit/net/NetStream.hpp>
#include <toolkit/reflect/RkModule.hpp>
#include <toolkit/reflect/RkObject.hpp>

namespace
{
    RkModule &netTestReflectModule()
    {
        static RkModule module(TkStringView("NetTest"));
        return module;
    }

    struct SerializerState
    {
        u32 serialized = 0;
        u32 deserialized = 0;
    };

    void serializeObject(void *instance, NetStream *stream, void *tag)
    {
        auto *state = static_cast<SerializerState *>(tag);
        auto *object = static_cast<RkObject *>(instance);
        state->serialized++;
        const TkUuid id = object->getId();
        stream->writeBytes(&id, sizeof(id));
    }

    void deserializeObject(void *, NetStream *stream, void *tag)
    {
        auto *state = static_cast<SerializerState *>(tag);
        TkUuid id;
        stream->readBytes(&id, sizeof(id));
        state->deserialized++;
        EXPECT_TRUE(id.isValid());
    }

    class SerializableObject final : public RkObject, public INetSerializable
    {
    public:
        static ::toolkit::TypeInitializer TypeInitializer;

        static TypeHandle staticTypeHandle()
        {
            return TypeInitializer.getHandle();
        }

        explicit SerializableObject(const ObjectSpawnParams &params)
            : RkObject(params)
        {
        }

        void serialize(NetStream *stream) override
        {
            stream->writeBytes(&value, sizeof(value));
        }

        void deserialize(NetStream *stream) override
        {
            stream->readBytes(&value, sizeof(value));
        }

        u32 value = 0;
    };

    ::toolkit::TypeInitializer SerializableObject::TypeInitializer(
        netTestReflectModule(),
        RkClassDesc{
            .name = "SerializableObject",
            .fullName = "NetTest.SerializableObject",
            .size = static_cast<u32>(sizeof(SerializableObject)),
            .align = static_cast<u32>(alignof(SerializableObject)),
        });

    void resetNetManager()
    {
        NetInternal::netReplicatorClear();
        NetManager::mode = NetManagerMode::kOffline;
        NetManager::state = NetConnectionState::kOffline;
        NetManager::localClientId = NetManager::kServerClientId;
    }
}

TEST(NetReplicatorTest, RegisteredSerializerReturnsTrueOnSuccess)
{
    resetNetManager();
    SerializerState state;
    NetReplicator::addSerializer(RkObject::staticTypeHandle(), serializeObject, deserializeObject, &state, &state);

    RkObject object(ObjectSpawnParams(TkUuid(21, 22, 23, 24), RkObject::staticTypeHandle()));
    NetStream stream;
    stream.init();

    EXPECT_TRUE(NetReplicator::invokeSerializer(object.getTypeHandle(), &object, &stream, true));
    EXPECT_EQ(state.serialized, 1u);
    EXPECT_GT(stream.pos(), 0u);

    stream.setPos(0);
    EXPECT_TRUE(NetReplicator::invokeSerializer(object.getTypeHandle(), &object, &stream, false));
    EXPECT_EQ(state.deserialized, 1u);

    EXPECT_FALSE(NetReplicator::invokeSerializer(TypeHandle(), &object, &stream, true));
}

TEST(NetReplicatorTest, InterfaceSerializerFallbackWorks)
{
    resetNetManager();
    SerializableObject src(ObjectSpawnParams(TkUuid(31, 32, 33, 34), SerializableObject::staticTypeHandle()));
    SerializableObject dst(ObjectSpawnParams(TkUuid(41, 42, 43, 44), SerializableObject::staticTypeHandle()));
    src.value = 777;

    NetStream stream;
    stream.init();
    EXPECT_TRUE(NetReplicator::invokeSerializer(src.getTypeHandle(), &src, &stream, true));

    stream.setPos(0);
    EXPECT_TRUE(NetReplicator::invokeSerializer(dst.getTypeHandle(), &dst, &stream, false));
    EXPECT_EQ(dst.value, 777u);
}

TEST(NetReplicatorTest, ObjectRegistrationMappingOwnershipAndDirtyState)
{
    resetNetManager();
    NetManager::mode = NetManagerMode::kServer;
    NetManager::state = NetConnectionState::kConnected;
    NetManager::localClientId = NetManager::kServerClientId;

    RkObject parent(ObjectSpawnParams(TkUuid(51, 52, 53, 54), RkObject::staticTypeHandle()));
    RkObject object(ObjectSpawnParams(TkUuid(61, 62, 63, 64), RkObject::staticTypeHandle()));
    NetReplicator::addObject(&object, &parent);

    EXPECT_TRUE(NetReplicator::hasObject(&object));
    EXPECT_EQ(NetReplicator::getObjectOwnerClientId(&object), NetManager::kServerClientId);
    EXPECT_EQ(NetReplicator::getObjectRole(&object), NetObjectRole::OwnedAuthoritative);

    const TkUuid foreignId(100, 101, 102, 103);
    NetReplicator::addObjectIdMapping(&object, foreignId);
    TkUuid mapped = foreignId;
    NetReplicator::mapObjectId(mapped);
    EXPECT_TRUE(mapped == object.getId());
    EXPECT_EQ(NetReplicator::resolveForeignObject(foreignId), &object);

    NetReplicator::setObjectOwnership(&object, 7, NetObjectRole::Replicated, false);
    EXPECT_EQ(NetReplicator::getObjectOwnerClientId(&object), 7u);
    EXPECT_EQ(NetReplicator::getObjectRole(&object), NetObjectRole::Replicated);

    NetReplicator::removeObject(&object);
    EXPECT_FALSE(NetReplicator::hasObject(&object));
    resetNetManager();
}

TEST(NetReplicatorTest, SpawnQueueCountsAsExistingObject)
{
    resetNetManager();
    NetManager::mode = NetManagerMode::kServer;
    NetManager::state = NetConnectionState::kConnected;

    RkObject object(ObjectSpawnParams(TkUuid(71, 72, 73, 74), RkObject::staticTypeHandle()));
    EXPECT_FALSE(NetReplicator::hasObject(&object));

    NetReplicator::spawnObject(&object);
    EXPECT_TRUE(NetReplicator::hasObject(&object));

    resetNetManager();
}

TEST(NetReplicatorTest, RpcQueueAcceptsRegisteredRpcAndRejectsMissingRpc)
{
    resetNetManager();
    NetManager::mode = NetManagerMode::kClient;
    NetManager::state = NetConnectionState::kConnected;

    RkObject object(ObjectSpawnParams(TkUuid(81, 82, 83, 84), RkObject::staticTypeHandle()));
    NetStream *stream = NetReplicator::beginInvokeRPC();
    const u32 value = 9;
    stream->writeBytes(&value, sizeof(value));

    const TkStringView rpcName("TestRpc");
    NetRpcInfo info;
    info.server = 1;
    info.channel = static_cast<u8>(NetChannelType::kReliable);
    NetRpcInfo::rpcsTable[NetRpcName(RkObject::staticTypeHandle(), rpcName)] = info;

    EXPECT_TRUE(NetReplicator::endInvokeRPC(&object, RkObject::staticTypeHandle(), rpcName, stream));
    EXPECT_FALSE(NetReplicator::endInvokeRPC(&object, RkObject::staticTypeHandle(), TkStringView("MissingRpc"), stream));

    NetRpcInfo::rpcsTable.remove(NetRpcName(RkObject::staticTypeHandle(), rpcName));
    resetNetManager();
}
