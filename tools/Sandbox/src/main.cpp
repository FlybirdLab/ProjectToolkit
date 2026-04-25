/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "toolkit/types/StringView.hpp"
#include <toolkit/audio/Audio.hpp>
#include <toolkit/audio/AudioClip.hpp>
#include <toolkit/audio/AudioDecoder.hpp>
#include <toolkit/audio/AudioSource.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/io/FileStream.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/threading/Thread.hpp>
#include <toolkit/windowing/Window.hpp>
#include <toolkit/windowing/WindowSystem.hpp>
#include <toolkit/types/Span.hpp>

#if TK_WINDOWS
#include <Windows.h>
#endif

using namespace toolkit;

namespace
{
    bool buildClipFromMp3(const TkStringView &path, AudioClip &clip)
    {
        FileIStream *stream = FileIStream::open(path);
        if (!stream)
        {
            LogError("Failed to open audio sample: %s", path.getText());
            return false;
        }

        AudioDecoder *decoder = AudioDecoder::createDecoder(AudioCodecs::kMp3);
        if (!decoder)
        {
            LogError("Failed to create MP3 decoder.");
            TkMemory::deletePtr(stream);
            return false;
        }

        AudioDataInfo info{};
        TkArray<byte> pcm;

        const bool decoded = decoder->convert(stream, info, pcm);

        AudioDecoder::destroyDecoder(decoder);
        TkMemory::deletePtr(stream);

        if (!decoded)
        {
            LogError("Failed to decode audio sample: %s", path.getText());
            return false;
        }

        AudioClip::Descriptor descriptor;
        descriptor.info = info;
        descriptor.is3D = false;
        descriptor.streamable = false;
        clip.setDescriptor(descriptor);
        

        if (clip.setChunkData(0, TkSpan<const byte>(pcm.get(), pcm.count())))
        {
            LogError("Failed to upload decoded PCM data.");
            pcm.clear();
            return false;
        }

        pcm.clear();

        if (clip.load())
        {
            LogError("Failed to load audio clip.");
            return false;
        }

        return true;
    }
}

int main(int, char **)
{
    System::init();
    System::beforeRun();

#if TK_WINDOWS
    WindowSystem::init(GetModuleHandleW(nullptr));
#else
    WindowSystem::init(nullptr);
#endif

    WindowParams params;
    params.title = "Toolkit Sandbox";
    params.size = Pos(960.0f, 540.0f);
    params.startPos = WindowStartPos::kCenterScreen;

    TkWindow *window = WindowSystem::createWindow(params);
    if (!window)
    {
        LogError("Failed to create sandbox window.");
        WindowSystem::dispose();
        System::beforeQuit();
        System::quit();
        return 1;
    }

    if (!Audio::init())
    {
        LogError("Failed to initialize audio runtime.");
        WindowSystem::dispose();
        System::beforeQuit();
        System::quit();
        return 1;
    }

    Audio::setMuteOnFocusLoss(false);
    Audio::setAppHasFocus(true);
    Audio::setPaused(false);
    Audio::setMasterVolume(1.0f);

    bool isRunning = true;
    window->onClosed.bind([&]()
                          { isRunning = false; });
    window->show();

    {
        AudioClip clip;
        const TkString mp3Path = TkString(SANDBOX_AUDIO_DOC_DIR) / "alive.mp3";
        if (!buildClipFromMp3(mp3Path, clip))
        {
            Audio::dispose();
            WindowSystem::dispose();
            System::beforeQuit();
            System::quit();
            return 1;
        }

        AudioSource source;
        source.setClip(&clip);
        source.setAllowSpatialization(false);
        source.setVolume(1.0f);
        source.setPitch(1.0f);
        source.play();

        while (isRunning && !window->isClosed())
        {
            WindowSystem::update();
            Audio::update();
            TkThread::sleep(5);
        }

        source.stop();
        source.setClip(nullptr);
    }

    Audio::dispose();
    WindowSystem::dispose();
    System::beforeQuit();
    System::quit();
    return 0;
}
