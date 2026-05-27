/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "AudioSandbox.hpp"

#include <toolkit/audio/Audio.hpp>
#include <toolkit/audio/AudioSource.hpp>
#include <toolkit/audio/AudioStreamClip.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/threading/Thread.hpp>
#include <toolkit/threading/ThreadPool.hpp>
#include <toolkit/utils/CommandLine.hpp>
#include <toolkit/utils/StringUtils.hpp>

namespace toolkit
{
    namespace
    {
        constexpr f32 kPlaybackStartTimeoutSeconds = 8.0f;

        TkString getTestFilePath(const TkCommandLine &commandLine)
        {
            TkString path;
            if (commandLine.get("file", path))
                return path;
            return TkString(SANDBOX_AUDIO_TEST_FILE);
        }

        AudioCodecs getTestCodec(const TkCommandLine &commandLine, const TkString &filePath)
        {
            TkString codec;
            if (commandLine.get("codec", codec))
            {
                if (codec == "mp3")
                    return AudioCodecs::kMp3;
                if (codec == "m4a")
                    return AudioCodecs::kM4a;
                return AudioCodecs::kUnknown;
            }

            const TkStringView path(filePath.getText(), filePath.length());
            if (TkStringUtils::endsWithInsensitive(path, ".m4a") ||
                TkStringUtils::endsWithInsensitive(path, ".mp4"))
            {
                return AudioCodecs::kM4a;
            }
            return AudioCodecs::kMp3;
        }
    }

    int AudioSandbox::run(const TkCommandLine &commandLine)
    {
        const TkString filePath = getTestFilePath(commandLine);
        const AudioCodecs codec = getTestCodec(commandLine, filePath);
        if (codec == AudioCodecs::kUnknown)
        {
            LogError("[AudioSandbox] --codec must be mp3 or m4a.");
            return 1;
        }
        LogInfo("[AudioSandbox] Test file: %s", filePath.getText());

        // --duration optionally limits playback; otherwise run until end-of-stream.
        f32 maxPlaySeconds = 0.0f;
        bool hasMaxPlayDuration = false;
        {
            TkString durStr;
            if (commandLine.get("duration", durStr))
            {
                if (!TkStringUtils::fromString(TkStringView(durStr.getText(), durStr.length()), maxPlaySeconds) ||
                    maxPlaySeconds <= 0.0f)
                {
                    LogError("[AudioSandbox] --duration must be a positive number of seconds.");
                    return 1;
                }
                hasMaxPlayDuration = true;
            }
        }

        if (!Audio::init())
        {
            LogError("[AudioSandbox] Failed to initialize audio system.");
            return 1;
        }

        if (!TkThreadPool::init())
        {
            LogError("[AudioSandbox] Failed to initialize streaming worker pool.");
            Audio::dispose();
            return 1;
        }

        int result = 0;
        {
            AudioStreamClip::Descriptor desc;
            desc.codec           = codec;
            desc.filePath        = filePath;
            desc.is3D            = false;
            desc.chunkDurationMs = 1000;

            AudioStreamClip clip;
            clip.setDescriptor(desc);

            AudioSource source;
            source.setClip(&clip);
            source.setIsLooping(false);
            source.setVolume(1.0f);

            source.play();

            // Streaming playback starts after the first asynchronous decode/upload pass.
            if (!clip.isLoaded() || source.getSourceId() == 0)
            {
                LogError("[AudioSandbox] AudioSource failed to prepare playback.");
                result = 1;
            }
            else
            {
                if (hasMaxPlayDuration)
                    LogInfo("[AudioSandbox] Playing for up to %.1f seconds. Press Ctrl+C to stop early.", maxPlaySeconds);
                else
                    LogInfo("[AudioSandbox] Playing full clip. Press Ctrl+C to stop early.");

                const double startDeadline = System::getTimeSeconds() + static_cast<double>(kPlaybackStartTimeoutSeconds);
                double playbackDeadline = 0.0;
                bool playbackStarted = false;
                while (true)
                {
                    Audio::update();
                    const double currentTime = System::getTimeSeconds();

                    if (source.getState() == AudioSource::State::kPlaying)
                    {
                        if (!playbackStarted)
                        {
                            playbackStarted = true;
                            if (hasMaxPlayDuration)
                                playbackDeadline = currentTime + static_cast<double>(maxPlaySeconds);
                        }
                    }
                    else if (playbackStarted && source.getState() == AudioSource::State::kStopped)
                    {
                        LogInfo("[AudioSandbox] Playback finished.");
                        break;
                    }

                    if (!playbackStarted && currentTime >= startDeadline)
                    {
                        LogError("[AudioSandbox] Playback did not start within %.1f seconds.", kPlaybackStartTimeoutSeconds);
                        result = 1;
                        break;
                    }
                    if (hasMaxPlayDuration && playbackStarted && currentTime >= playbackDeadline)
                    {
                        LogInfo("[AudioSandbox] Playback duration limit reached.");
                        break;
                    }

                    TkThread::sleep(16); // ~60 Hz update
                }

                source.stop();
            }
        }

        // Drain stream decoding before disposing the audio backend.
        TkThreadPool::beforeExit();
        TkThreadPool::dispose();

        // Audio runtime teardown must follow the destruction of registered sources.
        Audio::dispose();

        LogInfo("[AudioSandbox] Done.");
        return result;
    }
}
