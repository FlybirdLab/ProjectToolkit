/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/base/Math.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/types/String.hpp>
#include <toolkit/windowing/common.hpp>

namespace toolkit
{
    class TkWindow;

    enum class WindowStartPos
    {
        kCenterParent,
        kCenterScreen,
        kManual,
    };

    struct WindowParams
    {
        IAllocator& alloc = TkMemory::getDefault();
        TkString title;
        TkWindow *parent = nullptr;
        Pos pos = Pos(100, 400);
        Pos size = Pos(640, 480);
        Pos minSize = Pos(1, 1);
        Pos maxSize = Pos(0, 0);
        WindowStartPos startPos = WindowStartPos::kManual;
        bool fullscreen : 1 = false;
        bool hasBorder : 1  = true;
        bool supportsTransparency : 1  = false;
        bool showInTaskbar : 1  = true;
        bool activateWhenFirstShown : 1  = true;
        bool allowInput : 1  = true;
        bool allowMinimize : 1  = true;
        bool allowMaximize : 1  = true;
        bool allowDragAndDrop : 1  = false;
        bool isTopmost : 1  = false;
        bool isRegularWindow : 1  = true;
        bool hasSizingFrame : 1  = true;
        bool showAfterFirstPaint : 1  = true;
        void* data = nullptr;
    };

    // Window closing reasons.
    enum class ClosingReason
    {
        kUnknown = 0,
        kUser,
        kAppExit,
        kCloseEvent,
    };

    // Types of default cursors.
    enum class CursorType
    {
        kDefault = 0,
        kCross,
        kHand,
        kHelp,
        kIBeam,
        kNo,
        kWait,
        kSizeAll,
        kSizeNESW,
        kSizeNS,
        kSizeNWSE,
        kSizeWE,
        kHidden,
        MAX
    };

    // Data drag and drop effects.
    enum class DragDropEffect
    {
        kNone = 0,
        kCopy,
        kMove,
        kLink,
    };

    // Window hit test codes. Note: they are 1:1 mapping for Win32 values.
    enum class WindowHitCodes
    {
        kTransparent = -1,
        kNoWhere = 0,
        kClient = 1,
        kCaption = 2,
        kSystemMenu = 3,
        kGrowBox = 4,
        kMenu = 5,
        kHScroll = 6,
        kVScroll = 7,
        kMinButton = 8,
        kMaxButton = 9,
        kLeft = 10,
        kRight = 11,
        kTop = 12,
        kTopLeft = 13,
        kTopRight = 14,
        kBottom = 15,
        kBottomLeft = 16,
        kBottomRight = 17,
        kBorder = 18,
        kObject = 19,
        kClose = 20,
        kHelp = 21,
    };

}
