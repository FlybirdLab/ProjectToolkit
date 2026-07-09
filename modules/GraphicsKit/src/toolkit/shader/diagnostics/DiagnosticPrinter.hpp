/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/shader/diagnostics/DiagnosticSink.hpp>

namespace toolkit
{
    class SourceManager;

    // Abstract base for diagnostic formatters. Implementations include the
    // human-readable text formatter and the JSON formatter, but tools can
    // provide their own (LSP adapter, CI summary, etc.).
    class DiagnosticPrinter
    {
    public:
        virtual ~DiagnosticPrinter() = default;

        virtual void print(
            const DiagnosticSink &sink,
            const SourceManager &sourceManager
        ) = 0;
    };
}
