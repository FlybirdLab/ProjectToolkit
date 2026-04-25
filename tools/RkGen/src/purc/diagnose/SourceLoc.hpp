#pragma once

#include <toolkit/common.hpp>

namespace toolkit {

    struct SourceLoc
    {
        u32 line;
        u32 column;
        class CXXSourceFile* file;

        TkString toString() const;
    };
    
}