/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/video/common.hpp>

#if VIDEO_API_ANDROID

#include "toolkit/video/VideoBackend.hpp"
#include "toolkit/base/Assert.hpp"
#include "toolkit/base/Math.hpp"
#include "toolkit/collection/Array.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/utils/PixelFormat.hpp"
#include "toolkit/io/FileStream.hpp"
#include "toolkit/types/StringView.hpp"

#include <cstring>
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaExtractor.h>
#include <media/NdkMediaFormat.h>

#define VIDEO_API_ANDROID_DEBUG 0
#define VIDEO_API_ANDROID_ERROR(api, err) LogWarn("[VideoBackendAndroid] %s failed with error %lld", #api, (i64)(err))

namespace toolkit
{
    struct VideoPlayerAndroid
    {
        AMediaExtractor *extractor = nullptr;
        AMediaCodec *videoCodec = nullptr;
        AMediaCodec *audioCodec = nullptr;
        AMediaFormat *videoFormat = nullptr;
        AMediaFormat *audioFormat = nullptr;
        u8 loop : 1;
        u8 playing : 1;
        u8 inputEnded : 1;
        u8 outputEnded : 1;
        u16 videoStride = 0;
        u16 videoTrackIndex = 0xFFFF;
        u16 audioTrackIndex = 0xFFFF;
    };

    namespace Android
    {
        static TkArray<VideoBackendPlayer *> g_players;

        // Reference: http://developer.android.com/reference/android/media/MediaCodecInfo.CodecCapabilities.html
        enum ColourFormat
        {
            kFormat32bitABGR8888 = 0x7f00a000,
            kFormat32bitARGB8888 = 16,
            kFormat32bitBGRA8888 = 15,
            kFormatYUV420SemiPlanar = 21,
            kFormatYUV422SemiPlanar = 24,
        };

        static VideoPlayerAndroid &getState(VideoBackendPlayer &player)
        {
            if (player.backendData == nullptr)
                player.backendData = new VideoPlayerAndroid {};
            return *static_cast<VideoPlayerAndroid *>(player.backendData);
        }

        static ssize_t mediaDataSourceReadAt(void *userdata, off64_t offset, void *buffer, size_t size)
        {
            if (size == 0)
                return 0;
            auto *stream = static_cast<FileIStream *>(userdata);
            stream->setPos((u32)offset);
            stream->readBytes(buffer, (u32)size);
            return (ssize_t)size;
        }

        static ssize_t mediaDataSourceGetSize(void *userdata)
        {
            auto *stream = static_cast<FileIStream *>(userdata);
            return (ssize_t)stream->length();
        }

        static void mediaDataSourceClose(void *userdata)
        {
            auto *stream = static_cast<FileIStream *>(userdata);
            delete stream;
        }

        static void updateFormat(VideoBackendPlayer &player, VideoPlayerAndroid &playerAndroid, AMediaCodec *codec, AMediaFormat *format)
        {
            const bool isVideo = codec == playerAndroid.videoCodec;
            const bool isAudio = codec == playerAndroid.audioCodec;
            if (isVideo)
            {
                int32_t frameWidth = 0, frameHeight = 0, frameRate = 0, colorFormat = 0, stride = 0;
                float frameRateF = 0.0f;
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_WIDTH, &frameWidth);
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_HEIGHT, &frameHeight);
                if (AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_FRAME_RATE, &frameRate) && frameRate > 0)
                    player.frameRate = (f32)frameRate;
                else if (AMediaFormat_getFloat(format, AMEDIAFORMAT_KEY_FRAME_RATE, &frameRateF) && frameRateF > 0)
                    player.frameRate = frameRateF;
                else
                    player.frameRate = 60.0f;
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_STRIDE, &stride);
                playerAndroid.videoStride = (u16)stride;
                player.width = player.videoFrameWidth = frameWidth;
                player.height = player.videoFrameHeight = frameHeight;
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, &colorFormat);
                switch (colorFormat)
                {
                case kFormat32bitABGR8888:
                    player.format = PixelFormat::kR8G8B8A8_UNorm;
                    break;
                case kFormat32bitBGRA8888:
                    player.format = PixelFormat::kB8G8R8A8_UNorm;
                    break;
                case kFormatYUV420SemiPlanar:
                    player.format = PixelFormat::kNV12;
                    break;
                case kFormatYUV422SemiPlanar:
                    player.format = PixelFormat::kYUY2;
                    break;
                default:
                    player.format = PixelFormat::kUnknown;
                    LogError("[VideoBackendAndroid] Unsupported video color format %d", colorFormat);
                    break;
                }
#if VIDEO_API_ANDROID_DEBUG
                LogInfo("[VideoBackendAndroid] Video track: %dx%d, %ffps", player.width, player.height, player.frameRate);
#endif
            }
            else if (isAudio)
            {
                int32_t sampleRate = 0, channelCount = 0, bitsPerSample = 0;
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_SAMPLE_RATE, &sampleRate);
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &channelCount);
                player.audioInfo.sampleRate = (u32)sampleRate;
                player.audioInfo.numChannels = (u32)channelCount;
                if (AMediaFormat_getInt32(format, "bits-per-sample", &bitsPerSample) && bitsPerSample > 0)
                    player.audioInfo.bitDepth = (u32)bitsPerSample;
                else
                    player.audioInfo.bitDepth = 16;
            }
        }

        static void readCodecOutput(VideoBackendPlayer &player, VideoPlayerAndroid &playerAndroid, AMediaCodec *codec, AMediaFormat *&format)
        {
            if (!codec)
                return;
            AMediaCodecBufferInfo bufferInfo {};
            ssize_t bufferIndex = AMediaCodec_dequeueOutputBuffer(codec, &bufferInfo, 0);
            if (bufferIndex >= 0)
            {
                if (bufferInfo.size > 0)
                {
                    TkTimeSpan frameTime {bufferInfo.presentationTimeUs * 10};
                    TkTimeSpan frameDuration = player.frameRate > 0
                                                   ? TkTimeSpan::fromSeconds(1.0 / player.frameRate)
                                                   : TkTimeSpan {};
                    size_t bufferSize = 0;
                    uint8_t *buffer = AMediaCodec_getOutputBuffer(codec, bufferIndex, &bufferSize);
                    TK_ASSERT(buffer && bufferSize);
                    if (codec == playerAndroid.videoCodec)
                    {
                        switch (player.format)
                        {
                        case PixelFormat::kYUY2:
                        case PixelFormat::kNV12:
                            bufferSize = (size_t)player.videoFrameHeight * playerAndroid.videoStride * 3 / 2;
                            break;
                        default:
                            break;
                        }
                        player.updateVideoFrame(TkSpan<byte> {buffer, (i32)bufferSize}, frameTime, frameDuration);
                    }
                    else if (codec == playerAndroid.audioCodec)
                    {
                        player.updateAudioBuffer(TkSpan<byte> {buffer + bufferInfo.offset, (i32)bufferInfo.size}, frameTime, frameDuration);
                    }
                }
                media_status_t status = AMediaCodec_releaseOutputBuffer(codec, bufferIndex, false);
                if (status != AMEDIA_OK)
                    VIDEO_API_ANDROID_ERROR(AMediaCodec_releaseOutputBuffer, status);
            }
            else if (bufferIndex == AMEDIACODEC_INFO_TRY_AGAIN_LATER ||
                     bufferIndex == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED)
            {
                // Skip
            }
            else if (bufferIndex == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED)
            {
                if (format)
                {
                    AMediaFormat_delete(format);
                    format = nullptr;
                }
                format = AMediaCodec_getOutputFormat(codec);
                TK_ASSERT(format);
                updateFormat(player, playerAndroid, codec, format);
            }
            else
            {
                VIDEO_API_ANDROID_ERROR(AMediaCodec_dequeueOutputBuffer, bufferIndex);
            }
        }

        static void updatePlayer(VideoBackendPlayer &player)
        {
            VideoPlayerAndroid &playerAndroid = getState(player);

            if (!playerAndroid.playing || (playerAndroid.inputEnded && playerAndroid.outputEnded))
                return;
            media_status_t status;
            ssize_t bufferIndex;

            int64_t presentationTimeUs = AMediaExtractor_getSampleTime(playerAndroid.extractor);
            int trackIndex = AMediaExtractor_getSampleTrackIndex(playerAndroid.extractor);
            if (trackIndex < 0)
            {
                if (playerAndroid.loop)
                {
                    status = AMediaExtractor_seekTo(playerAndroid.extractor, 0, AMEDIAEXTRACTOR_SEEK_CLOSEST_SYNC);
                    if (status != AMEDIA_OK)
                        VIDEO_API_ANDROID_ERROR(AMediaExtractor_seekTo, status);
                    if (playerAndroid.videoCodec)
                        AMediaCodec_flush(playerAndroid.videoCodec);
                    if (playerAndroid.audioCodec)
                        AMediaCodec_flush(playerAndroid.audioCodec);
                }
                else
                {
                    playerAndroid.inputEnded = 1;
                    playerAndroid.outputEnded = 1;
                }
            }
            else if (trackIndex == playerAndroid.videoTrackIndex || trackIndex == playerAndroid.audioTrackIndex)
            {
                AMediaCodec *codec = trackIndex == playerAndroid.videoTrackIndex
                                         ? playerAndroid.videoCodec
                                         : playerAndroid.audioCodec;

                bufferIndex = AMediaCodec_dequeueInputBuffer(codec, 2000);
                if (bufferIndex >= 0)
                {
                    size_t bufferSize = 0;
                    uint8_t *buffer = AMediaCodec_getInputBuffer(codec, bufferIndex, &bufferSize);
                    ssize_t sampleSize = AMediaExtractor_readSampleData(playerAndroid.extractor, buffer, bufferSize);
                    uint32_t queueInputFlags = 0;
                    if (sampleSize < 0)
                    {
                        queueInputFlags |= AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM;
                        sampleSize = 0;
                    }
                    status = AMediaCodec_queueInputBuffer(codec, bufferIndex, 0, sampleSize, presentationTimeUs, queueInputFlags);
                    if (status != AMEDIA_OK)
                        VIDEO_API_ANDROID_ERROR(AMediaCodec_queueInputBuffer, status);
                    AMediaExtractor_advance(playerAndroid.extractor);
                }
                else if (bufferIndex == AMEDIACODEC_INFO_TRY_AGAIN_LATER)
                {
                    // Skip
                }
                else
                {
                    VIDEO_API_ANDROID_ERROR(AMediaCodec_dequeueInputBuffer, bufferIndex);
                }
            }

            if (!playerAndroid.outputEnded)
            {
                readCodecOutput(player, playerAndroid, playerAndroid.videoCodec, playerAndroid.videoFormat);
                readCodecOutput(player, playerAndroid, playerAndroid.audioCodec, playerAndroid.audioFormat);
            }

            player.tick();
        }
    }

    // ----------------------------- VideoSystem -----------------------------

    const char *VideoSystem::getName()
    {
        return "Android NDK Media";
    }

    bool VideoSystem::init()
    {
        return true;
    }

    void VideoSystem::update(f32 /*deltaTime*/)
    {
        for (i32 i = 0; i < Android::g_players.count(); ++i)
        {
            VideoBackendPlayer *player = Android::g_players[i];
            if (player != nullptr)
                Android::updatePlayer(*player);
        }
    }

    void VideoSystem::dispose()
    {
    }

    // ----------------------------- VNativePlayer ---------------------------

    bool VNativePlayer::create(const VideoBackendPlayerInfo &info, VideoBackendPlayer &player)
    {
        VideoPlayerAndroid &playerAndroid = Android::getState(player);
        media_status_t status;

        playerAndroid.extractor = AMediaExtractor_new();
        if (!playerAndroid.extractor)
        {
            VIDEO_API_ANDROID_ERROR(AMediaExtractor_new, 0);
            return false;
        }

        const bool isHttp = info.url.length() >= 4 &&
                            (memcmp(info.url.get(), "http", 4) == 0 || memcmp(info.url.get(), "HTTP", 4) == 0);

        FileIStream *fileStream = nullptr;
        if (!isHttp)
            fileStream = FileIStream::open(info.url);

        if (fileStream)
        {
            auto *mediaSource = AMediaDataSource_new();
            AMediaDataSource_setUserdata(mediaSource, fileStream);
            AMediaDataSource_setReadAt(mediaSource, Android::mediaDataSourceReadAt);
            AMediaDataSource_setGetSize(mediaSource, Android::mediaDataSourceGetSize);
            AMediaDataSource_setClose(mediaSource, Android::mediaDataSourceClose);
            status = AMediaExtractor_setDataSourceCustom(playerAndroid.extractor, mediaSource);
        }
        else
        {
            status = AMediaExtractor_setDataSource(playerAndroid.extractor, info.url.get());
        }

        if (status != AMEDIA_OK)
        {
            VIDEO_API_ANDROID_ERROR(AMediaExtractor_setDataSource, status);
            AMediaExtractor_delete(playerAndroid.extractor);
            playerAndroid.extractor = nullptr;
            return false;
        }

        playerAndroid.videoTrackIndex = 0xFFFF;
        playerAndroid.audioTrackIndex = 0xFFFF;
        player.frameRate = 24.0f;
        const size_t tracks = AMediaExtractor_getTrackCount(playerAndroid.extractor);
        for (size_t trackIndex = 0; trackIndex < tracks; ++trackIndex)
        {
            AMediaFormat *trackFormat = AMediaExtractor_getTrackFormat(playerAndroid.extractor, trackIndex);
            const char *mime = nullptr;
            if (AMediaFormat_getString(trackFormat, AMEDIAFORMAT_KEY_MIME, &mime))
            {
                if (playerAndroid.videoCodec == nullptr && strncmp(mime, "video/", 6) == 0)
                {
                    playerAndroid.videoCodec = AMediaCodec_createDecoderByType(mime);
                    status = AMediaCodec_configure(playerAndroid.videoCodec, trackFormat, nullptr, nullptr, 0);
                    if (status != AMEDIA_OK)
                    {
                        VIDEO_API_ANDROID_ERROR(AMediaCodec_configure, status);
                        AMediaCodec_delete(playerAndroid.videoCodec);
                        playerAndroid.videoCodec = nullptr;
                    }
                    else if (AMediaExtractor_selectTrack(playerAndroid.extractor, trackIndex) != AMEDIA_OK)
                    {
                        VIDEO_API_ANDROID_ERROR(AMediaExtractor_selectTrack, 0);
                        AMediaCodec_delete(playerAndroid.videoCodec);
                        playerAndroid.videoCodec = nullptr;
                    }
                    else
                    {
                        playerAndroid.videoTrackIndex = (u16)trackIndex;
                        Android::updateFormat(player, playerAndroid, playerAndroid.videoCodec, trackFormat);
                    }
                }
                else if (playerAndroid.audioCodec == nullptr && strncmp(mime, "audio/", 6) == 0)
                {
                    playerAndroid.audioCodec = AMediaCodec_createDecoderByType(mime);
                    status = AMediaCodec_configure(playerAndroid.audioCodec, trackFormat, nullptr, nullptr, 0);
                    if (status != AMEDIA_OK)
                    {
                        VIDEO_API_ANDROID_ERROR(AMediaCodec_configure, status);
                        AMediaCodec_delete(playerAndroid.audioCodec);
                        playerAndroid.audioCodec = nullptr;
                    }
                    else if (AMediaExtractor_selectTrack(playerAndroid.extractor, trackIndex) != AMEDIA_OK)
                    {
                        VIDEO_API_ANDROID_ERROR(AMediaExtractor_selectTrack, 0);
                        AMediaCodec_delete(playerAndroid.audioCodec);
                        playerAndroid.audioCodec = nullptr;
                    }
                    else
                    {
                        playerAndroid.audioTrackIndex = (u16)trackIndex;
                        Android::updateFormat(player, playerAndroid, playerAndroid.audioCodec, trackFormat);
                    }
                }
            }
            AMediaFormat_delete(trackFormat);
        }

        playerAndroid.loop = info.loop ? 1 : 0;
        Android::g_players.add(&player);
        return true;
    }

    void VNativePlayer::destroy(VideoBackendPlayer &player)
    {
        player.releaseResources();
        if (player.backendData)
        {
            VideoPlayerAndroid *playerAndroid = static_cast<VideoPlayerAndroid *>(player.backendData);
            if (playerAndroid->videoFormat)
                AMediaFormat_delete(playerAndroid->videoFormat);
            if (playerAndroid->videoCodec)
                AMediaCodec_delete(playerAndroid->videoCodec);
            if (playerAndroid->audioFormat)
                AMediaFormat_delete(playerAndroid->audioFormat);
            if (playerAndroid->audioCodec)
                AMediaCodec_delete(playerAndroid->audioCodec);
            if (playerAndroid->extractor)
                AMediaExtractor_delete(playerAndroid->extractor);
            delete playerAndroid;
            player.backendData = nullptr;
        }
        Android::g_players.remove(&player);
        player.isInitialized = false;
    }

    void VNativePlayer::updateInfo(VideoBackendPlayer &player, const VideoBackendPlayerInfo &info)
    {
        VideoPlayerAndroid &playerAndroid = Android::getState(player);
        playerAndroid.loop = info.loop ? 1 : 0;
        player.updated(info);
    }

    void VNativePlayer::play(VideoBackendPlayer &player)
    {
        VideoPlayerAndroid &playerAndroid = Android::getState(player);
        playerAndroid.playing = 1;
        playerAndroid.inputEnded = 0;
        playerAndroid.outputEnded = 0;
        if (playerAndroid.videoCodec)
            AMediaCodec_start(playerAndroid.videoCodec);
        if (playerAndroid.audioCodec)
            AMediaCodec_start(playerAndroid.audioCodec);
        player.playAudio();
    }

    void VNativePlayer::pause(VideoBackendPlayer &player)
    {
        VideoPlayerAndroid &playerAndroid = Android::getState(player);
        playerAndroid.playing = 0;
        if (playerAndroid.videoCodec)
            AMediaCodec_stop(playerAndroid.videoCodec);
        if (playerAndroid.audioCodec)
            AMediaCodec_stop(playerAndroid.audioCodec);
        player.pauseAudio();
    }

    void VNativePlayer::stop(VideoBackendPlayer &player)
    {
        VideoPlayerAndroid &playerAndroid = Android::getState(player);
        player.videoFrameDuration = TkTimeSpan {};
        player.audioBufferDuration = TkTimeSpan {};
        playerAndroid.playing = 0;
        playerAndroid.inputEnded = 0;
        playerAndroid.outputEnded = 0;
        media_status_t status = AMediaExtractor_seekTo(playerAndroid.extractor, 0, AMEDIAEXTRACTOR_SEEK_CLOSEST_SYNC);
        if (status != AMEDIA_OK)
            VIDEO_API_ANDROID_ERROR(AMediaExtractor_seekTo, status);
        if (playerAndroid.videoCodec)
        {
            AMediaCodec_stop(playerAndroid.videoCodec);
            AMediaCodec_flush(playerAndroid.videoCodec);
        }
        if (playerAndroid.audioCodec)
        {
            AMediaCodec_stop(playerAndroid.audioCodec);
            AMediaCodec_flush(playerAndroid.audioCodec);
        }
        player.stopAudio();
    }

    void VNativePlayer::seek(VideoBackendPlayer &player, TkTimeSpan time)
    {
        VideoPlayerAndroid &playerAndroid = Android::getState(player);
        player.videoFrameDuration = TkTimeSpan {};
        player.audioBufferDuration = TkTimeSpan {};
        media_status_t status = AMediaExtractor_seekTo(playerAndroid.extractor, time.ticks / 10, AMEDIAEXTRACTOR_SEEK_PREVIOUS_SYNC);
        if (status != AMEDIA_OK)
            VIDEO_API_ANDROID_ERROR(AMediaExtractor_seekTo, status);
        if (playerAndroid.videoCodec)
            AMediaCodec_flush(playerAndroid.videoCodec);
        if (playerAndroid.audioCodec)
            AMediaCodec_flush(playerAndroid.audioCodec);
        player.stopAudio();
    }

    TkTimeSpan VNativePlayer::getTime(const VideoBackendPlayer &player)
    {
        if (player.backendData == nullptr)
            return TkTimeSpan {};
        const VideoPlayerAndroid *playerAndroid = static_cast<const VideoPlayerAndroid *>(player.backendData);
        if (playerAndroid->extractor == nullptr)
            return TkTimeSpan {};
        const int64_t timeUs = AMediaExtractor_getSampleTime(playerAndroid->extractor);
        if (timeUs < 0)
            return TkTimeSpan {};
        return TkTimeSpan {(i64)timeUs * 10};
    }
}

#endif
