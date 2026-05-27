/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "OggDecoder.hpp"

#if TK_CODECS_VORBIS
#include "toolkit/base/Math.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/io/Stream.hpp"
#include "toolkit/memory/Memory.hpp"
#include <vorbis/codec.h>

namespace toolkit
{

    size_t oggRead(void *ptr, size_t size, size_t nmemb, void *data)
    {
        OggVorbisDecoder *decoderData = static_cast<OggVorbisDecoder *>(data);
        if (decoderData == nullptr || decoderData->stream == nullptr || size == 0 || nmemb == 0)
            return 0;

        const u32 pos = decoderData->stream->pos();
        const u32 len = decoderData->stream->length();
        if (pos >= len)
            return 0;

        const u32 bytesRequested = static_cast<u32>(size * nmemb);
        const u32 bytesAvailable = len - pos;
        const u32 bytesToRead = TkMath::min(bytesRequested, bytesAvailable);
        decoderData->stream->readBytes(ptr, bytesToRead);
        return static_cast<std::size_t>(bytesToRead / size);
    }

    int oggSeek(void *data, ogg_int64_t offset, int whence)
    {
        OggVorbisDecoder *decoderData = static_cast<OggVorbisDecoder *>(data);
        if (decoderData == nullptr || decoderData->stream == nullptr)
            return -1;

        const ogg_int64_t begin = static_cast<ogg_int64_t>(decoderData->offset);
        const ogg_int64_t end = static_cast<ogg_int64_t>(decoderData->stream->length());
        ogg_int64_t target = begin;

        switch (whence)
        {
        case SEEK_SET:
            target = begin + offset;
            break;
        case SEEK_CUR:
            target = static_cast<ogg_int64_t>(decoderData->stream->pos()) + offset;
            break;
        case SEEK_END:
            target = end + offset;
            break;
        default:
            return -1;
        }

        target = TkMath::clamp(target, begin, end);
        decoderData->stream->setPos(static_cast<u32>(target));
        return 0;
    }

    long oggTell(void *data)
    {
        OggVorbisDecoder *decoderData = static_cast<OggVorbisDecoder *>(data);
        if (decoderData == nullptr || decoderData->stream == nullptr)
            return 0;
        const u32 pos = decoderData->stream->pos();
        if (pos < decoderData->offset)
            return 0;
        return static_cast<long>(pos - decoderData->offset);
    }

    bool OggVorbisDecoder::open(ITkStream *stream, AudioDataInfo &info, u32 offset)
    {
        if (stream == nullptr)
            return false;

        if (file.datasource != nullptr)
        {
            ov_clear(&file);
            file = {};
        }

        stream->setPos(offset);
        this->stream = stream;
        this->offset = offset;

        const ov_callbacks callbacks = {&oggRead, &oggSeek, nullptr, &oggTell};
        const int status = ov_open_callbacks(this, &file, nullptr, 0, callbacks);
        if (status < 0)
        {
            LogWarn("Failed to open Ogg Vorbis file.");
            this->stream = nullptr;
            this->offset = 0;
            this->channelCount = 0;
            return false;
        }

        vorbis_info *vorbisInfo = ov_info(&file, -1);
        if (vorbisInfo == nullptr || vorbisInfo->channels <= 0 || vorbisInfo->rate <= 0)
        {
            ov_clear(&file);
            file = {};
            this->stream = nullptr;
            this->offset = 0;
            this->channelCount = 0;
            return false;
        }

        info.numChannels = vorbisInfo->channels;
        info.sampleRate = vorbisInfo->rate;
        const ogg_int64_t totalFrames = ov_pcm_total(&file, -1);
        if (totalFrames < 0)
            info.numSamples = 0;
        else if (static_cast<uint64_t>(totalFrames) * static_cast<uint64_t>(vorbisInfo->channels) > static_cast<uint64_t>(kMaxU32))
        {
            ov_clear(&file);
            file = {};
            this->stream = nullptr;
            this->offset = 0;
            this->channelCount = 0;
            return false;
        }
        else
            info.numSamples = static_cast<u32>(totalFrames) * static_cast<u32>(vorbisInfo->channels);
        info.bitDepth = 16;

        channelCount = info.numChannels;
        return true;
    }

    void OggVorbisDecoder::seek(u32 offset)
    {
        if (channelCount == 0 || file.datasource == nullptr)
            return;
        ov_pcm_seek(&file, offset / channelCount);
    }

    void OggVorbisDecoder::read(byte *samples, u32 numSamples)
    {
        readSamples(samples, numSamples);
    }

    u32 OggVorbisDecoder::readSamples(byte *samples, u32 numSamples)
    {
        if (samples == nullptr || numSamples == 0 || file.datasource == nullptr)
            return 0;

        u32 numReadSamples = 0;
        while (numReadSamples < numSamples)
        {
            const i32 bytesToRead = static_cast<i32>(numSamples - numReadSamples) * sizeof(i16);
            const long bytesRead = ov_read(&file, (char *)samples, bytesToRead, 0, 2, 1, nullptr);
            if (bytesRead > 0)
            {
                const u32 samplesRead = static_cast<u32>(bytesRead) / sizeof(i16);
                numReadSamples += samplesRead;
                samples += samplesRead * sizeof(i16);
            }
            else
            {
                break;
            }
        }
        if (numReadSamples < numSamples)
            TkMemory::memoryClear(samples, static_cast<psize>((numSamples - numReadSamples) * sizeof(i16)));
        return numReadSamples;
    }

    bool OggVorbisDecoder::isValid(ITkStream *stream, u32 offset)
    {
        if (stream == nullptr)
            return false;

        ITkStream *savedStream = this->stream;
        const u32 savedOffset = this->offset;
        const u32 savedPos = stream->pos();

        stream->setPos(offset);
        this->stream = stream;
        this->offset = offset;

        OggVorbis_File testFile{};
        const ov_callbacks callbacks = {&oggRead, &oggSeek, nullptr, &oggTell};
        const bool valid = (ov_test_callbacks(this, &testFile, nullptr, 0, callbacks) == 0);
        if (valid)
        {
            ov_clear(&testFile);
        }

        stream->setPos(savedPos);
        this->stream = savedStream;
        this->offset = savedOffset;
        return valid;
    }

    bool OggVorbisDecoder::isEOF() const
    {
        if (file.datasource == nullptr)
            return true;
        const ogg_int64_t current = ov_pcm_tell(const_cast<OggVorbis_File *>(&file));
        const ogg_int64_t total = ov_pcm_total(const_cast<OggVorbis_File *>(&file), -1);
        return current < 0 || total < 0 || current >= total;
    }

}
#endif
