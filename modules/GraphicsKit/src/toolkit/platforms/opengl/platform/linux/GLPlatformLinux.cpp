/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/platforms/opengl/platform/GLPlatform.hpp>

#if GFX_API_OPENGL && TK_LINUX

#include <GL/glx.h>

#include <toolkit/debug/Logger.hpp>
#include <toolkit/platforms/linux/LinuxWindowSys.hpp>

namespace toolkit
{
    namespace
    {
        GLPlatformContext *s_deviceContext = nullptr;

        X11::Window toXWindow(void *handle)
        {
            return static_cast<X11::Window>(reinterpret_cast<uintptr>(handle));
        }

        bool chooseVisual(X11::Display *display, XVisualInfo *&outVisual)
        {
            static int attrs[] = {
                GLX_RGBA,
                GLX_DOUBLEBUFFER,
                GLX_RED_SIZE, 8,
                GLX_GREEN_SIZE, 8,
                GLX_BLUE_SIZE, 8,
                GLX_ALPHA_SIZE, 8,
                GLX_DEPTH_SIZE, 24,
                GLX_STENCIL_SIZE, 8,
                0
            };

            outVisual = glXChooseVisual(display, X11::XDefaultScreen(display), attrs);
            if (outVisual == nullptr)
                LogError("GLPlatformLinux: glXChooseVisual failed");
            return outVisual != nullptr;
        }

        bool createContextForWindow(X11::Display *display, X11::Window window, GLPlatformContext &outContext, GLPlatformContext *sharedContext, bool isDummy)
        {
            XVisualInfo *visual = nullptr;
            if (!chooseVisual(display, visual))
                return false;

            GLXContext shared = sharedContext ? static_cast<GLXContext>(sharedContext->glxContext) : nullptr;
            GLXContext context = glXCreateContext(display, visual, shared, 1);
            X11::XFree(visual);
            if (context == nullptr)
            {
                LogError("GLPlatformLinux: glXCreateContext failed");
                return false;
            }

            outContext.display = display;
            outContext.glxContext = context;
            outContext.xWindow = window;
            outContext.windowHandle = reinterpret_cast<void *>(static_cast<uintptr>(window));
            outContext.isDummy = isDummy;
            outContext.ownsWindow = isDummy;
            return true;
        }

        X11::Window createDummyWindow(X11::Display *display)
        {
            XVisualInfo *visual = nullptr;
            if (!chooseVisual(display, visual))
                return 0;

            X11::Window root = X11::XRootWindow(display, visual->screen);
            XSetWindowAttributes attrs{};
            attrs.colormap = XCreateColormap(display, root, visual->visual, AllocNone);
            attrs.event_mask = 0;

            X11::Window window = X11::XCreateWindow(
                display,
                root,
                0,
                0,
                1,
                1,
                0,
                visual->depth,
                InputOutput,
                visual->visual,
                CWColormap | CWEventMask,
                &attrs);

            XFreeColormap(display, attrs.colormap);
            X11::XFree(visual);
            return window;
        }
    }

    namespace GLPlatformLinux
    {
        bool init()
        {
            return LinuxWindowSys::display() != nullptr;
        }

        void shutdown()
        {
            s_deviceContext = nullptr;
        }

        bool createDeviceContext(GLPlatformContext &outContext)
        {
            X11::Display *display = LinuxWindowSys::display();
            if (display == nullptr)
                return false;

            X11::Window window = createDummyWindow(display);
            if (window == 0)
            {
                LogError("GLPlatformLinux: failed to create dummy window");
                return false;
            }

            if (!createContextForWindow(display, window, outContext, nullptr, true))
            {
                X11::XDestroyWindow(display, window);
                return false;
            }

            if (!s_deviceContext)
                s_deviceContext = &outContext;
            return true;
        }

        bool createWindowContext(void *windowHandle, GLPlatformContext &outContext, GLPlatformContext *sharedContext)
        {
            X11::Display *display = LinuxWindowSys::display();
            X11::Window window = toXWindow(windowHandle);
            if (display == nullptr || window == 0)
                return false;

            return createContextForWindow(display, window, outContext, sharedContext, false);
        }

        void destroyContext(GLPlatformContext &context)
        {
            X11::Display *display = static_cast<X11::Display *>(context.display);
            GLXContext glxContext = static_cast<GLXContext>(context.glxContext);
            if (display != nullptr && glxContext != nullptr)
            {
                if (glXGetCurrentContext() == glxContext)
                    glXMakeCurrent(display, 0, nullptr);
                glXDestroyContext(display, glxContext);
            }

            if (display != nullptr && context.ownsWindow && context.xWindow != 0)
                X11::XDestroyWindow(display, static_cast<X11::Window>(context.xWindow));

            if (s_deviceContext == &context)
                s_deviceContext = nullptr;

            context.display = nullptr;
            context.glxContext = nullptr;
            context.xWindow = 0;
            context.windowHandle = nullptr;
            context.ownsWindow = false;
            context.isDummy = false;
        }

        bool makeCurrent(GLPlatformContext *context)
        {
            if (context == nullptr)
            {
                X11::Display *display = LinuxWindowSys::display();
                return display != nullptr && glXMakeCurrent(display, 0, nullptr) != 0;
            }

            X11::Display *display = static_cast<X11::Display *>(context->display);
            GLXContext glxContext = static_cast<GLXContext>(context->glxContext);
            if (display == nullptr || glxContext == nullptr || context->xWindow == 0)
                return false;
            return glXMakeCurrent(display, static_cast<X11::Window>(context->xWindow), glxContext) != 0;
        }

        void clearCurrent()
        {
            makeCurrent(nullptr);
        }

        void swapBuffers(GLPlatformContext &context)
        {
            X11::Display *display = static_cast<X11::Display *>(context.display);
            if (display != nullptr && context.xWindow != 0)
                glXSwapBuffers(display, static_cast<X11::Window>(context.xWindow));
        }

        void setSwapInterval(bool vsync)
        {
            using SwapIntervalExt = void (*)(X11::Display *, GLXDrawable, int);
            using SwapIntervalMesa = int (*)(unsigned int);

            X11::Display *display = LinuxWindowSys::display();
            GLXDrawable drawable = glXGetCurrentDrawable();
            const int interval = vsync ? 1 : 0;

            auto swapExt = reinterpret_cast<SwapIntervalExt>(glXGetProcAddressARB(reinterpret_cast<const GLubyte *>("glXSwapIntervalEXT")));
            if (swapExt != nullptr && display != nullptr && drawable != 0)
            {
                swapExt(display, drawable, interval);
                return;
            }

            auto swapMesa = reinterpret_cast<SwapIntervalMesa>(glXGetProcAddressARB(reinterpret_cast<const GLubyte *>("glXSwapIntervalMESA")));
            if (swapMesa != nullptr)
                swapMesa(static_cast<unsigned int>(interval));
        }

        void updateContext(GLPlatformContext &)
        {
        }

        void *getProcAddress(const char *name)
        {
            if (name == nullptr)
                return nullptr;
            return reinterpret_cast<void *>(glXGetProcAddressARB(reinterpret_cast<const GLubyte *>(name)));
        }

        bool isCurrent(GLPlatformContext *context)
        {
            if (context == nullptr)
                return glXGetCurrentContext() == nullptr;
            return glXGetCurrentContext() == static_cast<GLXContext>(context->glxContext);
        }
    }

    bool GLPlatform::init() { return GLPlatformLinux::init(); }
    void GLPlatform::shutdown() { GLPlatformLinux::shutdown(); }
    bool GLPlatform::createDeviceContext(GLPlatformContext &c) { return GLPlatformLinux::createDeviceContext(c); }
    bool GLPlatform::createWindowContext(void *h, GLPlatformContext &c, GLPlatformContext *s) { return GLPlatformLinux::createWindowContext(h, c, s); }
    void GLPlatform::destroyContext(GLPlatformContext &c) { GLPlatformLinux::destroyContext(c); }
    bool GLPlatform::makeCurrent(GLPlatformContext *c) { return GLPlatformLinux::makeCurrent(c); }
    void GLPlatform::clearCurrent() { GLPlatformLinux::clearCurrent(); }
    void GLPlatform::swapBuffers(GLPlatformContext &c) { GLPlatformLinux::swapBuffers(c); }
    void GLPlatform::setSwapInterval(bool vsync) { GLPlatformLinux::setSwapInterval(vsync); }
    void GLPlatform::updateContext(GLPlatformContext &c) { GLPlatformLinux::updateContext(c); }
    void *GLPlatform::getProcAddress(const char *name) { return GLPlatformLinux::getProcAddress(name); }
    bool GLPlatform::isCurrent(GLPlatformContext *c) { return GLPlatformLinux::isCurrent(c); }
}

#endif
