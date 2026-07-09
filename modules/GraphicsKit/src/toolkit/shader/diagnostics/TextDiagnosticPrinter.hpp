/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/shader/diagnostics/DiagnosticPrinter.hpp>
#include <toolkit/types/String.hpp>

#include <cstdio>

namespace toolkit
{
    // Emits diagnostics in a clang/MSVC-compatible single-line format with an
    // optional source-snippet underline, e.g.:
    //
    //   path/to/file.hlsl:42:13: error TKSHD1007: unknown render state 'ZWirte'
    //       ZWirte On
    //       ^~~~~~
    //   note: did you mean 'ZWrite'?
    //
    // The stream defaults to stderr so that successful compiler output on stdout
    // stays parseable. Pass a different FILE* (or override the writer) for tests.
    class TextDiagnosticPrinter final : public DiagnosticPrinter
    {
    public:
        explicit TextDiagnosticPrinter(std::FILE *stream = nullptr, bool useColor = false);

        void print(
            const DiagnosticSink &sink,
            const SourceManager &sourceManager
        ) override;

    private:
        void printOne(
            const Diagnostic &diagnostic,
            const SourceManager &sourceManager,
            bool asNote
        );

        std::FILE *m_stream;
        bool m_useColor;
    };
}
