/*
 * @Author: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/shader/diagnostics/JsonDiagnosticPrinter.hpp>
#include <toolkit/shader/source/SourceManager.hpp>

#include <toolkit/serialization/JsonWriters.hpp>

#include <cstdio>
#include <cstring>

namespace toolkit
{
    namespace
    {
        void writeRange(JsonWriter &w, const SourceRange &range, const SourceManager &sm)
        {
            if (!range.isValid())
                return;

            const TkStringView path = sm.getFilePath(range.begin.fileId);
            JSON_KEY(w, "file");
            w.string(path);
            JSON_KEY(w, "line");
            w.uintValue(range.begin.line);
            JSON_KEY(w, "column");
            w.uintValue(range.begin.column);

            if (range.end.isValid() &&
                (range.end.line != range.begin.line || range.end.column != range.begin.column))
            {
                JSON_KEY(w, "endLine");
                w.uintValue(range.end.line);
                JSON_KEY(w, "endColumn");
                w.uintValue(range.end.column);
            }
        }

        void writeDiagnostic(JsonWriter &w, const Diagnostic &d, const SourceManager &sm)
        {
            w.startObject();

            JSON_KEY(w, "severity");
            {
                const char *sev = toString(d.severity);
                w.string(sev, static_cast<i32>(std::strlen(sev)));
            }

            JSON_KEY(w, "code");
            w.string(d.code);
            JSON_KEY(w, "message");
            w.string(d.message);

            writeRange(w, d.range, sm);

            if (!d.backend.isEmpty()) { JSON_KEY(w, "backend"); w.string(d.backend); }
            if (!d.stage.isEmpty())   { JSON_KEY(w, "stage");   w.string(d.stage); }
            if (!d.variant.isEmpty()) { JSON_KEY(w, "variant"); w.string(d.variant); }

            if (d.notes.count() > 0)
            {
                JSON_KEY(w, "notes");
                w.startArray();
                for (i32 i = 0; i < d.notes.count(); i++)
                {
                    const DiagnosticNote &n = d.notes[i];
                    w.startObject();
                    JSON_KEY(w, "message");
                    w.string(n.message);
                    writeRange(w, n.range, sm);
                    w.endObject();
                }
                w.endArray(d.notes.count());
            }

            w.endObject();
        }
    }

    JsonDiagnosticPrinter::JsonDiagnosticPrinter(TkString *output, std::FILE *stream)
        : m_output(output), m_stream(stream != nullptr ? stream : stdout)
    {
    }

    void JsonDiagnosticPrinter::print(
        const DiagnosticSink &sink,
        const SourceManager &sourceManager
    )
    {
        TkString localBuffer;
        TkString &target = m_output != nullptr ? *m_output : localBuffer;

        {
            CompactJsonWriter writer(target);
            writer.startArray();
            const auto &diagnostics = sink.getDiagnostics();
            for (i32 i = 0; i < diagnostics.count(); i++)
                writeDiagnostic(writer, diagnostics[i], sourceManager);
            writer.endArray(diagnostics.count());
        }

        if (m_output == nullptr)
        {
            std::fprintf(m_stream, "%.*s\n", target.length(), target.get());
        }
    }
}
