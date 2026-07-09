#include "common.hpp"

#include <toolkit/net/NetClient.hpp>
#include <toolkit/net/NetManager.hpp>
#include <toolkit/net/NetReplicationHierarchy.hpp>
#include <toolkit/reflect/RkManager.hpp>
#include <toolkit/reflect/RkObject.hpp>

namespace
{
    NetClient *makeClient(u32 clientId, u32 connectionId)
    {
        return TkMemory::newObject<NetClient>(RkManager::getAllocator(), clientId, NetConnection{connectionId});
    }

    void clearClients()
    {
        for (NetClient *client : NetManager::clients)
            TkMemory::deletePtr(RkManager::getAllocator(), client);
        NetManager::clients.clear();
    }
}

TEST(NetClientsMaskTest, BitOperationsWorkAcrossWords)
{
    NetClientsMask mask;
    EXPECT_TRUE(mask.isEmpty());

    mask.setBit(1);
    mask.setBit(70);
    EXPECT_TRUE(mask.hasBit(1));
    EXPECT_TRUE(mask.hasBit(70));
    EXPECT_FALSE(mask.hasBit(2));
    EXPECT_FALSE(mask.isEmpty());

    mask.unsetBit(1);
    EXPECT_FALSE(mask.hasBit(1));
    EXPECT_TRUE(mask.hasBit(70));
}

TEST(NetReplicationHierarchyTest, AddGetSetDirtyRemoveAndUpdate)
{
    clearClients();
    NetManager::mode = NetManagerMode::kServer;
    NetManager::networkFPS = 60.0f;
    NetManager::clients.add(makeClient(1, 11));
    NetManager::clients.add(makeClient(2, 12));

    RkObject object(ObjectSpawnParams(TkUuid(10, 11, 12, 13), RkObject::staticTypeHandle()));
    NetReplicationHierarchy hierarchy;
    hierarchy.addObject(&object);
    hierarchy.addObject(&object);
    EXPECT_EQ(hierarchy.objects.count(), 1);

    NetReplicationHierarchyObject data;
    ASSERT_TRUE(hierarchy.getObject(&object, data));
    EXPECT_EQ(data.object, &object);

    data.replicationFPS = -1.0f;
    EXPECT_TRUE(hierarchy.setObject(data));
    EXPECT_TRUE(hierarchy.dirtyObject(&object));

    NetReplicationHierarchyUpdateResult result;
    result.init();
    EXPECT_EQ(result.getClientsCount(), 2u);
    EXPECT_TRUE(result.getClientsMask().hasBit(0));
    EXPECT_TRUE(result.getClientsMask().hasBit(1));

    hierarchy.update(&result);
    ASSERT_EQ(result.entries.count(), 1);
    EXPECT_EQ(result.entries[0].object, &object);

    EXPECT_TRUE(hierarchy.removeObject(&object));
    EXPECT_FALSE(hierarchy.removeObject(&object));
    EXPECT_EQ(hierarchy.objects.count(), 0);

    clearClients();
    NetManager::mode = NetManagerMode::kOffline;
}

TEST(NetReplicationHierarchyTest, ClientResultUsesAllMask)
{
    clearClients();
    NetManager::mode = NetManagerMode::kClient;

    NetReplicationHierarchyUpdateResult result;
    result.init();

    EXPECT_TRUE(result.getClientsMask().hasBit(0));
    EXPECT_TRUE(result.getClientsMask().hasBit(90));

    NetManager::mode = NetManagerMode::kOffline;
}
