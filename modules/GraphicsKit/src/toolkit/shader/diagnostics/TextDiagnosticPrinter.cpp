/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/diagnostics/TextDiagnosticPrinter.hpp>
#include <toolkit/shader/source/SourceManager.hpp>

#include <cstdio>

namespace toolkit
{
    namespace
    {
        u32 computeUnderlineLength(const SourceRange &range)
        {
            if (!range.isValid())
                return 0;
            if (range.end.fileId != range.begin.fileId)
                return 1;
            if (range.end.offset <= range.begin.offset)
                return 1;
            return range.end.offset - range.begin.offset;
        }
    }

    TextDiagnosticPrinter::TextDiagnosticPrinter(std::FILE *stream, bool useColor)
        : m_stream(stream != nullptr ? stream : stderr), m_useColor(useColor)
    {
        (void)m_useColor; // ANSI coloring is reserved for a future enhancement.
    }

    void TextDiagnosticPrinter::print(
        const DiagnosticSink &sink,
        const SourceManager &sourceManager
    )
    {
        const auto &diagnostics = sink.getDiagnostics();
        for (i32 i = 0; i < diagnostics.count(); i++)
            printOne(diagnostics[i], sourceManager, /*asNote=*/false);
    }

    void TextDiagnosticPrinter::printOne(
        const Diagnostic &diagnostic,
        const SourceManager &sourceManager,
        bool asNote
    )
    {
        const SourceRange &range = diagnostic.range;
        const char *severityText = asNote
            ? "note"
            : toString(diagnostic.severity);

        if (range.isValid())
        {
            const TkStringView path = sourceManager.getFilePath(range.begin.fileId);
            std::fprintf(m_stream, "%.*s:%u:%u: %s %s: %s",
                path.length(), path.get(),
                range.begin.line, range.begin.column,
                severityText,
                diagnostic.code.isEmpty() ? "" : diagnostic.code.get(),
                diagnostic.message.get());
        }
        else
        {
            std::fprintf(m_stream, "%s %s: %s",
                severityText,
                diagnostic.code.isEmpty() ? "" : diagnostic.code.get(),
                diagnostic.message.get());
        }

        // Append optional qualifiers.
        if (!diagnostic.backend.isEmpty() || !diagnostic.stage.isEmpty() || !diagnostic.variant.isEmpty())
        {
            std::fprintf(m_stream, " [");
            bool first = true;
            if (!diagnostic.backend.isEmpty()) { std::fprintf(m_stream, "%s", diagnostic.backend.get()); first = false; }
            if (!diagnostic.stage.isEmpty())   { std::fprintf(m_stream, "%s%s", first ? "" : "/", diagnostic.stage.get()); first = false; }
            if (!diagnostic.variant.isEmpty()) { std::fprintf(m_stream, "%s%s", first ? "" : "/", diagnostic.variant.get()); }
            std::fprintf(m_stream, "]");
        }
        std::fprintf(m_stream, "\n");

        // Render source snippet + underline if we can resolve the file.
        if (range.isValid())
        {
            const SourceFile *file = sourceManager.getFile(range.begin.fileId);
            if (file != nullptr)
            {
                const TkStringView lineText = file->getLineText(range.begin.line);
                if (lineText.length() > 0)
                {
                    std::fprintf(m_stream, "    %.*s\n", lineText.length(), lineText.get());

                    const u32 col = range.begin.column > 0 ? range.begin.column - 1 : 0;
                    const u32 underline = computeUnderlineLength(range);

                    std::fprintf(m_stream, "    ");
                    for (u32 c = 0; c < col; c++)
                        std::fputc(' ', m_stream);
                    std::fputc('^', m_stream);
                    for (u32 c = 1; c < underline; c++)
                        std::fputc('~', m_stream);
                    std::fputc('\n', m_stream);
                }
            }
        }

        // Print attached notes.
        for (i32 i = 0; i < diagnostic.notes.count(); i++)
        {
            Diagnostic noteDiag;
            noteDiag.severity = DiagnosticSeverity::Note;
            noteDiag.code = TkString::Empty;
            noteDiag.message = diagnostic.notes[i].message;
            noteDiag.range = diagnostic.notes[i].range;
            printOne(noteDiag, sourceManager, /*asNote=*/true);
        }
    }
}
