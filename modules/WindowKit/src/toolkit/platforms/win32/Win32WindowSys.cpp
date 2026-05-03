/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "Win32WindowSys.hpp"

#if TK_WINDOWS

#include "toolkit/memory/Memory.hpp"
#include "toolkit/platforms/ProcessInitArgs.hpp"
#include "toolkit/types/UUID.hpp"
#include "toolkit/utils/Convertor.hpp"
#include "toolkit/windowing/WindowSystem.hpp"
#include <toolkit/platforms/win32/IncludeWindows.hpp>
#include "toolkit/windowing/Window.hpp"
#include "Win32Window.hpp"
#include "Win32Input.hpp"
#include <toolkit/types/String.hpp>
#include <toolkit/windowing/Dialog.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/platforms/win32/ComPointer.hpp>
#include <toolkit/collection/HashMap.hpp>

#include <ShlObj.h>
#include <ShObjIdl_core.h>
#include <ShellAPI.h>
#include <KnownFolders.h>
#include <commdlg.h>
#undef ShellExecute

namespace toolkit
{
#define IDR_MAINFRAME 100001

    const wchar *Win32WindowSys::s_AppWindowClass = L"ToolkitWindow";
    void *Win32WindowSys::s_Instance = nullptr;

    namespace
    {
        bool s_initialized = false;
        bool s_oleInitialized = false;
        bool s_registeredClass = false;
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

        void syncHandleMapUnlocked(TkWindow* win)
        {
            if (win == nullptr)
                return;

            void* handle = win->getWindowPtr();
            if (handle != nullptr)
                s_handleMap[handle] = win;
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
    }

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        // Find window to process that message
        if (hwnd != nullptr)
        {
            // Find window by handle
            const auto win = WindowSystem::getByWindowPtr(hwnd);
            if (win && win->getNativeWindow())
            {
                return static_cast<TkWindow *>(win)->getNativeWindow()->wndProc(msg, wParam, lParam);
            }
        }

        // Default
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    void WindowSystem::init(void *hInstance)
    {
        TK_ASSERT(hInstance);
        if (s_initialized)
            return;

        // Disable the process from being showing "ghosted" while not responding messages during slow tasks
        DisableProcessWindowsGhosting();
        Win32WindowSys::s_Instance = hInstance;

        // Register window class
        ::WNDCLASSW windowsClass;
        TkMemory::memoryClear(&windowsClass, sizeof(WNDCLASS));
        windowsClass.style = CS_DBLCLKS;
        windowsClass.lpfnWndProc = WndProc;
        windowsClass.hInstance = (HINSTANCE)Win32WindowSys::s_Instance;
        windowsClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        windowsClass.lpszClassName = Win32WindowSys::s_AppWindowClass;
        if (!::RegisterClassW(&windowsClass))
        {
            const DWORD error = ::GetLastError();
            if (error != ERROR_CLASS_ALREADY_EXISTS)
            {
                Win32WindowSys::s_Instance = nullptr;
                TK_ASSERT(false && "Window class registration failed!");
                return;
            }
        }
        else
        {
            s_registeredClass = true;
        }

        // Init OLE
        const HRESULT oleResult = OleInitialize(nullptr);
        if (oleResult == S_OK || oleResult == S_FALSE)
        {
            s_oleInitialized = true;
        }
        else
        {
            if (s_registeredClass)
            {
                ::UnregisterClassW(Win32WindowSys::s_AppWindowClass, (HINSTANCE)Win32WindowSys::s_Instance);
                s_registeredClass = false;
            }
            Win32WindowSys::s_Instance = nullptr;
            TK_ASSERT(false && "OLE initalization failed!");
            return;
        }

        s_initialized = true;
    }

    void WindowSystem::update()
    {
        Win32Input::update();

        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    TkMutex WindowSystem::WindowSysMtx{};
    TkArray<TkWindow *> WindowSystem::Windows{};

    TkWindow *WindowSystem::getByWindowPtr(void *handle)
    {
        if (handle == nullptr)
            return nullptr;

        WindowSysMtx.lock();
        const auto iterator = s_handleMap.find(handle);
        TkWindow* result = iterator.isEnd() ? nullptr : iterator->value;
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
        if (win == nullptr)
            return false;

        WindowSysMtx.lock();
        const bool result = containsWindowUnlocked(win);
        WindowSysMtx.unlock();
        return result;
    }

    void WindowSystem::unregisterWindowHandle(void *handle)
    {
        if (handle == nullptr)
            return;

        WindowSysMtx.lock();
        s_handleMap.remove(handle);
        WindowSysMtx.unlock();
    }

    void WindowSystem::registerWnd(TkWindow *win)
    {
        if (win == nullptr)
            return;

        WindowSysMtx.lock();
        if (!containsWindowUnlocked(win))
            Windows.add(win);
        syncHandleMapUnlocked(win);
        WindowSysMtx.unlock();
    }

    void WindowSystem::unregisterWnd(TkWindow *win)
    {
        if (win == nullptr)
            return;

        WindowSysMtx.lock();
        Windows.remove(win);
        removeHandleMapEntriesForWindowUnlocked(win);
        WindowSysMtx.unlock();
    }

    TkWindow *WindowSystem::getMainWindow()
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
        if (!s_initialized && Windows.isEmpty())
            return;

        TkArray<TkWindow *> windows = getWindowsSnapshot();
        for (TkWindow *window : windows)
        {
            if (window != nullptr)
                destroyWindow(window);
        }

        WindowSysMtx.lock();
        Windows.clear();
        s_handleMap.clear();
        WindowSysMtx.unlock();

        if (s_registeredClass)
        {
            if (!::UnregisterClassW(Win32WindowSys::s_AppWindowClass, (HINSTANCE)Win32WindowSys::s_Instance))
            {
                const DWORD error = ::GetLastError();
                TK_ASSERT(error == ERROR_CLASS_DOES_NOT_EXIST);
            }
            s_registeredClass = false;
        }
        if (s_oleInitialized)
        {
            ::OleUninitialize();
            s_oleInitialized = false;
        }

        Win32WindowSys::s_Instance = nullptr;
        s_initialized = false;
    }

    DialogResult Dialog::fromPlatformDialogResult(const int result)
    {
        switch (result)
        {
        case IDABORT:
            return DialogResult::kAbort;
        case IDCANCEL:
            return DialogResult::kCancel;
        case IDIGNORE:
            return DialogResult::kIgnore;
        case IDNO:
            return DialogResult::kNo;
        case IDOK:
            return DialogResult::kOK;
        case IDRETRY:
            return DialogResult::kRetry;
        case IDYES:
            return DialogResult::kYes;
        default:
            return DialogResult::kNone;
        }
    }

    DialogResult Dialog::msgBox(TkWindow *parent, const TkStringView &text, const TkStringView &caption, MsgBoxButtons buttons, MsgBoxIcon icon)
    {
        // Construct input flags
        UINT flags = 0;
        switch (buttons)
        {
        case MsgBoxButtons::kAbortRetryIgnore:
            flags |= MB_ABORTRETRYIGNORE;
            break;
        case MsgBoxButtons::kOK:
            flags |= MB_OK;
            break;
        case MsgBoxButtons::kOKCancel:
            flags |= MB_OKCANCEL;
            break;
        case MsgBoxButtons::kRetryCancel:
            flags |= MB_RETRYCANCEL;
            break;
        case MsgBoxButtons::kYesNo:
            flags |= MB_YESNO;
            break;
        case MsgBoxButtons::kYesNoCancel:
            flags |= MB_YESNOCANCEL;
            break;
        default:
            break;
        }
        switch (icon)
        {
        case MsgBoxIcon::kAsterisk:
            flags |= MB_ICONASTERISK;
            break;
        case MsgBoxIcon::kError:
            flags |= MB_ICONERROR;
            break;
        case MsgBoxIcon::kExclamation:
            flags |= MB_ICONEXCLAMATION;
            break;
        case MsgBoxIcon::kHand:
            flags |= MB_ICONHAND;
            break;
        case MsgBoxIcon::kInfo:
            flags |= MB_ICONINFORMATION;
            break;
        case MsgBoxIcon::kQuestion:
            flags |= MB_ICONQUESTION;
            break;
        case MsgBoxIcon::kStop:
            flags |= MB_ICONSTOP;
            break;
        case MsgBoxIcon::kWarning:
            flags |= MB_ICONWARNING;
            break;
        default:
            break;
        }
        flags |= MB_TASKMODAL;

        // Show dialog
        int result = MessageBoxW(parent ? static_cast<HWND>(parent->getWindowPtr()) : nullptr, StringAsUTF16<>(text.get()).get(), StringAsUTF16<>(caption.get()).get(), flags);

        return fromPlatformDialogResult(result);
    }

    bool WindowSystem::getHasFocus()
    {
        DWORD foregroundProcess;
        GetWindowThreadProcessId(GetForegroundWindow(), &foregroundProcess);
        return foregroundProcess == ::GetCurrentProcessId();
    }

    TkWindow *WindowSystem::createWindow(const WindowParams &p)
    {
        WindowParams resolvedParams = p;
        if (resolvedParams.startPos == WindowStartPos::kCenterParent || resolvedParams.startPos == WindowStartPos::kCenterScreen)
        {
            Rect parentBounds = Rect(Pos{0, 0}, System::getDesktopSize());
            if (resolvedParams.parent != nullptr && resolvedParams.startPos == WindowStartPos::kCenterParent)
                parentBounds = resolvedParams.parent->getClientBounds();

            resolvedParams.pos = Pos(parentBounds.location) + (Pos(parentBounds.size) - resolvedParams.size) * 0.5f;
        }

        IAllocator& alloc = getWindowAllocator(resolvedParams);
        auto* ptr = TkMemory::newObject<TkWindow>(alloc, resolvedParams);
        if (ptr == nullptr)
            return nullptr;

        auto* nativeWnd = TkMemory::newObject<NativeWindow>(alloc, resolvedParams, ptr);
        if (nativeWnd == nullptr)
        {
            TkMemory::deletePtr(alloc, ptr);
            return nullptr;
        }

        ptr->m_native = nativeWnd;

        if (!nativeWnd->hasHWND())
        {
            ptr->m_native = nullptr;
            TkMemory::deletePtr(alloc, nativeWnd);
            TkMemory::deletePtr(alloc, ptr);
            return nullptr;
        }

        WindowSystem::registerWnd(ptr);
        return ptr;
    }

    void WindowSystem::destroyWindow(TkWindow* w)
    {
        if (w == nullptr)
            return;

        IAllocator& alloc = getWindowAllocator(w->getParams());
        if (w->getNativeWindow()) {
            TkMemory::deletePtr(alloc, w->getNativeWindow());
            w->m_native = nullptr;
        }
        WindowSystem::unregisterWnd(w);
        TkMemory::deletePtr(alloc, w);
    }

    TkWindow::TkWindow(const WindowParams &p)
        : m_events{},
          m_native{nullptr},
          m_state{},
          m_track{},
          m_params{p},
          m_dpi{96},
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
        m_track.mouseOffset = {0, 0};

    }

    TkWindow::~TkWindow()
    {
    }

    bool Dialog::showOpenFileDialog(TkWindow *parentWindow, const TkStringView &initialDirectory,
                                    const TkStringView &filter, bool multiSelect, const TkStringView &title,
                                    TkArray<TkString> &filenames)
    {
        bool result = false;

        // Allocate memory for the filenames
        const i32 maxFilenamesSize = (multiSelect ? 200 : 2) * MAX_PATH;
        TkArray<wchar> fileNamesBuffer;
        fileNamesBuffer.resize(maxFilenamesSize);
        fileNamesBuffer[0] = 0;

        // Setup description
        ::OPENFILENAMEW of;
        ZeroMemory(&of, sizeof(of));
        of.lStructSize = sizeof(of);
        of.lpstrFilter = filter.hasChars() ? StringAsUTF16<>(filter.get()).get() : nullptr;
        of.lpstrFile = fileNamesBuffer.get();
        of.nMaxFile = maxFilenamesSize;
        of.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ENABLESIZING | OFN_NOCHANGEDIR;
        of.lpstrTitle = title.hasChars() ? StringAsUTF16<>(title.get()).get() : nullptr;
        of.lpstrInitialDir = initialDirectory.hasChars() ? StringAsUTF16<>(initialDirectory.get()).get() : nullptr;
        if (parentWindow && WindowSystem::isLiveWindow(parentWindow))
            of.hwndOwner = static_cast<HWND>(parentWindow->getWindowPtr());
        if (multiSelect)
            of.Flags |= OFN_ALLOWMULTISELECT;

        // Show dialog
        if (GetOpenFileNameW(&of) != 0)
        {
            // Get filenames
            wchar *ptr = of.lpstrFile;
            ptr[of.nFileOffset - 1] = 0;
            TkString directory = StringAsUTF8<>(ptr).get();
            ptr += of.nFileOffset;
            while (*ptr)
            {
                TkString filename = StringAsUTF8<>(ptr).get();
                filenames.add(directory / filename);
                ptr += lstrlenW(ptr);
                if (multiSelect)
                    ptr++;
            }

            result = true;
        }

        return result;
    }

    bool Dialog::showSaveFileDialog(TkWindow *parentWindow, const TkStringView &initialDirectory, const TkStringView &filter, bool multiSelect, const TkStringView &title, TkArray<TkString> &filenames)
    {
        bool result = false;

        // Allocate memory for the filenames
        i32 maxFilenamesSize = (multiSelect ? 200 : 2) * MAX_PATH;
        TkArray<wchar> fileNamesBuffer;
        fileNamesBuffer.resize(maxFilenamesSize);
        fileNamesBuffer[0] = 0;

        // Setup description
        ::OPENFILENAMEW of;
        ZeroMemory(&of, sizeof(of));
        of.lStructSize = sizeof(of);
        of.lpstrFilter = filter.hasChars() ? StringAsUTF16<>(filter.get()).get() : nullptr;
        of.lpstrFile = fileNamesBuffer.get();
        of.nMaxFile = maxFilenamesSize;
        of.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
        of.lpstrTitle = title.hasChars() ? StringAsUTF16<>(title.get()).get() : nullptr;
        of.lpstrInitialDir = initialDirectory.hasChars() ? StringAsUTF16<>(initialDirectory.get()).get() : nullptr;
        if (parentWindow && WindowSystem::isLiveWindow(parentWindow))
            of.hwndOwner = static_cast<HWND>(parentWindow->getWindowPtr());
        if (multiSelect)
            of.Flags |= OFN_ALLOWMULTISELECT;

        // Show dialog
        if (GetSaveFileNameW(&of) != 0)
        {
            // Get filenames
            wchar *ptr = of.lpstrFile;
            ptr[of.nFileOffset - 1] = 0;
            TkString directory = StringAsUTF8<>(ptr).get();
            ptr += of.nFileOffset;
            while (*ptr)
            {
                TkString filename = StringAsUTF8<>(ptr).get();
                filenames.add(directory / filename);
                ptr += lstrlenW(ptr);
                if (multiSelect)
                    ptr++;
            }

            result = true;
        }

        return result;
    }

    bool Dialog::showBrowseFolderDialog(TkWindow *parentWindow, const TkStringView &initialDirectory, const TkStringView &title, TkString &path)
    {
        bool result = false;

        // Randomly generated GUID used for storing the last location of this dialog
        const TkUuid folderGuid(0x53890ed9, 0xa55e47ba, 0xa970bdae, 0x72acedff);

        ComPtr<IFileOpenDialog> fd;
        if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fd))))
        {
            DWORD options;
            fd->GetOptions(&options);
            fd->SetOptions(options | FOS_PICKFOLDERS | FOS_NOCHANGEDIR);

            if (title.hasChars())
                fd->SetTitle(StringAsUTF16<>(title.get()).get());

            // Associate the last selected folder with this GUID instead of overwriting the global one
            fd->SetClientGuid(*reinterpret_cast<const GUID *>(&folderGuid));

            ComPtr<IShellItem> defaultFolder;
            if (initialDirectory.hasChars() && SUCCEEDED(SHCreateItemFromParsingName(StringAsUTF16<>(initialDirectory.get()).get(), NULL, IID_PPV_ARGS(&defaultFolder))))
                fd->SetFolder(defaultFolder);

            HWND hwndOwner = parentWindow && WindowSystem::isLiveWindow(parentWindow) && parentWindow->getNativeWindow() ? parentWindow->getNativeWindow()->getHWND() : NULL;
            if (SUCCEEDED(fd->Show(hwndOwner)))
            {
                ComPtr<IShellItem> si;
                if (SUCCEEDED(fd->GetResult(&si)))
                {
                    LPWSTR resultPath;
                    if (SUCCEEDED(si->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &resultPath)))
                    {
                        path = StringAsUTF8<>(resultPath).get();
                        CoTaskMemFree(resultPath);
                        result = true;
                    }
                }
            }
        }

        return result;
    }

    bool Dialog::showFileExplorer(const TkStringView &path)
    {
        if (!path.hasChars())
            return false;
        ProcessInitArgs procSettings;
        procSettings.file = path;
        procSettings.hiddenWindow = false;
        procSettings.waitForQuit = false;
        procSettings.logOutput = false;
        procSettings.shellExecute = true;
        return System::newProcess(procSettings) != 0;
    }
}

#endif
