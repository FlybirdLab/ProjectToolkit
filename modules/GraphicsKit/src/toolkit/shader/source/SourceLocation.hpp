/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>

namespace toolkit
{
    // A single point inside a source file managed by SourceManager.
    // fileId == 0 represents an invalid / unset location; valid file ids are 1-based.
    struct SourceLocation
    {
        u32 fileId = 0;
        // 0-based byte offset from the start of the file.
        u32 offset = 0;
        // 1-based line number.
        u32 line = 0;
        // 1-based column number.
        u32 column = 0;

        bool isValid() const { return fileId != 0; }

        static SourceLocation invalid() { return SourceLocation{}; }
    };
}
