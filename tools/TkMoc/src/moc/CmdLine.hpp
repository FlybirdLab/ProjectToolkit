#pragma once

#include <toolkit/types/String.hpp>

namespace toolkit {

    struct OptionsData
    {
        const char* args = nullptr; // original arguments

        TkString sourcePath = TkString::Empty; // -source/-s [PATH] for source file (*.h/*.hpp)
        TkString outputPath = TkString::Empty; // -output/-o [PATH] for destination 
    };

    OptionsData* getCmdLine();
    bool parseCmdLine(const char* cmdLine);

}