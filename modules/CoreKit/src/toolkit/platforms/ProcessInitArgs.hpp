/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/collection/HashMap.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>

namespace toolkit {
    
    struct ProcessInitArgs
    {
        TkString file;
        TkString args;
        TkString workDir;
        bool logOutput = true;
        bool saveOutput = false;
        bool waitForQuit = true;
        bool hiddenWindow = true;
        bool shellExecute = false;

        TkHashMap<TkString, TkString> envArgs;
        TkArray<char> output;
    };
}