/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "toolkit/video/common.hpp"

#if VIDEO_API_AV

#include "toolkit/video/VideoBackend.hpp"
#include "toolkit/collection/Array.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/graphics/utils/PixelFormat.hpp"
#include "toolkit/types/StringView.hpp"

#import <AVFoundation/AVFoundation.h>

#define VIDEO_API_AV_ERROR(api, err) LogWarn("[VideoBackendAV] %s failed with error 0x%llx", #api, (u64)(err))

namespace toolkit
{
    struct VideoPlayerAV
    {
        AVPlayer *player = nullptr;
        AVPlayerItemVideoOutput *output = nullptr;
        i8 pendingPlay : 1;
        i8 pendingPause : 1;
        i8 pendingSeek : 1;
        TkTimeSpan seekTime;
    };

    namespace AV
    {
        static TkArray<VideoBackendPlayer *> g_players;

        static VideoPlayerAV &getState(VideoBackendPlayer &player)
        {
            if (player.backendData == nullptr)
                player.backendData = new VideoPlayerAV {};
            return *static_cast<VideoPlayerAV *>(player.backendData);
        }

        static TkTimeSpan convertTime(const CMTime &t)
        {
            return TkTimeSpan::fromSeconds(t.timescale != 0 ? (t.value / (double)t.timescale) : 0.0);
        }

        static CMTime convertTime(const TkTimeSpan &t)
        {
            return CMTime {(CMTimeValue)(100000.0 * t.getTotalSeconds()), (CMTimeScale)100000, kCMTimeFlags_Valid, {}};
        }

        // Wraps an info.url (UTF-8 c_str) as NSString.
        static NSString *toNSString(const TkStringView &url)
        {
            return [[NSString alloc] initWithBytes:url.get()
                                            length:(NSUInteger)url.length()
                                          encoding:NSUTF8StringEncoding];
        }

        static void updatePlayer(VideoBackendPlayer &player)
        {
            VideoPlayerAV &playerAV = getState(player);

            AVPlayerItem *playerItem = [playerAV.player currentItem];
            if (!playerItem)
                return;
            if (player.width == 0)
            {
                CGSize size = [playerItem presentationSize];
                player.width = player.videoFrameWidth = (i32)size.width;
                player.height = player.videoFrameHeight = (i32)size.height;
                NSArray *tracks = [playerItem tracks];
                for (NSUInteger i = 0; i < [tracks count]; i++)
                {
                    AVPlayerItemTrack *track = (AVPlayerItemTrack *)[tracks objectAtIndex:i];
                    AVAssetTrack *assetTrack = track.assetTrack;
                    NSString *mediaType = assetTrack.mediaType;
                    if ([mediaType isEqualToString:AVMediaTypeVideo] && playerAV.output == nullptr)
                    {
                        player.frameRate = assetTrack.nominalFrameRate;
                        if (player.frameRate <= 0.0f)
                        {
                            CMTime frameDuration = assetTrack.minFrameDuration;
                            if ((frameDuration.flags & kCMTimeFlags_Valid) != 0 && frameDuration.value > 0)
                                player.frameRate = (f32)frameDuration.timescale / (f32)frameDuration.value;
                            else
                                player.frameRate = 25.0f;
                        }
                        CGSize frameSize = assetTrack.naturalSize;
                        player.width = player.videoFrameWidth = (i32)frameSize.width;
                        player.height = player.videoFrameHeight = (i32)frameSize.height;

                        i32 pixelFormat = kCVPixelFormatType_32BGRA; // TODO: use packed video format
                        player.format = PixelFormat::kB8G8R8A8_UNorm;

                        NSMutableDictionary *attributes = [NSMutableDictionary dictionary];
                        [attributes setObject:[NSNumber numberWithInt:pixelFormat]
                                       forKey:(NSString *)kCVPixelBufferPixelFormatTypeKey];
                        [attributes setObject:[NSNumber numberWithInteger:1]
                                       forKey:(NSString *)kCVPixelBufferBytesPerRowAlignmentKey];

                        playerAV.output = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:attributes];
                        [playerItem addOutput:playerAV.output];
                    }
                    else if ([mediaType isEqualToString:AVMediaTypeAudio])
                    {
                        CMFormatDescriptionRef desc = (CMFormatDescriptionRef)[assetTrack.formatDescriptions objectAtIndex:0];
                        const AudioStreamBasicDescription *audioDesc = CMAudioFormatDescriptionGetStreamBasicDescription(desc);
                        player.audioInfo.sampleRate = (u32)audioDesc->mSampleRate;
                        player.audioInfo.numChannels = audioDesc->mChannelsPerFrame;
                        player.audioInfo.bitDepth = audioDesc->mBitsPerChannel > 0 ? audioDesc->mBitsPerChannel : 16;
                    }
                }
            }

            if (playerAV.output == nullptr)
                return;

            if (playerAV.pendingPlay)
            {
                playerAV.pendingPlay = 0;
                [playerAV.player play];
            }
            else if (playerAV.pendingPause)
            {
                playerAV.pendingPause = 0;
                [playerAV.player pause];
            }
            if (playerAV.pendingSeek)
            {
                playerAV.pendingSeek = 0;
                [playerAV.player seekToTime:convertTime(playerAV.seekTime)];
            }

            CMTime currentTime = [playerAV.player currentTime];
            if (playerAV.output && [playerAV.output hasNewPixelBufferForItemTime:currentTime])
            {
                CVPixelBufferRef buffer = [playerAV.output copyPixelBufferForItemTime:currentTime itemTimeForDisplay:nullptr];
                if (buffer)
                {
                    const i32 bufferHeight = (i32)CVPixelBufferGetHeight(buffer);
                    const i32 bufferStride = (i32)CVPixelBufferGetBytesPerRow(buffer);
                    const i32 bufferSize = bufferStride * bufferHeight;

                    if (CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly) == kCVReturnSuccess)
                    {
                        byte *bufferData = (byte *)CVPixelBufferGetBaseAddress(buffer);
                        player.updateVideoFrame(TkSpan<byte> {bufferData, bufferSize}, convertTime(currentTime),
                                                TkTimeSpan::fromSeconds(player.frameRate > 0.0f ? 1.0 / player.frameRate : 0.0));
                        CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
                    }
                    CVPixelBufferRelease(buffer);
                }
            }

            player.tick();
        }
    }

    // ----------------------------- VideoSystem -----------------------------

    const char *VideoSystem::getName()
    {
        return "AVFoundation";
    }

    bool VideoSystem::init()
    {
        return true;
    }

    void VideoSystem::update(f32 /*deltaTime*/)
    {
        for (i32 i = 0; i < AV::g_players.count(); ++i)
        {
            VideoBackendPlayer *player = AV::g_players[i];
            if (player != nullptr)
                AV::updatePlayer(*player);
        }
    }

    void VideoSystem::dispose()
    {
    }

    // ----------------------------- VNativePlayer ---------------------------

    bool VNativePlayer::create(const VideoBackendPlayerInfo &info, VideoBackendPlayer &player)
    {
        VideoPlayerAV &playerAV = AV::getState(player);

        NSString *urlString = AV::toNSString(info.url);
        NSURL *url = nullptr;
        if (info.url.length() >= 4 &&
            (memcmp(info.url.get(), "http", 4) == 0 || memcmp(info.url.get(), "HTTP", 4) == 0))
        {
            url = [NSURL URLWithString:urlString];
        }
        else
        {
            url = [NSURL fileURLWithPath:urlString isDirectory:NO];
        }
        [urlString release];

        playerAV.player = [AVPlayer playerWithURL:url];
        if (playerAV.player == nullptr)
            return false;
        [playerAV.player retain];

        playerAV.player.actionAtItemEnd = info.loop ? AVPlayerActionAtItemEndNone : AVPlayerActionAtItemEndPause;
        [playerAV.player setVolume:info.volume];

        AV::g_players.add(&player);
        return true;
    }

    void VNativePlayer::destroy(VideoBackendPlayer &player)
    {
        player.releaseResources();
        if (player.backendData)
        {
            VideoPlayerAV *playerAV = static_cast<VideoPlayerAV *>(player.backendData);
            if (playerAV->pendingPause && playerAV->player)
                [playerAV->player pause];
            if (playerAV->output)
                [playerAV->output release];
            if (playerAV->player)
                [playerAV->player release];
            delete playerAV;
            player.backendData = nullptr;
        }
        AV::g_players.remove(&player);
        player.isInitialized = false;
    }

    void VNativePlayer::updateInfo(VideoBackendPlayer &player, const VideoBackendPlayerInfo &info)
    {
        VideoPlayerAV &playerAV = AV::getState(player);
        playerAV.player.actionAtItemEnd = info.loop ? AVPlayerActionAtItemEndNone : AVPlayerActionAtItemEndPause;
        // TODO: spatial audio routing.
        // TODO: audio pan routing.
        [playerAV.player setVolume:info.volume];
        player.updated(info);
    }

    void VNativePlayer::play(VideoBackendPlayer &player)
    {
        VideoPlayerAV &playerAV = AV::getState(player);
        playerAV.pendingPlay = 1;
        playerAV.pendingPause = 0;
        player.playAudio();
    }

    void VNativePlayer::pause(VideoBackendPlayer &player)
    {
        VideoPlayerAV &playerAV = AV::getState(player);
        playerAV.pendingPlay = 0;
        playerAV.pendingPause = 1;
        player.pauseAudio();
    }

    void VNativePlayer::stop(VideoBackendPlayer &player)
    {
        VideoPlayerAV &playerAV = AV::getState(player);
        playerAV.pendingPlay = 0;
        playerAV.pendingPause = 1;
        playerAV.pendingSeek = 1;
        playerAV.seekTime = TkTimeSpan {};
        player.stopAudio();
    }

    void VNativePlayer::seek(VideoBackendPlayer &player, TkTimeSpan time)
    {
        VideoPlayerAV &playerAV = AV::getState(player);
        playerAV.pendingSeek = 1;
        playerAV.seekTime = time;
    }

    TkTimeSpan VNativePlayer::getTime(const VideoBackendPlayer &player)
    {
        if (player.backendData == nullptr)
            return TkTimeSpan {};
        const VideoPlayerAV *playerAV = static_cast<const VideoPlayerAV *>(player.backendData);
        if (playerAV->pendingSeek)
            return playerAV->seekTime;
        if (playerAV->player == nullptr)
            return TkTimeSpan {};
        return AV::convertTime([playerAV->player currentTime]);
    }
}

#endif
