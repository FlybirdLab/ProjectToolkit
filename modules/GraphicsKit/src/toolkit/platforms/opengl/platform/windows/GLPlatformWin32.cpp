/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GLPlatformWin32.hpp"

#if GFX_API_OPENGL && TK_WINDOWS

#include <toolkit/platforms/opengl/platform/GLPlatform.hpp>
#include "toolkit/debug/Logger.hpp"

namespace toolkit
{

    // ------------------------------------------------------------------
    // WGL extension constants and function-pointer typedefs. Declared here
    // so we never depend on whether <GL/wglext.h> ships with the SDK.
    // ------------------------------------------------------------------

    namespace
    {
        constexpr int WGL_CONTEXT_MAJOR_VERSION_ARB = 0x2091;
        constexpr int WGL_CONTEXT_MINOR_VERSION_ARB = 0x2092;
        constexpr int WGL_CONTEXT_PROFILE_MASK_ARB = 0x9126;
        constexpr int WGL_CONTEXT_CORE_PROFILE_BIT_ARB = 0x00000001;
        constexpr int WGL_CONTEXT_FLAGS_ARB = 0x2094;
        constexpr int WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB = 0x00000002;

        typedef HGLRC (WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int *);
        typedef BOOL (WINAPI *PFNWGLSWAPINTERVALEXTPROC)(int);

        // Cached ARB extension entry points.
        PFNWGLCREATECONTEXTATTRIBSARBPROC s_wglCreateContextAttribsARB = nullptr;
        PFNWGLSWAPINTERVALEXTPROC s_wglSwapIntervalEXT = nullptr;

        // The first context the backend created. Shared as `shareList` for any
        // future window contexts so resources cross-bind.
        GLPlatformContext *s_deviceContext = nullptr;

        const wchar_t *kDummyClassName = L"TkGLPlatformWin32DummyWnd";

        ATOM ensureDummyWindowClass()
        {
            static ATOM s_atom = 0;
            if (s_atom != 0)
                return s_atom;

            WNDCLASSW wc = {};
            wc.style = CS_OWNDC;
            wc.lpfnWndProc = DefWindowProcW;
            wc.hInstance = GetModuleHandleW(nullptr);
            wc.lpszClassName = kDummyClassName;
            s_atom = RegisterClassW(&wc);
            return s_atom;
        }

        HWND createDummyWindow()
        {
            if (ensureDummyWindowClass() == 0)
                return nullptr;
            HWND hwnd = CreateWindowExW(
                0,
                kDummyClassName,
                L"TkGLDummy",
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT, 1, 1,
                nullptr, nullptr,
                GetModuleHandleW(nullptr),
                nullptr);
            return hwnd;
        }

        bool choosePixelFormatBasic(HDC hdc)
        {
            PIXELFORMATDESCRIPTOR pfd = {};
            pfd.nSize = sizeof(pfd);
            pfd.nVersion = 1;
            pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.cColorBits = 32;
            pfd.cDepthBits = 24;
            pfd.cStencilBits = 8;
            pfd.iLayerType = PFD_MAIN_PLANE;
            const int pf = ChoosePixelFormat(hdc, &pfd);
            if (pf == 0)
            {
                LogError("ChoosePixelFormat failed: 0x%08x", GetLastError());
                return false;
            }
            if (!SetPixelFormat(hdc, pf, &pfd))
            {
                LogError("SetPixelFormat failed: 0x%08x", GetLastError());
                return false;
            }
            return true;
        }

        // Create a 4.1 Core context on hdc using a one-shot legacy context to
        // grab the ARB extension entry point. shareList may be null.
        HGLRC createCoreContext(HDC hdc, HGLRC shareList)
        {
            if (!s_wglCreateContextAttribsARB)
            {
                // Bootstrap: spin up a legacy context just long enough to
                // resolve wglCreateContextAttribsARB.
                HGLRC legacy = wglCreateContext(hdc);
                if (!legacy)
                {
                    LogError("wglCreateContext (legacy bootstrap) failed: 0x%08x", GetLastError());
                    return nullptr;
                }
                if (!wglMakeCurrent(hdc, legacy))
                {
                    LogError("wglMakeCurrent (legacy bootstrap) failed: 0x%08x", GetLastError());
                    wglDeleteContext(legacy);
                    return nullptr;
                }

                s_wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(
                    wglGetProcAddress("wglCreateContextAttribsARB"));
                s_wglSwapIntervalEXT = reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(
                    wglGetProcAddress("wglSwapIntervalEXT"));

                wglMakeCurrent(nullptr, nullptr);
                wglDeleteContext(legacy);

                if (!s_wglCreateContextAttribsARB)
                {
                    LogError("wglCreateContextAttribsARB not available on this driver");
                    return nullptr;
                }
            }

            const int attribs[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                WGL_CONTEXT_MINOR_VERSION_ARB, 1,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                0,
            };

            HGLRC hglrc = s_wglCreateContextAttribsARB(hdc, shareList, attribs);
            if (!hglrc)
            {
                LogError("wglCreateContextAttribsARB(4.1 Core) failed: 0x%08x", GetLastError());
                return nullptr;
            }
            return hglrc;
        }
    }

    // ------------------------------------------------------------------
    // GLPlatformWin32 public surface
    // ------------------------------------------------------------------

    namespace GLPlatformWin32
    {

        bool init()
        {
            return true;
        }

        void shutdown()
        {
            s_wglCreateContextAttribsARB = nullptr;
            s_wglSwapIntervalEXT = nullptr;
            s_deviceContext = nullptr;
        }

        bool createDeviceContext(GLPlatformContext &outContext)
        {
            HWND hwnd = createDummyWindow();
            if (!hwnd)
            {
                LogError("createDummyWindow failed: 0x%08x", GetLastError());
                return false;
            }
            HDC hdc = GetDC(hwnd);
            if (!hdc)
            {
                LogError("GetDC(dummy) failed: 0x%08x", GetLastError());
                DestroyWindow(hwnd);
                return false;
            }
            if (!choosePixelFormatBasic(hdc))
            {
                ReleaseDC(hwnd, hdc);
                DestroyWindow(hwnd);
                return false;
            }
            HGLRC hglrc = createCoreContext(hdc, nullptr);
            if (!hglrc)
            {
                ReleaseDC(hwnd, hdc);
                DestroyWindow(hwnd);
                return false;
            }

            outContext.windowHandle = hwnd;
            outContext.hdc = hdc;
            outContext.hglrc = hglrc;
            outContext.ownsWindow = true;
            outContext.ownsHwnd = true;
            if (!s_deviceContext)
                s_deviceContext = &outContext;
            return true;
        }

        bool createWindowContext(HWND hwnd, GLPlatformContext &outContext, GLPlatformContext *sharedContext)
        {
            if (!hwnd)
                return false;

            HDC hdc = GetDC(hwnd);
            if (!hdc)
            {
                LogError("GetDC(window) failed: 0x%08x", GetLastError());
                return false;
            }
            if (!choosePixelFormatBasic(hdc))
            {
                ReleaseDC(hwnd, hdc);
                return false;
            }

            HGLRC share = sharedContext ? static_cast<HGLRC>(sharedContext->hglrc) : nullptr;
            HGLRC hglrc = createCoreContext(hdc, share);
            if (!hglrc)
            {
                ReleaseDC(hwnd, hdc);
                return false;
            }

            outContext.windowHandle = hwnd;
            outContext.hdc = hdc;
            outContext.hglrc = hglrc;
            outContext.ownsWindow = false;
            outContext.ownsHwnd = false;
            return true;
        }

        void destroyContext(GLPlatformContext &context)
        {
            HGLRC currentRC = wglGetCurrentContext();
            if (context.hglrc && currentRC == static_cast<HGLRC>(context.hglrc))
                wglMakeCurrent(nullptr, nullptr);

            if (context.hglrc)
            {
                wglDeleteContext(static_cast<HGLRC>(context.hglrc));
                context.hglrc = nullptr;
            }
            if (context.hdc && context.windowHandle)
            {
                ReleaseDC(static_cast<HWND>(context.windowHandle), static_cast<HDC>(context.hdc));
                context.hdc = nullptr;
            }
            if (context.ownsHwnd && context.windowHandle)
            {
                DestroyWindow(static_cast<HWND>(context.windowHandle));
                context.windowHandle = nullptr;
                context.ownsHwnd = false;
            }
            if (s_deviceContext == &context)
                s_deviceContext = nullptr;
        }

        bool makeCurrent(GLPlatformContext *context)
        {
            if (!context)
                return wglMakeCurrent(nullptr, nullptr) != FALSE;
            if (!context->hdc || !context->hglrc)
                return false;
            return wglMakeCurrent(static_cast<HDC>(context->hdc), static_cast<HGLRC>(context->hglrc)) != FALSE;
        }

        void clearCurrent()
        {
            wglMakeCurrent(nullptr, nullptr);
        }

        void swapBuffers(GLPlatformContext &context)
        {
            if (context.hdc)
                SwapBuffers(static_cast<HDC>(context.hdc));
        }

        void updateContext(GLPlatformContext &)
        {
            // WGL has no equivalent to [NSOpenGLContext update]; the driver
            // tracks the window via HDC and re-reads its size on next
            // SwapBuffers/glViewport. No-op kept here so the cross-platform
            // GLSwapChain::resize path stays branch-free.
        }

        void setSwapInterval(bool vsync)
        {
            if (s_wglSwapIntervalEXT)
                s_wglSwapIntervalEXT(vsync ? 1 : 0);
        }

        void *getProcAddress(const char *name)
        {
            void *p = reinterpret_cast<void *>(wglGetProcAddress(name));
            // wglGetProcAddress returns NULL or 0x1/0x2/0x3/-1 for OpenGL 1.0/1.1
            // entry points -- those must be resolved through opengl32.dll.
            if (p == nullptr || p == reinterpret_cast<void *>(0x1) || p == reinterpret_cast<void *>(0x2)
                || p == reinterpret_cast<void *>(0x3) || p == reinterpret_cast<void *>(-1))
            {
                static HMODULE s_opengl32 = GetModuleHandleW(L"opengl32.dll");
                if (!s_opengl32)
                    s_opengl32 = LoadLibraryW(L"opengl32.dll");
                if (s_opengl32)
                    p = reinterpret_cast<void *>(GetProcAddress(s_opengl32, name));
            }
            return p;
        }

        bool isCurrent(GLPlatformContext *context)
        {
            if (!context)
                return wglGetCurrentContext() == nullptr;
            return wglGetCurrentContext() == static_cast<HGLRC>(context->hglrc);
        }

    }

    // ------------------------------------------------------------------
    // Static GLPlatform routes -> GLPlatformWin32 on Windows
    // ------------------------------------------------------------------

    bool GLPlatform::init() { return GLPlatformWin32::init(); }
    void GLPlatform::shutdown() { GLPlatformWin32::shutdown(); }
    bool GLPlatform::createDeviceContext(GLPlatformContext &c) { return GLPlatformWin32::createDeviceContext(c); }
    bool GLPlatform::createWindowContext(void *h, GLPlatformContext &c, GLPlatformContext *s) { return GLPlatformWin32::createWindowContext(static_cast<HWND>(h), c, s); }
    void GLPlatform::destroyContext(GLPlatformContext &c) { GLPlatformWin32::destroyContext(c); }
    bool GLPlatform::makeCurrent(GLPlatformContext *c) { return GLPlatformWin32::makeCurrent(c); }
    void GLPlatform::clearCurrent() { GLPlatformWin32::clearCurrent(); }
    void GLPlatform::swapBuffers(GLPlatformContext &c) { GLPlatformWin32::swapBuffers(c); }
    void GLPlatform::setSwapInterval(bool vsync) { GLPlatformWin32::setSwapInterval(vsync); }
    void GLPlatform::updateContext(GLPlatformContext &c) { GLPlatformWin32::updateContext(c); }
    void *GLPlatform::getProcAddress(const char *name) { return GLPlatformWin32::getProcAddress(name); }
    bool GLPlatform::isCurrent(GLPlatformContext *c) { return GLPlatformWin32::isCurrent(c); }

}

#endif
