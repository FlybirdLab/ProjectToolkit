/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/base/Math.hpp"
#include <toolkit/base/Function.hpp>
#include <toolkit/input/InputDevice.hpp>
#include <toolkit/windowing/WindowParams.hpp>

namespace toolkit
{
    class DragDropData;

    class WindowEventGroup
    {
    public:
        typedef TkAction<char> CharEvent;
        typedef TkAction<KeyboardKeys> KeyboardEvent;
        typedef TkAction<const Pos &> MouseEvent;
        typedef TkAction<const Pos &, MouseButton> MouseButtonEvent;
        typedef TkAction<const Pos &, float> MouseWheelEvent;
        typedef TkAction<const Pos &, i32> TouchEvent;
        typedef TkAction<DragDropData *, const Pos &, DragDropEffect &> DragEvent;
        typedef TkAction<const Pos &, WindowHitCodes &, bool &> HitTestEvent;
        typedef TkAction<WindowHitCodes, bool &> ButtonHitEvent;
        typedef TkAction<ClosingReason, bool &> ClosingEvent;

    public:
        // Event fired on character input.
        CharEvent onCharInput;
        // Event fired on key pressed.
        KeyboardEvent onKeyDown;
        // Event fired on key released.
        KeyboardEvent onKeyUp;
        // Event fired when mouse button goes down.
        MouseButtonEvent onMouseDown;
        // Event fired when mouse button goes up.
        MouseButtonEvent onMouseUp;
        // Event fired when mouse button double clicks.
        MouseButtonEvent onMouseDoubleClick;
        // Event fired when mouse wheel is scrolling (wheel delta is normalized).
        MouseWheelEvent onMouseWheel;
        // Event fired when mouse moves.
        MouseEvent onMouseMove;
        // Event fired when mouse leaves window.
        TkAction<> onMouseLeave;
        // Event fired when touch action begins.
        TouchEvent onTouchDown;
        // Event fired when touch action moves.
        TouchEvent onTouchMove;
        // Event fired when touch action ends.
        TouchEvent onTouchUp;
        // Event fired when drag&drop enters window.
        DragEvent onDragEnter;
        // Event fired when drag&drop moves over window.
        DragEvent onDragOver;
        // Event fired when drag&drop ends over window with drop.
        DragEvent onDragDrop;
        // Event fired when drag&drop leaves window.
        TkAction<> onDragLeave;
        // Event fired when system tests if the specified location is part of the window.
        HitTestEvent onHitTest;
        // Event fired when system tests if the left button hit the window for the given hit code.
        ButtonHitEvent onLeftButtonHit;
        // Event fired when window is closing. Can be used to cancel the operation.
        ClosingEvent onClosing;
    };
}