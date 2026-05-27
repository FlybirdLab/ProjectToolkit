/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/shader/source/SourceLocation.hpp>

namespace toolkit
{
    // A half-open [begin, end) range inside a single source file.
    // A range with begin == end represents a single insertion point.
    struct SourceRange
    {
        SourceLocation begin;
        SourceLocation end;

        bool isValid() const { return begin.isValid(); }

        static SourceRange invalid() { return SourceRange{}; }
        static SourceRange point(const SourceLocation &at) { return SourceRange{at, at}; }
    };
}
