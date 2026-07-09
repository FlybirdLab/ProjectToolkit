/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GLPlatformMacOS.hpp"

#if GFX_API_OPENGL && TK_APPLE

#include "toolkit/debug/Logger.hpp"

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#include <dlfcn.h>

// Apple deprecated the entire OpenGL framework in macOS 10.14. We knowingly
// target 4.1 Core as a fallback backend; silence the deprecation warnings
// here rather than at CMake level so other .mm files in the tree are not
// affected.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

// Thread / lifetime contract:
//   - All NSOpenGLContext / NSOpenGLPixelFormat / NSView operations below
//     assume they are issued from the main/UI thread (the one running the
//     Cocoa run loop). Phase 7 does not support multi-threaded GL contexts.
//   - Memory management: the GraphicsKit CMake does NOT set -fobjc-arc, so
//     this translation unit runs under manual reference counting (MRR). We
//     explicitly retain on store, release on destroy, and never autorelease.

namespace toolkit
{

    namespace
    {
        // First context the backend created; future window contexts share it
        // so textures/buffers cross-bind. Matches the WGL backend behaviour.
        GLPlatformContext *s_deviceContext = nullptr;

        // dlopen handle to the OpenGL framework, cached for getProcAddress.
        void *s_oglImage = nullptr;

        NSOpenGLPixelFormat *createCorePixelFormat()
        {
            NSOpenGLPixelFormatAttribute attrs[] = {
                NSOpenGLPFAOpenGLProfile, (NSOpenGLPixelFormatAttribute)NSOpenGLProfileVersion4_1Core,
                NSOpenGLPFAAccelerated,
                NSOpenGLPFADoubleBuffer,
                NSOpenGLPFAColorSize,    (NSOpenGLPixelFormatAttribute)24,
                NSOpenGLPFAAlphaSize,    (NSOpenGLPixelFormatAttribute)8,
                NSOpenGLPFADepthSize,    (NSOpenGLPixelFormatAttribute)24,
                NSOpenGLPFAStencilSize,  (NSOpenGLPixelFormatAttribute)8,
                (NSOpenGLPixelFormatAttribute)0,
            };

            NSOpenGLPixelFormat *format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
            if (!format)
            {
                // We intentionally refuse to fall back to a legacy profile:
                // this backend is locked to 4.1 Core on macOS.
                LogError("Failed to create NSOpenGLPixelFormat for OpenGL 4.1 Core");
                return nil;
            }
            return format;
        }

        // Resolve the user's window handle into a concrete NSView*. The
        // contract from WindowKit is currently void* (HWND on Win32). The
        // macOS WindowKit layer doesn't exist yet, so we accept either
        // NSWindow* or NSView* via runtime type checking; whichever WindowKit
        // ultimately ships, this resolver keeps working.
        bool resolveViewAndWindow(void *opaqueHandle, NSView *&outView, NSWindow *&outWindow)
        {
            outView = nil;
            outWindow = nil;
            if (!opaqueHandle)
                return false;

            id obj = (__bridge id)opaqueHandle;
            if (![obj isKindOfClass:[NSObject class]])
            {
                LogError("GLPlatformMacOS: window handle is not an Objective-C object");
                return false;
            }

            if ([obj isKindOfClass:[NSWindow class]])
            {
                outWindow = (NSWindow *)obj;
                outView = [outWindow contentView];
                if (!outView)
                {
                    LogError("GLPlatformMacOS: NSWindow has no contentView");
                    return false;
                }
                return true;
            }
            if ([obj isKindOfClass:[NSView class]])
            {
                outView = (NSView *)obj;
                outWindow = [outView window]; // may be nil if view is detached; not fatal
                return true;
            }

            LogError("GLPlatformMacOS: unsupported window handle class '%s'", object_getClassName(obj));
            return false;
        }
    }

    namespace GLPlatformMacOS
    {
        bool init()
        {
            // No global state to set up; createDeviceContext is allowed to
            // be called multiple times across device retries.
            return true;
        }

        void shutdown()
        {
            s_deviceContext = nullptr;
            if (s_oglImage)
            {
                dlclose(s_oglImage);
                s_oglImage = nullptr;
            }
        }

        bool createDeviceContext(GLPlatformContext &outContext)
        {
            NSOpenGLPixelFormat *format = createCorePixelFormat();
            if (!format)
                return false;

            NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat:format
                                                                  shareContext:nil];
            if (!context)
            {
                LogError("Failed to create device-shared NSOpenGLContext (4.1 Core)");
                [format release];
                return false;
            }

            outContext.windowHandle  = nullptr;
            outContext.nsWindow      = nullptr;
            outContext.nsView        = nullptr;
            outContext.nsContext     = (void *)context;     // owning ref
            outContext.nsPixelFormat = (void *)format;      // owning ref
            outContext.isDummy       = true;
            outContext.ownsWindow    = false;

            [context makeCurrentContext];

            if (!s_deviceContext)
                s_deviceContext = &outContext;
            return true;
        }

        bool createWindowContext(void *windowHandle, GLPlatformContext &outContext, GLPlatformContext *sharedContext)
        {
            NSView *view = nil;
            NSWindow *window = nil;
            if (!resolveViewAndWindow(windowHandle, view, window))
                return false;

            NSOpenGLPixelFormat *format = createCorePixelFormat();
            if (!format)
                return false;

            NSOpenGLContext *share = sharedContext ? (NSOpenGLContext *)sharedContext->nsContext : nil;
            NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat:format
                                                                  shareContext:share];
            if (!context)
            {
                LogError("Failed to create window NSOpenGLContext (4.1 Core)");
                [format release];
                return false;
            }

            // Bind the GL context to the view's drawable. Must happen on the
            // main thread; the caller is responsible for that.
            [context setView:view];

            outContext.windowHandle  = windowHandle;
            outContext.nsWindow      = (void *)window;      // weak (not retained); user owns it
            outContext.nsView        = (void *)view;        // weak (not retained); window owns it
            outContext.nsContext     = (void *)context;     // owning ref
            outContext.nsPixelFormat = (void *)format;      // owning ref
            outContext.isDummy       = false;
            outContext.ownsWindow    = false;

            [context makeCurrentContext];
            // Default to vsync on. Callers can flip via setSwapInterval.
            GLint interval = 1;
            [context setValues:&interval forParameter:NSOpenGLCPSwapInterval];

            return true;
        }

        void destroyContext(GLPlatformContext &context)
        {
            if (context.nsContext)
            {
                NSOpenGLContext *ctx = (NSOpenGLContext *)context.nsContext;
                if ([NSOpenGLContext currentContext] == ctx)
                    [NSOpenGLContext clearCurrentContext];
                [ctx clearDrawable];
                [ctx release];
                context.nsContext = nullptr;
            }
            if (context.nsPixelFormat)
            {
                NSOpenGLPixelFormat *fmt = (NSOpenGLPixelFormat *)context.nsPixelFormat;
                [fmt release];
                context.nsPixelFormat = nullptr;
            }
            // nsWindow / nsView are weak references; we never retained them so
            // we must never release them here.
            context.nsView = nullptr;
            context.nsWindow = nullptr;
            context.windowHandle = nullptr;
            context.isDummy = false;

            if (s_deviceContext == &context)
                s_deviceContext = nullptr;
        }

        bool makeCurrent(GLPlatformContext *context)
        {
            if (!context)
            {
                [NSOpenGLContext clearCurrentContext];
                return true;
            }
            NSOpenGLContext *ctx = (NSOpenGLContext *)context->nsContext;
            if (!ctx)
                return false;
            [ctx makeCurrentContext];
            return [NSOpenGLContext currentContext] == ctx;
        }

        void clearCurrent()
        {
            [NSOpenGLContext clearCurrentContext];
        }

        void swapBuffers(GLPlatformContext &context)
        {
            NSOpenGLContext *ctx = (NSOpenGLContext *)context.nsContext;
            if (!ctx)
                return;
            [ctx flushBuffer];
        }

        void setSwapInterval(bool vsync)
        {
            GLint interval = vsync ? 1 : 0;
            NSOpenGLContext *ctx = [NSOpenGLContext currentContext];
            if (ctx)
                [ctx setValues:&interval forParameter:NSOpenGLCPSwapInterval];
        }

        void updateContext(GLPlatformContext &context)
        {
            // Mandatory on macOS after the view geometry, the backing scale,
            // or the parent window of the bound view changes. Without this
            // the NSOpenGLContext keeps drawing into the previous framebuffer
            // size and Retina displays render at half resolution.
            NSOpenGLContext *ctx = (NSOpenGLContext *)context.nsContext;
            if (ctx)
                [ctx update];
        }

        void *getProcAddress(const char *name)
        {
            if (!name)
                return nullptr;
            if (!s_oglImage)
            {
                s_oglImage = dlopen("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_LAZY | RTLD_LOCAL);
                if (!s_oglImage)
                {
                    LogError("GLPlatformMacOS::getProcAddress: dlopen(OpenGL.framework) failed: %s", dlerror());
                    return nullptr;
                }
            }
            return dlsym(s_oglImage, name);
        }

        bool isCurrent(GLPlatformContext *context)
        {
            if (!context)
                return [NSOpenGLContext currentContext] == nil;
            return [NSOpenGLContext currentContext] == (NSOpenGLContext *)context->nsContext;
        }
    }

    // ------------------------------------------------------------------
    // Static GLPlatform routes -> GLPlatformMacOS on Apple
    // ------------------------------------------------------------------

    bool GLPlatform::init() { return GLPlatformMacOS::init(); }
    void GLPlatform::shutdown() { GLPlatformMacOS::shutdown(); }
    bool GLPlatform::createDeviceContext(GLPlatformContext &c) { return GLPlatformMacOS::createDeviceContext(c); }
    bool GLPlatform::createWindowContext(void *h, GLPlatformContext &c, GLPlatformContext *s) { return GLPlatformMacOS::createWindowContext(h, c, s); }
    void GLPlatform::destroyContext(GLPlatformContext &c) { GLPlatformMacOS::destroyContext(c); }
    bool GLPlatform::makeCurrent(GLPlatformContext *c) { return GLPlatformMacOS::makeCurrent(c); }
    void GLPlatform::clearCurrent() { GLPlatformMacOS::clearCurrent(); }
    void GLPlatform::swapBuffers(GLPlatformContext &c) { GLPlatformMacOS::swapBuffers(c); }
    void GLPlatform::setSwapInterval(bool vsync) { GLPlatformMacOS::setSwapInterval(vsync); }
    void GLPlatform::updateContext(GLPlatformContext &c) { GLPlatformMacOS::updateContext(c); }
    void *GLPlatform::getProcAddress(const char *name) { return GLPlatformMacOS::getProcAddress(name); }
    bool GLPlatform::isCurrent(GLPlatformContext *c) { return GLPlatformMacOS::isCurrent(c); }

}

#pragma clang diagnostic pop

#endif
