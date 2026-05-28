/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "LinuxWindow.hpp"

#if TK_LINUX

#include "LinuxInput.hpp"
#include "LinuxWindowSys.hpp"
#include <toolkit/debug/Logger.hpp>
#include <toolkit/debug/Profiler.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/windowing/WindowSystem.hpp>

namespace toolkit
{
    static constexpr i32 DefaultDPI = 96;

    static X11::Display *display()
    {
        return LinuxWindowSys::getDisplay();
    }

    static X11::Window rootWindow()
    {
        X11::Display *d = display();
        return d ? X11::XDefaultRootWindow(d) : 0;
    }

    static Rect desktopBounds()
    {
        X11::Display *d = display();
        if (!d)
            return Rect(0, 0, 0, 0);
        const i32 screen = X11::XDefaultScreen(d);
        return Rect(0, 0, static_cast<float>(X11::XDisplayWidth(d, screen)), static_cast<float>(X11::XDisplayHeight(d, screen)));
    }

    static void setWindowType(X11::Window window, bool showInTaskbar)
    {
        X11::Display *d = display();
        if (!d)
            return;
        const X11::Atom typeAtom = X11::XInternAtom(d, "_NET_WM_WINDOW_TYPE", 0);
        const X11::Atom valueAtom = X11::XInternAtom(d, showInTaskbar ? "_NET_WM_WINDOW_TYPE_NORMAL" : "_NET_WM_WINDOW_TYPE_UTILITY", 0);
        X11::XChangeProperty(d, window, typeAtom, XA_ATOM, 32, PropModeReplace, reinterpret_cast<const unsigned char *>(&valueAtom), 1);
    }

    NativeWindow::NativeWindow(const WindowParams &p, TkWindow *window)
        : m_window(window)
    {
        X11::Display *d = display();
        if (!d)
        {
            LogError("[LinuxWindow] XOpenDisplay failed.");
            return;
        }

        const i32 screen = X11::XDefaultScreen(d);
        const i32 x = static_cast<i32>(p.pos.x);
        const i32 y = static_cast<i32>(p.pos.y);
        const u32 width = static_cast<u32>(p.size.x > 1.0f ? p.size.x : 1.0f);
        const u32 height = static_cast<u32>(p.size.y > 1.0f ? p.size.y : 1.0f);
        X11::XSetWindowAttributes attrs {};
        attrs.event_mask = ExposureMask | StructureNotifyMask | FocusChangeMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask;
        attrs.override_redirect = p.isRegularWindow ? 0 : 1;
        attrs.background_pixel = X11::XBlackPixel(d, screen);

        m_handle = X11::XCreateWindow(d, X11::XRootWindow(d, screen), x, y, width, height, 0, CopyFromParent, InputOutput, CopyFromParent, CWEventMask | CWBackPixel | CWOverrideRedirect, &attrs);
        if (!m_handle)
        {
            LogError("[LinuxWindow] XCreateWindow failed.");
            return;
        }

        X11::XStoreName(d, m_handle, p.title.getText());
        X11::Atom deleteAtom = LinuxWindowSys::getDeleteWindowAtom();
        X11::XSetWMProtocols(d, m_handle, &deleteAtom, 1);
        setWindowType(m_handle, p.showInTaskbar);
        updateMinMaxHints();

        m_window->m_dpi = DefaultDPI;
        m_window->m_dpiScale = 1.0f;
        m_window->m_clientSize = Pos(static_cast<float>(width), static_cast<float>(height));

        if (p.fullscreen)
            setFullscreenState(true);
        X11::XFlush(d);
    }

    NativeWindow::~NativeWindow()
    {
        X11::Display *d = display();
        if (!d || !m_handle)
            return;
        if (m_window)
        {
            if (m_window->m_track.isClippingCursor)
                m_window->endClippingCursor();
            if (m_window->m_track.isTrackingMouse)
                m_window->endTrackingMouse();
        }
        const X11::Window handle = m_handle;
        m_destroying = true;
        m_handle = 0;
        WindowSystem::unregisterWindowHandle(reinterpret_cast<void *>(handle));
        X11::XDestroyWindow(d, handle);
        X11::XFlush(d);
    }

    void NativeWindow::updateMinMaxHints()
    {
        if (!m_handle)
            return;
        X11::XSizeHints hints {};
        hints.flags = PMinSize;
        hints.min_width = static_cast<i32>(m_window->m_params.minSize.x);
        hints.min_height = static_cast<i32>(m_window->m_params.minSize.y);
        if (m_window->m_params.maxSize.sumValues() > 0)
        {
            hints.flags |= PMaxSize;
            hints.max_width = static_cast<i32>(m_window->m_params.maxSize.x);
            hints.max_height = static_cast<i32>(m_window->m_params.maxSize.y);
        }
        X11::XSetWMNormalHints(display(), m_handle, &hints);
    }

    void NativeWindow::setFullscreenState(bool fullscreen)
    {
        X11::Display *d = display();
        if (!d || !m_handle)
            return;
        X11::XEvent event {};
        event.type = ClientMessage;
        event.xclient.window = m_handle;
        event.xclient.message_type = LinuxWindowSys::getWmStateAtom();
        event.xclient.format = 32;
        event.xclient.data.l[0] = fullscreen ? 1 : 0;
        event.xclient.data.l[1] = LinuxWindowSys::getWmStateFullscreenAtom();
        event.xclient.data.l[2] = 0;
        event.xclient.data.l[3] = 1;
        X11::XSendEvent(d, rootWindow(), 0, SubstructureNotifyMask | SubstructureRedirectMask, &event);
    }

    void NativeWindow::getScreenInfo(i32 &x, i32 &y, i32 &width, i32 &height) const
    {
        X11::Display *d = display();
        if (!d)
        {
            x = y = width = height = 0;
            return;
        }
        const i32 screen = X11::XDefaultScreen(d);
        x = 0;
        y = 0;
        width = X11::XDisplayWidth(d, screen);
        height = X11::XDisplayHeight(d, screen);
    }

    void NativeWindow::checkForWindowResize()
    {
        if (!m_handle)
            return;
        X11::XWindowAttributes attrs {};
        if (!X11::XGetWindowAttributes(display(), m_handle, &attrs))
            return;
        const Pos newSize(static_cast<float>(attrs.width), static_cast<float>(attrs.height));
        if (!Pos::nearEqual(m_window->m_clientSize, newSize))
        {
            m_window->m_clientSize = newSize;
            m_window->onResizeTrigger(attrs.width, attrs.height);
        }
    }

    void NativeWindow::handleEvent(X11::XEvent &event)
    {
        switch (event.type)
        {
        case ClientMessage:
            if (static_cast<X11::Atom>(event.xclient.data.l[0]) == LinuxWindowSys::getDeleteWindowAtom())
                m_window->close(ClosingReason::kUser);
            break;
        case ConfigureNotify:
            checkForWindowResize();
            break;
        case FocusIn:
            m_window->onGotFocusTrigger();
            break;
        case FocusOut:
            m_window->onLostFocusTrigger();
            break;
        case MapNotify:
            if (!m_window->m_state.visible)
            {
                m_window->m_state.visible = true;
                m_window->onShown();
            }
            break;
        case UnmapNotify:
            if (m_window->m_state.visible)
            {
                m_window->m_state.visible = false;
                m_window->onHidden();
            }
            break;
        case DestroyNotify:
            if (!m_destroying)
                m_window->onClosedTrigger();
            break;
        default:
            break;
        }

        if (m_window->m_params.allowInput)
            LinuxInput::handleEvent(m_window, event);
    }

    void *TkWindow::getWindowPtr() const
    {
        return m_native ? reinterpret_cast<void *>(m_native->m_handle) : nullptr;
    }

    void TkWindow::show()
    {
        if (!m_native || !m_native->hasXWindow() || m_state.closed)
            return;
        X11::XMapRaised(display(), m_native->m_handle);
        X11::XFlush(display());
    }

    void TkWindow::hide()
    {
        if (!m_native || !m_native->hasXWindow() || m_state.closed)
            return;
        endClippingCursor();
        endTrackingMouse();
        X11::XUnmapWindow(display(), m_native->m_handle);
        X11::XFlush(display());
    }

    void TkWindow::minimize()
    {
        if (!m_params.allowMinimize || !m_native || !m_native->hasXWindow() || isFullscreen())
            return;
        X11::XIconifyWindow(display(), m_native->m_handle, X11::XDefaultScreen(display()));
        m_state.minimized = true;
    }

    void TkWindow::maximize()
    {
        if (!m_params.allowMaximize || !m_native || !m_native->hasXWindow() || isFullscreen())
            return;
        X11::Atom maxHorz = X11::XInternAtom(display(), "_NET_WM_STATE_MAXIMIZED_HORZ", 0);
        X11::Atom maxVert = X11::XInternAtom(display(), "_NET_WM_STATE_MAXIMIZED_VERT", 0);
        X11::XEvent event {};
        event.type = ClientMessage;
        event.xclient.window = m_native->m_handle;
        event.xclient.message_type = LinuxWindowSys::getWmStateAtom();
        event.xclient.format = 32;
        event.xclient.data.l[0] = 1;
        event.xclient.data.l[1] = maxHorz;
        event.xclient.data.l[2] = maxVert;
        event.xclient.data.l[3] = 1;
        X11::XSendEvent(display(), rootWindow(), 0, SubstructureNotifyMask | SubstructureRedirectMask, &event);
        m_state.maximized = true;
        m_state.minimized = false;
    }

    void TkWindow::setBorderless(bool isBorderless, bool maximized)
    {
        if (!m_native || !m_native->hasXWindow())
            return;
        m_params.hasBorder = !isBorderless;
        X11::Atom hintsAtom = X11::XInternAtom(display(), "_MOTIF_WM_HINTS", 0);
        struct MotifHints
        {
            unsigned long flags;
            unsigned long functions;
            unsigned long decorations;
            long inputMode;
            unsigned long status;
        } hints {2, 0, isBorderless ? 0ul : 1ul, 0, 0};
        X11::XChangeProperty(display(), m_native->m_handle, hintsAtom, hintsAtom, 32, PropModeReplace, reinterpret_cast<unsigned char *>(&hints), 5);
        if (maximized)
            maximize();
        X11::XFlush(display());
    }

    void TkWindow::restore()
    {
        if (!m_native || !m_native->hasXWindow())
            return;
        if (isFullscreen())
            setFullscreen(false);
        X11::XMapRaised(display(), m_native->m_handle);
        m_state.minimized = false;
        m_state.maximized = false;
    }

    bool TkWindow::isClosed() const
    {
        return m_state.closed || m_native == nullptr || !m_native->hasXWindow();
    }

    bool TkWindow::isForegroundWindow() const
    {
        if (!m_native || !m_native->hasXWindow())
            return false;
        X11::Window focused = 0;
        i32 revert = 0;
        X11::XGetInputFocus(display(), &focused, &revert);
        return focused == m_native->m_handle;
    }

    void TkWindow::close(ClosingReason reason)
    {
        if (m_state.isClosing || m_state.closed)
            return;
        m_state.isClosing = true;
        bool cancel = false;
        m_events.onClosing(reason, cancel);
        if (cancel)
        {
            m_state.isClosing = false;
            return;
        }
        hide();
        onClosedTrigger();
    }

    void TkWindow::bringToFront(bool)
    {
        if (m_native && m_native->hasXWindow())
            X11::XRaiseWindow(display(), m_native->m_handle);
    }

    void TkWindow::setClientBounds(const Rect &clientArea)
    {
        if (!m_native || !m_native->hasXWindow())
            return;
        X11::XMoveResizeWindow(display(), m_native->m_handle, static_cast<i32>(clientArea.x), static_cast<i32>(clientArea.y), static_cast<u32>(clientArea.w > 1.0f ? clientArea.w : 1.0f), static_cast<u32>(clientArea.h > 1.0f ? clientArea.h : 1.0f));
        m_native->checkForWindowResize();
    }

    void TkWindow::setPos(const Pos &position)
    {
        if (m_native && m_native->hasXWindow())
            X11::XMoveWindow(display(), m_native->m_handle, static_cast<i32>(position.x), static_cast<i32>(position.y));
    }

    void TkWindow::setClientPos(const Pos &position)
    {
        setPos(position);
    }

    bool TkWindow::isFirstWindow() const
    {
        return WindowSystem::getMainWindow() == this;
    }

    bool TkWindow::isFullscreen() const
    {
        return m_state.fullscreen;
    }

    void TkWindow::setFullscreen(bool isFullscreen)
    {
        if (!m_native || !m_native->hasXWindow() || m_state.fullscreen == isFullscreen)
            return;
        if (isFullscreen && !m_native->m_hasFullscreenRestore)
        {
            m_native->m_restoreRect = Rect(getPos(), getSize());
            m_native->m_hasFullscreenRestore = true;
        }
        m_native->setFullscreenState(isFullscreen);
        m_state.fullscreen = isFullscreen;
        if (!isFullscreen && m_native->m_hasFullscreenRestore)
        {
            setClientBounds(m_native->m_restoreRect);
            m_native->m_hasFullscreenRestore = false;
        }
    }

    Pos TkWindow::getPos() const
    {
        if (!m_native || !m_native->hasXWindow())
            return Pos::Minimum;
        X11::Window child = 0;
        i32 x = 0;
        i32 y = 0;
        X11::XTranslateCoordinates(display(), m_native->m_handle, rootWindow(), 0, 0, &x, &y, &child);
        return Pos(static_cast<float>(x), static_cast<float>(y));
    }

    Pos TkWindow::getSize() const
    {
        return getClientSize();
    }

    Pos TkWindow::getClientSize() const
    {
        return m_clientSize;
    }

    Pos TkWindow::screenToClient(const Pos &screenPos) const
    {
        if (!m_native || !m_native->hasXWindow())
            return Pos::Minimum;
        X11::Window child = 0;
        i32 x = 0;
        i32 y = 0;
        X11::XTranslateCoordinates(display(), rootWindow(), m_native->m_handle, static_cast<i32>(screenPos.x), static_cast<i32>(screenPos.y), &x, &y, &child);
        return Pos(static_cast<float>(x), static_cast<float>(y));
    }

    Pos TkWindow::clientToScreen(const Pos &clientPos) const
    {
        if (!m_native || !m_native->hasXWindow())
            return Pos::Minimum;
        X11::Window child = 0;
        i32 x = 0;
        i32 y = 0;
        X11::XTranslateCoordinates(display(), m_native->m_handle, rootWindow(), static_cast<i32>(clientPos.x), static_cast<i32>(clientPos.y), &x, &y, &child);
        return Pos(static_cast<float>(x), static_cast<float>(y));
    }

    TkString TkWindow::getTitle() const
    {
        if (!m_native || !m_native->hasXWindow())
            return TkString::Empty;
        char *name = nullptr;
        if (!X11::XFetchName(display(), m_native->m_handle, &name) || !name)
            return TkString::Empty;
        TkString result(name);
        X11::XFree(name);
        return result;
    }

    void TkWindow::setTitle(const TkStringView &title)
    {
        m_params.title = title;
        if (m_native && m_native->hasXWindow())
            X11::XStoreName(display(), m_native->m_handle, TkString(title).getText());
    }

    f32 TkWindow::getOpacity() const
    {
        return 1.0f;
    }

    void TkWindow::setOpacity(f32)
    {
    }

    void TkWindow::focus()
    {
        if (m_native && m_native->hasXWindow())
            X11::XSetInputFocus(display(), m_native->m_handle, RevertToParent, CurrentTime);
    }

    void TkWindow::flashWindow()
    {
    }

    DragDropEffect TkWindow::doDragDrop(const TkStringView &)
    {
        return DragDropEffect::kNone;
    }

    void TkWindow::startTrackingMouse(bool useMouseScreenOffset)
    {
        m_track.isTrackingMouse = true;
        m_track.isUsingMouseOffset = useMouseScreenOffset;
        m_track.mouseOffsetScreenSize = desktopBounds();
    }

    void TkWindow::endTrackingMouse()
    {
        m_track.isTrackingMouse = false;
        m_track.isUsingMouseOffset = false;
        m_track.isHorizontalFlippingMouse = false;
        m_track.isVerticalFlippingMouse = false;
    }

    void TkWindow::startClippingCursor(const Rect &)
    {
        m_track.isClippingCursor = true;
    }

    void TkWindow::endClippingCursor()
    {
        m_track.isClippingCursor = false;
    }

    void TkWindow::setCursor(CursorType type)
    {
        m_cursor = type;
    }

    void TkWindow::setIcon(const TkStringView)
    {
    }

    void TkWindow::onShowTrigger()
    {
        PROFILE_NAMED("Window.OnShow");
        onShown();
    }

    void TkWindow::onResizeTrigger(i32 width, i32 height)
    {
        PROFILE_NAMED("Window.OnResize");
        onResized({static_cast<float>(width), static_cast<float>(height)});
    }

    void TkWindow::onClosedTrigger()
    {
        if (m_state.closed)
            return;
        m_state.isClosing = true;
        m_state.closed = true;
        m_state.visible = false;
        endClippingCursor();
        endTrackingMouse();
        onClosed();
    }

    void TkWindow::onGotFocusTrigger()
    {
        if (m_state.focused)
            return;
        m_state.focused = true;
        if (!m_state.suppressFocusEvents)
            onGotFocus();
    }

    void TkWindow::onLostFocusTrigger()
    {
        if (!m_state.focused)
            return;
        m_state.focused = false;
        if (!m_state.suppressFocusEvents)
            onLostFocus();
    }
}

#endif
