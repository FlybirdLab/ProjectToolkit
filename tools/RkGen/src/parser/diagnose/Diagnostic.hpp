#pragma once

#include "SourceLoc.hpp"
#include "../Source.hpp"
#include "../Tokens.hpp"

#include <toolkit/collection/Array.hpp>

namespace toolkit {

    enum class DiagnosticLevel : u32 {
        kNone = 0,
        kError = 1 << 0,
        kWarning = 1 << 1,
        kNote = 1 << 2,
    };


    struct Diagnostic {
        DiagnosticLevel level;
        SourceLoc location;
        TkString message;
    };


    class DiagnosticCollector
    {
    public:
        DiagnosticCollector() = default;

    public:
        void report(DiagnosticLevel level, const SourceLoc& loc, const TkString& message);

        TK_FORCE_INLINE void error(const SourceLoc& loc, const TkString& msg) {
            report(DiagnosticLevel::kError, loc, msg);
        }
        TK_FORCE_INLINE void warning(const SourceLoc& loc, const TkString& msg) {
            report(DiagnosticLevel::kWarning, loc, msg);
        }
        TK_FORCE_INLINE void note(const SourceLoc& loc, const TkString& msg) {
            report(DiagnosticLevel::kNote, loc, msg);
        }

        TK_FORCE_INLINE bool hasError() const { return m_errorCount > 0; }
        TK_FORCE_INLINE int getErrorCount() const { return m_errorCount; }
        const TkArray<Diagnostic>& diagnostics() const { return m_diagnostics; }

        void printAll() const;
        void clear(DiagnosticLevel flags = DiagnosticLevel::kNone);

    private:
        TkArray<Diagnostic> m_diagnostics;
        int m_errorCount = 0;
    };


    template<typename T>
    class ErrorOr {
    public:
        static ErrorOr<T> success(T value) {
            return ErrorOr(std::move(value));
        }

        static ErrorOr<T> failure(TkString errorMsg) {
            return ErrorOr(std::move(errorMsg));
        }

        bool hasError() const { return m_hasError; }
        const TkString& errorMessage() const { return m_errorMessage; }
        const T& value() const { TK_ASSERT(!m_hasError); return m_value; }
        T& value() { TK_ASSERT(!m_hasError); return m_value; }

        explicit operator bool() const { return !m_hasError; }

    private:
        ErrorOr(T val)
            : m_value(std::move(val)), m_hasError(false) {
        }

        ErrorOr(TkString msg)
            : m_errorMessage(std::move(msg)), m_hasError(true) {
        }

        T m_value;
        TkString m_errorMessage;
        bool m_hasError = true;
    };


    template<>
    class ErrorOr<void> {
    public:
        static ErrorOr success() {
            return {};
        }

        static ErrorOr<void> failure(TkString errorMsg) {
            return {std::move(errorMsg)};
        }

        bool hasError() const { return m_hasError; }
        const TkString& errorMessage() const { return m_errorMessage; }

        explicit operator bool() const { return !m_hasError; }

    private:
        ErrorOr() : m_hasError(false) {}
        ErrorOr(TkString msg)
            : m_errorMessage(std::move(msg)) {
        }

        TkString m_errorMessage;
        bool m_hasError = true;
    };


    class Error
    {
    public:
        Error() = default;

        static Error success() { return {}; }
        static Error failure(TkString msg) { return Error(std::move(msg)); }
        static Error failure(TkString msg, DiagnosticCollector* c, Token& t, CXXSourceFile* f) {
            c->error(t.locate(f), msg);
            return Error(std::move(msg));
        }

        bool hasError() const { return !m_messages.isEmpty(); }
        const TkArray<TkString>& messages() const { return m_messages; }

        void append(TkString msg) {
            m_messages.add(std::move(msg));
        }

        Error& operator<<(const TkString& msg) {
            append(msg);
            return *this;
        }

    private:
        TkArray<TkString> m_messages;

        explicit Error(TkString msg) {
            m_messages.add(std::move(msg));
        }
    };


    template<typename T>
    class Expected
    {
    public:
        Expected(T value)
            : m_value(std::move(value)), m_hasError(false), m_got{ false } {
        }
        Expected(Error error)
            : m_value{}, m_error(std::move(error)), m_hasError(m_error.hasError()), m_got{ false } {
        }
        ~Expected() {
            if (m_got) return;
            if constexpr (std::is_pointer<T>::value) {
                if (!m_value) return;
                TkMemory::deletePtr(m_value);
            }
        }

        bool hasError() const { return m_hasError; }
        const Error& error() const { return m_error; }
        Error& error() { return m_error; }

        const T& value() const { 
            TK_ASSERT(!m_hasError); 
            m_got = true;
            return m_value; 
        }
        T& value() {
            TK_ASSERT(!m_hasError);
            m_got = true;
            return m_value;
        }
        T& operator->() {
            return m_value;
        }

        explicit operator bool() const { return !m_hasError; }

    private:
        T m_value;
        Error m_error;
        bool m_hasError;
        mutable bool m_got;
    };


    template<>
    class Expected<void>
    {
    public:
        Expected() : m_hasError(false) {}
        Expected(Error error) : m_error(std::move(error)), m_hasError(m_error.hasError()) {}

        bool hasError() const { return m_hasError; }
        const Error& error() const { return m_error; }
        Error& error() { return m_error; }

        explicit operator bool() const { return !m_hasError; }

    private:
        Error m_error;
        bool m_hasError;
    };

#define CHECK_ERROR(V) do { \
    if (!V) { \
        return V.error(); \
    } \
} while(false)


    template<typename E>
    constexpr auto to_underlying(E e) noexcept {
        return static_cast<std::underlying_type_t<E>>(e);
    }
    constexpr DiagnosticLevel operator|(DiagnosticLevel lhs, DiagnosticLevel rhs) {
        return static_cast<DiagnosticLevel>(to_underlying(lhs) | to_underlying(rhs));
    }
    constexpr DiagnosticLevel operator&(DiagnosticLevel lhs, DiagnosticLevel rhs) {
        return static_cast<DiagnosticLevel>(to_underlying(lhs) & to_underlying(rhs));
    }
    constexpr DiagnosticLevel operator~(DiagnosticLevel v) {
        return static_cast<DiagnosticLevel>(~to_underlying(v));
    }
    inline DiagnosticLevel& operator|=(DiagnosticLevel& lhs, DiagnosticLevel rhs) {
        lhs = lhs | rhs;
        return lhs;
    }
    inline DiagnosticLevel& operator&=(DiagnosticLevel& lhs, DiagnosticLevel rhs) {
        lhs = lhs & rhs;
        return lhs;
    }
}
