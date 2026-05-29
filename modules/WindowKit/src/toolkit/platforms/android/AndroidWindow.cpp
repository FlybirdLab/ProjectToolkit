/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "AndroidWindow.hpp"

#if TK_ANDROID

#include <toolkit/collection/HashMap.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/windowing/Dialog.hpp>
#include <toolkit/windowing/WindowSystem.hpp>

struct ANativeWindow;

namespace toolkit
{
    constexpr i32 DefaultDPI = 96;

    bool s_initialized = false;
    ANativeWindow* s_nativeWindow = nullptr;
    TkHashMap<void*, TkWindow*> s_handleMap;

    IAllocator& getWindowAllocator(const WindowParams& params)
    {
        return params.alloc != nullptr ? *params.alloc : TkMemory::getDefault();
    }

    bool containsWindowUnlocked(TkWindow* win)
    {
        for (TkWindow* window : WindowSystem::Windows)
        {
            if (window == win)
                return true;
        }
        return false;
    }

    void removeHandleMapEntriesForWindowUnlocked(TkWindow* win)
    {
        if (win == nullptr)
            return;

        TkArray<void*> keysToRemove;
        for (auto iterator = s_handleMap.Begin(); iterator.isNotEnd(); ++iterator)
        {
            if (iterator->value == win)
                keysToRemove.add(iterator->key);
        }
        for (void* key : keysToRemove)
            s_handleMap.remove(key);
    }

    void syncHandleMapUnlocked(TkWindow* win)
    {
        if (win == nullptr)
            return;

        void* handle = win->getWindowPtr();
        if (handle != nullptr)
            s_handleMap[handle] = win;
    }

    WindowParams resolveWindowParams(const WindowParams& p)
    {
        WindowParams resolved = p;
        if (resolved.size.x <= 0.0f || resolved.size.y <= 0.0f)
            resolved.size = System::getDesktopSize();
        if (resolved.size.x <= 0.0f || resolved.size.y <= 0.0f)
            resolved.size = Pos(800, 600);
        return resolved;
    }

    ANativeWindow* getAndroidNativeWindow()
    {
        return s_nativeWindow;
    }

    void setAndroidNativeWindow(ANativeWindow* window)
    {
        WindowSystem::WindowSysMtx.lock();
        void* oldHandle = s_nativeWindow;
        if (oldHandle != nullptr)
            s_handleMap.remove(oldHandle);

        s_nativeWindow = window;
        if (s_nativeWindow != nullptr)
        {
            for (TkWindow* win : WindowSystem::Windows)
                syncHandleMapUnlocked(win);
        }
        WindowSystem::WindowSysMtx.unlock();
    }

    void notifyAndroidWindowSize(int32_t width, int32_t height)
    {
        if (width <= 0 || height <= 0)
            return;

        const Pos size(static_cast<float>(width), static_cast<float>(height));
        for (TkWindow* window : WindowSystem::getWindowsSnapshot())
        {
            if (window != nullptr && !window->isClosed() && !Pos::nearEqual(window->getClientSize(), size))
                window->setClientBounds(Rect(window->getClientPos(), size));
        }
    }

    void setAndroidWindowFocus(bool focused)
    {
        for (TkWindow* window : WindowSystem::getWindowsSnapshot())
        {
            if (window == nullptr || window->isClosed())
                continue;
            if (focused)
                window->onGotFocusTrigger();
            else
                window->onLostFocusTrigger();
        }
    }

    NativeWindow::NativeWindow(const WindowParams& p, TkWindow* window)
        : m_window(window)
    {
        if (m_window != nullptr)
        {
            m_window->m_clientSize = p.size;
            m_window->m_dpi = System::getDpi();
            m_window->m_dpiScale = static_cast<float>(m_window->m_dpi) / static_cast<float>(DefaultDPI);
            if (p.fullscreen)
                m_window->m_state.fullscreen = true;
        }
    }

    NativeWindow::~NativeWindow()
    {
        if (m_window != nullptr)
        {
            m_window->endClippingCursor();
            m_window->endTrackingMouse();
            m_window->m_state.visible = false;
        }
    }

    ANativeWindow* NativeWindow::getANativeWindow() const
    {
        return s_nativeWindow;
    }

    bool NativeWindow::hasHandle() const
    {
        return true;
    }

    void* TkWindow::getWindowPtr() const
    {
        return m_native != nullptr ? m_native->getANativeWindow() : nullptr;
    }

    void TkWindow::show()
    {
        if (m_native == nullptr || m_state.closed || m_state.visible)
            return;

        m_state.visible = true;
        if (m_params.allowInput && m_params.activateWhenFirstShown)
            onGotFocusTrigger();
        onShown();
    }

    void TkWindow::hide()
    {
        if (m_native == nullptr || m_state.closed || !m_state.visible)
            return;

        endClippingCursor();
        endTrackingMouse();
        m_state.visible = false;
        m_state.showAfterFirstPaint = m_params.showAfterFirstPaint;
        onLostFocusTrigger();
        onHidden();
    }

    void TkWindow::minimize()
    {
        if (m_state.closed || !m_params.allowMinimize)
            return;
        m_state.minimized = true;
        m_state.maximized = false;
        hide();
    }

    void TkWindow::maximize()
    {
        if (m_state.closed || !m_params.allowMaximize)
            return;
        m_state.maximized = true;
        m_state.minimized = false;
        setClientBounds(Rect(Pos(0, 0), System::getDesktopSize()));
    }

    void TkWindow::restore()
    {
        if (m_state.closed)
            return;
        m_state.minimized = false;
        m_state.maximized = false;
        if (!m_state.visible)
            show();
    }

    void TkWindow::setBorderless(bool isBorderless, bool maximized)
    {
        m_params.hasBorder = !isBorderless;
        if (maximized)
            maximize();
    }

    bool TkWindow::isClosed() const
    {
        return m_state.closed || m_native == nullptr;
    }

    bool TkWindow::isForegroundWindow() const
    {
        return m_state.focused;
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

    void TkWindow::bringToFront(bool force)
    {
        (void)force;
        focus();
    }

    void TkWindow::setClientBounds(const Rect& clientArea)
    {
        const Pos size(TkMath::max(clientArea.w, 1.0f), TkMath::max(clientArea.h, 1.0f));
        m_params.pos = clientArea.location;
        m_params.size = size;

        if (!Pos::nearEqual(m_clientSize, size))
        {
            m_clientSize = size;
            onResizeTrigger(static_cast<i32>(size.x), static_cast<i32>(size.y));
        }
    }

    void TkWindow::setPos(const Pos& position)
    {
        m_params.pos = position;
    }

    void TkWindow::setClientPos(const Pos& position)
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
        if (m_state.fullscreen == isFullscreen)
            return;

        m_state.fullscreen = isFullscreen;
        if (isFullscreen)
            setClientBounds(Rect(Pos(0, 0), System::getDesktopSize()));
    }

    bool TkWindow::isVisible() const
    {
        return m_state.visible;
    }

    void TkWindow::setVisible(bool isVisible)
    {
        if (isVisible)
            show();
        else
            hide();
    }

    Pos TkWindow::getPos() const
    {
        return m_params.pos;
    }

    Pos TkWindow::getSize() const
    {
        return m_clientSize;
    }

    Pos TkWindow::getClientSize() const
    {
        return m_clientSize;
    }

    Pos TkWindow::screenToClient(const Pos& screenPos) const
    {
        return screenPos - m_params.pos;
    }

    Pos TkWindow::clientToScreen(const Pos& clientPos) const
    {
        return clientPos + m_params.pos;
    }

    void TkWindow::flashWindow()
    {
    }

    float TkWindow::getOpacity() const
    {
        return m_native != nullptr ? m_native->m_opacity : 1.0f;
    }

    void TkWindow::setOpacity(f32 opacity)
    {
        if (m_native != nullptr)
            m_native->m_opacity = TkMath::saturate(opacity);
    }

    void TkWindow::focus()
    {
        onGotFocusTrigger();
    }

    TkString TkWindow::getTitle() const
    {
        return m_params.title;
    }

    void TkWindow::setTitle(const TkStringView& title)
    {
        m_params.title = title;
    }

    void TkWindow::startTrackingMouse(bool useMouseScreenOffset)
    {
        m_track.isTrackingMouse = true;
        m_track.isUsingMouseOffset = useMouseScreenOffset;
        m_track.mouseOffset = Pos(0, 0);
        m_track.mouseOffsetScreenSize = Rect(Pos(0, 0), System::getDesktopSize());
        m_track.isHorizontalFlippingMouse = false;
        m_track.isVerticalFlippingMouse = false;
    }

    void TkWindow::endTrackingMouse()
    {
        m_track.isTrackingMouse = false;
        m_track.isHorizontalFlippingMouse = false;
        m_track.isVerticalFlippingMouse = false;
    }

    void TkWindow::startClippingCursor(const Rect& bounds)
    {
        (void)bounds;
        m_track.isClippingCursor = true;
    }

    void TkWindow::endClippingCursor()
    {
        m_track.isClippingCursor = false;
    }

    void TkWindow::setCursor(CursorType type)
    {
        if (static_cast<i32>(type) < 0 || type >= CursorType::MAX)
            type = CursorType::kDefault;
        m_cursor = type;
    }

    void TkWindow::setIcon(const TkStringView icon)
    {
        (void)icon;
    }

    DragDropEffect TkWindow::doDragDrop(const TkStringView& data)
    {
        (void)data;
        return DragDropEffect::kNone;
    }

    void TkWindow::onShowTrigger()
    {
        onShown();
    }

    void TkWindow::onResizeTrigger(i32 width, i32 height)
    {
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
        onLostFocusTrigger();
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

    void WindowSystem::init(void* instance)
    {
        (void)instance;
        s_initialized = true;
    }

    void WindowSystem::update()
    {
    }

    TkMutex WindowSystem::WindowSysMtx{};
    TkArray<TkWindow*> WindowSystem::Windows{};

    TkWindow* WindowSystem::getByWindowPtr(void* handle)
    {
        if (handle == nullptr)
            return nullptr;

        WindowSysMtx.lock();
        const auto iterator = s_handleMap.find(handle);
        TkWindow* result = iterator.isEnd() ? nullptr : iterator->value;
        WindowSysMtx.unlock();
        return result;
    }

    TkArray<TkWindow*> WindowSystem::getWindowsSnapshot()
    {
        TkArray<TkWindow*> result;
        WindowSysMtx.lock();
        result = Windows;
        WindowSysMtx.unlock();
        return result;
    }

    bool WindowSystem::isLiveWindow(TkWindow* win)
    {
        if (win == nullptr)
            return false;

        WindowSysMtx.lock();
        const bool result = containsWindowUnlocked(win);
        WindowSysMtx.unlock();
        return result;
    }

    void WindowSystem::unregisterWindowHandle(void* handle)
    {
        if (handle == nullptr)
            return;

        WindowSysMtx.lock();
        s_handleMap.remove(handle);
        WindowSysMtx.unlock();
    }

    void WindowSystem::registerWnd(TkWindow* win)
    {
        if (win == nullptr)
            return;

        WindowSysMtx.lock();
        if (!containsWindowUnlocked(win))
            Windows.add(win);
        syncHandleMapUnlocked(win);
        WindowSysMtx.unlock();
    }

    void WindowSystem::unregisterWnd(TkWindow* win)
    {
        if (win == nullptr)
            return;

        WindowSysMtx.lock();
        Windows.remove(win);
        removeHandleMapEntriesForWindowUnlocked(win);
        WindowSysMtx.unlock();
    }

    TkWindow* WindowSystem::getMainWindow()
    {
        WindowSysMtx.lock();
        TkWindow* result = nullptr;
        for (TkWindow* window : Windows)
        {
            if (window != nullptr && !window->isClosed())
            {
                result = window;
                break;
            }
        }
        WindowSysMtx.unlock();
        return result;
    }

    void WindowSystem::dispose()
    {
        TkArray<TkWindow*> windows = getWindowsSnapshot();
        for (TkWindow* window : windows)
        {
            if (window != nullptr)
                destroyWindow(window);
        }

        WindowSysMtx.lock();
        Windows.clear();
        s_handleMap.clear();
        WindowSysMtx.unlock();
        s_initialized = false;
    }

    bool WindowSystem::getHasFocus()
    {
        for (TkWindow* window : getWindowsSnapshot())
        {
            if (window != nullptr && window->isFocused())
                return true;
        }
        return false;
    }

    TkWindow* WindowSystem::createWindow(const WindowParams& p)
    {
        WindowParams resolvedParams = resolveWindowParams(p);
        IAllocator& alloc = getWindowAllocator(resolvedParams);
        auto* ptr = TkMemory::newObject<TkWindow>(alloc, resolvedParams);
        if (ptr == nullptr)
            return nullptr;

        if (!WindowSystem::createWindow(ptr))
        {
            TkMemory::deletePtr(alloc, ptr);
            return nullptr;
        }
        return ptr;
    }

    bool WindowSystem::createWindow(TkWindow* window)
    {
        if (window == nullptr)
            return false;

        if (!s_initialized)
            WindowSystem::init(nullptr);

        WindowParams resolvedParams = resolveWindowParams(window->m_params);
        window->m_params = resolvedParams;
        window->m_clientSize = resolvedParams.size;

        IAllocator& alloc = getWindowAllocator(resolvedParams);
        auto* nativeWnd = TkMemory::newObject<NativeWindow>(alloc, resolvedParams, window);
        if (nativeWnd == nullptr)
            return false;

        window->m_native = nativeWnd;
        WindowSystem::registerWnd(window);
        return true;
    }

    void WindowSystem::destroyWindow(TkWindow* window)
    {
        if (window == nullptr)
            return;

        IAllocator& alloc = getWindowAllocator(window->getParams());
        window->onBeforeDestroy();
        if (window->getNativeWindow() != nullptr)
        {
            TkMemory::deletePtr(alloc, window->getNativeWindow());
            window->m_native = nullptr;
        }
        WindowSystem::unregisterWnd(window);
        TkMemory::deletePtr(alloc, window);
    }

    TkWindow::TkWindow(const WindowParams& p)
        : m_events{},
          m_native{nullptr},
          m_state{},
          m_track{},
          m_params{p},
          m_dpi{DefaultDPI},
          m_dpiScale{1.0f},
          m_cursor{CursorType::kDefault},
          m_clientSize{p.size}
    {
        m_state.visible = false;
        m_state.minimized = false;
        m_state.maximized = false;
        m_state.fullscreen = p.fullscreen;
        m_state.isClosing = false;
        m_state.closed = false;
        m_state.showAfterFirstPaint = p.showAfterFirstPaint;
        m_state.focused = false;
        m_state.suppressFocusEvents = false;
        m_track.mouseOffset = Pos(0, 0);
    }

    TkWindow::~TkWindow()
    {
    }

    void TkWindow::onBeforeDestroy()
    {
    }

    DialogResult Dialog::fromPlatformDialogResult(const int result)
    {
        return result == 0 ? DialogResult::kCancel : DialogResult::kOK;
    }

    DialogResult Dialog::msgBox(TkWindow* parent, const TkStringView& text, const TkStringView& caption, MsgBoxButtons buttons, MsgBoxIcon icon)
    {
        (void)parent;
        (void)text;
        (void)caption;
        (void)buttons;
        (void)icon;
        return DialogResult::kOK;
    }

    bool Dialog::showOpenFileDialog(TkWindow* parentWindow, const TkStringView& initialDirectory, const TkStringView& filter, bool multiSelect, const TkStringView& title, TkArray<TkString>& filenames)
    {
        (void)parentWindow;
        (void)initialDirectory;
        (void)filter;
        (void)multiSelect;
        (void)title;
        (void)filenames;
        return false;
    }

    bool Dialog::showSaveFileDialog(TkWindow* parentWindow, const TkStringView& initialDirectory, const TkStringView& filter, bool multiSelect, const TkStringView& title, TkArray<TkString>& filenames)
    {
        (void)parentWindow;
        (void)initialDirectory;
        (void)filter;
        (void)multiSelect;
        (void)title;
        (void)filenames;
        return false;
    }

    bool Dialog::showBrowseFolderDialog(TkWindow* parentWindow, const TkStringView& initialDirectory, const TkStringView& title, TkString& path)
    {
        (void)parentWindow;
        (void)initialDirectory;
        (void)title;
        (void)path;
        return false;
    }

    bool Dialog::showFileExplorer(const TkStringView& path)
    {
        if (!path.hasChars())
            return false;
        return System::openUrl(path);
    }
}

extern "C" TK_API void TkAndroidSetNativeWindow(ANativeWindow* window)
{
    toolkit::setAndroidNativeWindow(window);
}

extern "C" TK_API void TkAndroidNotifyWindowSize(int32_t width, int32_t height)
{
    toolkit::notifyAndroidWindowSize(width, height);
}

extern "C" TK_API void TkAndroidSetWindowFocus(int32_t focused)
{
    toolkit::setAndroidWindowFocus(focused != 0);
}

#endif
