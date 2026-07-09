/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/serialization/typedef.hpp>

namespace toolkit {
    
    struct TkDateTime;
    struct TkTimeSpan;
    struct TkVersion;

    enum class JsonValueType : u8
    {
        kNull,
        kBool,
        kNumber,
        kString,
        kArray,
        kObject,
    };

}