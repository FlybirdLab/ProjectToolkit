/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "LinuxWindowSys.hpp"

#if TK_LINUX

#include "LinuxInput.hpp"
#include "LinuxWindow.hpp"
#include <toolkit/collection/HashMap.hpp>
#include <toolkit/debug/Logger.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/platforms/ProcessInitArgs.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/windowing/Dialog.hpp>
#include <toolkit/windowing/Window.hpp>

namespace toolkit
{
    namespace
    {
        X11::Display *s_display = nullptr;
        X11::Atom s_deleteWindow = 0;
        X11::Atom s_wmState = 0;
        X11::Atom s_wmStateFullscreen = 0;
        TkHashMap<void *, TkWindow *> s_handleMap;
        bool s_initialized = false;

        IAllocator &getWindowAllocator(const WindowParams &params)
        {
            return params.alloc != nullptr ? *params.alloc : TkMemory::getDefault();
        }

        bool containsWindowUnlocked(TkWindow *win)
        {
            for (TkWindow *window : WindowSystem::Windows)
            {
                if (window == win)
                    return true;
            }
            return false;
        }

        void syncHandleMapUnlocked(TkWindow *win)
        {
            if (win && win->getWindowPtr())
                s_handleMap[win->getWindowPtr()] = win;
        }

        void removeHandleMapEntriesForWindowUnlocked(TkWindow *win)
        {
            TkArray<void *> keysToRemove;
            for (auto iterator = s_handleMap.Begin(); iterator.isNotEnd(); ++iterator)
            {
                if (iterator->value == win)
                    keysToRemove.add(iterator->key);
            }
            for (void *key : keysToRemove)
                s_handleMap.remove(key);
        }

        TkString shellQuote(const TkStringView value)
        {
            TkString result("'");
            const TkString text(value);
            for (i32 i = 0; i < text.length(); ++i)
            {
                if (text[i] == '\'')
                    result += "'\\''";
                else
                    result += text[i];
            }
            result += "'";
            return result;
        }

        bool runDialogCommand(const TkString &args, TkString *output = nullptr)
        {
            ProcessInitArgs process;
            process.file = "sh";
            process.args = "-c " + shellQuote(args);
            process.waitForQuit = true;
            process.logOutput = false;
            process.saveOutput = output != nullptr;
            const i32 code = System::newProcess(process);
            if (output)
            {
                output->clear();
                if (process.output.count() > 0)
                {
                    output->set(process.output.get(), process.output.count());
                    while (output->length() > 0 && ((*output)[output->length() - 1] == '\n' || (*output)[output->length() - 1] == '\r'))
                        output->resize(output->length() - 1);
                }
            }
            return code == 0;
        }

        WindowParams resolveWindowParams(const WindowParams &p)
        {
            WindowParams resolvedParams = p;
            if (resolvedParams.startPos == WindowStartPos::kCenterParent || resolvedParams.startPos == WindowStartPos::kCenterScreen)
            {
                Rect parentBounds = Rect(Pos{0, 0}, Pos(800, 600));
                if (s_display)
                {
                    const i32 screen = X11::XDefaultScreen(s_display);
                    parentBounds = Rect(0, 0, static_cast<float>(X11::XDisplayWidth(s_display, screen)), static_cast<float>(X11::XDisplayHeight(s_display, screen)));
                }
                if (resolvedParams.parent != nullptr && resolvedParams.startPos == WindowStartPos::kCenterParent)
                    parentBounds = resolvedParams.parent->getClientBounds();
                resolvedParams.pos = Pos(parentBounds.location) + (Pos(parentBounds.size) - resolvedParams.size) * 0.5f;
            }
            return resolvedParams;
        }

        void appendDialogOutputFiles(const TkString &output, TkArray<TkString> &filenames)
        {
            const char *start = output.getText();
            const char *cursor = start;
            while (*cursor)
            {
                if (*cursor == '\n')
                {
                    if (cursor > start)
                        filenames.add(TkString(start, static_cast<i32>(cursor - start)));
                    start = cursor + 1;
                }
                ++cursor;
            }
            if (cursor > start)
                filenames.add(TkString(start, static_cast<i32>(cursor - start)));
        }
    }

    X11::Display *LinuxWindowSys::getDisplay()
    {
        return s_display;
    }

    X11::Atom LinuxWindowSys::getDeleteWindowAtom()
    {
        return s_deleteWindow;
    }

    X11::Atom LinuxWindowSys::getWmStateAtom()
    {
        return s_wmState;
    }

    X11::Atom LinuxWindowSys::getWmStateFullscreenAtom()
    {
        return s_wmStateFullscreen;
    }

    TkMutex WindowSystem::WindowSysMtx {};
    TkArray<TkWindow *> WindowSystem::Windows {};

    void WindowSystem::init(void *)
    {
        if (s_initialized)
            return;
        s_display = X11::XOpenDisplay(nullptr);
        if (!s_display)
        {
            LogError("[LinuxWindowSys] XOpenDisplay failed.");
            return;
        }
        s_deleteWindow = X11::XInternAtom(s_display, "WM_DELETE_WINDOW", 0);
        s_wmState = X11::XInternAtom(s_display, "_NET_WM_STATE", 0);
        s_wmStateFullscreen = X11::XInternAtom(s_display, "_NET_WM_STATE_FULLSCREEN", 0);
        LinuxInput::init();
        s_initialized = true;
    }

    void WindowSystem::update()
    {
        if (!s_display)
            return;
        LinuxInput::update();
        while (X11::XPending(s_display) > 0)
        {
            X11::XEvent event {};
            X11::XNextEvent(s_display, &event);
            TkWindow *window = WindowSystem::getByWindowPtr(reinterpret_cast<void *>(event.xany.window));
            if (window && window->getNativeWindow())
                window->getNativeWindow()->handleEvent(event);
        }
    }

    void WindowSystem::dispose()
    {
        TkArray<TkWindow *> windows = getWindowsSnapshot();
        for (TkWindow *window : windows)
        {
            if (window)
                destroyWindow(window);
        }

        WindowSysMtx.lock();
        Windows.clear();
        s_handleMap.clear();
        WindowSysMtx.unlock();

        if (s_display)
        {
            X11::XCloseDisplay(s_display);
            s_display = nullptr;
        }
        s_initialized = false;
    }

    bool WindowSystem::getHasFocus()
    {
        if (!s_display)
            return false;
        X11::Window focused = 0;
        i32 revert = 0;
        X11::XGetInputFocus(s_display, &focused, &revert);
        return getByWindowPtr(reinterpret_cast<void *>(focused)) != nullptr;
    }

    TkWindow *WindowSystem::getByWindowPtr(void *handle)
    {
        if (!handle)
            return nullptr;
        WindowSysMtx.lock();
        const auto iterator = s_handleMap.find(handle);
        TkWindow *result = iterator.isEnd() ? nullptr : iterator->value;
        WindowSysMtx.unlock();
        return result;
    }

    TkArray<TkWindow *> WindowSystem::getWindowsSnapshot()
    {
        TkArray<TkWindow *> result;
        WindowSysMtx.lock();
        result = Windows;
        WindowSysMtx.unlock();
        return result;
    }

    bool WindowSystem::isLiveWindow(TkWindow *win)
    {
        WindowSysMtx.lock();
        const bool result = containsWindowUnlocked(win);
        WindowSysMtx.unlock();
        return result;
    }

    void WindowSystem::unregisterWindowHandle(void *handle)
    {
        WindowSysMtx.lock();
        s_handleMap.remove(handle);
        WindowSysMtx.unlock();
    }

    void WindowSystem::registerWnd(TkWindow *win)
    {
        if (!win)
            return;
        WindowSysMtx.lock();
        if (!containsWindowUnlocked(win))
            Windows.add(win);
        syncHandleMapUnlocked(win);
        WindowSysMtx.unlock();
    }

    void WindowSystem::unregisterWnd(TkWindow *win)
    {
        if (!win)
            return;
        WindowSysMtx.lock();
        Windows.remove(win);
        removeHandleMapEntriesForWindowUnlocked(win);
        WindowSysMtx.unlock();
    }

    TkWindow *WindowSystem::getMainWindow()
    {
        WindowSysMtx.lock();
        TkWindow *result = nullptr;
        for (TkWindow *window : Windows)
        {
            if (window && !window->isClosed())
            {
                result = window;
                break;
            }
        }
        WindowSysMtx.unlock();
        return result;
    }

    TkWindow *WindowSystem::createWindow(const WindowParams &p)
    {
        WindowParams resolvedParams = resolveWindowParams(p);
        IAllocator &alloc = getWindowAllocator(resolvedParams);
        auto *ptr = TkMemory::newObject<TkWindow>(alloc, resolvedParams);
        if (!ptr)
            return nullptr;
        if (!WindowSystem::createWindow(ptr))
        {
            TkMemory::deletePtr(alloc, ptr);
            return nullptr;
        }
        return ptr;
    }

    bool WindowSystem::createWindow(TkWindow *window)
    {
        if (!window)
            return false;
        if (!s_initialized)
            WindowSystem::init(nullptr);
        if (!s_display)
            return false;

        WindowParams resolvedParams = resolveWindowParams(window->m_params);
        window->m_params = resolvedParams;
        window->m_clientSize = resolvedParams.size;
        IAllocator &alloc = getWindowAllocator(resolvedParams);
        auto *nativeWnd = TkMemory::newObject<NativeWindow>(alloc, resolvedParams, window);
        if (!nativeWnd)
            return false;
        if (!nativeWnd->hasXWindow())
        {
            TkMemory::deletePtr(alloc, nativeWnd);
            return false;
        }
        window->m_native = nativeWnd;
        WindowSystem::registerWnd(window);
        return true;
    }

    void WindowSystem::destroyWindow(TkWindow *w)
    {
        if (!w)
            return;
        IAllocator &alloc = getWindowAllocator(w->getParams());
        w->onBeforeDestroy();
        if (w->getNativeWindow())
        {
            TkMemory::deletePtr(alloc, w->getNativeWindow());
            w->m_native = nullptr;
        }
        WindowSystem::unregisterWnd(w);
        TkMemory::deletePtr(alloc, w);
    }

    TkWindow::TkWindow(const WindowParams &p)
        : m_events {},
          m_native {nullptr},
          m_state {},
          m_track {},
          m_params {p},
          m_dpi {96},
          m_dpiScale {1.0f},
          m_cursor {CursorType::kDefault},
          m_clientSize {p.size}
    {
        m_state.showAfterFirstPaint = p.showAfterFirstPaint;
        m_state.fullscreen = p.fullscreen;
        m_track.mouseOffset = {0, 0};
    }

    TkWindow::~TkWindow()
    {
    }

    void TkWindow::onBeforeDestroy()
    {
    }

    DialogResult Dialog::fromPlatformDialogResult(int result)
    {
        return result == 0 ? DialogResult::kOK : DialogResult::kCancel;
    }

    DialogResult Dialog::msgBox(TkWindow *, const TkStringView &text, const TkStringView &caption, MsgBoxButtons, MsgBoxIcon)
    {
        const TkString command = "zenity --info --title=" + shellQuote(caption) + " --text=" + shellQuote(text) + " 2>/dev/null || xmessage " + shellQuote(text);
        return runDialogCommand(command) ? DialogResult::kOK : DialogResult::kCancel;
    }

    bool Dialog::showOpenFileDialog(TkWindow *, const TkStringView &initialDirectory, const TkStringView &, bool multiSelect, const TkStringView &title, TkArray<TkString> &filenames)
    {
        TkString command = "zenity --file-selection";
        if (title.hasChars())
            command += " --title=" + shellQuote(title);
        if (initialDirectory.hasChars())
            command += " --filename=" + shellQuote(initialDirectory);
        if (multiSelect)
            command += " --multiple --separator='\\n'";

        TkString output;
        if (!runDialogCommand(command, &output) || output.isEmpty())
            return false;
        appendDialogOutputFiles(output, filenames);
        return !filenames.isEmpty();
    }

    bool Dialog::showSaveFileDialog(TkWindow *, const TkStringView &initialDirectory, const TkStringView &, bool, const TkStringView &title, TkArray<TkString> &filenames)
    {
        TkString command = "zenity --file-selection --save --confirm-overwrite";
        if (title.hasChars())
            command += " --title=" + shellQuote(title);
        if (initialDirectory.hasChars())
            command += " --filename=" + shellQuote(initialDirectory);

        TkString output;
        if (!runDialogCommand(command, &output) || output.isEmpty())
            return false;
        filenames.add(output);
        return true;
    }

    bool Dialog::showBrowseFolderDialog(TkWindow *, const TkStringView &initialDirectory, const TkStringView &title, TkString &path)
    {
        TkString command = "zenity --file-selection --directory";
        if (title.hasChars())
            command += " --title=" + shellQuote(title);
        if (initialDirectory.hasChars())
            command += " --filename=" + shellQuote(initialDirectory);

        return runDialogCommand(command, &path) && path.hasChars();
    }

    bool Dialog::showFileExplorer(const TkStringView &path)
    {
        if (!path.hasChars())
            return false;
        const TkString command = "xdg-open " + shellQuote(path);
        return runDialogCommand(command);
    }
}

#endif
