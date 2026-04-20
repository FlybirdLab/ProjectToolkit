/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/types/String.hpp"
#include <toolkit/audio/common.hpp>

namespace toolkit
{
    // Represents a single audio device.
    class AudioDevice
    {
    public:
        explicit AudioDevice() = default;
        AudioDevice(const AudioDevice &other)
        {
            name = other.name;
            internalName = other.internalName;
        }

        AudioDevice &operator=(const AudioDevice &other)
        {
            name = other.name;
            internalName = other.internalName;
            return *this;
        }

        // The device name.
        TkString name;
        // The internal device name used by the audio backend.
        TkString internalName;
    };

}
