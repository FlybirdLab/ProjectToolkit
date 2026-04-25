/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "common.hpp"

#include <toolkit/serialization/JsonSerializer.hpp>
#include <toolkit/serialization/JsonTools.hpp>
#include <toolkit/serialization/JsonWriters.hpp>
#include <toolkit/serialization/Serialization.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/collection/HashMap.hpp>
#include <toolkit/types/DateTime.hpp>
#include <toolkit/types/TimeSpan.hpp>
#include <toolkit/types/UUID.hpp>
#include <toolkit/types/Version.hpp>

#include <cstring>

namespace toolkit
{
    enum class SerializationTestEnum : u32
    {
        kFirst = 1,
        kSecond = 2,
    };

    struct SerializationTestPoint
    {
        i32 x = 0;
        i32 y = 0;

        bool operator==(const SerializationTestPoint &other) const
        {
            return x == other.x && y == other.y;
        }
    };

    class SerializationTestObject final : public ISerializable
    {
    public:
        i32 id = 0;
        TkString name;
        TkArray<i32> values;
        TkUuid reference;

        void serialize(SerializeStream &stream, const void *otherObj) override
        {
            const auto *other = static_cast<const SerializationTestObject *>(otherObj);
            serialization::serializeMemberIfChanged(stream, "Id", id, other ? &other->id : nullptr);
            serialization::serializeMemberIfChanged(stream, "Name", name, other ? &other->name : nullptr);
            serialization::serializeMemberIfChanged(stream, "Values", values, other ? &other->values : nullptr);
            serialization::serializeMemberIfChanged(stream, "Reference", reference, other ? &other->reference : nullptr);
        }

        void deserialize(DeserializeStream stream, ISerializeModifier *modifier) override
        {
            serialization::deserializeMember(stream, "Id", id, modifier);
            serialization::deserializeMember(stream, "Name", name, modifier);
            serialization::deserializeMember(stream, "Values", values, modifier);
            serialization::deserializeMember(stream, "Reference", reference, modifier);
        }
    };
}

SERIALIZE_CUSTOM(SerializationTestPoint,
{
    stream.startObject();
    serialization::serializeMember(stream, "X", t.x);
    serialization::serializeMember(stream, "Y", t.y);
    stream.endObject();
})

DESERIALIZE_CUSTOM(SerializationTestPoint,
{
    serialization::deserializeMember(stream, "X", t.x, modifier);
    serialization::deserializeMember(stream, "Y", t.y, modifier);
})

SHOULD_SERIALIZE_CUSTOM(SerializationTestPoint,
{
    if (!otherObj)
        return true;
    return !(t == *static_cast<const SerializationTestPoint *>(otherObj));
})

TEST(SerializationTest, CompactAndPrettyWritersWriteObjectsArraysAndRawValues)
{
    TkString compact;
    {
        CompactJsonWriter writer(compact);
        writer.startObject();
        JSON_KEY(writer, "Name");
        writer.string("Toolkit");
        JSON_KEY(writer, "Enabled");
        writer.boolValue(true);
        JSON_KEY(writer, "Raw");
        writer.rawValue("{\"Nested\":7}");
        JSON_KEY(writer, "Items");
        writer.startArray();
        writer.intValue(1);
        writer.intValue(2);
        writer.endArray(2);
        writer.endObject();
    }

    EXPECT_TRUE(compact.contains("\"Name\""));
    EXPECT_TRUE(compact.contains("\"Toolkit\""));
    EXPECT_TRUE(compact.contains("\"Nested\":7"));
    EXPECT_TRUE(compact.contains("[1,2]"));

    TkString pretty;
    {
        PrettyJsonWriter writer(pretty);
        writer.startObject();
        JSON_KEY(writer, "Value");
        writer.intValue(42);
        writer.endObject();
    }

    EXPECT_TRUE(pretty.contains("\n"));
    EXPECT_TRUE(pretty.contains("\"Value\""));
}

TEST(SerializationTest, JsonDocumentAndToolsReadPrimitiveMembers)
{
    JsonDocument document;
    ASSERT_TRUE(document.parse("{\"Bool\":true,\"Int\":-7,\"Uint\":9,\"Float\":1.5,\"Text\":\"hello\",\"Array\":[3,4]}"));

    JsonValue root = document.getRoot();
    EXPECT_TRUE(root.isObject());
    EXPECT_EQ(root.getMemberCount(), 6);
    EXPECT_TRUE(root.hasMember("Text"));
    EXPECT_TRUE(root.findMember("Array").isArray());
    EXPECT_EQ(root.findMember("Array").getArrayCount(), 2);
    EXPECT_EQ(root.findMember("Array").getArrayItem(1).getInt(), 4);

    EXPECT_TRUE(JsonTools::getBool(root, "Bool"));
    EXPECT_EQ(JsonTools::getInt(root, "Int"), -7);
    EXPECT_EQ(JsonTools::getUint(root, "Uint"), 9u);
    EXPECT_FLOAT_EQ(JsonTools::getFloat(root, "Float"), 1.5f);
    EXPECT_STREQ(JsonTools::getString(root, "Text").getText(), "hello");
    EXPECT_STREQ(JsonTools::getString(root, "Missing", "fallback").getText(), "fallback");
}

TEST(SerializationTest, JsonDocumentReportsInvalidInput)
{
    JsonDocument document;
    EXPECT_FALSE(document.parse("{\"Broken\":"));
    EXPECT_FALSE(document.getError().isEmpty());
}

namespace toolkit
{
    struct SerializationTestPrimitiveData
    {
        bool flag = false;
        i32 integer = 0;
        u64 large = 0;
        f32 scalar = 0.0f;
        TkString text;
        SerializationTestEnum mode = SerializationTestEnum::kFirst;
    };
}

SERIALIZE_CUSTOM(SerializationTestPrimitiveData,
{
    stream.startObject();
    serialization::serializeMember(stream, "Flag", t.flag);
    serialization::serializeMember(stream, "Integer", t.integer);
    serialization::serializeMember(stream, "Large", t.large);
    serialization::serializeMember(stream, "Scalar", t.scalar);
    serialization::serializeMember(stream, "Text", t.text);
    serialization::serializeMember(stream, "Mode", t.mode);
    stream.endObject();
})

DESERIALIZE_CUSTOM(SerializationTestPrimitiveData,
{
    serialization::deserializeMember(stream, "Flag", t.flag, modifier);
    serialization::deserializeMember(stream, "Integer", t.integer, modifier);
    serialization::deserializeMember(stream, "Large", t.large, modifier);
    serialization::deserializeMember(stream, "Scalar", t.scalar, modifier);
    serialization::deserializeMember(stream, "Text", t.text, modifier);
    serialization::deserializeMember(stream, "Mode", t.mode, modifier);
})

TEST(SerializationTest, PrimitiveTypesAndEnumSerializeRoundTrip)
{
    SerializationTestPrimitiveData input;
    input.flag = true;
    input.integer = -123;
    input.large = 1234567890123ull;
    input.scalar = 3.25f;
    input.text = "roundtrip";
    input.mode = SerializationTestEnum::kSecond;

    TkString text = serialization::saveToString(input);
    EXPECT_TRUE(text.contains("\"Flag\":true"));
    EXPECT_TRUE(text.contains("\"Mode\":2"));

    SerializationTestPrimitiveData output;
    ASSERT_TRUE(serialization::loadFromString(output, text));
    EXPECT_TRUE(output.flag);
    EXPECT_EQ(output.integer, -123);
    EXPECT_EQ(output.large, 1234567890123ull);
    EXPECT_FLOAT_EQ(output.scalar, 3.25f);
    EXPECT_STREQ(output.text.getText(), "roundtrip");
    EXPECT_EQ(output.mode, SerializationTestEnum::kSecond);
}

namespace toolkit
{
    struct SerializationTestTypedData
    {
        TkDateTime date;
        TkTimeSpan span;
        TkVersion version;
        TkUuid uuid;
    };
}

SERIALIZE_CUSTOM(SerializationTestTypedData,
{
    stream.startObject();
    serialization::serializeMember(stream, "Date", t.date);
    serialization::serializeMember(stream, "Span", t.span);
    serialization::serializeMember(stream, "Version", t.version);
    serialization::serializeMember(stream, "Uuid", t.uuid);
    stream.endObject();
})

DESERIALIZE_CUSTOM(SerializationTestTypedData,
{
    serialization::deserializeMember(stream, "Date", t.date, modifier);
    serialization::deserializeMember(stream, "Span", t.span, modifier);
    serialization::deserializeMember(stream, "Version", t.version, modifier);
    serialization::deserializeMember(stream, "Uuid", t.uuid, modifier);
})

TEST(SerializationTest, TypedValuesSerializeRoundTrip)
{
    SerializationTestTypedData input;
    input.date = TkDateTime(2026, 4, 23, 10, 20, 30, 40);
    input.span = TkTimeSpan::fromSeconds(12.5);
    input.version = TkVersion(1, 2, 3, 4);
    input.uuid = TkUuid(0x11111111, 0x22222222, 0x33333333, 0x44444444);

    SerializationTestTypedData output;
    ASSERT_TRUE(serialization::loadFromString(output, serialization::saveToString(input, true)));
    EXPECT_EQ(output.date, input.date);
    EXPECT_EQ(output.span, input.span);
    EXPECT_EQ(output.version, input.version);
    EXPECT_EQ(output.uuid, input.uuid);
}

TEST(SerializationTest, ArraysHashMapsAndBlobRoundTrip)
{
    TkArray<i32> numbers;
    numbers.add(2);
    numbers.add(4);
    numbers.add(8);

    TkArray<i32> decodedNumbers;
    ASSERT_TRUE(serialization::loadFromString(decodedNumbers, serialization::saveToString(numbers)));
    ASSERT_EQ(decodedNumbers.count(), 3);
    EXPECT_EQ(decodedNumbers[0], 2);
    EXPECT_EQ(decodedNumbers[1], 4);
    EXPECT_EQ(decodedNumbers[2], 8);

    TkHashMap<TkString, i32> map;
    map.add(TkString("One"), 1);
    map.add(TkString("Two"), 2);

    TkHashMap<TkString, i32> decodedMap;
    ASSERT_TRUE(serialization::loadFromString(decodedMap, serialization::saveToString(map)));
    ASSERT_EQ(decodedMap.count(), 2);
    EXPECT_EQ(decodedMap[TkString("One")], 1);
    EXPECT_EQ(decodedMap[TkString("Two")], 2);

    TkString objectMapJson = "{\"A\":10,\"B\":20}";
    TkHashMap<TkString, i32> objectMap;
    ASSERT_TRUE(serialization::loadFromString(objectMap, objectMapJson));
    EXPECT_EQ(objectMap[TkString("A")], 10);
    EXPECT_EQ(objectMap[TkString("B")], 20);

    const byte raw[] = {0, 1, 2, 3, 254, 255};
    TkString encoded;
    {
        CompactJsonWriter writer(encoded);
        serialization::serializeBlob(writer, TkSpan<const byte>(raw, 6));
    }

    TkArray<byte> blob;
    JsonDocument blobDocument;
    ASSERT_TRUE(blobDocument.parse(encoded));
    serialization::deserializeBlob(blobDocument.getRoot(), blob);
    ASSERT_EQ(blob.count(), 6);
    EXPECT_EQ(std::memcmp(raw, blob.get(), 6), 0);
}

TEST(SerializationTest, SerializableObjectRoundTripAndDefaultMemberLoading)
{
    SerializationTestObject input;
    input.id = 42;
    input.name = "object";
    input.values.add(5);
    input.values.add(6);
    input.reference = TkUuid(1, 2, 3, 4);

    TkArray<byte> bytes = JsonSerializer::saveToBytes(static_cast<ISerializable *>(&input), true);
    SerializationTestObject output;
    ASSERT_TRUE(JsonSerializer::loadFromBytes(static_cast<ISerializable *>(&output), bytes));

    EXPECT_EQ(output.id, 42);
    EXPECT_STREQ(output.name.getText(), "object");
    ASSERT_EQ(output.values.count(), 2);
    EXPECT_EQ(output.values[0], 5);
    EXPECT_EQ(output.values[1], 6);
    EXPECT_EQ(output.reference, input.reference);

    JsonDocument document;
    ASSERT_TRUE(document.parse("{\"Patch\":{\"Name\":\"partial\"}}"));
    output.id = 99;
    output.deserializeIfExists(document.getRoot(), "Patch");
    EXPECT_EQ(output.id, 99);
    EXPECT_STREQ(output.name.getText(), "partial");

    serialization::deserializeMember(document.getRoot(), "Missing", output.id, 7);
    EXPECT_EQ(output.id, 7);
}

TEST(SerializationTest, SerializeMemberIfChangedSkipsMatchingValues)
{
    SerializationTestObject current;
    current.id = 10;
    current.name = "same";

    SerializationTestObject other;
    other.id = 10;
    other.name = "same";

    TkString text;
    {
        CompactJsonWriter writer(text);
        writer.startObject();
        current.serialize(writer, &other);
        writer.endObject();
    }
    EXPECT_STREQ(text.getText(), "{}");

    current.name = "changed";
    text.clear();
    {
        CompactJsonWriter writer(text);
        writer.startObject();
        current.serialize(writer, &other);
        writer.endObject();
    }
    EXPECT_FALSE(text.contains("\"Id\""));
    EXPECT_TRUE(text.contains("\"Name\""));
}

TEST(SerializationTest, ModifierRemapsUuidsDuringDeserialize)
{
    const TkUuid source(0xaaaaaaaa, 0xbbbbbbbb, 0xcccccccc, 0xdddddddd);
    const TkUuid target(0x11111111, 0x22222222, 0x33333333, 0x44444444);

    TkString json;
    {
        CompactJsonWriter writer(json);
        writer.startObject();
        JSON_KEY(writer, "Reference");
        writer.uuid(source);
        writer.endObject();
    }

    ISerializeModifier modifier;
    modifier.idsMapping.add(source, target);

    SerializationTestObject output;
    ASSERT_TRUE(JsonSerializer::loadFromString(static_cast<ISerializable *>(&output), json, &modifier));
    EXPECT_EQ(output.reference, target);
}

TEST(SerializationTest, JsonToolsChangeIdsRewritesNestedUuidStrings)
{
    const TkUuid source(0xaaaaaaaa, 0xbbbbbbbb, 0xcccccccc, 0xdddddddd);
    const TkUuid target(0x11111111, 0x22222222, 0x33333333, 0x44444444);

    TkString json;
    {
        CompactJsonWriter writer(json);
        writer.startObject();
        JSON_KEY(writer, "Items");
        writer.startArray();
        writer.startObject();
        JSON_KEY(writer, "Id");
        writer.uuid(source);
        writer.endObject();
        writer.endArray(1);
        writer.endObject();
    }

    JsonDocument document;
    ASSERT_TRUE(document.parse(json));

    TkHashMap<TkUuid, TkUuid> mapping;
    mapping.add(source, target);
    JsonTools::changeIds(document, mapping);

    JsonValue idNode = document.getRoot().findMember("Items").getArrayItem(0).findMember("Id");
    EXPECT_EQ(JsonTools::getUuid(idNode), target);
}

TEST(SerializationTest, CustomSerializationExtensionRoundTripsAndShouldSerializeWorks)
{
    SerializationTestPoint point;
    point.x = 12;
    point.y = -34;

    TkString text = serialization::saveToString(point);
    EXPECT_TRUE(text.contains("\"X\":12"));
    EXPECT_TRUE(text.contains("\"Y\":-34"));

    SerializationTestPoint decoded;
    ASSERT_TRUE(serialization::loadFromString(decoded, text));
    EXPECT_EQ(decoded, point);

    SerializationTestPoint same = point;
    EXPECT_FALSE(serialization::shouldSerialize(point, &same));
    same.y = 0;
    EXPECT_TRUE(serialization::shouldSerialize(point, &same));
}
