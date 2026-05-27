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
    // Emits diagnostics as a JSON array using the project's CompactJsonWriter.
    // Output schema (top-level is an array of these objects):
    //   {
    //     "severity": "error",
    //     "code":     "TKSHD0001",
    //     "message":  "failed to read source file",
    //     "file":     "Triangle.tkshader",   // omitted if range is invalid
    //     "line":     42,
    //     "column":   13,
    //     "endLine":  42,
    //     "endColumn":18,
    //     "backend":  "vulkan",              // omitted if empty
    //     "stage":    "pixel",
    //     "variant":  "...",
    //     "notes":    [ { "message": ..., "file": ..., "line": ..., "column": ... } ]
    //   }
    class JsonDiagnosticPrinter final : public DiagnosticPrinter
    {
    public:
        // If `output` is non-null, the JSON is appended to that string and
        // nothing is written to the stream. Otherwise the JSON is written to
        // `stream` (defaults to stdout so that CI tools can pipe it).
        explicit JsonDiagnosticPrinter(TkString *output = nullptr, std::FILE *stream = nullptr);

        void print(
            const DiagnosticSink &sink,
            const SourceManager &sourceManager
        ) override;

    private:
        TkString *m_output;
        std::FILE *m_stream;
    };
}
