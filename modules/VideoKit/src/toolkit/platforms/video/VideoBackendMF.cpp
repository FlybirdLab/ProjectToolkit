/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/video/common.hpp>

#if VIDEO_API_MF

#include "toolkit/video/VideoBackend.hpp"
#include "toolkit/base/Math.hpp"
#include "toolkit/collection/Array.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/utils/PixelFormat.hpp"
#include "toolkit/utils/Convertor.hpp"

#include <sdkddkver.h>
#if WINVER >= _WIN32_WINNT_WINBLUE && WINVER < _WIN32_WINNT_WIN10
// Fix compilation for Windows 8.1 on the latest Windows SDK
typedef enum _MFVideoSphericalFormat
{
} MFVideoSphericalFormat;
#endif

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#define VIDEO_API_MF_ERROR(api, err) LogWarn("[VideoBackendMF] %s failed with error 0x%llx", #api, (u64)(err))

namespace toolkit
{
    struct VideoPlayerMF
    {
        IMFSourceReader *sourceReader = nullptr;
        u8 loop : 1;
        u8 playing : 1;
        u8 firstFrame : 1;
        u8 seek : 1;
        TkTimeSpan time;
    };

    namespace MF
    {
        static TkArray<VideoBackendPlayer *> g_players;

        static VideoPlayerMF &getState(VideoBackendPlayer &player)
        {
            if (player.backendData == nullptr)
                player.backendData = new VideoPlayerMF {};
            return *static_cast<VideoPlayerMF *>(player.backendData);
        }

        static bool configure(VideoBackendPlayer &player, VideoPlayerMF &playerMF, DWORD streamIndex)
        {
            IMFMediaType *mediaType = nullptr;
            IMFMediaType *nativeType = nullptr;
            bool result = true;

            HRESULT hr = playerMF.sourceReader->GetNativeMediaType(streamIndex, MF_SOURCE_READER_CURRENT_TYPE_INDEX, &nativeType);
            if (FAILED(hr))
            {
                VIDEO_API_MF_ERROR(GetNativeMediaType, hr);
                goto END;
            }
            hr = playerMF.sourceReader->GetCurrentMediaType(streamIndex, &mediaType);
            if (FAILED(hr))
            {
                VIDEO_API_MF_ERROR(GetCurrentMediaType, hr);
                goto END;
            }
            GUID majorType, subtype;
            hr = mediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
            if (FAILED(hr))
            {
                VIDEO_API_MF_ERROR(GetGUID, hr);
                goto END;
            }
            hr = mediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
            if (FAILED(hr))
            {
                VIDEO_API_MF_ERROR(GetGUID, hr);
                goto END;
            }

            if (majorType == MFMediaType_Video)
            {
                UINT32 width = 0, height = 0;
                hr = MFGetAttributeSize(mediaType, MF_MT_FRAME_SIZE, &width, &height);
                if (SUCCEEDED(hr))
                {
                    player.width = player.videoFrameWidth = (i32)width;
                    player.height = player.videoFrameHeight = (i32)height;
                }
                MFVideoArea videoArea;
                hr = mediaType->GetBlob(MF_MT_MINIMUM_DISPLAY_APERTURE, (UINT8 *)&videoArea, sizeof(MFVideoArea), nullptr);
                if (SUCCEEDED(hr) && videoArea.Area.cx > 0 && videoArea.Area.cy > 0)
                {
                    // Video frame has different in-memory size than display size (eg. 1080p video uses 1088 height due to H264 decoding).
                    player.width = videoArea.Area.cx;
                    player.height = videoArea.Area.cy;
                }
                u64 fpsValue = 0;
                hr = mediaType->GetUINT64(MF_MT_FRAME_RATE, &fpsValue);
                if (SUCCEEDED(hr))
                {
                    const u32 num = (u32)(fpsValue >> 32);
                    const u32 den = (u32)(fpsValue & 0xFFFFFFFF);
                    player.frameRate = den > 0 ? (f32)num / (f32)den : 0.0f;
                }
                if (subtype != MFVideoFormat_RGB32)
                {
                    // The default frame upload path takes a directly sampleable
                    // packed-color texture; request CPU-converted BGRA output
                    // instead of NV12/YUY2, whose shader conversion is not wired.
                    IMFMediaType *customType = nullptr;
                    hr = MFCreateMediaType(&customType);
                    if (FAILED(hr))
                    {
                        VIDEO_API_MF_ERROR(MFCreateMediaType, hr);
                        goto END;
                    }
                    customType->SetGUID(MF_MT_MAJOR_TYPE, majorType);
                    customType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
                    MFSetAttributeSize(customType, MF_MT_FRAME_SIZE, width, height);
                    hr = playerMF.sourceReader->SetCurrentMediaType(streamIndex, nullptr, customType);
                    if (FAILED(hr))
                    {
                        VIDEO_API_MF_ERROR(SetCurrentMediaType, hr);
                        customType->Release();
                        goto END;
                    }
                    customType->Release();
                }
                player.format = PixelFormat::kB8G8R8X8_UNorm;
            }
            else if (majorType == MFMediaType_Audio)
            {
                if (subtype != MFAudioFormat_PCM)
                {
                    IMFMediaType *customType = nullptr;
                    hr = MFCreateMediaType(&customType);
                    if (FAILED(hr))
                    {
                        VIDEO_API_MF_ERROR(MFCreateMediaType, hr);
                        goto END;
                    }
                    customType->SetGUID(MF_MT_MAJOR_TYPE, majorType);
                    customType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
                    hr = playerMF.sourceReader->SetCurrentMediaType(streamIndex, nullptr, customType);
                    customType->Release();
                    if (FAILED(hr))
                    {
                        VIDEO_API_MF_ERROR(SetCurrentMediaType, hr);
                        goto END;
                    }

                    mediaType->Release();
                    mediaType = nullptr;
                    hr = playerMF.sourceReader->GetCurrentMediaType(streamIndex, &mediaType);
                    if (FAILED(hr))
                    {
                        VIDEO_API_MF_ERROR(GetCurrentMediaType, hr);
                        goto END;
                    }
                }
                player.audioInfo.sampleRate = MFGetAttributeUINT32(mediaType, MF_MT_AUDIO_SAMPLES_PER_SECOND, 0);
                player.audioInfo.numChannels = MFGetAttributeUINT32(mediaType, MF_MT_AUDIO_NUM_CHANNELS, 0);
                player.audioInfo.bitDepth = MFGetAttributeUINT32(mediaType, MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
                LogInfo("[VideoBackendMF] Audio output: %u Hz, %u channels, %u-bit PCM",
                        player.audioInfo.sampleRate,
                        player.audioInfo.numChannels,
                        player.audioInfo.bitDepth);
            }

            result = false;
        END:
            if (mediaType)
                mediaType->Release();
            if (nativeType)
                nativeType->Release();
            return result;
        }

        static bool readStream(VideoBackendPlayer &player, VideoPlayerMF &playerMF, DWORD streamIndex, TkTimeSpan dt)
        {
            const bool isVideo = streamIndex == MF_SOURCE_READER_FIRST_VIDEO_STREAM;
            const bool isAudio = streamIndex == MF_SOURCE_READER_FIRST_AUDIO_STREAM;
            i32 goodSamples = 1;
            TkTimeSpan validTimeRangeStart;
            TkTimeSpan validTimeRangeEnd;

            if (isAudio)
            {
                goodSamples = player.getFreeAudioBufferCount();
                if (goodSamples <= 0)
                    return false;
            }
            else if (isVideo)
            {
                if (player.videoFrameDuration.ticks > 0 &&
                    TkMath::isInRange(playerMF.time, player.videoFrameTime, player.videoFrameTime + player.videoFrameDuration))
                {
                    return false;
                }
            }

            i32 samplesLeft = 500;
            i32 goodSamplesLeft = goodSamples;
            HRESULT hr = S_OK;
            for (; samplesLeft > 0 && goodSamplesLeft > 0; samplesLeft--)
            {
                DWORD flags = 0;
                LONGLONG samplePos = 0, sampleDuration = 0;
                IMFSample *sample = nullptr;
                hr = playerMF.sourceReader->ReadSample(streamIndex, 0, &streamIndex, &flags, &samplePos, &sample);
                if (FAILED(hr))
                {
                    VIDEO_API_MF_ERROR(ReadSample, hr);
                    break;
                }

                TkTimeSpan frameTime {(i64)samplePos};
                TkTimeSpan frameDuration = player.frameRate > 0
                                               ? TkTimeSpan::fromSeconds(1.0 / player.frameRate)
                                               : dt;
                if (sample && sample->GetSampleDuration(&sampleDuration) == S_OK && sampleDuration > 0)
                {
                    frameDuration.ticks = sampleDuration;
                }

                // The source reader is repositioned on an explicit seek. Audio
                // must remain sequential after that point; filtering it against
                // render time creates audible gaps whenever video presentation
                // falls behind the audio queue.
                bool isGoodSample = isAudio || goodSamples != goodSamplesLeft;
                if (!isAudio)
                {
                    if (validTimeRangeStart.ticks != 0)
                        isGoodSample |= TkMath::isInRange(frameTime, validTimeRangeStart, validTimeRangeEnd);
                    else
                        isGoodSample |= TkMath::isInRange(playerMF.time, frameTime, frameTime + frameDuration);
                }

                if (sample && isGoodSample)
                {
                    IMFMediaBuffer *buffer = nullptr;
                    IMF2DBuffer *buffer2D = nullptr;
                    BYTE *bufferData = nullptr;
                    LONG bufferStride = 0;
                    LONG bufferRowPitch = 0;

                    if (isVideo && player.format != PixelFormat::kNV12 &&
                        sample->GetBufferByIndex(0, &buffer) == S_OK &&
                        buffer->QueryInterface(IID_PPV_ARGS(&buffer2D)) == S_OK)
                    {
                        LONG bufferPitch = 0;
                        hr = buffer2D->Lock2D(&bufferData, &bufferPitch);
                        if (FAILED(hr))
                        {
                            VIDEO_API_MF_ERROR(Lock2D, hr);
                            goto PROCESS_SAMPLE_END;
                        }
                        bufferRowPitch = bufferPitch;
                        const LONG absolutePitch = bufferPitch < 0 ? -bufferPitch : bufferPitch;
                        bufferStride = absolutePitch * player.videoFrameHeight;
                    }
                    else
                    {
                        if (buffer)
                        {
                            buffer->Release();
                            buffer = nullptr;
                        }
                        hr = sample->ConvertToContiguousBuffer(&buffer);
                        if (FAILED(hr))
                        {
                            VIDEO_API_MF_ERROR(ConvertToContiguousBuffer, hr);
                            goto PROCESS_SAMPLE_END;
                        }
                        DWORD bufferMaxLen = 0, bufferCurrentLength = 0;
                        hr = buffer->Lock(&bufferData, &bufferMaxLen, &bufferCurrentLength);
                        if (FAILED(hr))
                        {
                            VIDEO_API_MF_ERROR(Lock, hr);
                            goto PROCESS_SAMPLE_END;
                        }
                        bufferStride = bufferCurrentLength;
                    }

                    if (isVideo)
                        player.updateVideoFrame(TkSpan<byte> {bufferData, bufferStride}, frameTime, frameDuration, bufferRowPitch);
                    else if (isAudio)
                        player.updateAudioBuffer(TkSpan<byte> {bufferData, bufferStride}, frameTime, frameDuration);

                    if (buffer2D)
                    {
                        hr = buffer2D->Unlock2D();
                        if (FAILED(hr))
                            VIDEO_API_MF_ERROR(Unlock2D, hr);
                    }
                    else if (buffer)
                    {
                        hr = buffer->Unlock();
                        if (FAILED(hr))
                            VIDEO_API_MF_ERROR(Unlock, hr);
                    }

                PROCESS_SAMPLE_END:
                    if (buffer2D)
                        buffer2D->Release();
                    if (buffer)
                        buffer->Release();
                }
                if (sample)
                    sample->Release();
                if (isGoodSample)
                {
                    goodSamplesLeft--;
                    if (!sample)
                        return true;
                }

                if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
                    break;
                if (flags & MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED || flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
                    configure(player, playerMF, streamIndex);
            }

            return samplesLeft == 0;
        }

        static void updatePlayer(VideoBackendPlayer &player, TkTimeSpan dt)
        {
            VideoPlayerMF &playerMF = getState(player);

            if (!playerMF.playing && !playerMF.seek)
                return;

            if (playerMF.firstFrame)
            {
                playerMF.firstFrame = 0;
                playerMF.seek = 1;
            }
            else if (playerMF.playing)
            {
                playerMF.time += dt;
            }

            if (player.duration.ticks > 0 && playerMF.time >= player.duration)
            {
                if (playerMF.loop)
                {
                    if (player.duration.ticks > 0)
                        playerMF.time.ticks %= player.duration.ticks;
                    else
                        playerMF.time.ticks = 0;
                    playerMF.seek = 1;
                    player.playbackEnded = false;
                    player.playAudio();
                }
                else
                {
                    playerMF.time = player.duration;
                    playerMF.playing = 0;
                    player.playbackEnded = true;
                    player.stopAudio();
                    player.tick();
                    return;
                }
            }

            bool canSeek = true;
        SEEK_START:
            if (playerMF.seek)
            {
                player.videoFrameDuration = TkTimeSpan {};
                player.audioBufferDuration = TkTimeSpan {};

                playerMF.seek = 0;
                PROPVARIANT var;
                PropVariantInit(&var);
                var.vt = VT_I8;
                var.hVal.QuadPart = playerMF.time.ticks;
                playerMF.sourceReader->SetCurrentPosition(GUID_NULL, var);
                PropVariantClear(&var);
            }

            if (readStream(player, playerMF, MF_SOURCE_READER_FIRST_VIDEO_STREAM, dt))
            {
                playerMF.seek = 1;
                if (canSeek)
                {
                    canSeek = false;
                    goto SEEK_START;
                }
            }
            if (player.audioInfo.bitDepth != 0)
                readStream(player, playerMF, MF_SOURCE_READER_FIRST_AUDIO_STREAM, dt);

            player.tick();
        }
    }

    // ----------------------------- VideoSystem -----------------------------

    const char *VideoSystem::getName()
    {
        return "Media Foundation";
    }

    bool VideoSystem::init()
    {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        {
            VIDEO_API_MF_ERROR(CoInitializeEx, hr);
            return false;
        }

        hr = MFStartup(MF_VERSION);
        if (FAILED(hr))
        {
            VIDEO_API_MF_ERROR(MFStartup, hr);
            return false;
        }
        return true;
    }

    void VideoSystem::update(f32 deltaTime)
    {
        const TkTimeSpan dt = TkTimeSpan::fromSeconds(deltaTime);
        for (i32 i = 0; i < MF::g_players.count(); ++i)
        {
            VideoBackendPlayer *player = MF::g_players[i];
            if (player != nullptr)
                MF::updatePlayer(*player, dt);
        }
    }

    void VideoSystem::dispose()
    {
        MFShutdown();
    }

    // ----------------------------- VNativePlayer ---------------------------

    bool VNativePlayer::create(const VideoBackendPlayerInfo &info, VideoBackendPlayer &player)
    {
        VideoPlayerMF &playerMF = MF::getState(player);

        IMFAttributes *attributes = nullptr;
        HRESULT hr = MFCreateAttributes(&attributes, 3);
        if (FAILED(hr))
        {
            VIDEO_API_MF_ERROR(MFCreateAttributes, hr);
            return false;
        }
        attributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, 1);
        attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, 1);
        attributes->SetUINT32(MF_LOW_LATENCY, 1);

        // MFCreateSourceReaderFromURL takes LPCWSTR — convert from UTF-8 path.
        StringAsUTF16<260> urlW {info.url.get(), info.url.length()};

        IMFSourceReader *sourceReader = nullptr;
        hr = MFCreateSourceReaderFromURL((LPCWSTR)urlW.get(), attributes, &sourceReader);
        attributes->Release();
        if (FAILED(hr) || sourceReader == nullptr)
        {
            VIDEO_API_MF_ERROR(MFCreateSourceReaderFromURL, hr);
            return false;
        }
        sourceReader->SetStreamSelection(MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);
        const bool hasAudio = sourceReader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE) == S_OK;
        if (!hasAudio)
            LogInfo("[VideoBackendMF] Media has no selectable audio stream.");
        playerMF.sourceReader = sourceReader;

        if (MF::configure(player, playerMF, MF_SOURCE_READER_FIRST_VIDEO_STREAM) ||
            (hasAudio && MF::configure(player, playerMF, MF_SOURCE_READER_FIRST_AUDIO_STREAM)))
        {
            sourceReader->Release();
            playerMF.sourceReader = nullptr;
            return false;
        }

        PROPVARIANT var;
        PropVariantInit(&var);
        hr = sourceReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var);
        if (SUCCEEDED(hr))
        {
            player.duration.ticks = var.vt == VT_UI8 ? (i64)var.uhVal.QuadPart : 0;
            PropVariantClear(&var);
        }

        playerMF.loop = info.loop ? 1 : 0;
        playerMF.firstFrame = 1;
        player.playbackEnded = false;
        MF::g_players.add(&player);
        return true;
    }

    void VNativePlayer::destroy(VideoBackendPlayer &player)
    {
        player.releaseResources();
        if (player.backendData)
        {
            VideoPlayerMF *playerMF = static_cast<VideoPlayerMF *>(player.backendData);
            if (playerMF->sourceReader)
                playerMF->sourceReader->Release();
            delete playerMF;
            player.backendData = nullptr;
        }
        MF::g_players.remove(&player);
        player.isInitialized = false;
    }

    void VNativePlayer::updateInfo(VideoBackendPlayer &player, const VideoBackendPlayerInfo &info)
    {
        VideoPlayerMF &playerMF = MF::getState(player);
        playerMF.loop = info.loop ? 1 : 0;
        player.updated(info);
    }

    void VNativePlayer::play(VideoBackendPlayer &player)
    {
        VideoPlayerMF &playerMF = MF::getState(player);
        if (player.playbackEnded)
        {
            playerMF.time = TkTimeSpan {};
            playerMF.firstFrame = 1;
            playerMF.seek = 1;
        }
        player.playbackEnded = false;
        playerMF.playing = 1;
        player.playAudio();
    }

    void VNativePlayer::pause(VideoBackendPlayer &player)
    {
        VideoPlayerMF &playerMF = MF::getState(player);
        playerMF.playing = 0;
        player.pauseAudio();
    }

    void VNativePlayer::stop(VideoBackendPlayer &player)
    {
        VideoPlayerMF &playerMF = MF::getState(player);
        playerMF.time = TkTimeSpan {};
        playerMF.playing = 0;
        playerMF.firstFrame = 1;
        playerMF.seek = 1;
        player.playbackEnded = false;
        player.stopAudio();
    }

    void VNativePlayer::seek(VideoBackendPlayer &player, TkTimeSpan time)
    {
        VideoPlayerMF &playerMF = MF::getState(player);
        if (playerMF.time != time)
        {
            playerMF.time = time;
            playerMF.seek = 1;
            player.playbackEnded = false;
            player.stopAudio();
            if (playerMF.playing)
                player.playAudio();
        }
    }

    TkTimeSpan VNativePlayer::getTime(const VideoBackendPlayer &player)
    {
        if (player.backendData == nullptr)
            return TkTimeSpan {};
        return static_cast<const VideoPlayerMF *>(player.backendData)->time;
    }
}

#endif
