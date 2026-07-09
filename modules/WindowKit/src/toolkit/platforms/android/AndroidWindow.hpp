/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <stdint.h>

#if TK_ANDROID

#include <toolkit/windowing/Window.hpp>

struct ANativeWindow;

namespace toolkit
{
    class NativeWindow
    {
        friend class TkWindow;

    public:
        NativeWindow(const WindowParams& p, TkWindow* window);
        ~NativeWindow();

    public:
        ANativeWindow* getANativeWindow() const;
        bool hasHandle() const;

    private:
        TkWindow* m_window = nullptr;
        f32 m_opacity = 1.0f;
    };

    ANativeWindow* getAndroidNativeWindow();
    void setAndroidNativeWindow(ANativeWindow* window);
    void notifyAndroidWindowSize(int32_t width, int32_t height);
    void setAndroidWindowFocus(bool focused);
}

extern "C" TK_API void TkAndroidSetNativeWindow(ANativeWindow* window);
extern "C" TK_API void TkAndroidNotifyWindowSize(int32_t width, int32_t height);
extern "C" TK_API void TkAndroidSetWindowFocus(int32_t focused);

#endif
