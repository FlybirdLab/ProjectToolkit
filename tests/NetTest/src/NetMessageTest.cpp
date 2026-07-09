#include "common.hpp"

#include <toolkit/net/NetMessage.hpp>

namespace
{
    struct MessagePayload
    {
        toolkit::u16 a = 0;
        toolkit::u32 b = 0;

        bool operator==(const MessagePayload &other) const
        {
            return a == other.a && b == other.b;
        }
    };
}

TEST(NetMessageTest, PrimitiveStructureStringAndUuidRoundTrip)
{
    byte buffer[256] = {};
    NetMessage message(buffer, 1, sizeof(buffer), 0, 0);

    const TkUuid uuid(1, 2, 3, 4);
    const MessagePayload payload{7, 42};

    message.writeU8(11);
    message.writeU16(222);
    message.writeU32(3333);
    message.writeStructure(payload);
    message.writeUuid(uuid);
    message.writeString(TkStringView("direct-name"));
    message.writeNetId(uuid);
    message.writeNetName(TkStringView("network-name"));

    EXPECT_GT(message.length, 0u);
    EXPECT_EQ(message.position, message.length);

    message.position = 0;

    MessagePayload readPayload;
    TkUuid readUuid;
    TkUuid readNetId;
    TkStringView readNetName;

    EXPECT_EQ(message.readU8(), 11);
    EXPECT_EQ(message.readU16(), 222);
    EXPECT_EQ(message.readU32(), 3333u);
    message.readStructure(readPayload);
    EXPECT_TRUE(readPayload == payload);
    EXPECT_TRUE(message.readUuid() == uuid);
    EXPECT_EQ(message.readString(), TkStringView("direct-name"));
    message.readNetId(readNetId);
    message.readNetName(readNetName);
    EXPECT_TRUE(readNetId == uuid);
    EXPECT_EQ(readNetName, TkStringView("network-name"));
    EXPECT_EQ(message.position, message.length);
}

TEST(NetMessageTest, SkipBytesReturnsCurrentSpanAndAdvances)
{
    byte buffer[32] = {};
    NetMessage message(buffer, 1, sizeof(buffer), 0, 0);
    const byte values[4] = {1, 2, 3, 4};
    message.writeBytes(values, 4);

    message.position = 0;
    auto *data = static_cast<byte *>(message.skipBytes(4));

    EXPECT_EQ(data[0], 1);
    EXPECT_EQ(data[3], 4);
    EXPECT_EQ(message.position, 4u);
}
