/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/diagnostics/Diagnostic.hpp>

namespace toolkit
{
    const char *toString(DiagnosticSeverity severity)
    {
        switch (severity)
        {
        case DiagnosticSeverity::Note:    return "note";
        case DiagnosticSeverity::Warning: return "warning";
        case DiagnosticSeverity::Error:   return "error";
        case DiagnosticSeverity::Fatal:   return "fatal";
        }
        return "error";
    }
}
