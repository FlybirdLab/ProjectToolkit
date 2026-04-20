#include "SourceLoc.hpp"

#include "../Source.hpp"

namespace toolkit {

    TkString SourceLoc::toString() const
    {
        return TkString::format("{0} [{1}:{2}]", file->getSource(), TkStringUtils::toString(line), TkStringUtils::toString(column));
    }

}