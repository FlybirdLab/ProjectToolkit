/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/windowing/common.hpp>

namespace toolkit
{
    enum class DragDropType
    {
        kUnknown = 0,
        kText,
        kFiles
    };

    class DragDropData
    {
    public:
        // Get the data type.
        DragDropType getType() const;
        // Get data value as text.
        TkString getAsText() const;
        // Get data value as array of file paths.
        void getAsFiles(TkArray<TkString> *files) const;
    };
}