/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/audio/Audio.hpp>
#include <toolkit/audio/AudioClip.hpp>
#include <toolkit/audio/AudioDecoder.hpp>
#include <toolkit/audio/AudioEncoder.hpp>
#include <toolkit/audio/AudioListener.hpp>
#include <toolkit/audio/AudioSource.hpp>
#include <toolkit/audio/AudioSystem.hpp>
#include <toolkit/audio/AudioTool.hpp>
#include <toolkit/audio/codecs/Mp3Decoder.hpp>
#include <toolkit/audio/codecs/OggDecoder.hpp>
#include <toolkit/audio/codecs/OggEncorder.hpp>
#include <toolkit/audio/codecs/WavDecoder.hpp>
#include <toolkit/io/FileStream.hpp>
#include <toolkit/io/MemoryStream.hpp>
#include <toolkit/memory/Memory.hpp>

#define GTEST_HAS_SEH 0
#include <gtest/gtest.h>

using namespace toolkit;

namespace audio_test
{
    class AudioTestBase : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            Audio::dispose();
            Audio::Listeners.clear();
            Audio::Sources.clear();
            Audio::Devices.clear();
            Audio::setMasterVolume(1.0f);
            Audio::setMuteOnFocusLoss(true);
            Audio::setDopplerFactor(1.0f);
            Audio::setEnableHRTF(false);
            Audio::setAppHasFocus(true);
            Audio::setPaused(false);
            AudioSystem::s_disableAudio = false;
        }

        void TearDown() override
        {
            Audio::dispose();
            Audio::Listeners.clear();
            Audio::Sources.clear();
            Audio::Devices.clear();
            Audio::setMasterVolume(1.0f);
            Audio::setMuteOnFocusLoss(true);
            Audio::setDopplerFactor(1.0f);
            Audio::setEnableHRTF(false);
            Audio::setAppHasFocus(true);
            Audio::setPaused(false);
            AudioSystem::s_disableAudio = false;
        }
    };

    inline TkString audioDocPath(const char *fileName)
    {
        return TkString(AUDIO_TEST_DOC_DIR) / fileName;
    }

    template<typename T, typename... Ts>
    inline TkArray<byte> packPcm(T first, Ts... rest)
    {
        const T values[] = {first, static_cast<T>(rest)...};
        TkArray<byte> bytes;
        bytes.resize(static_cast<i32>(sizeof(values)));
        TkMemory::memoryCopy(bytes.get(), values, sizeof(values));
        return bytes;
    }

    inline AudioDataInfo makeInfo(u32 numSamples, u32 sampleRate, u32 numChannels, u32 bitDepth)
    {
        AudioDataInfo info{};
        info.numSamples = numSamples;
        info.sampleRate = sampleRate;
        info.numChannels = numChannels;
        info.bitDepth = bitDepth;
        return info;
    }

    inline TkArray<byte> makeKnownWav16MonoSamples()
    {
        return packPcm<i16>(0, 16384, -16384, 32767);
    }

    inline TkArray<byte> makeTwoChunk16MonoBytes()
    {
        return packPcm<i16>(-32767, 0, 16384, 32767);
    }

    inline TkArray<byte> makeStereo16PcmBytes()
    {
        return packPcm<i16>(0, 32767, 16384, -16384, -16384, 16384, 32767, 0);
    }

    inline TkArray<byte> makeBytesFromData(const void *data, i32 size)
    {
        TkArray<byte> bytes;
        bytes.resize(size);
        if (size > 0)
            TkMemory::memoryCopy(bytes.get(), data, size);
        return bytes;
    }

    inline TkSpan<const byte> asSpan(const TkArray<byte> &bytes)
    {
        return TkSpan<const byte>(bytes.get(), bytes.count());
    }

    inline AudioClip::Descriptor makeClipDescriptor(const AudioDataInfo &info, bool is3D = false, bool streamable = false)
    {
        AudioClip::Descriptor descriptor{};
        descriptor.info = info;
        descriptor.is3D = is3D;
        descriptor.streamable = streamable;
        return descriptor;
    }
}
