/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * References:
 *    https://www.openal.org/documentation/openal-1.1-specification.pdf
 *    https://www.researchgate.net/publication/235080603_A_Novel_Multichannel_Panning_Method_for_Standard_and_Arbitrary_Loudspeaker_Configurations
 */

#include "AudioSystem.hpp"
#include "toolkit/math/Transform.hpp"

namespace toolkit
{
    SoundMix SoundMix::calSoundMix(const SoundMixParams &p, const ANativeListener &listener, const ANativeSource &source, i32 channelCount)
    {
        TK_ASSERT(channelCount <= (i32)Channels::kMaxChannels);
        SoundMix mix;
        mix.pitch = source.pitch;
        mix.volume = source.volume * p.volume;
        TkMemory::memoryClear(mix.channels, sizeof(mix.channels));
        if (source.is3D)
        {
            const Transform listenerTransform(listener.pos, listener.orientation);
            float distance = (float)Vec3::distance(listener.pos, source.pos);
            float gain = 1;

            distance = TkMath::clamp(distance, source.minDistance, kMaxF32);
            const float dst = source.minDistance + source.attenuation * (distance - source.minDistance);
            if (dst > 0)
                gain = source.minDistance / dst;
            mix.volume *= TkMath::saturate(gain);

            static const FVec3 ChannelDirections[(i32)Channels::kMaxChannels] =
                {
                    FVec3(-1.0, 0.0, -1.0).getNormalized(),
                    FVec3(1.0, 0.0, -1.0).getNormalized(),
                    FVec3(0.0, 0.0, -1.0).getNormalized(),
                    FVec3(-1.0, 0.0, 1.0).getNormalized(),
                    FVec3(1.0, 0.0, 1.0).getNormalized(),
                    FVec3(-1.0, 0.0, 0.0).getNormalized(),
                    FVec3(1.0, 0.0, 0.0).getNormalized(),
                };
            const FVec3 sourceInListenerSpace = (FVec3)listenerTransform.worldToLocal(source.pos);
            const FVec3 sourceToListener = FVec3::normalize(sourceInListenerSpace);
            float sqGainsSum = 0.0f;
            for (i32 i = 0; i < channelCount; i++)
            {
                float othersSum = 0.0f;
                for (i32 j = 0; j < channelCount; j++)
                    othersSum += (1.0f + FVec3::dot(ChannelDirections[i], ChannelDirections[j])) * 0.5f;
                const float sqGain = TkMath::square(0.5f * TkMath::pow(1.0f + FVec3::dot(ChannelDirections[i], sourceToListener), 2.0f) / othersSum);
                sqGainsSum += sqGain;
                mix.channels[i] = sqGain;
            }
            for (i32 i = 0; i < channelCount; i++)
                mix.channels[i] = TkMath::sqrt(mix.channels[i] / sqGainsSum);

            // Calculate doppler
            const FVec3 velocityInListenerSpace = (FVec3)listenerTransform.worldToLocalVector(source.velocity - listener.velocity);
            const float velocity = velocityInListenerSpace.length();
            const float dopplerFactor = p.dopplerFactor * source.dopplerFactor;
            if (dopplerFactor > 0.0f && velocity > 0.0f)
            {
                constexpr float speedOfSound = 343.3f * 100.0f * 100.0f; // in air, in Flax units
                const float approachingFactor = FVec3::dot(sourceToListener, velocityInListenerSpace.getNormalized());
                const float dopplerPitch = speedOfSound / (speedOfSound + velocity * approachingFactor);
                mix.pitch *= TkMath::clamp(dopplerPitch, 0.1f, 10.0f);
            }
        }
        else
        {
            const float panLeft = TkMath::min(1.0f - source.pan, 1.0f);
            const float panRight = TkMath::min(1.0f + source.pan, 1.0f);
            switch (channelCount)
            {
            case 1:
                mix.channels[0] = 1.0f;
                break;
            case 2:
            default: // TODO: handle other channel configuration (eg. 7.1 or 5.1)
                mix.channels[(i32)Channels::kFrontLeft] = panLeft;
                mix.channels[(i32)Channels::kFrontRight] = panRight;
                break;
            }
        }
        return mix;
    }

    void SoundMix::mapChannels(i32 sourceChannels, i32 outputChannels, f32 channels[(i32)Channels::kMaxChannels], f32 *outputMatrix)
    {
        TkMemory::memoryClear(outputMatrix, sizeof(float) * sourceChannels * outputChannels);
        switch (outputChannels)
        {
        case 1:
            outputMatrix[0] = channels[(i32)Channels::kFrontLeft];
            break;
        case 2:
        default: // TODO: implement multi-channel support (eg. 5.1, 7.1)
            outputMatrix[0] = channels[(i32)Channels::kFrontLeft];
            if (sourceChannels == 1)
                outputMatrix[1] = channels[(i32)Channels::kFrontRight];
            else
                outputMatrix[sourceChannels + 1] = channels[(i32)Channels::kFrontRight];
            break;
        }
    }
}