/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "window_test_utils.hpp"

TEST(WindowEventsTest, AllDispatchersInvokeBoundCallbacksWithExpectedData)
{
    WindowEventGroup events;

    char receivedChar = 0;
    KeyboardKeys keyDown = KeyboardKeys::kNone;
    KeyboardKeys keyUp = KeyboardKeys::kNone;
    Pos mouseDownPos = Pos::Minimum;
    Pos mouseUpPos = Pos::Minimum;
    Pos mouseDoubleClickPos = Pos::Minimum;
    Pos mouseMovePos = Pos::Minimum;
    Pos mouseWheelPos = Pos::Minimum;
    Pos touchDownPos = Pos::Minimum;
    Pos touchMovePos = Pos::Minimum;
    Pos touchUpPos = Pos::Minimum;
    MouseButton mouseDownButton = MouseButton::kNone;
    MouseButton mouseUpButton = MouseButton::kNone;
    MouseButton mouseDoubleClickButton = MouseButton::kNone;
    float mouseWheelDelta = 0.0f;
    i32 touchDownIndex = -1;
    i32 touchMoveIndex = -1;
    i32 touchUpIndex = -1;
    bool mouseLeaveCalled = false;
    bool dragLeaveCalled = false;
    DragDropEffect dragEnterEffect = DragDropEffect::kNone;
    DragDropEffect dragOverEffect = DragDropEffect::kNone;
    DragDropEffect dragDropEffect = DragDropEffect::kNone;
    Pos dragEnterPos = Pos::Minimum;
    Pos dragOverPos = Pos::Minimum;
    Pos dragDropPos = Pos::Minimum;
    WindowHitCodes hitResult = WindowHitCodes::kClient;
    bool hitHandled = false;
    WindowHitCodes hitButtonCode = WindowHitCodes::kClient;
    bool leftButtonHandled = false;
    ClosingReason closingReason = ClosingReason::kUnknown;
    bool closingCancelled = false;

    events.onCharInput.bind([&](char c) { receivedChar = c; });
    events.onKeyDown.bind([&](KeyboardKeys key) { keyDown = key; });
    events.onKeyUp.bind([&](KeyboardKeys key) { keyUp = key; });
    events.onMouseDown.bind([&](const Pos &pos, MouseButton button) { mouseDownPos = pos; mouseDownButton = button; });
    events.onMouseUp.bind([&](const Pos &pos, MouseButton button) { mouseUpPos = pos; mouseUpButton = button; });
    events.onMouseDoubleClick.bind([&](const Pos &pos, MouseButton button) { mouseDoubleClickPos = pos; mouseDoubleClickButton = button; });
    events.onMouseWheel.bind([&](const Pos &pos, float delta) { mouseWheelPos = pos; mouseWheelDelta = delta; });
    events.onMouseMove.bind([&](const Pos &pos) { mouseMovePos = pos; });
    events.onMouseLeave.bind([&]() { mouseLeaveCalled = true; });
    events.onTouchDown.bind([&](const Pos &pos, i32 index) { touchDownPos = pos; touchDownIndex = index; });
    events.onTouchMove.bind([&](const Pos &pos, i32 index) { touchMovePos = pos; touchMoveIndex = index; });
    events.onTouchUp.bind([&](const Pos &pos, i32 index) { touchUpPos = pos; touchUpIndex = index; });
    events.onDragEnter.bind([&](DragDropData *, const Pos &pos, DragDropEffect &effect) { dragEnterPos = pos; effect = DragDropEffect::kCopy; dragEnterEffect = effect; });
    events.onDragOver.bind([&](DragDropData *, const Pos &pos, DragDropEffect &effect) { dragOverPos = pos; effect = DragDropEffect::kMove; dragOverEffect = effect; });
    events.onDragDrop.bind([&](DragDropData *, const Pos &pos, DragDropEffect &effect) { dragDropPos = pos; effect = DragDropEffect::kLink; dragDropEffect = effect; });
    events.onDragLeave.bind([&]() { dragLeaveCalled = true; });
    events.onHitTest.bind([&](const Pos &, WindowHitCodes &result, bool &handled) { result = WindowHitCodes::kCaption; handled = true; hitResult = result; hitHandled = handled; });
    events.onLeftButtonHit.bind([&](WindowHitCodes code, bool &result) { hitButtonCode = code; result = true; leftButtonHandled = result; });
    events.onClosing.bind([&](ClosingReason reason, bool &cancel) { closingReason = reason; cancel = true; closingCancelled = cancel; });

    DragDropEffect dragEnter = DragDropEffect::kNone;
    DragDropEffect dragOver = DragDropEffect::kNone;
    DragDropEffect dragDrop = DragDropEffect::kNone;
    WindowHitCodes hit = WindowHitCodes::kClient;
    bool handled = false;
    bool leftButtonResult = false;
    bool cancel = false;

    events.onCharInput('Z');
    events.onKeyDown(KeyboardKeys::kF5);
    events.onKeyUp(KeyboardKeys::kEscape);
    events.onMouseDown(Pos(10.0, 20.0), MouseButton::kLeft);
    events.onMouseUp(Pos(11.0, 21.0), MouseButton::kRight);
    events.onMouseDoubleClick(Pos(12.0, 22.0), MouseButton::kMiddle);
    events.onMouseWheel(Pos(13.0, 23.0), 2.5f);
    events.onMouseMove(Pos(14.0, 24.0));
    events.onMouseLeave();
    events.onTouchDown(Pos(15.0, 25.0), 1);
    events.onTouchMove(Pos(16.0, 26.0), 2);
    events.onTouchUp(Pos(17.0, 27.0), 3);
    events.onDragEnter(nullptr, Pos(18.0, 28.0), dragEnter);
    events.onDragOver(nullptr, Pos(19.0, 29.0), dragOver);
    events.onDragDrop(nullptr, Pos(20.0, 30.0), dragDrop);
    events.onDragLeave();
    events.onHitTest(Pos(21.0, 31.0), hit, handled);
    events.onLeftButtonHit(WindowHitCodes::kClose, leftButtonResult);
    events.onClosing(ClosingReason::kUser, cancel);

    EXPECT_EQ(receivedChar, 'Z');
    EXPECT_EQ(keyDown, KeyboardKeys::kF5);
    EXPECT_EQ(keyUp, KeyboardKeys::kEscape);
    expectPosNear(mouseDownPos, Pos(10.0, 20.0));
    expectPosNear(mouseUpPos, Pos(11.0, 21.0));
    expectPosNear(mouseDoubleClickPos, Pos(12.0, 22.0));
    expectPosNear(mouseWheelPos, Pos(13.0, 23.0));
    expectPosNear(mouseMovePos, Pos(14.0, 24.0));
    EXPECT_EQ(mouseDownButton, MouseButton::kLeft);
    EXPECT_EQ(mouseUpButton, MouseButton::kRight);
    EXPECT_EQ(mouseDoubleClickButton, MouseButton::kMiddle);
    EXPECT_FLOAT_EQ(mouseWheelDelta, 2.5f);
    EXPECT_TRUE(mouseLeaveCalled);
    expectPosNear(touchDownPos, Pos(15.0, 25.0));
    expectPosNear(touchMovePos, Pos(16.0, 26.0));
    expectPosNear(touchUpPos, Pos(17.0, 27.0));
    EXPECT_EQ(touchDownIndex, 1);
    EXPECT_EQ(touchMoveIndex, 2);
    EXPECT_EQ(touchUpIndex, 3);
    expectPosNear(dragEnterPos, Pos(18.0, 28.0));
    expectPosNear(dragOverPos, Pos(19.0, 29.0));
    expectPosNear(dragDropPos, Pos(20.0, 30.0));
    EXPECT_EQ(dragEnter, DragDropEffect::kCopy);
    EXPECT_EQ(dragOver, DragDropEffect::kMove);
    EXPECT_EQ(dragDrop, DragDropEffect::kLink);
    EXPECT_EQ(dragEnterEffect, DragDropEffect::kCopy);
    EXPECT_EQ(dragOverEffect, DragDropEffect::kMove);
    EXPECT_EQ(dragDropEffect, DragDropEffect::kLink);
    EXPECT_TRUE(dragLeaveCalled);
    EXPECT_EQ(hit, WindowHitCodes::kCaption);
    EXPECT_TRUE(handled);
    EXPECT_EQ(hitResult, WindowHitCodes::kCaption);
    EXPECT_TRUE(hitHandled);
    EXPECT_EQ(hitButtonCode, WindowHitCodes::kClose);
    EXPECT_TRUE(leftButtonResult);
    EXPECT_TRUE(leftButtonHandled);
    EXPECT_EQ(closingReason, ClosingReason::kUser);
    EXPECT_TRUE(cancel);
    EXPECT_TRUE(closingCancelled);
}

TEST(WindowEventsTest, EmptyDispatchersAreSafeToCall)
{
    WindowEventGroup events;
    DragDropEffect effect = DragDropEffect::kNone;
    WindowHitCodes hit = WindowHitCodes::kClient;
    bool handled = false;
    bool leftButtonHandled = false;
    bool cancel = false;

    events.onCharInput('A');
    events.onKeyDown(KeyboardKeys::kA);
    events.onKeyUp(KeyboardKeys::kB);
    events.onMouseDown(Pos(1.0, 2.0), MouseButton::kLeft);
    events.onMouseUp(Pos(3.0, 4.0), MouseButton::kRight);
    events.onMouseDoubleClick(Pos(5.0, 6.0), MouseButton::kMiddle);
    events.onMouseWheel(Pos(7.0, 8.0), 1.0f);
    events.onMouseMove(Pos(9.0, 10.0));
    events.onMouseLeave();
    events.onTouchDown(Pos(11.0, 12.0), 1);
    events.onTouchMove(Pos(13.0, 14.0), 2);
    events.onTouchUp(Pos(15.0, 16.0), 3);
    events.onDragEnter(nullptr, Pos(17.0, 18.0), effect);
    events.onDragOver(nullptr, Pos(19.0, 20.0), effect);
    events.onDragDrop(nullptr, Pos(21.0, 22.0), effect);
    events.onDragLeave();
    events.onHitTest(Pos(23.0, 24.0), hit, handled);
    events.onLeftButtonHit(WindowHitCodes::kClient, leftButtonHandled);
    events.onClosing(ClosingReason::kUnknown, cancel);

    SUCCEED();
}
