#include "common.hpp"

#include <toolkit/net/INetSerializable.hpp>
#include <toolkit/net/NetStream.hpp>

namespace
{
    class StreamPayload final : public INetSerializable
    {
    public:
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
}

TEST(NetStreamTest, WritesGrowsAndReadsRawData)
{
    NetStream stream;
    stream.init(4);

    const u32 a = 0x11223344;
    const u64 b = 0x0102030405060708ull;
    stream.writeBytes(&a, sizeof(a));
    stream.writeBytes(&b, sizeof(b));

    EXPECT_EQ(stream.pos(), sizeof(a) + sizeof(b));
    EXPECT_GE(stream.length(), stream.pos());

    stream.setPos(0);
    u32 readA = 0;
    u64 readB = 0;
    stream.readBytes(&readA, sizeof(readA));
    stream.readBytes(&readB, sizeof(readB));

    EXPECT_EQ(readA, a);
    EXPECT_EQ(readB, b);
}

TEST(NetStreamTest, SerializableObjectRoundTrips)
{
    StreamPayload src;
    src.value = 123456;

    NetStream stream;
    stream.init();
    stream.write(src);

    StreamPayload dst;
    stream.setPos(0);
    stream.read(dst);

    EXPECT_EQ(dst.value, src.value);
}

TEST(NetStreamTest, ReadOnlyInitDoesNotOwnExternalBuffer)
{
    byte buffer[8] = {};
    const u32 value = 99;
    TkMemory::memoryCopy(buffer, &value, sizeof(value));

    NetStream stream;
    stream.init(buffer, sizeof(buffer));

    u32 readValue = 0;
    stream.readBytes(&readValue, sizeof(readValue));

    EXPECT_EQ(readValue, value);
    EXPECT_EQ(stream.pos(), sizeof(value));
}
