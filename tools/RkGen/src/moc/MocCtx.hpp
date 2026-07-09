#pragma once

#include <toolkit/collection/Array.hpp>
#include <toolkit/types/String.hpp>

namespace toolkit {

    class MocCtx
    {
    public:
        MocCtx(const TkString& sourcePath, const TkString& outputPath);
        MocCtx(const TkString& sourcePath, const TkString& outputPath, const TkArray<TkString>& sourceFiles);
        MocCtx(const TkString& sourcePath, const TkString& outputPath, const TkArray<TkString>& sourceFiles, const TkString& outputName);
        MocCtx(const TkString& sourcePath, const TkString& outputPath, const TkArray<TkString>& sourceFiles, const TkString& outputName, bool incremental);
        ~MocCtx();

        int runMoc();

    private:
        TkString m_sourcePath;
        TkString m_outputPath;
        TkString m_outputName;
        TkArray<TkString> m_sourceFiles;
        bool m_incremental = true;
    };
}
