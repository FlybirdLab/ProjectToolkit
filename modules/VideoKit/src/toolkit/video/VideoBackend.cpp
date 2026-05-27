/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VideoBackend.hpp"

#include "toolkit/audio/Audio.hpp"
#include "toolkit/audio/AudioSystem.hpp"
#include "toolkit/base/Math.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/GfxBuffer.hpp"
#include "toolkit/graphics/GfxContext.hpp"
#include "toolkit/graphics/GfxDevice.hpp"
#include "toolkit/graphics/GfxTexture.hpp"
#include "toolkit/graphics/async/GfxTask.hpp"
#include "toolkit/graphics/async/GfxTaskContext.hpp"
#include "toolkit/graphics/utils/GfxUtils.hpp"
#include "toolkit/graphics/utils/PixelFormat.hpp"

namespace toolkit
{
    // GPU video frame upload task. Mirrors the original engine's GPUUploadVideoFrameTask,
    // but uses GfxTask instead of GPUTask. Raw RGBA/BGRA formats are uploaded directly;
    // packed video formats (YUY2/NV12) currently require a shader-based decoder which is
    // left as a backend extension point.
    class VFrameUploadTask : public GfxTask
    {
    public:
        explicit VFrameUploadTask(VideoBackendPlayer *player)
            : GfxTask {GfxTask::Type::kUploadTexture, 0}, m_player {player}
        {
        }

    protected:
        Result run(GfxTaskContext *context) override
        {
            if (m_player == nullptr || m_player->videoFrameMemory.isInvalid())
                return Result::kMissingResources;

            GfxTexture *frame = m_player->frame;
            if (frame == nullptr || !frame->isAllocated())
                return Result::kMissingResources;

            if (GfxPixelFormat::isVideo(m_player->format))
            {
                // Packed video formats (YUY2/NV12) need shader-based decoding to RGBA.
                // The shader pipeline (decode YUY2/NV12 PSO + quad shader CB) is backend-specific
                // and is not part of the toolkit core, so we leave it to the backend implementation
                // to override this path (or to convert to RGBA on the CPU before calling).
                LogWarn("VideoKit: packed video format upload is not implemented in the default backend");
                return Result::kFailed;
            }

            if (frame->format() == m_player->format)
            {
                u32 rowPitch = 0, slicePitch = 0;
                frame->computePitch(0, rowPitch, slicePitch);
                context->context->updateTexture(frame, 0, 0, m_player->videoFrameMemory.get(), rowPitch, slicePitch);
            }
            else
            {
                LogWarn("VideoKit: video frame format mismatch (data: %d, texture: %d)", (i32)m_player->format, (i32)frame->format());
                return Result::kFailed;
            }

            m_player->framesCount++;
            return Result::kOk;
        }

        void onSync() override
        {
            if (m_player && m_player->uploadVideoFrameTask == this)
                m_player->uploadVideoFrameTask = nullptr;
            m_player = nullptr;
        }

    private:
        VideoBackendPlayer *m_player;
    };

    void VideoBackendPlayer::created(const VideoBackendPlayerInfo &info)
    {
        updated(info);
    }

    void VideoBackendPlayer::updated(const VideoBackendPlayerInfo &info)
    {
        isAudioSpatial = info.spatial;
        audioVolume = info.volume;
        audioPan = info.pan;
        audioMinDistance = info.minDistance;
        audioAttenuation = info.attenuation;
        hasTransform = info.hasTransform;
        position = info.position;
        orientation = info.orientation;
        if (audioSource != 0)
        {
            ANativeSource::volumeChanged(audioSource, audioVolume);
            ANativeSource::panChanged(audioSource, audioPan);
            ANativeSource::spatialSetupChanged(audioSource, isAudioSpatial, audioAttenuation, audioMinDistance, 1.0f);
            if (isAudioSpatial && hasTransform)
                ANativeSource::transformChanged(audioSource, position, orientation);
        }
    }

    void VideoBackendPlayer::playAudio()
    {
        isAudioPlayPending = 1;
        if (audioSource != 0)
        {
            i32 queuedBuffers = 0;
            ANativeSource::getQueuedBuffersCount(audioSource, queuedBuffers);
            if (queuedBuffers > 0)
            {
                ANativeSource::play(audioSource);
                isAudioPlayPending = 0;
            }
        }
    }

    void VideoBackendPlayer::pauseAudio()
    {
        isAudioPlayPending = 0;
        if (audioSource != 0)
            ANativeSource::pause(audioSource);
    }

    void VideoBackendPlayer::stopAudio()
    {
        isAudioPlayPending = 0;
        if (audioSource != 0)
        {
            ANativeSource::stop(audioSource);
            ANativeSource::clearQueuedBuffers(audioSource);
        }
        nextAudioBuffer = 0;
    }

    void VideoBackendPlayer::initVideoFrame()
    {
        GfxDevice *device = GfxDevice::getInstance();
        if (device == nullptr)
            return;
        if (frame == nullptr)
            frame = device->createTexture(TkStringView {"VideoFrame", 10});
    }

    void VideoBackendPlayer::updateVideoFrame(TkSpan<byte> data, TkTimeSpan time, TkTimeSpan duration, i32 dataRowPitch)
    {
        videoFrameTime = time;
        videoFrameDuration = duration;

        GfxDevice *device = GfxDevice::getInstance();
        if (device == nullptr)
            return;

        u32 sourceRowPitch = 0, sourceSlicePitch = 0;
        GfxUtils::computePitch(format, videoFrameWidth, videoFrameHeight, sourceRowPitch, sourceSlicePitch);
        const i32 sourceStride = dataRowPitch != 0 ? dataRowPitch : (i32)sourceRowPitch;
        const i32 sourceStrideAbs = sourceStride < 0 ? -sourceStride : sourceStride;
        const i32 requiredSourceBytes = dataRowPitch != 0
                                            ? sourceStrideAbs * videoFrameHeight
                                            : (i32)sourceSlicePitch;
        if (sourceStrideAbs < (i32)sourceRowPitch || data.length() < requiredSourceBytes)
        {
            LogWarn("VideoKit: incorrect video frame buffer %d bytes with row pitch %d, expected at least %d bytes for %dx%d format %d",
                    data.length(), sourceStrideAbs, requiredSourceBytes, videoFrameWidth, videoFrameHeight, (i32)format);
            return;
        }

        u32 rowPitch = 0, slicePitch = 0;
        GfxUtils::computePitch(format, width, height, rowPitch, slicePitch);
        if (sourceStrideAbs < (i32)rowPitch || videoFrameHeight < height)
        {
            LogWarn("VideoKit: decoded video frame is smaller than display frame (%dx%d -> %dx%d)",
                    videoFrameWidth, videoFrameHeight, width, height);
            return;
        }

        initVideoFrame();
        if (frame == nullptr)
            return;

        auto desc = GfxTextureDescription::new2D(width, height, PixelFormat::kR8G8B8A8_UNorm,
                                                 GfxTextureFlags::kShaderResource);
        if (!GfxPixelFormat::isVideo(format))
            desc.format = format;
        if (frame->getDescription() != desc)
        {
            if (!frame->init(desc))
            {
                LogError("VideoKit: failed to allocate video frame texture");
                return;
            }
        }

        // The host is expected to call Video::update() inside an active frame.
        // uploadMipMapAsync uses its synchronous rendering path here and also
        // establishes the texture's first resident mip/SRV for sampling.
        if (GfxPixelFormat::isVideo(format))
        {
            LogWarn("VideoKit: packed video format %d is not implemented in the default upload path", (i32)format);
            return;
        }
        if (frame->format() != format)
            return;

        if (device->getMainContext() != nullptr)
        {
            const bool uploadSourceDirectly = sourceStride == (i32)rowPitch &&
                                              videoFrameWidth == width &&
                                              videoFrameHeight == height &&
                                              device->isRendering();
            if (uploadSourceDirectly)
            {
                if (videoFrameMemory.length() > 0)
                    videoFrameMemory.release();
                device->getMainContext()->updateTexture(frame, 0, 0, data.get(), rowPitch, slicePitch);
                if (frame->residentMipLevels() == 0)
                    frame->setResidentMipLevels(1);
            }
            else
            {
                // Strip source row padding or coded-frame padding when a direct upload is not possible.
                if (videoFrameMemory.length() < (i32)slicePitch)
                    videoFrameMemory.allocate(slicePitch);
                const byte *source = data.get();
                byte *destination = videoFrameMemory.get();
                for (i32 row = 0; row < height; row++)
                    memcpy(destination + row * rowPitch, source + row * sourceStride, rowPitch);
                frame->uploadMipMapAsync(videoFrameMemory, 0, rowPitch, slicePitch, false);
            }
            framesCount++;
        }
    }

    i32 VideoBackendPlayer::getFreeAudioBufferCount()
    {
        if (audioSource == 0)
            return lengthOf(audioBuffers);

        i32 processedBuffers = 0;
        ANativeSource::getProcessedBuffersCount(audioSource, processedBuffers);
        if (processedBuffers > 0)
            ANativeSource::dequeueProcessedBuffers(audioSource);

        i32 queuedBuffers = 0;
        ANativeSource::getQueuedBuffersCount(audioSource, queuedBuffers);
        return TkMath::max(0, (i32)lengthOf(audioBuffers) - queuedBuffers);
    }

    bool VideoBackendPlayer::updateAudioBuffer(TkSpan<byte> data, TkTimeSpan time, TkTimeSpan duration)
    {
        audioBufferTime = time;
        audioBufferDuration = duration;

        if (!Audio::isInitialized())
            return false;

        const u32 bytesPerSample = audioInfo.bitDepth / 8;
        if (data.length() <= 0 || bytesPerSample == 0 || audioInfo.sampleRate == 0 || audioInfo.numChannels == 0)
            return false;

        AudioDataInfo bufferInfo = audioInfo;
        bufferInfo.numSamples = (u32)data.length() / bytesPerSample;
        if (bufferInfo.numSamples == 0)
            return false;

        if (audioSource == 0)
        {
            audioSource = ANativeSource::add(
                bufferInfo,
                position,
                orientation,
                audioVolume,
                1.0f,
                audioPan,
                false,
                isAudioSpatial,
                audioAttenuation,
                audioMinDistance,
                1.0f);
            if (audioSource == 0)
            {
                LogWarn("VideoKit: failed to allocate an audio source for video playback");
                return false;
            }
        }

        if (getFreeAudioBufferCount() <= 0)
            return false;

        u32 &bufferId = audioBuffers[nextAudioBuffer];
        if (bufferId == 0)
            bufferId = ANativeBuffer::create();
        if (bufferId == 0 || !ANativeBuffer::write(bufferId, data.get(), bufferInfo))
        {
            LogWarn("VideoKit: failed to upload decoded audio buffer");
            return false;
        }

        ANativeSource::queueBuffer(audioSource, bufferId);
        nextAudioBuffer = (nextAudioBuffer + 1) % lengthOf(audioBuffers);
        if (!hasSubmittedAudioBuffer)
        {
            hasSubmittedAudioBuffer = true;
            LogInfo("VideoKit: audio playback queued (%u Hz, %u channels, %u-bit PCM, %d bytes)",
                    bufferInfo.sampleRate,
                    bufferInfo.numChannels,
                    bufferInfo.bitDepth,
                    data.length());
        }
        if (isAudioPlayPending)
            playAudio();
        return true;
    }

    void VideoBackendPlayer::tick()
    {
        if (audioSource != 0 && isAudioSpatial && hasTransform)
            ANativeSource::transformChanged(audioSource, position, orientation);
    }

    void VideoBackendPlayer::releaseResources()
    {
        if (audioSource != 0)
        {
            stopAudio();
            ANativeSource::remove(audioSource);
            audioSource = 0;
        }
        for (u32 &bufferId : audioBuffers)
        {
            if (bufferId != 0)
            {
                ANativeBuffer::destroy(bufferId);
                bufferId = 0;
            }
        }
        if (uploadVideoFrameTask != nullptr)
            uploadVideoFrameTask->cancel();
        videoFrameMemory.release();
        if (frame != nullptr)
        {
            frame->releaseData();
            TkMemory::deletePtr(TkMemory::getDefault(), frame);
            frame = nullptr;
        }
        if (frameUpload != nullptr)
        {
            frameUpload->releaseData();
            TkMemory::deletePtr(TkMemory::getDefault(), frameUpload);
            frameUpload = nullptr;
        }
    }

    // --------------------------------------------------------------------
    // Default (null) backend implementation.
    // Platform backends (Media Foundation / AVFoundation / Android NDK)
    // provide their own definitions of these symbols when enabled.
    // --------------------------------------------------------------------

#if !VIDEO_API_MF && !VIDEO_API_AV && !VIDEO_API_ANDROID

    const char *VideoSystem::getName()
    {
        return "Null";
    }

    bool VideoSystem::init()
    {
        return true;
    }

    void VideoSystem::update(f32 /*deltaTime*/)
    {
    }

    void VideoSystem::dispose()
    {
    }

    bool VNativePlayer::create(const VideoBackendPlayerInfo & /*info*/, VideoBackendPlayer & /*player*/)
    {
        // Null backend: never creates a real player.
        return false;
    }

    void VNativePlayer::destroy(VideoBackendPlayer &player)
    {
        player.releaseResources();
        player.isInitialized = false;
        player.backendData = nullptr;
    }

    void VNativePlayer::updateInfo(VideoBackendPlayer &player, const VideoBackendPlayerInfo &info)
    {
        player.updated(info);
    }

    void VNativePlayer::play(VideoBackendPlayer & /*player*/)
    {
    }

    void VNativePlayer::pause(VideoBackendPlayer & /*player*/)
    {
    }

    void VNativePlayer::stop(VideoBackendPlayer & /*player*/)
    {
    }

    void VNativePlayer::seek(VideoBackendPlayer & /*player*/, TkTimeSpan /*time*/)
    {
    }

    TkTimeSpan VNativePlayer::getTime(const VideoBackendPlayer & /*player*/)
    {
        return TkTimeSpan {};
    }

#endif
}
