/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/audio/common.hpp>
#include <toolkit/audio/AudioDevice.hpp>

namespace toolkit
{

    // The audio service used for music and sound effects playback.
    class TK_API Audio
    {
        friend class AudioStreamingHandler;
        friend class AudioClip;

    public:
        // The audio listeners collection registered by the service.
        static TkArray<AudioListener *> Listeners;
        // The audio sources collection registered by the service.
        static TkArray<AudioSource *> Sources;
        // The all audio devices.
        static TkArray<AudioDevice> Devices;

        // Event called when audio devices collection gets changed.
        static TkAction<> onDevicesChanged;
        // Event called when the active audio device gets changed.
        static TkAction<> onActiveDeviceChanged;

    public:
        // Initializes audio runtime and backend.
        // Returns true on success, false on failure.
        static bool init();
        // Updates runtime state and backend.
        // Use this once per frame. Delta time is computed internally.
        static void update();
        // Updates runtime state and backend with external frame delta time (seconds).
        // Use this overload if your runtime already has a stable frame delta.
        static void update(f32 deltaTime);
        // Releases runtime and backend resources.
        static void dispose();
        // Notifies audio runtime about app focus state (used by mute-on-focus-loss).
        // Should be called by the host app/window layer on focus change.
        static void setAppHasFocus(bool hasFocus);
        // Pauses/unpauses audio output while keeping playback state.
        // Should be called by the host app when simulation is paused/resumed.
        static void setPaused(bool paused);
        // Gets whether runtime is initialized.
        static bool isInitialized();

    public:
        // Gets the active device.
        static AudioDevice *getActiveDevice();
        // Gets the index of the active device.
        static i32 getActiveDeviceIdx();
        // Sets the index of the active device.
        static void setActiveDeviceIdx(i32 index);

    public:
        // Gets the master volume applied to all the audio sources (normalized to range 0-1).
        static float getMasterVolume();
        // Sets the master volume applied to all the audio sources (normalized to range 0-1).
        static void setMasterVolume(float value);
        // Gets the actual master volume (including all side effects and mute effectors).
        static float getVolume();
        // Gets whether audio should mute when app loses focus.
        static bool getMuteOnFocusLoss();
        // Sets whether audio should mute when app loses focus.
        // Applies immediately to the current runtime volume.
        static void setMuteOnFocusLoss(bool value);
        // Sets the doppler effect factor. Scale for source and listener velocities. Default is 1.
        static void setDopplerFactor(float value);
        // Gets the preference to use HRTF audio (when available on platform). Default is true.
        static bool getEnableHRTF();
        // Sets the preference to use HRTF audio (when available on platform). Default is true.
        // If runtime is initialized this may trigger listener backend reinitialization.
        static void setEnableHRTF(bool value);
    };

}
