#include "SourceLoc.hpp"

#include "../Source.hpp"

namespace toolkit {

    TkString SourceLoc::toString() const
    {
        return TkString::format("%s [%u:%u]", file ? file->getPath().getText() : "<unknown>", line, column);
    }

}
