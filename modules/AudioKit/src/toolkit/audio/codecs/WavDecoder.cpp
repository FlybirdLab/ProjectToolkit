/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "WavDecoder.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/io/Stream.hpp"
#include "toolkit/audio/AudioTool.hpp"
#include "toolkit/base/Math.hpp"

#define WAVE_FORMAT_PCM 0x0001
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#define WAVE_FORMAT_ALAW 0x0006
#define WAVE_FORMAT_MULAW 0x0007
#define WAVE_FORMAT_EXTENDED 0xFFFE
#define MAIN_CHUNK_SIZE 12

namespace toolkit
{

    bool WavDecoder::parseHeader(AudioDataInfo &info)
    {
        bool foundData = false;
        bool foundFmt = false;
        while (!foundData)
        {
            if (m_stream->pos() + 8 > m_stream->length())
                return false;

            // Get sub-chunk ID and size
            u8 subChunkId[4];
            m_stream->readBytes(subChunkId, sizeof(subChunkId));

            u32 subChunkSize = 0;
            m_stream->readU32(&subChunkSize);

            u32 totalRead = 0;

            // FMT chunk
            if (subChunkId[0] == 'f' && subChunkId[1] == 'm' && subChunkId[2] == 't' && subChunkId[3] == ' ')
            {
                if (subChunkSize < 16 || m_stream->pos() + subChunkSize > m_stream->length())
                    return false;

                u16 format;
                m_stream->readU16(&format);
                totalRead += 2;

                if (format != WAVE_FORMAT_PCM && format != WAVE_FORMAT_IEEE_FLOAT && format != WAVE_FORMAT_EXTENDED)
                {
                    LogWarn("Wav file doesn't contain raw PCM data. Not supported.");
                    return false;
                }

                u16 numChannels = 0;
                m_stream->readU16(&numChannels);
                totalRead += 2;

                u32 sampleRate = 0;
                m_stream->readU32(&sampleRate);
                totalRead += 4;

                u32 byteRate = 0;
                m_stream->readU32(&byteRate);
                totalRead += 4;

                u16 blockAlign = 0;
                m_stream->readU16(&blockAlign);
                totalRead += 2;

                u16 bitDepth = 0;
                m_stream->readU16(&bitDepth);
                totalRead += 2;

                if (bitDepth != 8 && bitDepth != 16 && bitDepth != 24 && bitDepth != 32)
                {
                    LogWarn("Unsupported number of bits per sample: %i", bitDepth);
                    return false;
                }

                info.numChannels = numChannels;
                info.sampleRate = sampleRate;
                info.bitDepth = bitDepth;

                // Read extension data, and get the actual format
                if (format == WAVE_FORMAT_EXTENDED)
                {
                    if (totalRead + 2 > subChunkSize)
                        return false;

                    u16 extensionSize = 0;
                    m_stream->readU16(&extensionSize);
                    totalRead += 2;

                    if (extensionSize < 22 || totalRead + extensionSize > subChunkSize)
                    {
                        LogWarn("Wav file doesn't contain raw PCM data. Not supported.");
                        return false;
                    }

                    u16 validBitDepth = 0;
                    m_stream->readU16(&validBitDepth);
                    totalRead += 2;

                    u32 channelMask = 0;
                    m_stream->readU32(&channelMask);
                    totalRead += 4;

                    u8 subFormat[16];
                    m_stream->readBytes(subFormat, sizeof(subFormat));
                    totalRead += 16;

                    TkMemory::memoryCopy(&format, subFormat, sizeof(format));
                    if (format != WAVE_FORMAT_PCM && format != WAVE_FORMAT_IEEE_FLOAT)
                    {
                        LogWarn("Wav file doesn't contain raw PCM data. Not supported.");
                        return false;
                    }
                }

                // Support wav with "extra format bytes", just ignore not needed
                while (totalRead < subChunkSize)
                {
                    u8 b;
                    m_stream->readBytes(&b, sizeof(b));
                    totalRead++;
                }

                mBytesPerSample = bitDepth / 8;
                mFormat = format;
                foundFmt = true;
                if (subChunkSize & 1U)
                {
                    if (m_stream->pos() + 1 > m_stream->length())
                        return false;
                    m_stream->setPos(m_stream->pos() + 1);
                }
            }
            // DATA chunk
            else if (subChunkId[0] == 'd' && subChunkId[1] == 'a' && subChunkId[2] == 't' && subChunkId[3] == 'a')
            {
                if (!foundFmt || mBytesPerSample == 0 || m_stream->pos() + subChunkSize > m_stream->length())
                    return false;

                info.numSamples = subChunkSize / mBytesPerSample;
                mDataOffset = (u32)m_stream->pos();
                mDataSize = subChunkSize;

                foundData = true;
            }
            // Unsupported chunk type
            else
            {
                const u32 skip = subChunkSize + (subChunkSize & 1U);
                if (m_stream->pos() + skip > m_stream->length())
                    return false;
                m_stream->setPos(m_stream->pos() + skip);
            }
        }

        return true;
    }

    bool WavDecoder::open(ITkStream *stream, AudioDataInfo &info, u32 offset)
    {
        TK_ASSERT(stream);

        m_stream = stream;
        m_stream->setPos(offset + MAIN_CHUNK_SIZE);

        if (!parseHeader(info))
        {
            LogWarn("Provided file is not a valid WAVE file.");
            return false;
        }

        return true;
    }

    void WavDecoder::seek(u32 offset)
    {
        m_stream->setPos(mDataOffset + offset * mBytesPerSample);
    }

    void WavDecoder::read(byte *samples, u32 numSamples)
    {
        const u32 readSamplesCount = readSamples(samples, numSamples);
        if (readSamplesCount < numSamples)
            return;
    }

    u32 WavDecoder::readSamples(byte *samples, u32 numSamples)
    {
        if (samples == nullptr || m_stream == nullptr || mBytesPerSample == 0 || numSamples == 0)
            return 0;

        const u32 offsetBytes = m_stream->pos() > mDataOffset ? m_stream->pos() - mDataOffset : 0;
        const u32 dataRemaining = offsetBytes < mDataSize ? mDataSize - offsetBytes : 0;
        const u32 streamRemaining = m_stream->pos() < m_stream->length() ? m_stream->length() - m_stream->pos() : 0;
        const u64 bytesRequested64 = static_cast<u64>(numSamples) * mBytesPerSample;
        if (bytesRequested64 > static_cast<u64>(kMaxU32))
            return 0;
        const u32 bytesRequested = static_cast<u32>(bytesRequested64);
        const u32 bytesToRead = TkMath::min(bytesRequested, TkMath::min(dataRemaining, streamRemaining));

        if (bytesToRead > 0)
            m_stream->readBytes(samples, bytesToRead);
        if (bytesToRead < bytesRequested)
            TkMemory::memoryClear(samples + bytesToRead, static_cast<psize>(bytesRequested - bytesToRead));

        // 8-bit samples are stored as unsigned, but engine convention is to store all bit depths as signed
        if (mBytesPerSample == 1)
        {
            for (u32 i = 0; i < bytesToRead; i++)
            {
                i8 val = samples[i] - 128;
                TkMemory::memoryCopy(samples + i, &val, sizeof(val));
            }
        }
        // IEEE float need to be converted into signed PCM data
        else if (mFormat == WAVE_FORMAT_IEEE_FLOAT)
        {
            AudioTool::convertFromFloat(reinterpret_cast<const float *>(samples), reinterpret_cast<i32 *>(samples), bytesToRead / sizeof(float));
        }

        return bytesToRead / mBytesPerSample;
    }

    bool WavDecoder::isValid(ITkStream *stream, u32 offset)
    {
        TK_ASSERT(stream);

        const u32 savedPos = stream->pos();
        if (offset + MAIN_CHUNK_SIZE > stream->length())
        {
            stream->setPos(savedPos);
            return false;
        }

        stream->setPos(offset);

        byte header[MAIN_CHUNK_SIZE];
        stream->readBytes(header, sizeof(header));

        const bool valid = (header[0] == 'R') && (header[1] == 'I') && (header[2] == 'F') && (header[3] == 'F') && (header[8] == 'W') && (header[9] == 'A') && (header[10] == 'V') && (header[11] == 'E');
        stream->setPos(savedPos);
        return valid;
    }

    bool WavDecoder::isEOF() const
    {
        if (m_stream == nullptr || mDataSize == 0)
            return true;
        const u32 currentPos = m_stream->pos();
        return currentPos >= mDataOffset + mDataSize;
    }

}
