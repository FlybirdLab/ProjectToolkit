/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include <toolkit/audio/common.hpp>

namespace toolkit
{
    f32 AudioDataInfo::length() const
    {
        return GetLength();
    }

    f32 AudioDataInfo::GetLength() const
    {
        if (sampleRate == 0 || numChannels == 0)
            return 0.0f;
        return static_cast<f32>(numSamples) / static_cast<f32>(sampleRate * numChannels);
    }
}
