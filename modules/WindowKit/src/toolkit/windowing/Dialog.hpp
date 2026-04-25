/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/windowing/common.hpp>
#include <toolkit/types/StringView.hpp>

namespace toolkit
{
    enum class DialogResult
    {
        kAbort = 0,
        kCancel,
        kIgnore,
        kNo,
        kNone,
        kOK,
        kRetry,
        kYes,
    };

    // Specify constants defining which information to display.
    enum class MsgBoxIcon
    {
        kAsterisk,
        kError,
        kExclamation,
        kHand,
        kInfo,
        kNone,
        kQuestion,
        kStop,
        kWarning,
    };

    // Specify constants defining which buttons to display on a Message Box.
    enum class MsgBoxButtons
    {
        kAbortRetryIgnore,
        kOK,
        kOKCancel,
        kRetryCancel,
        kYesNo,
        kYesNoCancel,
    };

    class Dialog
    {
    public:
        static DialogResult fromPlatformDialogResult(int result);

        // Display a message box with specified text.
        TK_FORCE_INLINE static DialogResult msgBox(const TkStringView &text)
        {
            return msgBox(nullptr, text, "Info", MsgBoxButtons::kOK, MsgBoxIcon::kNone);
        }

        // Display a message box with specified text and caption.
        TK_FORCE_INLINE static DialogResult msgBox(const TkStringView &text, const TkStringView &caption)
        {
            return msgBox(nullptr, text, caption, MsgBoxButtons::kOK, MsgBoxIcon::kNone);
        }

        // Display a message box with specified text, caption, buttons, and icon.
        TK_FORCE_INLINE static DialogResult msgBox(const TkStringView &text, const TkStringView &caption, MsgBoxButtons buttons)
        {
            return msgBox(nullptr, text, caption, buttons, MsgBoxIcon::kNone);
        }

        // Display a message box with specified text, caption, buttons, and icon.
        TK_FORCE_INLINE static DialogResult msgBox(const TkStringView &text, const TkStringView &caption, MsgBoxButtons buttons, MsgBoxIcon icon)
        {
            return msgBox(nullptr, text, caption, buttons, icon);
        }

        // Display a message box with specified text.
        TK_FORCE_INLINE static DialogResult msgBox(class TkWindow *parent, const TkStringView &text)
        {
            return msgBox(parent, *text, "Info", MsgBoxButtons::kOK, MsgBoxIcon::kNone);
        }

        // Display a message box with specified text and caption.
        TK_FORCE_INLINE static DialogResult msgBox(TkWindow *parent, const TkStringView &text, const TkStringView &caption)
        {
            return msgBox(parent, *text, *caption, MsgBoxButtons::kOK, MsgBoxIcon::kNone);
        }

        // Display a message box with specified text, caption and buttons.
        TK_FORCE_INLINE static DialogResult msgBox(TkWindow *parent, const TkStringView &text, const TkStringView &caption, MsgBoxButtons buttons)
        {
            return msgBox(parent, text, caption, buttons, MsgBoxIcon::kNone);
        }

        // Display a message box with specified text, caption, buttons, and icon.
        static DialogResult msgBox(TkWindow *parent, const TkStringView &text, const TkStringView &caption, MsgBoxButtons buttons, MsgBoxIcon icon);

    public:
        static bool showOpenFileDialog(TkWindow *parentWindow, const TkStringView &initialDirectory, const TkStringView &filter, bool multiSelect, const TkStringView &title, TkArray<TkString> &filenames);
        static bool showSaveFileDialog(TkWindow *parentWindow, const TkStringView &initialDirectory, const TkStringView &filter, bool multiSelect, const TkStringView &title, TkArray<TkString> &filenames);
        static bool showBrowseFolderDialog(TkWindow *parentWindow, const TkStringView &initialDirectory, const TkStringView &title, TkString &path);
        static bool showFileExplorer(const TkStringView &path);

    };

}
