/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "NetMessage.hpp"

namespace toolkit
{
    void NetMessage::writeNetId(const TkUuid &id)
    {
        writeUuid(id);
    }

    void NetMessage::readNetId(TkUuid &id)
    {
        id = readUuid();
    }

    void NetMessage::writeNetName(const TkStringView &name)
    {
        writeString(name);
    }

    void NetMessage::readNetName(TkStringView &name)
    {
        name = readString();
    }
}
