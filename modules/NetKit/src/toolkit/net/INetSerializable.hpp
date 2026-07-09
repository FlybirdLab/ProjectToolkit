/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/net/common.hpp>

namespace toolkit
{
    // Interface for values and objects that can be serialized/deserialized for network replication.
    class TK_API INetSerializable
    {
    public:
        // Serialize object to the output stream.
        virtual void serialize(NetStream *stream) = 0;
        // Deserialize object from the input stream.
        virtual void deserialize(NetStream *stream) = 0;
    };

}