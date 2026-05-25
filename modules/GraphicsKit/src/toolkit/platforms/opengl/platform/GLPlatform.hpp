/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/platforms/opengl/common.hpp>

#if GFX_API_OPENGL

namespace toolkit
{

    // Platform-specific data attached to a GL context. Concrete fields are
    // filled in by the per-platform GLPlatform implementation. The struct is
    // intentionally narrow: callers treat it as an opaque handle and let
    // GLPlatform do the work.
    struct GLPlatformContext
    {
        void *windowHandle = nullptr; // HWND on Windows, NSWindow*/NSView* on macOS (as returned by getWindowPtr()).
        bool ownsWindow = false;

#if TK_WINDOWS
        void *hdc = nullptr;          // HDC
        void *hglrc = nullptr;        // HGLRC
        bool ownsHwnd = false;        // true for hidden dummy windows
#endif

#if TK_APPLE
        // macOS NSOpenGL state. Pointers are opaque to keep this header free of
        // Cocoa/AppKit includes; concrete types are cast inside GLPlatformMacOS.mm.
        //   nsWindow     -> NSWindow*  (may be nil when caller passed an NSView*)
        //   nsView       -> NSView*    (the drawable; set via [ctx setView:])
        //   nsContext    -> NSOpenGLContext*
        //   nsPixelFormat-> NSOpenGLPixelFormat*
        void *nsWindow = nullptr;
        void *nsView = nullptr;
        void *nsContext = nullptr;
        void *nsPixelFormat = nullptr;
        bool isDummy = false;         // true for hidden device-shared contexts
#endif
    };

    // Platform abstraction layer for OpenGL context lifecycle and presentation.
    // Implemented per-platform under platforms/opengl/platform/<os>/.
    //
    // Threading: every GLPlatform method assumes it is called on the same
    // thread that owns the GL context. On macOS specifically, NSOpenGL context
    // creation and view binding must happen on the main/UI thread (the one
    // running the Cocoa run loop). This backend does not implement multi-
    // threaded GL contexts in Phase 7.
    class GLPlatform
    {
    public:
        // One-shot, idempotent. Loads core GL functions via glad once a
        // current context is available.
        static bool init();
        static void shutdown();

        // Create a hidden device-shared context. Used by GLDevice as the
        // "owning" context so child swapchain contexts can share with it.
        static bool createDeviceContext(GLPlatformContext &outContext);

        // Create a context bound to the user's window. `sharedContext` may be
        // null but is normally the device's context so resources are shared.
        static bool createWindowContext(void *windowHandle, GLPlatformContext &outContext, GLPlatformContext *sharedContext);

        static void destroyContext(GLPlatformContext &context);

        static bool makeCurrent(GLPlatformContext *context);
        static void clearCurrent();
        static void swapBuffers(GLPlatformContext &context);
        static void setSwapInterval(bool vsync);

        // Notify the platform layer that the drawable area of a window context
        // has changed (resize, screen change, parent-view reattachment). The
        // Win32 backend treats this as a no-op; the macOS backend forwards it
        // to `[NSOpenGLContext update]` so the framebuffer stays in sync with
        // the underlying NSView geometry / backing scale factor.
        static void updateContext(GLPlatformContext &context);

        // Used by glad as its loader function.
        static void *getProcAddress(const char *name);
        static bool isCurrent(GLPlatformContext *context);

        // Drives gladLoadGLLoader once. Safe to call multiple times.
        static bool loadGLFunctions();
    };

}

#endif
