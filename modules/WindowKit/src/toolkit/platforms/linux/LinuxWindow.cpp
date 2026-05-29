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
#include <toolkit/platforms/System.hpp>
#include <toolkit/windowing/WindowSystem.hpp>

namespace toolkit
{
    constexpr i32 DefaultDPI = 96;
    constexpr long NetWmStateRemove = 0L;
    constexpr long NetWmStateAdd = 1L;
    constexpr long WmNormalState = 1L;
    constexpr long WmIconicState = 3L;

    struct MotifWmHints
    {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
        long inputMode;
        unsigned long status;
    };

    i32 toInt(float value)
    {
        return TkMath::truncToInt(value);
    }

    X11::Cursor createHiddenCursor(X11::Display *display, X11::Window window)
    {
        static char emptyBits[1] = {0};
        X11::Pixmap pixmap = X11::XCreateBitmapFromData(display, window, emptyBits, 1, 1);
        X11::XColor color{};
        X11::Cursor cursor = X11::XCreatePixmapCursor(display, pixmap, pixmap, &color, &color, 0, 0);
        X11::XFreePixmap(display, pixmap);
        return cursor;
    }

    X11::Cursor createCursorForType(X11::Display *display, X11::Window window, CursorType type)
    {
        if (type == CursorType::kHidden)
            return createHiddenCursor(display, window);

        unsigned int cursorShape = XC_left_ptr;
        switch (type)
        {
        case CursorType::kCross:
            cursorShape = XC_crosshair;
            break;
        case CursorType::kHand:
            cursorShape = XC_hand2;
            break;
        case CursorType::kHelp:
            cursorShape = XC_question_arrow;
            break;
        case CursorType::kIBeam:
            cursorShape = XC_xterm;
            break;
        case CursorType::kNo:
            cursorShape = XC_X_cursor;
            break;
        case CursorType::kWait:
            cursorShape = XC_watch;
            break;
        case CursorType::kSizeAll:
            cursorShape = XC_fleur;
            break;
        case CursorType::kSizeNESW:
            cursorShape = XC_bottom_left_corner;
            break;
        case CursorType::kSizeNS:
            cursorShape = XC_sb_v_double_arrow;
            break;
        case CursorType::kSizeNWSE:
            cursorShape = XC_bottom_right_corner;
            break;
        case CursorType::kSizeWE:
            cursorShape = XC_sb_h_double_arrow;
            break;
        default:
            break;
        }
        return X11::XCreateFontCursor(display, cursorShape);
    }

    void clearEvent(X11::XEvent &event)
    {
        TkMemory::memoryClear(&event, sizeof(event));
    }

    NativeWindow::NativeWindow(const WindowParams &p, TkWindow *window)
        : m_window(window)
    {
        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr)
            return;

        const i32 screen = X11::XDefaultScreen(display);
        const X11::Window root = X11::XRootWindow(display, screen);
        const i32 x = toInt(p.pos.x);
        const i32 y = toInt(p.pos.y);
        const i32 width = TkMath::max(toInt(p.size.x), 1);
        const i32 height = TkMath::max(toInt(p.size.y), 1);
        window->m_clientSize = Pos(static_cast<float>(width), static_cast<float>(height));

        X11::XSetWindowAttributes attributes{};
        attributes.background_pixel = X11::XBlackPixel(display, screen);
        attributes.border_pixel = X11::XBlackPixel(display, screen);
        attributes.event_mask =
            ExposureMask |
            FocusChangeMask |
            KeyPressMask |
            KeyReleaseMask |
            ButtonPressMask |
            ButtonReleaseMask |
            EnterWindowMask |
            LeaveWindowMask |
            PointerMotionMask |
            ButtonMotionMask |
            StructureNotifyMask |
            PropertyChangeMask |
            VisibilityChangeMask;
        if (!p.isRegularWindow)
        {
            attributes.override_redirect = 1;
            attributes.save_under = 1;
        }

        unsigned long valueMask = CWBackPixel | CWBorderPixel | CWEventMask;
        if (!p.isRegularWindow)
            valueMask |= CWOverrideRedirect | CWSaveUnder;

        m_handle = X11::XCreateWindow(
            display,
            root,
            x,
            y,
            static_cast<unsigned int>(width),
            static_cast<unsigned int>(height),
            0,
            X11::XDefaultDepth(display, screen),
            InputOutput,
            X11::XDefaultVisual(display, screen),
            valueMask,
            &attributes);
        if (m_handle == 0)
        {
            LogError("Cannot create X11 window.");
            return;
        }

        setTitle(p.title);

        if (LinuxWindowSys::s_NetWmPid != 0)
        {
            const unsigned long pid = static_cast<unsigned long>(System::getCurrentProcessId());
            X11::XChangeProperty(display, m_handle, LinuxWindowSys::s_NetWmPid, XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<const unsigned char *>(&pid), 1);
        }

        X11::XSizeHints hints{};
        hints.flags = PPosition | PSize | PMinSize | PMaxSize;
        hints.x = x;
        hints.y = y;
        hints.width = width;
        hints.height = height;
        hints.min_width = TkMath::max(toInt(p.minSize.x), 1);
        hints.min_height = TkMath::max(toInt(p.minSize.y), 1);
        hints.max_width = p.maxSize.x > 0.0f ? toInt(p.maxSize.x) : 32767;
        hints.max_height = p.maxSize.y > 0.0f ? toInt(p.maxSize.y) : 32767;
        if (!p.hasSizingFrame)
        {
            hints.min_width = width;
            hints.max_width = width;
            hints.min_height = height;
            hints.max_height = height;
        }
        X11::XSetNormalHints(display, m_handle, &hints);

        if (p.parent != nullptr && p.parent->getNativeWindow() != nullptr)
            X11::XSetTransientForHint(display, m_handle, p.parent->getNativeWindow()->getXWindow());

        if (LinuxWindowSys::s_DeleteWindow != 0)
            X11::XSetWMProtocols(display, m_handle, &LinuxWindowSys::s_DeleteWindow, 1);

        if (!p.hasBorder)
            setMotifDecorations(false);

        if (!p.isRegularWindow && LinuxWindowSys::s_NetWmWindowType != 0 && LinuxWindowSys::s_NetWmWindowTypeDock != 0)
        {
            X11::Atom type = LinuxWindowSys::s_NetWmWindowTypeDock;
            X11::XChangeProperty(display, m_handle, LinuxWindowSys::s_NetWmWindowType, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char *>(&type), 1);
        }

        X11::Atom states[4]{};
        i32 statesCount = 0;
        if (p.isTopmost && LinuxWindowSys::s_NetWmStateAbove != 0)
            states[statesCount++] = LinuxWindowSys::s_NetWmStateAbove;
        if (!p.showInTaskbar)
        {
            if (LinuxWindowSys::s_NetWmStateSkipTaskbar != 0)
                states[statesCount++] = LinuxWindowSys::s_NetWmStateSkipTaskbar;
            if (LinuxWindowSys::s_NetWmStateSkipPager != 0)
                states[statesCount++] = LinuxWindowSys::s_NetWmStateSkipPager;
        }
        if (p.fullscreen && LinuxWindowSys::s_NetWmStateFullscreen != 0)
            states[statesCount++] = LinuxWindowSys::s_NetWmStateFullscreen;
        if (statesCount > 0 && LinuxWindowSys::s_NetWmState != 0)
            X11::XChangeProperty(display, m_handle, LinuxWindowSys::s_NetWmState, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char *>(states), statesCount);

        if (p.allowDragAndDrop && LinuxWindowSys::s_XdndAware != 0)
        {
            const long xdndVersion = 5;
            X11::XChangeProperty(display, m_handle, LinuxWindowSys::s_XdndAware, XA_ATOM, 32, PropModeReplace, reinterpret_cast<const unsigned char *>(&xdndVersion), 1);
        }

        m_window->m_dpi = System::getDpi();
        m_window->m_dpiScale = static_cast<float>(m_window->m_dpi) / static_cast<float>(DefaultDPI);

        updateCursor();
        X11::XFlush(display);
    }

    NativeWindow::~NativeWindow()
    {
        X11::Display *display = LinuxWindowSys::display();
        if (m_window != nullptr)
        {
            if (m_window->m_track.isClippingCursor)
                m_window->endClippingCursor();
            if (m_window->m_track.isTrackingMouse)
                m_window->endTrackingMouse();
        }

        if (display != nullptr && m_cursor != 0)
        {
            X11::XFreeCursor(display, m_cursor);
            m_cursor = 0;
        }

        if (display != nullptr && hasHandle())
        {
            X11::Window handle = m_handle;
            X11::XDestroyWindow(display, handle);
            X11::XFlush(display);
            WindowSystem::unregisterWindowHandle(LinuxWindowSys::toHandle(handle));
            m_handle = 0;
            if (m_window != nullptr)
                m_window->m_state.visible = false;
        }
    }

    void *TkWindow::getWindowPtr() const
    {
        return m_native ? LinuxWindowSys::toHandle(m_native->m_handle) : nullptr;
    }

    void TkWindow::show()
    {
        if (m_native == nullptr || !m_native->hasHandle() || m_state.closed)
            return;
        if (!m_state.visible)
        {
            X11::Display *display = LinuxWindowSys::display();
            if (display == nullptr)
                return;

            m_state.suppressFocusEvents = true;
            X11::XMapWindow(display, m_native->m_handle);
            X11::XFlush(display);
            m_state.suppressFocusEvents = false;

            if (m_state.visible)
                return;
            m_state.visible = true;
            onShown();
        }
    }

    void TkWindow::hide()
    {
        if (m_native == nullptr || !m_native->hasHandle() || m_state.closed)
            return;
        if (m_state.visible)
        {
            X11::Display *display = LinuxWindowSys::display();
            if (display == nullptr)
                return;

            m_state.suppressFocusEvents = true;
            X11::XUnmapWindow(display, m_native->m_handle);
            X11::XFlush(display);
            m_state.suppressFocusEvents = false;

            if (!m_state.visible)
                return;
            endClippingCursor();
            endTrackingMouse();
            m_state.visible = false;
            m_state.showAfterFirstPaint = m_params.showAfterFirstPaint;
            onHidden();
        }
    }

    void TkWindow::minimize()
    {
        if (!m_params.allowMinimize || m_native == nullptr || !m_native->hasHandle() || isFullscreen())
            return;

        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr || LinuxWindowSys::s_WmChangeState == 0)
            return;

        X11::XEvent event;
        clearEvent(event);
        event.type = ClientMessage;
        event.xclient.window = m_native->m_handle;
        event.xclient.message_type = LinuxWindowSys::s_WmChangeState;
        event.xclient.format = 32;
        event.xclient.data.l[0] = WmIconicState;
        X11::XSendEvent(display, LinuxWindowSys::rootWindow(), 0, SubstructureRedirectMask | SubstructureNotifyMask, &event);
        X11::XFlush(display);
    }

    void TkWindow::maximize()
    {
        if (!m_params.allowMaximize || m_native == nullptr || !m_native->hasHandle() || isFullscreen())
            return;

        m_native->sendNetWmState(LinuxWindowSys::s_NetWmStateMaximizedHorz, true, LinuxWindowSys::s_NetWmStateMaximizedVert);
        m_state.maximized = true;
        m_state.minimized = false;
    }

    void TkWindow::restore()
    {
        if (m_native == nullptr || !m_native->hasHandle() || isFullscreen())
            return;

        if (m_state.maximized)
        {
            m_native->sendNetWmState(LinuxWindowSys::s_NetWmStateMaximizedHorz, false, LinuxWindowSys::s_NetWmStateMaximizedVert);
            m_state.maximized = false;
        }

        if (m_state.minimized)
        {
            X11::Display *display = LinuxWindowSys::display();
            if (display != nullptr && LinuxWindowSys::s_WmChangeState != 0)
            {
                X11::XEvent event;
                clearEvent(event);
                event.type = ClientMessage;
                event.xclient.window = m_native->m_handle;
                event.xclient.message_type = LinuxWindowSys::s_WmChangeState;
                event.xclient.format = 32;
                event.xclient.data.l[0] = WmNormalState;
                X11::XSendEvent(display, LinuxWindowSys::rootWindow(), 0, SubstructureRedirectMask | SubstructureNotifyMask, &event);
                X11::XMapRaised(display, m_native->m_handle);
                X11::XFlush(display);
            }
            m_state.minimized = false;
        }
    }

    void TkWindow::setBorderless(bool isBorderless, bool maximized)
    {
        if (m_native == nullptr || !m_native->hasHandle())
            return;
        if (isFullscreen())
            setFullscreen(false);
        if (isMaximized())
            restore();

        m_params.hasBorder = !isBorderless;
        m_native->setMotifDecorations(!isBorderless);
        if (maximized)
            maximize();
        m_native->checkForWindowResize();
    }

    bool TkWindow::isClosed() const
    {
        return m_state.closed || m_native == nullptr || !m_native->hasHandle();
    }

    bool TkWindow::isForegroundWindow() const
    {
        if (m_native == nullptr || !m_native->hasHandle())
            return false;

        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr)
            return false;

        X11::Window focus = 0;
        int revert = 0;
        X11::XGetInputFocus(display, &focus, &revert);
        return focus == m_native->m_handle;
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

        if (m_state.visible)
            hide();
        if (m_native != nullptr && m_native->hasHandle())
        {
            X11::Display *display = LinuxWindowSys::display();
            X11::Window handle = m_native->m_handle;
            m_native->m_handle = 0;
            if (display != nullptr)
            {
                X11::XDestroyWindow(display, handle);
                X11::XFlush(display);
            }
            WindowSystem::unregisterWindowHandle(LinuxWindowSys::toHandle(handle));
        }
        onClosedTrigger();
    }

    void TkWindow::bringToFront(bool force)
    {
        if (m_native == nullptr || !m_native->hasHandle())
            return;

        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr)
            return;

        X11::XRaiseWindow(display, m_native->m_handle);
        if (force || m_params.isRegularWindow)
        {
            X11::XEvent event;
            clearEvent(event);
            event.type = ClientMessage;
            event.xclient.window = m_native->m_handle;
            event.xclient.message_type = LinuxWindowSys::s_NetActiveWindow;
            event.xclient.format = 32;
            event.xclient.data.l[0] = 1;
            event.xclient.data.l[1] = CurrentTime;
            X11::XSendEvent(display, LinuxWindowSys::rootWindow(), 0, SubstructureRedirectMask | SubstructureNotifyMask, &event);
        }
        X11::XFlush(display);
    }

    void TkWindow::setClientBounds(const Rect &clientArea)
    {
        if (m_native == nullptr || !m_native->hasHandle())
            return;

        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr)
            return;

        const i32 x = toInt(clientArea.x);
        const i32 y = toInt(clientArea.y);
        const i32 width = TkMath::max(toInt(clientArea.w), 1);
        const i32 height = TkMath::max(toInt(clientArea.h), 1);
        X11::XMoveResizeWindow(display, m_native->m_handle, x, y, static_cast<unsigned int>(width), static_cast<unsigned int>(height));
        X11::XFlush(display);
        m_native->checkForWindowResize();
    }

    void TkWindow::setPos(const Pos &position)
    {
        if (m_native == nullptr || !m_native->hasHandle())
            return;
        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr)
            return;
        X11::XMoveWindow(display, m_native->m_handle, toInt(position.x), toInt(position.y));
        X11::XFlush(display);
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
        if (m_native == nullptr || !m_native->hasHandle() || m_state.fullscreen == isFullscreen)
            return;

        LogInfo("Changing window fullscreen mode to %v", isFullscreen);
        if (isFullscreen)
        {
            m_native->m_restoreBounds = getClientBounds();
            m_native->m_hasFullscreenRestore = true;
        }

        m_native->sendNetWmState(LinuxWindowSys::s_NetWmStateFullscreen, isFullscreen);
        if (isFullscreen)
        {
            const Rect monitor = System::getMonitorBounds(getClientPos());
            setClientBounds(monitor);
        }
        else if (m_native->m_hasFullscreenRestore)
        {
            setClientBounds(m_native->m_restoreBounds);
            m_native->m_hasFullscreenRestore = false;
        }
        m_state.fullscreen = isFullscreen;
        m_native->checkForWindowResize();
    }

    bool TkWindow::isVisible() const
    {
        return m_state.visible;
    }

    void TkWindow::setVisible(bool isVisible)
    {
        if (this->isVisible() != isVisible)
        {
            if (isVisible)
                show();
            else
                hide();
        }
    }

    Pos TkWindow::getPos() const
    {
        return getClientPos();
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
        if (m_native == nullptr || !m_native->hasHandle())
            return Pos{0, 0};

        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr)
            return Pos{0, 0};

        int x = 0;
        int y = 0;
        X11::Window child = 0;
        X11::XTranslateCoordinates(display, LinuxWindowSys::rootWindow(), m_native->m_handle, toInt(screenPos.x), toInt(screenPos.y), &x, &y, &child);
        return Pos(static_cast<float>(x), static_cast<float>(y));
    }

    Pos TkWindow::clientToScreen(const Pos &clientPos) const
    {
        if (m_native == nullptr || !m_native->hasHandle())
            return Pos{0, 0};

        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr)
            return Pos{0, 0};

        int x = 0;
        int y = 0;
        X11::Window child = 0;
        X11::XTranslateCoordinates(display, m_native->m_handle, LinuxWindowSys::rootWindow(), toInt(clientPos.x), toInt(clientPos.y), &x, &y, &child);
        return Pos(static_cast<float>(x), static_cast<float>(y));
    }

    void TkWindow::flashWindow()
    {
        if (m_native == nullptr || !m_native->hasHandle() || isFocused())
            return;
        m_native->sendNetWmState(LinuxWindowSys::s_NetWmStateDemandsAttention, true);
    }

    void NativeWindow::getScreenInfo(i32 &x, i32 &y, i32 &width, i32 &height) const
    {
        const Rect bounds = System::getMonitorBounds(m_window != nullptr ? m_window->getClientPos() : Pos{0, 0});
        x = toInt(bounds.x);
        y = toInt(bounds.y);
        width = toInt(bounds.w);
        height = toInt(bounds.h);
    }

    float TkWindow::getOpacity() const
    {
        return m_native != nullptr ? m_native->m_opacity : 1.0f;
    }

    void TkWindow::setOpacity(f32 opacity)
    {
        if (m_native == nullptr || !m_native->hasHandle())
            return;

        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr)
            return;

        opacity = TkMath::saturate(opacity);
        if (TkMath::nearEqual(opacity, 1.0f))
        {
            X11::XDeleteProperty(display, m_native->m_handle, LinuxWindowSys::s_WmWindowOpacity);
        }
        else
        {
            const unsigned long alpha = static_cast<unsigned long>(opacity * static_cast<float>(0xFFFFFFFFu));
            X11::XChangeProperty(display, m_native->m_handle, LinuxWindowSys::s_WmWindowOpacity, XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<const unsigned char *>(&alpha), 1);
        }
        m_native->m_opacity = opacity;
        X11::XFlush(display);
    }

    void TkWindow::focus()
    {
        if (m_native == nullptr || !m_native->hasHandle() || !m_native->isWindowMapped())
            return;

        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr)
            return;

        X11::XSetInputFocus(display, m_native->m_handle, RevertToPointerRoot, CurrentTime);
        X11::XFlush(display);
    }

    TkString TkWindow::getTitle() const
    {
        return m_params.title;
    }

    void TkWindow::setTitle(const TkStringView &title)
    {
        if (m_native == nullptr || !m_native->hasHandle())
            return;

        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr)
            return;

        const char *text = title.getText();
        X11::XStoreName(display, m_native->m_handle, text);
        X11::XSetIconName(display, m_native->m_handle, text);
        if (LinuxWindowSys::s_NetWmName != 0 && LinuxWindowSys::s_Utf8String != 0)
            X11::XChangeProperty(display, m_native->m_handle, LinuxWindowSys::s_NetWmName, LinuxWindowSys::s_Utf8String, 8, PropModeReplace, reinterpret_cast<const unsigned char *>(text), title.length());
        m_params.title = title;
    }

    void TkWindow::startTrackingMouse(bool useMouseScreenOffset)
    {
        if (m_native == nullptr || !m_native->hasHandle())
            return;
        if (!m_track.isTrackingMouse)
        {
            m_track.isTrackingMouse = true;
            m_track.mouseOffset = {0, 0};
            m_track.isUsingMouseOffset = useMouseScreenOffset;
            m_track.isHorizontalFlippingMouse = false;
            m_track.isVerticalFlippingMouse = false;

            i32 x = 0;
            i32 y = 0;
            i32 width = 0;
            i32 height = 0;
            m_native->getScreenInfo(x, y, width, height);
            m_track.mouseOffsetScreenSize = Rect(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height));

            X11::Display *display = LinuxWindowSys::display();
            if (display != nullptr)
                X11::XGrabPointer(display, m_native->m_handle, 1, PointerMotionMask | ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, 0, 0, CurrentTime);
        }
    }

    void TkWindow::endTrackingMouse()
    {
        if (m_track.isTrackingMouse)
        {
            m_track.isTrackingMouse = false;
            m_track.isHorizontalFlippingMouse = false;
            m_track.isVerticalFlippingMouse = false;

            X11::Display *display = LinuxWindowSys::display();
            if (display != nullptr)
                X11::XUngrabPointer(display, CurrentTime);
        }
    }

    void TkWindow::startClippingCursor(const Rect &bounds)
    {
        (void)bounds;
        if (m_native == nullptr || !m_native->hasHandle())
            return;
        m_track.isClippingCursor = true;
        if (isFocused())
        {
            X11::Display *display = LinuxWindowSys::display();
            if (display != nullptr)
            {
                X11::XGrabPointer(display, m_native->m_handle, 1, PointerMotionMask | ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, m_native->m_handle, 0, CurrentTime);
                m_native->m_clipCursorSet = true;
            }
        }
    }

    void TkWindow::endClippingCursor()
    {
        if (m_track.isClippingCursor)
        {
            m_track.isClippingCursor = false;
            if (m_native != nullptr)
                m_native->m_clipCursorSet = false;
            X11::Display *display = LinuxWindowSys::display();
            if (display != nullptr)
                X11::XUngrabPointer(display, CurrentTime);
        }
    }

    void TkWindow::setCursor(CursorType type)
    {
        if (static_cast<i32>(type) < 0 || type >= CursorType::MAX)
            type = CursorType::kDefault;
        m_cursor = type;
        if (m_native != nullptr && m_native->hasHandle())
            m_native->updateCursor();
    }

    void TkWindow::setIcon(const TkStringView icon)
    {
        (void)icon;
    }

    DragDropEffect TkWindow::doDragDrop(const TkStringView &data)
    {
        (void)data;
        return DragDropEffect::kNone;
    }

    void NativeWindow::checkForWindowResize()
    {
        if (m_window == nullptr || m_window->m_state.minimized || !hasHandle())
            return;

        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr)
            return;

        X11::XWindowAttributes attributes{};
        if (X11::XGetWindowAttributes(display, m_handle, &attributes) == 0)
            return;

        const i32 width = TkMath::max(attributes.width, 0);
        const i32 height = TkMath::max(attributes.height, 0);
        const Pos newSize(static_cast<float>(width), static_cast<float>(height));
        if (!Pos::nearEqual(m_window->m_clientSize, newSize) && width > 0 && height > 0)
        {
            m_window->m_clientSize = newSize;
            m_window->onResizeTrigger(width, height);
        }
    }

    void NativeWindow::sendNetWmState(X11::Atom state, bool enable, X11::Atom state2)
    {
        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr || !hasHandle() || LinuxWindowSys::s_NetWmState == 0 || state == 0)
            return;

        X11::XEvent event;
        clearEvent(event);
        event.type = ClientMessage;
        event.xclient.window = m_handle;
        event.xclient.message_type = LinuxWindowSys::s_NetWmState;
        event.xclient.format = 32;
        event.xclient.data.l[0] = enable ? NetWmStateAdd : NetWmStateRemove;
        event.xclient.data.l[1] = state;
        event.xclient.data.l[2] = state2;
        event.xclient.data.l[3] = 1;
        X11::XSendEvent(display, LinuxWindowSys::rootWindow(), 0, SubstructureRedirectMask | SubstructureNotifyMask, &event);
        X11::XFlush(display);
    }

    void NativeWindow::setMotifDecorations(bool enabled)
    {
        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr || !hasHandle() || LinuxWindowSys::s_MotifWmHints == 0)
            return;

        MotifWmHints hints{};
        hints.flags = 2;
        hints.decorations = enabled ? 1UL : 0UL;
        X11::XChangeProperty(display, m_handle, LinuxWindowSys::s_MotifWmHints, LinuxWindowSys::s_MotifWmHints, 32, PropModeReplace, reinterpret_cast<unsigned char *>(&hints), 5);
        X11::XFlush(display);
    }

    void NativeWindow::updateCursor()
    {
        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr || !hasHandle())
            return;

        if (m_cursor != 0)
            X11::XFreeCursor(display, m_cursor);
        m_cursor = createCursorForType(display, m_handle, m_window != nullptr ? m_window->m_cursor : CursorType::kDefault);
        if (m_cursor != 0)
            X11::XDefineCursor(display, m_handle, m_cursor);
        X11::XFlush(display);
    }

    bool NativeWindow::isWindowMapped() const
    {
        X11::Display *display = LinuxWindowSys::display();
        if (display == nullptr || !hasHandle())
            return false;

        X11::XWindowAttributes attributes{};
        if (X11::XGetWindowAttributes(display, m_handle, &attributes) == 0)
            return false;
        return attributes.map_state != IsUnmapped;
    }

    bool NativeWindow::processEvent(X11::XEvent &event)
    {
        if (m_window == nullptr)
            return false;

        switch (event.type)
        {
        case ClientMessage:
            if (static_cast<X11::Atom>(event.xclient.data.l[0]) == LinuxWindowSys::s_DeleteWindow)
            {
                m_window->close(ClosingReason::kUser);
                return true;
            }
            break;
        case ConfigureNotify:
            checkForWindowResize();
            break;
        case MapNotify:
            if (!m_window->m_state.visible)
            {
                m_window->m_state.visible = true;
                m_window->onShown();
            }
            if (m_window->m_params.allowInput && m_window->m_params.activateWhenFirstShown)
                m_window->focus();
            break;
        case UnmapNotify:
            if (m_window->m_state.visible)
            {
                m_window->endClippingCursor();
                m_window->endTrackingMouse();
                m_window->m_state.visible = false;
                m_window->m_state.showAfterFirstPaint = m_window->m_params.showAfterFirstPaint;
                m_window->onHidden();
            }
            break;
        case FocusIn:
            m_window->onGotFocusTrigger();
            updateCursor();
            if (m_window->m_track.isClippingCursor && !m_clipCursorSet)
                m_window->startClippingCursor(m_window->getClientBounds());
            break;
        case FocusOut:
            if (m_clipCursorSet)
                m_window->endClippingCursor();
            m_window->onLostFocusTrigger();
            updateCursor();
            break;
        case MotionNotify:
            if (m_window->m_track.isTrackingMouse && m_window->m_track.isUsingMouseOffset)
            {
                Pos desktopLocation = m_window->m_track.mouseOffsetScreenSize.location;
                Pos desktopSize = m_window->m_track.mouseOffsetScreenSize.getBottomRight();
                Pos mousePosition(static_cast<float>(event.xmotion.x_root), static_cast<float>(event.xmotion.y_root));
                Pos newMousePosition = mousePosition;
                if ((m_window->m_track.isHorizontalFlippingMouse = mousePosition.x <= desktopLocation.x + 2))
                    newMousePosition.x = desktopSize.x - 3;
                else if ((m_window->m_track.isHorizontalFlippingMouse = mousePosition.x >= desktopSize.x - 1))
                    newMousePosition.x = desktopLocation.x + 3;
                if ((m_window->m_track.isVerticalFlippingMouse = mousePosition.y <= desktopLocation.y + 2))
                    newMousePosition.y = desktopSize.y - 3;
                else if ((m_window->m_track.isVerticalFlippingMouse = mousePosition.y >= desktopSize.y - 1))
                    newMousePosition.y = desktopLocation.y + 3;
                if (!Pos::nearEqual(mousePosition, newMousePosition))
                {
                    m_window->m_track.mouseOffset -= newMousePosition - mousePosition;
                    m_window->setMousePos(m_window->screenToClient(newMousePosition));
                }
            }
            break;
        case LeaveNotify:
            break;
        case DestroyNotify:
        {
            X11::Window destroyedHandle = m_handle;
            m_handle = 0;
            WindowSystem::unregisterWindowHandle(LinuxWindowSys::toHandle(destroyedHandle));
            m_window->m_state.visible = false;
            m_window->onClosedTrigger();
            return true;
        }
        default:
            break;
        }

        if (!m_window->m_params.allowInput)
            return false;
        return LinuxInput::processEvent(m_window, event);
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
        if (m_state.suppressFocusEvents)
            return;

        onGotFocus();
    }

    void TkWindow::onLostFocusTrigger()
    {
        if (!m_state.focused)
            return;
        m_state.focused = false;
        if (m_state.suppressFocusEvents)
            return;

        onLostFocus();
    }
}

#endif
