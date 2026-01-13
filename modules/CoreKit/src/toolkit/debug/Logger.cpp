/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Logger.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/collection/Array.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace toolkit
{
    static TkLogger *setupLogger()
    {
        LoggerInitParams params{
            .name = TK_SHARED_LOGGER_NAME,
            .useFileRecord = TK_SHARED_LOGGER_RECORD,
            .alloc = TkMemory::getDefault()};
        return TkMemory::newObject<TkLogger>(TkMemory::getDefault(), params);
    }

    TkLogger* TkLogger::getShared()
    {
        static TkLogger *s_logger = setupLogger();
        return s_logger;
    }

    void TkLogger::emitLog(const char *msg, LogLevel level)
    {
        switch(level)
		{
		case LogLevel::kWarn:
			m_logger->warn(msg);
			break;
		case LogLevel::kError:
			m_logger->error(msg);
			break;
		case LogLevel::kDebug:
			m_logger->debug(msg);
			break;
		case LogLevel::kInfo:
			m_logger->info(msg);
			break;
		case LogLevel::kTrace:
			m_logger->trace(msg);
			break;
        case LogLevel::kNone:
        default:
            break;
		}
    }

    void TkLogger::setup(LoggerInitParams &params)
    {
        // clear old logger instance
        if (m_logger)
            TkMemory::deletePtr(m_alloc, m_logger);

        // setup logger sinks
        TkArray<spdlog::sink_ptr> sinks {m_alloc};
        sinks.reserve(2);

        auto colorSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        colorSink->set_pattern(params.format);
        sinks.push(colorSink);

        if (params.useFileRecord)
        {
            auto fileName = TkString::format("%s.log", params.name).get();
            auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(fileName, true);
            fileSink->set_pattern(params.name);
            sinks.push(fileSink);
        }

        m_logger = TkMemory::newObject<spdlog::logger>(
            m_alloc,
            params.name,
            sinks.begin(),
            sinks.end());

        m_logger->set_level(spdlog::level::trace);
        m_logger->flush_on(spdlog::level::trace);
    }

    TkLogger::TkLogger(LoggerInitParams params)
        : m_alloc{params.alloc}
    {
        setup(params);
    }

    TkLogger::TkLogger(TkLogger *parent)
        : m_alloc{parent->m_alloc}, m_isShared{true}, m_logger{parent->m_logger}
    {
    }

    TkLogger::~TkLogger()
    {
        if (m_logger && !m_isShared)
            TkMemory::deletePtr(m_alloc, m_logger);
    }

    void TkLogger::setFormat(const char *format)
    {
        for (auto &sink : m_logger->sinks())
        {
            sink->set_pattern(format);
        }
    }

} // namespace toolkit