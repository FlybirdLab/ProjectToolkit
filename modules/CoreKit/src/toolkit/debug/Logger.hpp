/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/memory/Memory.hpp"
#include <toolkit/common.hpp>
#include <toolkit/base/Format.hpp>

#ifndef TK_SHARED_LOGGER_NAME
#define TK_SHARED_LOGGER_NAME "ToolkitApp"
#endif

#ifndef TK_SHARED_LOGGER_RECORD
#define TK_SHARED_LOGGER_RECORD false
#endif

namespace spdlog
{
    class logger;
}

namespace toolkit
{
    enum class LogLevel : u8
    {
        kNone,
        kInfo,
        kWarn,
        kError,
        kDebug,
        kTrace,
        kSuccess
    };


    struct LoggerInitParams
    {
        // Spdlog format pattern
        static constexpr const char *sDefaultFormat = "[%L] %v"; //%^%$

        const char *name = "Anonmous";
        const char *format = sDefaultFormat;
        const char *infoFormat = nullptr;
        const char *successFormat = nullptr;
        const char *fileFormat = nullptr;
        bool useFileRecord : 1 = false;
        IAllocator &alloc = TkMemory::getDefault();
    };


    class TkLogger final
    {
    public:
        TkLogger(LoggerInitParams data = LoggerInitParams{});
        TkLogger(TkLogger* parent);
        ~TkLogger();

        void setFormat(const char* format);
        void setup(LoggerInitParams& params);

    public:
        template <typename... Args>
        TK_FORCE_INLINE void info(const char *fmtStr, const Args &...args)
        {
            FormatBuffer buf{m_alloc};
            Formatter::format(buf, fmtStr, std::forward<const Args &>(args)...);
            emitLog(buf.getPointer(), LogLevel::kInfo);
        }
        TK_FORCE_INLINE void info(const char *msg)
        {
            emitLog(msg, LogLevel::kInfo);
        }

        template <typename... Args>
        TK_FORCE_INLINE void warning(const char *fmtStr, const Args &...args)
        {
            FormatBuffer buf{m_alloc};
            Formatter::format(buf, fmtStr, std::forward<const Args &>(args)...);
            emitLog(buf.getPointer(), LogLevel::kWarn);
        }
        TK_FORCE_INLINE void warning(const char *msg)
        {
            emitLog(msg, LogLevel::kWarn);
        }

        template <typename... Args>
        TK_FORCE_INLINE void error(const char *fmtStr, const Args &...args)
        {
            FormatBuffer buf{m_alloc};
            Formatter::format(buf, fmtStr, std::forward<const Args &>(args)...);
            emitLog(buf.getPointer(), LogLevel::kError);
        }
        TK_FORCE_INLINE void error(const char *msg)
        {
            emitLog(msg, LogLevel::kError);
        }

        template <typename... Args>
        TK_FORCE_INLINE void debug(const char *fmtStr, const Args &...args)
        {
#if TK_DEBUG
            FormatBuffer buf{m_alloc};
            Formatter::format(buf, fmtStr, std::forward<const Args &>(args)...);
            emitLog(buf.getPointer(), LogLevel::kDebug);
#else
            (void)fmtStr;
            (void)sizeof...(args);
#endif
        }
        TK_FORCE_INLINE void debug(const char *msg)
        {
#if TK_DEBUG
            emitLog(msg, LogLevel::kDebug);
#else
            (void)msg;
#endif
        }

        template <typename... Args>
        TK_FORCE_INLINE void trace(const char *fmtStr, const Args &...args)
        {
            FormatBuffer buf{m_alloc};
            Formatter::format(buf, fmtStr, std::forward<const Args &>(args)...);
            emitLog(buf.getPointer(), LogLevel::kTrace);
        }
        TK_FORCE_INLINE void trace(const char *msg)
        {
            emitLog(msg, LogLevel::kTrace);
        }

        template <typename... Args>
        TK_FORCE_INLINE void success(const char *fmtStr, const Args &...args)
        {
            FormatBuffer buf{m_alloc};
            Formatter::format(buf, fmtStr, std::forward<const Args &>(args)...);
            emitLog(buf.getPointer(), LogLevel::kSuccess);
        }
        TK_FORCE_INLINE void success(const char *msg)
        {
            emitLog(msg, LogLevel::kSuccess);
        }

        TK_FORCE_INLINE void log(LogLevel level, const char* msg) {
            emitLog(msg, level);
        }

    public:
        static TkLogger* getShared();

    private:
        void emitLog(const char *msg, LogLevel level);

    private:
        IAllocator &m_alloc;
        spdlog::logger *m_logger = nullptr;
        spdlog::logger *m_infoLogger = nullptr;
        spdlog::logger *m_successLogger = nullptr;
        bool m_isShared : 1 = false;
    };

}

#define LogInfo(...)    ::toolkit::TkLogger::getShared()->info(__VA_ARGS__)
#define LogWarn(...)    ::toolkit::TkLogger::getShared()->warning(__VA_ARGS__)
#define LogError(...)   ::toolkit::TkLogger::getShared()->error(__VA_ARGS__)
#define LogDebug(...)   ::toolkit::TkLogger::getShared()->debug(__VA_ARGS__)
#define LogTrace(...)   ::toolkit::TkLogger::getShared()->trace(__VA_ARGS__)
#define LogSuccess(...) ::toolkit::TkLogger::getShared()->success(__VA_ARGS__)
