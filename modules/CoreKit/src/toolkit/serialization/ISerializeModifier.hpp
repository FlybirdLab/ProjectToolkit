/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/collection/HashMap.hpp>
#include <toolkit/types/UUID.hpp>

namespace toolkit
{
    // Optional context passed into deserialization. CoreKit keeps this data-only:
    // resource lookup and object resolution are expected to live in higher modules.
    class ISerializeModifier
    {
    public:
        u32 formatVer = 0;
        i32 currentInstance = -1;
        TkHashMap<TkUuid, TkUuid> idsMapping;
    };
}
