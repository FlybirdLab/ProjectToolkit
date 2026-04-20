#include "Diagnostic.hpp"
#include "toolkit/debug/Logger.hpp"

namespace toolkit {

    void DiagnosticCollector::report(DiagnosticLevel level, const SourceLoc& loc, const TkString& message)
    {
        m_diagnostics.add(Diagnostic{ level, loc, message });
        if (level == DiagnosticLevel::kError)
            ++m_errorCount;
    }


    void DiagnosticCollector::printAll() const
    {
        for (auto& dig : m_diagnostics)
        {
            switch (dig.level)
            {
            case DiagnosticLevel::kError:
                LogError("{0} | {1}", dig.location.toString(), dig.message);
                break;
            case DiagnosticLevel::kWarning:
                LogWarn("{0} | {1}", dig.location.toString(), dig.message);
                break;
            case DiagnosticLevel::kNote:
                LogInfo("{0} | {1}", dig.location.toString(), dig.message);
                break;
            default:
                break;
            }
        }
    }


    void DiagnosticCollector::clear(DiagnosticLevel flags)
    {
        if (flags == DiagnosticLevel::kNone) {
            m_diagnostics.clear();
            return;
        }
        TkArray<Diagnostic> items {};
        for (auto& item : m_diagnostics) {
            if (item.level != flags) {
                items.add(item);
            }
        }
        m_diagnostics.clear();
        items.swap(m_diagnostics);
    }


}