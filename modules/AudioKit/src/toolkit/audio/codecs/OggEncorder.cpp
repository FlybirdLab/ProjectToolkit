/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "OggEncorder.hpp"

#if TK_CODECS_VORBIS

#include "toolkit/debug/Logger.hpp"
#include "toolkit/collection/Array.hpp"
#include "toolkit/audio/AudioTool.hpp"
#include "toolkit/base/Math.hpp"

namespace toolkit
{

// Writes to the internal cached buffer and flushes it if needed
#define WRITE_TO_BUFFER(data, length)                                 \
    if ((m_bufferOffset + length) > BUFFER_SIZE)                      \
        flush();                                                      \
    if (length > BUFFER_SIZE)                                         \
        m_writeCallback(data, length, m_userData);                     \
    else                                                              \
    {                                                                 \
        TkMemory::memoryCopy(m_buffer + m_bufferOffset, data, length); \
        m_bufferOffset += length;                                     \
    }

    OggVorbisEncoder::OggVorbisEncoder()
        : m_bufferOffset(0), m_numChannels(0), m_bitDepth(0), m_closed(true)
    {
    }

    OggVorbisEncoder::~OggVorbisEncoder()
    {
        close();
    }

    bool OggVorbisEncoder::open(WriteCallback writeCallback, u32 sampleRate, u32 bitDepth, u32 numChannels, float quality, void *userData)
    {
        m_numChannels = numChannels;
        m_bitDepth = bitDepth;
        m_writeCallback = writeCallback;
        m_userData = userData;
        m_closed = false;

        ogg_stream_init(&_oggState, rand());
        vorbis_info_init(&_vorbisInfo);

        i32 status = vorbis_encode_init_vbr(&_vorbisInfo, numChannels, sampleRate, quality);
        if (status != 0)
        {
            LogWarn("Failed to write Ogg Vorbis file.");
            close();
            return false;
        }

        vorbis_analysis_init(&_vorbisState, &_vorbisInfo);
        vorbis_block_init(&_vorbisState, &_vorbisBlock);

        // Generate header
        vorbis_comment comment;
        vorbis_comment_init(&comment);

        ogg_packet headerPacket, commentPacket, codePacket;
        status = vorbis_analysis_headerout(&_vorbisState, &comment, &headerPacket, &commentPacket, &codePacket);
        vorbis_comment_clear(&comment);

        if (status != 0)
        {
            LogWarn("Failed to write Ogg Vorbis file.");
            close();
            return false;
        }

        // Write header
        ogg_stream_packetin(&_oggState, &headerPacket);
        ogg_stream_packetin(&_oggState, &commentPacket);
        ogg_stream_packetin(&_oggState, &codePacket);

        ogg_page page;
        while (ogg_stream_flush(&_oggState, &page) > 0)
        {
            WRITE_TO_BUFFER(page.header, page.header_len);
            WRITE_TO_BUFFER(page.body, page.body_len);
        }

        return true;
    }

    void OggVorbisEncoder::write(u8 *samples, u32 numSamples)
    {
        static const u32 WRITE_LENGTH = 1024;

        u32 numFrames = numSamples / m_numChannels;
        while (numFrames > 0)
        {
            const u32 numFramesToWrite = TkMath::min(numFrames, WRITE_LENGTH);
            float **buffer = vorbis_analysis_buffer(&_vorbisState, numFramesToWrite);

            if (m_bitDepth == 8)
            {
                for (u32 i = 0; i < numFramesToWrite; i++)
                {
                    for (u32 j = 0; j < m_numChannels; j++)
                    {
                        const i8 sample = *(i8 *)samples;
                        const float encodedSample = sample / 127.0f;
                        buffer[j][i] = encodedSample;

                        samples++;
                    }
                }
            }
            else if (m_bitDepth == 16)
            {
                for (u32 i = 0; i < numFramesToWrite; i++)
                {
                    for (u32 j = 0; j < m_numChannels; j++)
                    {
                        const i16 sample = *(i16 *)samples;
                        const float encodedSample = sample / 32767.0f;
                        buffer[j][i] = encodedSample;

                        samples += 2;
                    }
                }
            }
            else if (m_bitDepth == 24)
            {
                for (u32 i = 0; i < numFramesToWrite; i++)
                {
                    for (u32 j = 0; j < m_numChannels; j++)
                    {
                        const i32 sample = AudioTool::convert24To32Bits(samples);
                        const float encodedSample = sample / 2147483647.0f;
                        buffer[j][i] = encodedSample;

                        samples += 3;
                    }
                }
            }
            else if (m_bitDepth == 32)
            {
                for (u32 i = 0; i < numFramesToWrite; i++)
                {
                    for (u32 j = 0; j < m_numChannels; j++)
                    {
                        const i32 sample = *(i32 *)samples;
                        const float encodedSample = sample / 2147483647.0f;
                        buffer[j][i] = encodedSample;

                        samples += 4;
                    }
                }
            }
            else
            {
                TK_ASSERT(false);
            }

            // Signal how many frames were written
            vorbis_analysis_wrote(&_vorbisState, numFramesToWrite);
            writeBlocks();

            numFrames -= numFramesToWrite;
        }
    }

    void OggVorbisEncoder::writeBlocks()
    {
        while (vorbis_analysis_blockout(&_vorbisState, &_vorbisBlock) == 1)
        {
            // Analyze and determine optimal bit rate
            vorbis_analysis(&_vorbisBlock, nullptr);
            vorbis_bitrate_addblock(&_vorbisBlock);

            // Write block into ogg packets
            ogg_packet packet;
            while (vorbis_bitrate_flushpacket(&_vorbisState, &packet))
            {
                ogg_stream_packetin(&_oggState, &packet);

                // If new page, write it to the internal buffer
                ogg_page page;
                while (ogg_stream_flush(&_oggState, &page) > 0)
                {
                    WRITE_TO_BUFFER(page.header, page.header_len);
                    WRITE_TO_BUFFER(page.body, page.body_len);
                }
            }
        }
    }

    struct EncodedBlock
    {
        u8 *data;
        u32 size;
    };

    struct ConvertWriteCallbackData
    {
        TkArray<EncodedBlock> blocks;
        u32 TotalEncodedSize = 0;
    };

    void ConvertWriteCallback(u8 *buffer, u32 size, void *userData)
    {
        EncodedBlock newBlock;
        newBlock.data = (u8 *)TkMemory::allocate(TkMemory::getDefault(), size);
        newBlock.size = size;

        TkMemory::memoryCopy(newBlock.data, buffer, size);

        auto data = (ConvertWriteCallbackData *)userData;
        data->blocks.add(newBlock);
        data->TotalEncodedSize += size;
    };

    bool OggVorbisEncoder::convert(byte *samples, AudioDataInfo &info, ByteCollection &result, float quality)
    {
        ConvertWriteCallbackData data;

        if (!open(ConvertWriteCallback, info.sampleRate, info.bitDepth, info.numChannels, quality, &data))
            return false;
        write(samples, info.numSamples);
        close();

        result.allocate(data.TotalEncodedSize);
        u32 offset = 0;
        for (auto &block : data.blocks)
        {
            TkMemory::memoryCopy(result.get() + offset, block.data, block.size);
            offset += block.size;

            TkMemory::free(TkMemory::getDefault(), block.data);
        }

        return true;
    }

    void OggVorbisEncoder::flush()
    {
        if (m_bufferOffset > 0 && m_writeCallback != nullptr)
            m_writeCallback(m_buffer, m_bufferOffset, m_userData);

        m_bufferOffset = 0;
    }

    void OggVorbisEncoder::close()
    {
        if (m_closed)
            return;

        // Mark end of data and flush any remaining data in the buffers
        vorbis_analysis_wrote(&_vorbisState, 0);
        writeBlocks();
        flush();

        ogg_stream_clear(&_oggState);
        vorbis_block_clear(&_vorbisBlock);
        vorbis_dsp_clear(&_vorbisState);
        vorbis_info_clear(&_vorbisInfo);

        m_closed = true;
    }

}

#endif