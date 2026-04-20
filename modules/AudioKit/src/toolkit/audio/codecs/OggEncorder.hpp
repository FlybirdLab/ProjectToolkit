/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/audio/AudioEncoder.hpp>

#if TK_CODECS_VORBIS

#include <vorbis/vorbisenc.h>

namespace toolkit
{
    // Raw PCM data encoder to Ogg Vorbis audio format.
    class OggVorbisEncoder : public AudioEncoder
    {
    public:
        typedef void (*WriteCallback)(byte *, u32, void *);

    public:
        OggVorbisEncoder();
        ~OggVorbisEncoder();

    public:
        // Set up the writer. Should be called before calling Write().
        bool open(WriteCallback writeCallback, u32 sampleRate, u32 bitDepth, u32 numChannels, float quality, void *userData = nullptr);
        // Write a new set of samples and converts them to Ogg Vorbis.
        void write(byte *samples, u32 numSamples);
        // Flush the last of the data into the write buffer (triggers the write callback). This is called automatically when the writer is closed or goes out of scope.
        void flush();
        // Close the encoder and flushes the last of the data into the write buffer (triggers the write callback). This is called automatically when the writer goes out of scope.
        void close();

    private:
        // Write Vorbis blocks into Ogg packets.
        void writeBlocks();

    public:
        bool convert(byte *samples, AudioDataInfo &info, ByteCollection &result, float quality = 0.5f) override;

    private:
        static const u32 BUFFER_SIZE = 4096;

        WriteCallback m_writeCallback;
        void *m_userData;
        byte m_buffer[BUFFER_SIZE];
        u32 m_bufferOffset;
        u32 m_numChannels;
        u32 m_bitDepth;
        bool m_closed;

        ogg_stream_state _oggState;
        vorbis_info _vorbisInfo;
        vorbis_dsp_state _vorbisState;
        vorbis_block _vorbisBlock;
    };

}

#endif