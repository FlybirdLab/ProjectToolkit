/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/shader/diagnostics/Diagnostic.hpp>

namespace toolkit
{
    // Collects diagnostics during compilation.
    //
    // Thread-safety: a sink is intended to be used per-compilation-thread, or
    // wrapped in caller-side synchronization. Phase 6 (parallel compile graph)
    // can introduce per-job sinks merged at the end of a build.
    class DiagnosticSink
    {
    public:
        void report(const Diagnostic &diagnostic);

        // Convenience shortcuts. `code` and `message` must outlive the call.
        void note(const SourceRange &range, TkStringView code, TkStringView message);
        void warning(const SourceRange &range, TkStringView code, TkStringView message);
        void error(const SourceRange &range, TkStringView code, TkStringView message);
        void fatal(const SourceRange &range, TkStringView code, TkStringView message);

        bool hasError() const { return m_errorCount > 0 || m_fatalCount > 0; }
        bool hasFatal() const { return m_fatalCount > 0; }

        u32 getNoteCount() const { return m_noteCount; }
        u32 getWarningCount() const { return m_warningCount; }
        u32 getErrorCount() const { return m_errorCount; }
        u32 getFatalCount() const { return m_fatalCount; }

        const TkArray<Diagnostic> &getDiagnostics() const { return m_diagnostics; }

        void clear();

    private:
        TkArray<Diagnostic> m_diagnostics;
        u32 m_noteCount = 0;
        u32 m_warningCount = 0;
        u32 m_errorCount = 0;
        u32 m_fatalCount = 0;
    };
}
