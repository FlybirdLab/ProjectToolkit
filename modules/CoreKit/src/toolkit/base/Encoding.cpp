/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "Encoding.hpp"

namespace toolkit {
    
    static const char **Encoding_const_string()
    {
        static const char *str[] = {"kANSI", "kUnicode", "kUnicodeBigEndian", "kUTF8"};
        return str;
    }
    
    static inline const char *toString(Encoding e)
    {
        return Encoding_const_string()[(int)e];
    };
}