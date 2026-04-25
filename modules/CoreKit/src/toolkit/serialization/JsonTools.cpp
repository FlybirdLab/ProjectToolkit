/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "JsonTools.hpp"

#include <toolkit/utils/Convertor.hpp>

namespace toolkit
{
    void JsonTools::changeIds(JsonDocument &document, const TkHashMap<TkUuid, TkUuid> &mapping)
    {
        document.changeIds(mapping);
    }

    TkString JsonTools::getString(JsonValue node, TkStringView name, TkStringView defaultValue)
    {
        JsonValue member = node.findMember(name);
        return member.isString() ? member.getString().toString() : defaultValue.toString();
    }

    bool JsonTools::getBool(JsonValue node, TkStringView name, bool defaultValue)
    {
        return node.findMember(name).getBool(defaultValue);
    }

    i32 JsonTools::getInt(JsonValue node, TkStringView name, i32 defaultValue)
    {
        return node.findMember(name).getInt(defaultValue);
    }

    u32 JsonTools::getUint(JsonValue node, TkStringView name, u32 defaultValue)
    {
        return node.findMember(name).getUint(defaultValue);
    }

    i64 JsonTools::getInt64(JsonValue node, TkStringView name, i64 defaultValue)
    {
        return node.findMember(name).getInt64(defaultValue);
    }

    u64 JsonTools::getUint64(JsonValue node, TkStringView name, u64 defaultValue)
    {
        return node.findMember(name).getUint64(defaultValue);
    }

    f32 JsonTools::getFloat(JsonValue node, TkStringView name, f32 defaultValue)
    {
        return node.findMember(name).getFloat(defaultValue);
    }

    f64 JsonTools::getDouble(JsonValue node, TkStringView name, f64 defaultValue)
    {
        return node.findMember(name).getDouble(defaultValue);
    }

    TkUuid JsonTools::getUuid(JsonValue value)
    {
        TkUuid result;
        return TkUuid::parse(value.getString(), result) ? result : TkUuid::Empty;
    }

    TkUuid JsonTools::getUuid(JsonValue node, TkStringView name, const TkUuid &defaultValue)
    {
        JsonValue member = node.findMember(name);
        return member ? getUuid(member) : defaultValue;
    }

    TkDateTime JsonTools::getDateTime(JsonValue value)
    {
        return TkDateTime(value.getInt64());
    }

    TkDateTime JsonTools::getDateTime(JsonValue node, TkStringView name, const TkDateTime &defaultValue)
    {
        JsonValue member = node.findMember(name);
        return member ? getDateTime(member) : defaultValue;
    }

    TkTimeSpan JsonTools::getTimeSpan(JsonValue value)
    {
        return TkTimeSpan(value.getInt64());
    }

    TkTimeSpan JsonTools::getTimeSpan(JsonValue node, TkStringView name, const TkTimeSpan &defaultValue)
    {
        JsonValue member = node.findMember(name);
        return member ? getTimeSpan(member) : defaultValue;
    }

    TkVersion JsonTools::getVersion(JsonValue value)
    {
        TkVersion result;
        if (value.isString())
            TkVersion::parse(value.getString().toString(), result);
        else if (value.isObject())
        {
            result = TkVersion(
                value.findMember("Major").getInt(),
                value.findMember("Minor").getInt(),
                value.findMember("Build").getInt(-1),
                value.findMember("Revision").getInt(-1));
        }
        return result;
    }

    TkVersion JsonTools::getVersion(JsonValue node, TkStringView name, const TkVersion &defaultValue)
    {
        JsonValue member = node.findMember(name);
        return member ? getVersion(member) : defaultValue;
    }

    void JsonTools::getBlob(JsonValue value, TkArray<byte> &output)
    {
        output.clear();
        if (!value.isString())
            return;

        TkConvertor convertor;
        TkStringView text = value.getString();
        convertor.base64Decode(text.getText(), text.length(), output);
    }

    void JsonTools::getBlob(JsonValue node, TkStringView name, TkArray<byte> &output)
    {
        getBlob(node.findMember(name), output);
    }
}
