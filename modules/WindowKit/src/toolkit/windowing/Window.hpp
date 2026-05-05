/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "WindowEvents.hpp"
#include "WindowParams.hpp"
#include <toolkit/windowing/common.hpp>

#include <toolkit/base/Function.hpp>
#include <toolkit/platforms/System.hpp>

namespace toolkit
{
    typedef struct NativeWindow* WindowHandle;

    // Native platform window object.
    class TkWindow
    {
        friend class NativeWindow;
        friend class WindowSystem;
        // Placeholder for graphics module
        friend class GfxWindow;
        friend class GfxSwapChain;

        struct WindowState
        {
            bool visible : 1;
            bool minimized : 1;
            bool maximized : 1;
            bool fullscreen : 1;
            bool isClosing : 1;
            bool closed : 1;
            bool showAfterFirstPaint : 1;
            bool focused : 1;
            bool suppressFocusEvents : 1;
        };

        struct TrackState
        {
            Pos mouseOffset;
            Rect mouseOffsetScreenSize;
            bool isUsingMouseOffset : 1 = false;
            bool isTrackingMouse : 1 = false;
            bool isHorizontalFlippingMouse : 1 = false;
            bool isVerticalFlippingMouse : 1 = false;
            bool isClippingCursor : 1 = false;
        };

    public:
        // Event fired when window get shown.
        TkAction<> onShown;
        // Event fired when window get hidden.
        TkAction<> onHidden;
        // Event fired when window get closed.
        TkAction<> onClosed;
        // Event fired when window get resized.
        TkAction<Pos> onResized;
        // Event fired when window get focused.
        TkAction<> onGotFocus;
        // Event fired when window lost focus.
        TkAction<> onLostFocus;

    public:
        // Return true if that window is the main window.
        bool isFirstWindow() const;

        // Get window params constant reference
        TK_FORCE_INLINE const WindowParams &getParams() const
        {
            return m_params;
        }

        // Get a value that indicates whether a window is in a fullscreen mode.
        bool isFullscreen() const;
        // Get a value that indicates whether a window is not in a fullscreen mode.
        TK_FORCE_INLINE bool isWindowed() const
        {
            return !isFullscreen();
        }
        // Set a value that indicates whether a window is in a fullscreen mode.
        void setFullscreen(bool isFullscreen);

        // Get a value that indicates whether a window is visible (hidden or shown).
        bool isVisible() const;
        // Set a value that indicates whether a window is visible (hidden or shown).
        void setVisible(bool isVisible);

        // Get a value that indicates whether a window is minimized.
        TK_FORCE_INLINE bool isMinimized() const
        {
            return m_state.minimized;
        }
        // Get a value that indicates whether a window is maximized.
        TK_FORCE_INLINE bool isMaximized() const
        {
            return m_state.maximized;
        }

        // Get the native window handle.
        void *getWindowPtr() const;
        // Get the implement of TkWindow instance.
        NativeWindow* getNativeWindow() const {
            return m_native;
        }
        WindowEventGroup& getEventGroup() {
            return m_events;
        }

    public:
        // Show the window.
        void show();
        // Hide the window.
        void hide();
        // Minimize the window.
        void minimize();
        // Maximize the window.
        void maximize();
        // Set the window to be borderless or not and to be fullscreen.
        void setBorderless(bool isBorderless, bool maximized = false);
        // Restore the window state before minimizing or maximizing.
        void restore();

        // Close the window.
        void close(ClosingReason reason = ClosingReason::kCloseEvent);
        // Check if window is closed.
        bool isClosed() const;

        // Check if window is foreground (the window with which the user is currently working).
        bool isForegroundWindow() const;

    public:
        // Get the client bounds of the window (client area not including border).

        TK_FORCE_INLINE Rect getClientBounds() const
        {
            return Rect{getClientPos(), getClientSize()};
        }
        // Set the client bounds of the window (client area not including border).
        void setClientBounds(const Rect &clientArea);

        // Get the window position (in screen coordinates).
        Pos getPos() const;
        // Set the window position (in screen coordinates).
        void setPos(const Pos &position);

        // Get the client position of the window (client area not including border).
        TK_FORCE_INLINE Pos getClientPos() const
        {
            return clientToScreen({0, 0});
        }
        // Set the client position of the window (client area not including border)
        void setClientPos(const Pos &position);

        // Get the window size (including border).
        Pos getSize() const;
        // Get the size of the client area of the window (not including border).
        Pos getClientSize() const;

        // Set the size of the client area of the window (not including border).
        void setClientSize(const Pos &size)
        {
            setClientBounds(Rect{getClientPos(), size});
        }

        // Convert screen space location into window space coordinates.
        Pos screenToClient(const Pos &screenPos) const;
        // Convert window space location into screen space coordinates.
        Pos clientToScreen(const Pos &clientPos) const;

        // Get the window DPI setting.
        int getDpi() const
        {
            return m_dpi;
        }
        // Get the window DPI scale factor (1 is default). This is windowDpi / 96, not multiplied by system scale.
        f32 getDpiScale() const
        {
            return m_dpiScale;
        }

    public:
        // Get the window title.
        TkString getTitle() const;
        // Set the window title.
        void setTitle(const TkStringView &title);

        // Get window opacity value (valid only for windows created with SupportsTransparency flag). Opacity values are normalized to range [0;1].
        f32 getOpacity() const;
        // Set window opacity value (valid only for windows created with SupportsTransparency flag). Opacity values are normalized to range [0;1].
        void setOpacity(f32 opacity);

        // Determine whether this window is focused.
        TK_FORCE_INLINE bool isFocused() const
        {
            return m_state.focused;
        }
        // Focuse this window.
        void focus();
        // Bring window to the front of the Z order.
        void bringToFront(bool force = false);
        // Flashe the window to bring use attention.
        void flashWindow();

    public:
        // Start drag and drop operation. Win32 active OLE drag is not implemented yet, so this currently returns DragDropEffect::kNone.
        DragDropEffect doDragDrop(const TkStringView &data);

        // Start the mouse tracking.
        void startTrackingMouse(bool useMouseScreenOffset);
        // Get the mouse tracking offset.
        Pos getTrackingMouseOffset() const
        {
            return m_track.mouseOffset;
        }
        // Get the value indicating whenever mouse input is tracked by this window.
        bool isMouseTracking() const
        {
            return m_track.isTrackingMouse;
        }

        // Get the value indicating if the mouse flipped to the other screen edge horizontally
        bool isMouseFlippingHorizontally() const
        {
            return m_track.isHorizontalFlippingMouse;
        }
        // Get the value indicating if the mouse flipped to the other screen edge vertically
        bool isMouseFlippingVertically() const
        {
            return m_track.isVerticalFlippingMouse;
        }

        // Ends the mouse tracking.
        void endTrackingMouse();

        // Start cursor clipping with a screen-space rectangle. getClientBounds() already returns screen-space client bounds.
        void startClippingCursor(const Rect &bounds);
        // Get the value indicating whenever the cursor is being clipped.
        bool isCursorClipping() const
        {
            return m_track.isClippingCursor;
        }
        // Ends the cursor clipping.
        void endClippingCursor();

        // Get the mouse cursor.
        TK_FORCE_INLINE CursorType getCursor() const
        {
            return m_cursor;
        }
        // Set the mouse cursor.
        void setCursor(CursorType type);

        // Set the window icon.
        void setIcon(const TkStringView icon);

    public:
        // Get the text entered during the current frame.
        TkStringView getInputText() const;

        // Get the key state (true if key is being pressed during this frame).
        bool getKey(KeyboardKeys key) const;
        // Get the key 'down' state (true if key was pressed in this frame).
        bool getKeyDown(KeyboardKeys key) const;
        // Get the key 'up' state (true if key was released in this frame).
        bool getKeyUp(KeyboardKeys key) const;

    public:
        // Get the mouse position in window coordinates.
        Pos getMousePos() const;
        // Set the mouse position in window coordinates.
        void setMousePos(const Pos &position) const;

        // Get the mouse position change during the last frame.
        Pos getMousePosDelta() const;
        // Get the mouse wheel change during the last frame.
        f32 getMouseScrollDelta() const;

        // Get the mouse button state.
        bool getMouseButton(MouseButton button) const;
        // Get the mouse button down state.
        bool getMouseButtonDown(MouseButton button) const;
        // Get the mouse button up state.
        bool getMouseButtonUp(MouseButton button) const;

    public:
        void onShowTrigger();
        void onResizeTrigger(i32 width, i32 height);
        void onClosedTrigger();
        void onGotFocusTrigger();
        void onLostFocusTrigger();
        
    public:
        TkWindow(const WindowParams& p);
        ~TkWindow();

    protected:
        virtual void onBeforeDestroy();

    private:
        WindowEventGroup m_events;
        WindowHandle m_native;
        WindowState m_state;
        TrackState m_track;
        WindowParams m_params;
        int m_dpi;
        f32 m_dpiScale;

        CursorType m_cursor;
        Pos m_clientSize;
    };

}
