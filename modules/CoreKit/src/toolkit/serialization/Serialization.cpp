/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Serialization.hpp"

namespace toolkit
{
    void ISerializable::deserializeIfExists(DeserializeStream& stream, TkStringView memberName, ISerializeModifier *modifier)
    {
        JsonValue member = stream.findMember(memberName);
        if (member)
            deserialize(member, modifier);
    }
}
