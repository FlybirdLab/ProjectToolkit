/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by Flybird Games. All rights reserved.
 */

#include "Mp3Decoder.hpp"

#include "toolkit/base/Math.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/io/Stream.hpp"

#define MINIMP3_IMPLEMENTATION
#include <minimp3_ex.h>

namespace toolkit
{
    namespace
    {
        struct Mp3StreamContext
        {
            ITkStream *stream = nullptr;
            u32 baseOffset = 0;
        };

        static size_t readCallback(void *buf, size_t size, void *userData)
        {
            auto *ctx = static_cast<Mp3StreamContext *>(userData);
            if (ctx == nullptr || ctx->stream == nullptr || buf == nullptr || size == 0)
                return 0;

            const u32 streamLength = ctx->stream->length();
            const u32 currentPos = ctx->stream->pos();
            if (currentPos >= streamLength)
                return 0;

            const u32 remaining = streamLength - currentPos;
            const u32 toRead = remaining < static_cast<u32>(size) ? remaining : static_cast<u32>(size);
            if (toRead == 0)
                return 0;

            ctx->stream->readBytes(buf, toRead);
            return toRead;
        }

        static int seekCallback(uint64_t position, void *userData)
        {
            auto *ctx = static_cast<Mp3StreamContext *>(userData);
            if (ctx == nullptr || ctx->stream == nullptr)
                return 1;

            const uint64_t target = static_cast<uint64_t>(ctx->baseOffset) + position;
            if (target > ctx->stream->length())
                return 1;

            ctx->stream->setPos(static_cast<u32>(target));
            return 0;
        }
    }

    MP3Decoder::~MP3Decoder()
    {
        close();
    }

    void MP3Decoder::close()
    {
        if (m_isOpen)
        {
            mp3dec_ex_close(&m_exd);
            m_isOpen = false;
        }

        m_stream = nullptr;
        m_readSampleOffset = 0;
        m_ioCtx = {};
        m_info = {};
        m_info.bitDepth = 16;
    }

    bool MP3Decoder::open(ITkStream *stream, AudioDataInfo &info, u32 offset)
    {
        TK_ASSERT(stream);

        close();

        m_stream = stream;
        m_ioCtx.stream = stream;
        m_ioCtx.baseOffset = offset;
        m_io.read = &readCallback;
        m_io.read_data = &m_ioCtx;
        m_io.seek = &seekCallback;
        m_io.seek_data = &m_ioCtx;

        stream->setPos(offset);
        const int result = mp3dec_ex_open_cb(&m_exd, &m_io, 0);
        if (result != 0)
        {
            close();
            return false;
        }

        m_isOpen = true;
        m_readSampleOffset = 0;
        m_info.numSamples = static_cast<u32>(m_exd.samples);
        m_info.sampleRate = static_cast<u32>(m_exd.info.hz);
        m_info.numChannels = static_cast<u32>(m_exd.info.channels);
        m_info.bitDepth = 16;
        info = m_info;
        return true;
    }

    bool MP3Decoder::convert(ITkStream *stream, AudioDataInfo &info, TkArray<byte> &result, u32 offset)
    {
        if (!open(stream, info, offset))
            return false;

        const size_t bytesPerSample = sizeof(mp3d_sample_t);
        const size_t totalBytes = static_cast<size_t>(info.numSamples) * bytesPerSample;
        result.resize(static_cast<i32>(totalBytes));

        size_t decodedSamples = mp3dec_ex_read(&m_exd, reinterpret_cast<mp3d_sample_t *>(result.get()), info.numSamples);
        if (decodedSamples != info.numSamples)
        {
            result.resize(static_cast<i32>(decodedSamples * bytesPerSample));
            info.numSamples = static_cast<u32>(decodedSamples);
        }

        close();
        return true;
    }

    void MP3Decoder::seek(u32 offset)
    {
        if (!m_isOpen)
        {
            m_readSampleOffset = 0;
            return;
        }

        m_readSampleOffset = TkMath::clamp(offset, 0U, m_info.numSamples);
        if (mp3dec_ex_seek(&m_exd, m_readSampleOffset) != 0)
            m_readSampleOffset = 0;
    }

    void MP3Decoder::read(byte *samples, u32 numSamples)
    {
        if (!m_isOpen || samples == nullptr || numSamples == 0 || m_info.bitDepth == 0)
            return;

        const u32 availableSamples = (m_readSampleOffset < m_info.numSamples) ? (m_info.numSamples - m_readSampleOffset) : 0;
        const u32 readSamples = numSamples < availableSamples ? numSamples : availableSamples;
        const size_t bytesPerSample = sizeof(mp3d_sample_t);
        const size_t readBytes = static_cast<size_t>(readSamples) * bytesPerSample;

        if (readSamples > 0)
        {
            const size_t gotSamples = mp3dec_ex_read(&m_exd, reinterpret_cast<mp3d_sample_t *>(samples), readSamples);
            const size_t gotBytes = gotSamples * bytesPerSample;
            if (gotBytes < readBytes)
            {
                TkMemory::memoryClear(samples + gotBytes, static_cast<psize>(readBytes - gotBytes));
                m_readSampleOffset += static_cast<u32>(gotSamples);
                return;
            }

            m_readSampleOffset += static_cast<u32>(gotSamples);
        }

        if (readSamples < numSamples)
        {
            const size_t remainBytes = static_cast<size_t>(numSamples - readSamples) * bytesPerSample;
            TkMemory::memoryClear(samples + readBytes, static_cast<psize>(remainBytes));
        }
    }

    bool MP3Decoder::isValid(ITkStream *stream, u32 offset)
    {
        TK_ASSERT(stream);

        Mp3StreamContext ctx;
        ctx.stream = stream;
        ctx.baseOffset = offset;

        mp3dec_io_t io{};
        io.read = &readCallback;
        io.read_data = &ctx;
        io.seek = &seekCallback;
        io.seek_data = &ctx;

        TkArray<byte> probe;
        probe.resize(MINIMP3_BUF_SIZE);
        return mp3dec_detect_cb(&io, probe.get(), probe.count()) == 0;
    }
}
