#pragma once

#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>

namespace toolkit {

    class MocCtx
    {
    public:
        MocCtx(const TkString& sourcePath, const TkString& outputPath);
        MocCtx(const TkString& sourcePath, const TkString& outputPath, const TkArray<TkString>& sourceFiles);
        ~MocCtx();

        int runMoc();

    private:
        TkString m_sourcePath;
        TkString m_outputPath;
        TkArray<TkString> m_sourceFiles;
    };
}
