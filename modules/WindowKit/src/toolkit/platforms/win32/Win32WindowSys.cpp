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
        Win32WindowSys::s_Instance = hInstance;

        // Disable the process from being showing "ghosted" while not responding messages during slow tasks
        DisableProcessWindowsGhosting();

        // Register window class
        ::WNDCLASSW windowsClass;
        TkMemory::memoryClear(&windowsClass, sizeof(WNDCLASS));
        windowsClass.style = CS_DBLCLKS;
        windowsClass.lpfnWndProc = WndProc;
        windowsClass.hInstance = (HINSTANCE)Win32WindowSys::s_Instance;
        windowsClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        windowsClass.lpszClassName = Win32WindowSys::s_AppWindowClass;
        if (!::RegisterClassW(&windowsClass))
            TK_ASSERT(false && "Window class registration failed!");

        // Init OLE
        if (OleInitialize(nullptr) != S_OK)
            TK_ASSERT(false && "OLE initalization failed!");
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
        TkWindow *result = nullptr;
        WindowSysMtx.lock();
        for (i32 i = 0; i < Windows.count(); i++)
        {
            TkWindow *window = Windows[i];
            if (window == nullptr || window->getNativeWindow() == nullptr)
                continue;

            if (window->getWindowPtr() == handle)
            {
                result = window;
                break;
            }
        }
        WindowSysMtx.unlock();
        return result;
    }

    void WindowSystem::registerWnd(TkWindow *win)
    {
        WindowSysMtx.lock();
        Windows.add(win);
        WindowSysMtx.unlock();
    }

    void WindowSystem::unregisterWnd(TkWindow *win)
    {
        WindowSysMtx.lock();
        Windows.remove(win);
        WindowSysMtx.unlock();
    }

    TkWindow *WindowSystem::getMainWindow()
    {
        if (Windows.count() > 0)
            return Windows[0];
        return nullptr;
    }

    void WindowSystem::dispose()
    {
        TkArray<TkWindow *> windows;
        WindowSysMtx.lock();
        windows = Windows;
        WindowSysMtx.unlock();

        for (TkWindow *window : windows)
        {
            if (window != nullptr)
                destroyWindow(window);
        }

        ::UnregisterClassW(Win32WindowSys::s_AppWindowClass, nullptr);
        ::OleUninitialize();
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

        // Translate result to dialog result
        DialogResult dialogResult;
        switch (result)
        {
        case IDABORT:
            dialogResult = DialogResult::kAbort;
            break;
        case IDCANCEL:
            dialogResult = DialogResult::kCancel;
            break;
        case IDCONTINUE:
            dialogResult = DialogResult::kOK;
            break;
        case IDIGNORE:
            dialogResult = DialogResult::kIgnore;
            break;
        case IDNO:
            dialogResult = DialogResult::kNo;
            break;
        case IDOK:
            dialogResult = DialogResult::kOK;
            break;
        case IDRETRY:
            dialogResult = DialogResult::kRetry;
            break;
        case IDYES:
            dialogResult = DialogResult::kYes;
            break;
        default:
            dialogResult = DialogResult::kNone;
            break;
        }
        return dialogResult;
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

        IAllocator& alloc = resolvedParams.alloc;
        auto* ptr = TkMemory::newObject<TkWindow>(alloc, resolvedParams);
        auto* nativeWnd = TkMemory::newObject<NativeWindow>(alloc, resolvedParams, ptr);
        ptr->m_native = nativeWnd;
        
        return ptr;
    }

    void WindowSystem::destroyWindow(TkWindow* w)
    {
        IAllocator& alloc = w->getParams().alloc;
        if (w->getNativeWindow()) {
            TkMemory::deletePtr(alloc, w->getNativeWindow());
        }
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
        m_state.showAfterFirstPaint = p.showAfterFirstPaint;
        m_state.focused = false;
        m_state.suppressFocusEvents = false;
        m_track.mouseOffset = {0, 0};

        WindowSystem::registerWnd((TkWindow *)this);
    }

    TkWindow::~TkWindow()
    {
        WindowSystem::unregisterWnd((TkWindow *)this);
    }

    bool Dialog::showOpenFileDialog(TkWindow *parentWindow, const TkStringView &initialDirectory,
                                    const TkStringView &filter, bool multiSelect, const TkStringView &title,
                                    TkArray<TkString> &filenames)
    {
        bool result = true;

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
        if (parentWindow)
            of.hwndOwner = static_cast<HWND>(parentWindow->getWindowPtr());
        if (multiSelect)
            of.Flags |= OFN_ALLOWMULTISELECT;

        // Show dialog
        if (GetOpenFileNameW(&of) != 0)
        {
            // Get filenames
            wchar *ptr = of.lpstrFile;
            ptr[of.nFileOffset - 1] = 0;
            const char *uptr = StringAsUTF8<>(ptr).get();
            TkString directory = TkString(uptr);
            ptr += of.nFileOffset;
            while (*ptr)
            {
                filenames.add(directory / uptr);
                ptr += lstrlenW(ptr);
                if (multiSelect)
                    ptr++;
            }

            result = false;
        }

        return result;
    }

    bool Dialog::showSaveFileDialog(TkWindow *parentWindow, const TkStringView &initialDirectory, const TkStringView &filter, bool multiSelect, const TkStringView &title, TkArray<TkString> &filenames)
    {
        bool result = true;

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
        if (parentWindow)
            of.hwndOwner = static_cast<HWND>(parentWindow->getWindowPtr());
        if (multiSelect)
            of.Flags |= OFN_ALLOWMULTISELECT;

        // Show dialog
        if (GetSaveFileNameW(&of) != 0)
        {
            // Get filenames
            wchar *ptr = of.lpstrFile;
            ptr[of.nFileOffset - 1] = 0;
            const char *uptr = StringAsUTF8<>(ptr).get();
            TkString directory = TkString(uptr);
            ptr += of.nFileOffset;
            while (*ptr)
            {
                filenames.add(directory / uptr);
                ptr += lstrlenW(ptr);
                if (multiSelect)
                    ptr++;
            }

            result = false;
        }

        return result;
    }

    bool Dialog::showBrowseFolderDialog(TkWindow *parentWindow, const TkStringView &initialDirectory, const TkStringView &title, TkString &path)
    {
        bool result = true;

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
            if (SUCCEEDED(SHCreateItemFromParsingName(StringAsUTF16<>(initialDirectory.get()).get(), NULL, IID_PPV_ARGS(&defaultFolder))))
                fd->SetFolder(defaultFolder);

            HWND hwndOwner = parentWindow ? parentWindow->getNativeWindow()->getHWND() : NULL;
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
                        result = false;
                    }
                }
            }
        }

        return result;
    }

    bool Dialog::showFileExplorer(const TkStringView &path)
    {
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
