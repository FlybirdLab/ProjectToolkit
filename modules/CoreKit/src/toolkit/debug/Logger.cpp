/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Logger.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/threading/Sync.hpp"
#include "toolkit/types/String.hpp"
#include "toolkit/collection/Array.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace toolkit
{
    static TkLogger s_logger{{.name = TK_SHARED_LOGGER_NAME,
                              .useFileRecord = TK_SHARED_LOGGER_RECORD,
                              .alloc = TkMemory::getDefault()}};

    TkLogger *TkLogger::getShared()
    {
        return &s_logger;
    }

    void TkLogger::emitLog(const char *msg, LogLevel level)
    {
        switch (level)
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
            (m_infoLogger != nullptr ? m_infoLogger : m_logger)->info(msg);
            break;
        case LogLevel::kTrace:
            m_logger->trace(msg);
            break;
        case LogLevel::kSuccess:
            (m_successLogger != nullptr ? m_successLogger : m_logger)->info(msg);
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
        if (m_infoLogger)
            TkMemory::deletePtr(m_alloc, m_infoLogger);
        if (m_successLogger)
            TkMemory::deletePtr(m_alloc, m_successLogger);
        m_logger = nullptr;
        m_infoLogger = nullptr;
        m_successLogger = nullptr;

        // setup logger sinks
        TkArray<spdlog::sink_ptr> sinks{m_alloc};
        sinks.reserve(2);
        TkArray<spdlog::sink_ptr> infoSinks{m_alloc};
        infoSinks.reserve(2);
        TkArray<spdlog::sink_ptr> successSinks{m_alloc};
        successSinks.reserve(2);

        auto colorSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        colorSink->set_pattern(params.format);
        sinks.push(colorSink);

        if (params.infoFormat != nullptr)
        {
            auto infoSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            infoSink->set_pattern(params.infoFormat);
            infoSinks.push(infoSink);
        }

        if (params.successFormat != nullptr)
        {
            auto successSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            successSink->set_pattern(params.successFormat);
            successSinks.push(successSink);
        }

        if (params.useFileRecord)
        {
            auto fileName = TkString::format("%s.log", params.name).get();
            auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(fileName, true);
            fileSink->set_pattern(params.fileFormat != nullptr ? params.fileFormat : params.name);
            sinks.push(fileSink);
            if (params.infoFormat != nullptr)
                infoSinks.push(fileSink);
            if (params.successFormat != nullptr)
                successSinks.push(fileSink);
        }

        m_logger = TkMemory::newObject<spdlog::logger>(
            m_alloc,
            params.name,
            sinks.begin(),
            sinks.end());

        m_logger->set_level(spdlog::level::trace);
        m_logger->flush_on(spdlog::level::trace);

        if (params.infoFormat != nullptr)
        {
            m_infoLogger = TkMemory::newObject<spdlog::logger>(
                m_alloc,
                TkString::format("%s_info", params.name).get(),
                infoSinks.begin(),
                infoSinks.end());
            m_infoLogger->set_level(spdlog::level::trace);
            m_infoLogger->flush_on(spdlog::level::trace);
        }

        if (params.successFormat != nullptr)
        {
            m_successLogger = TkMemory::newObject<spdlog::logger>(
                m_alloc,
                TkString::format("%s_success", params.name).get(),
                successSinks.begin(),
                successSinks.end());
            m_successLogger->set_level(spdlog::level::info);
            m_successLogger->flush_on(spdlog::level::info);
        }
    }

    TkLogger::TkLogger(LoggerInitParams params)
        : m_alloc{params.alloc}
    {
        setup(params);
    }

    TkLogger::TkLogger(TkLogger *parent)
        : m_alloc{parent->m_alloc}, m_logger{parent->m_logger}, m_infoLogger{parent->m_infoLogger}, m_successLogger{parent->m_successLogger}, m_isShared{true}
    {
    }

    TkLogger::~TkLogger()
    {
        if (m_logger && !m_isShared)
            TkMemory::deletePtr(m_alloc, m_logger);
        if (m_infoLogger && !m_isShared)
            TkMemory::deletePtr(m_alloc, m_infoLogger);
        if (m_successLogger && !m_isShared)
            TkMemory::deletePtr(m_alloc, m_successLogger);
    }

    void TkLogger::setFormat(const char *format)
    {
        for (auto &sink : m_logger->sinks())
        {
            sink->set_pattern(format);
        }
    }

} // namespace toolkit
