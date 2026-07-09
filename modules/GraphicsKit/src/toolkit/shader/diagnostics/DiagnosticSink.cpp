/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/diagnostics/DiagnosticSink.hpp>

namespace toolkit
{
    void DiagnosticSink::report(const Diagnostic &diagnostic)
    {
        switch (diagnostic.severity)
        {
        case DiagnosticSeverity::Note:    m_noteCount++; break;
        case DiagnosticSeverity::Warning: m_warningCount++; break;
        case DiagnosticSeverity::Error:   m_errorCount++; break;
        case DiagnosticSeverity::Fatal:   m_fatalCount++; break;
        }
        m_diagnostics.add(diagnostic);
    }

    void DiagnosticSink::note(const SourceRange &range, TkStringView code, TkStringView message)
    {
        Diagnostic d;
        d.severity = DiagnosticSeverity::Note;
        d.code = TkString(code);
        d.message = TkString(message);
        d.range = range;
        report(d);
    }

    void DiagnosticSink::warning(const SourceRange &range, TkStringView code, TkStringView message)
    {
        Diagnostic d;
        d.severity = DiagnosticSeverity::Warning;
        d.code = TkString(code);
        d.message = TkString(message);
        d.range = range;
        report(d);
    }

    void DiagnosticSink::error(const SourceRange &range, TkStringView code, TkStringView message)
    {
        Diagnostic d;
        d.severity = DiagnosticSeverity::Error;
        d.code = TkString(code);
        d.message = TkString(message);
        d.range = range;
        report(d);
    }

    void DiagnosticSink::fatal(const SourceRange &range, TkStringView code, TkStringView message)
    {
        Diagnostic d;
        d.severity = DiagnosticSeverity::Fatal;
        d.code = TkString(code);
        d.message = TkString(message);
        d.range = range;
        report(d);
    }

    void DiagnosticSink::clear()
    {
        m_diagnostics.clear();
        m_noteCount = 0;
        m_warningCount = 0;
        m_errorCount = 0;
        m_fatalCount = 0;
    }
}
