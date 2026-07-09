/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "window_test_utils.hpp"

#include <type_traits>
#include <utility>
#include <toolkit/windowing/WindowParams.hpp>

TEST(WindowingBasicsTest, EnumValuesAndDefaultParams)
{
    EXPECT_EQ(static_cast<int>(ClosingReason::kUnknown), 0);
    EXPECT_EQ(static_cast<int>(ClosingReason::kUser), 1);
    EXPECT_EQ(static_cast<int>(ClosingReason::kAppExit), 2);
    EXPECT_EQ(static_cast<int>(ClosingReason::kCloseEvent), 3);

    EXPECT_EQ(static_cast<int>(CursorType::kDefault), 0);
    EXPECT_EQ(static_cast<int>(CursorType::kHidden), 12);
    EXPECT_EQ(static_cast<int>(CursorType::MAX), 13);

    EXPECT_EQ(static_cast<int>(DragDropEffect::kNone), 0);
    EXPECT_EQ(static_cast<int>(DragDropEffect::kCopy), 1);
    EXPECT_EQ(static_cast<int>(DragDropEffect::kMove), 2);
    EXPECT_EQ(static_cast<int>(DragDropEffect::kLink), 3);

    EXPECT_EQ(static_cast<int>(WindowHitCodes::kTransparent), -1);
    EXPECT_EQ(static_cast<int>(WindowHitCodes::kClient), 1);
    EXPECT_EQ(static_cast<int>(WindowHitCodes::kClose), 20);
    EXPECT_EQ(static_cast<int>(WindowHitCodes::kHelp), 21);

    EXPECT_EQ(static_cast<int>(DialogResult::kAbort), 0);
    EXPECT_EQ(static_cast<int>(DialogResult::kYes), 7);

    EXPECT_EQ(static_cast<int>(MsgBoxButtons::kAbortRetryIgnore), 0);
    EXPECT_EQ(static_cast<int>(MsgBoxButtons::kYesNoCancel), 5);

    EXPECT_EQ(static_cast<int>(MsgBoxIcon::kAsterisk), 0);
    EXPECT_EQ(static_cast<int>(MsgBoxIcon::kWarning), 8);

    EXPECT_EQ(static_cast<int>(WindowStartPos::kCenterParent), 0);
    EXPECT_EQ(static_cast<int>(WindowStartPos::kCenterScreen), 1);
    EXPECT_EQ(static_cast<int>(WindowStartPos::kManual), 2);

    WindowParams params;
    EXPECT_EQ(params.alloc, &TkMemory::getDefault());
    EXPECT_TRUE(params.title.isEmpty());
    EXPECT_EQ(params.parent, nullptr);
    expectPosNear(params.pos, Pos(100.0, 400.0));
    expectPosNear(params.size, Pos(640.0, 480.0));
    expectPosNear(params.minSize, Pos(1.0, 1.0));
    expectPosNear(params.maxSize, Pos(0.0, 0.0));
    EXPECT_EQ(params.startPos, WindowStartPos::kManual);
    EXPECT_FALSE(params.fullscreen);
    EXPECT_TRUE(params.hasBorder);
    EXPECT_FALSE(params.supportsTransparency);
    EXPECT_TRUE(params.showInTaskbar);
    EXPECT_TRUE(params.activateWhenFirstShown);
    EXPECT_TRUE(params.allowInput);
    EXPECT_TRUE(params.allowMinimize);
    EXPECT_TRUE(params.allowMaximize);
    EXPECT_FALSE(params.allowDragAndDrop);
    EXPECT_FALSE(params.isTopmost);
    EXPECT_TRUE(params.isRegularWindow);
    EXPECT_TRUE(params.hasSizingFrame);
    EXPECT_TRUE(params.showAfterFirstPaint);
    EXPECT_EQ(params.data, nullptr);
}

TEST(WindowingBasicsTest, WindowParamsCanBeCustomizedAndCopied)
{
    WindowParams params;
    params.title = "Configured";
    params.parent = reinterpret_cast<TkWindow *>(0x1234);
    params.pos = Pos(10.0, 20.0);
    params.size = Pos(800.0, 600.0);
    params.minSize = Pos(320.0, 240.0);
    params.maxSize = Pos(1920.0, 1080.0);
    params.startPos = WindowStartPos::kCenterScreen;
    params.fullscreen = true;
    params.hasBorder = false;
    params.supportsTransparency = true;
    params.showInTaskbar = false;
    params.activateWhenFirstShown = false;
    params.allowInput = false;
    params.allowMinimize = false;
    params.allowMaximize = false;
    params.allowDragAndDrop = true;
    params.isTopmost = true;
    params.isRegularWindow = false;
    params.hasSizingFrame = false;
    params.showAfterFirstPaint = false;
    params.data = reinterpret_cast<void *>(0x5678);

    WindowParams copy = params;
    EXPECT_EQ(copy.alloc, params.alloc);
    EXPECT_EQ(copy.title, "Configured");
    EXPECT_EQ(copy.parent, params.parent);
    expectPosNear(copy.pos, Pos(10.0, 20.0));
    expectPosNear(copy.size, Pos(800.0, 600.0));
    expectPosNear(copy.minSize, Pos(320.0, 240.0));
    expectPosNear(copy.maxSize, Pos(1920.0, 1080.0));
    EXPECT_EQ(copy.startPos, WindowStartPos::kCenterScreen);
    EXPECT_TRUE(copy.fullscreen);
    EXPECT_FALSE(copy.hasBorder);
    EXPECT_TRUE(copy.supportsTransparency);
    EXPECT_FALSE(copy.showInTaskbar);
    EXPECT_FALSE(copy.activateWhenFirstShown);
    EXPECT_FALSE(copy.allowInput);
    EXPECT_FALSE(copy.allowMinimize);
    EXPECT_FALSE(copy.allowMaximize);
    EXPECT_TRUE(copy.allowDragAndDrop);
    EXPECT_TRUE(copy.isTopmost);
    EXPECT_FALSE(copy.isRegularWindow);
    EXPECT_FALSE(copy.hasSizingFrame);
    EXPECT_FALSE(copy.showAfterFirstPaint);
    EXPECT_EQ(copy.data, params.data);

    WindowParams assigned;
    assigned = params;
    EXPECT_EQ(assigned.alloc, params.alloc);
    EXPECT_EQ(assigned.title, params.title);

    assigned.alloc = nullptr;
    EXPECT_EQ(assigned.alloc, nullptr);
}

TEST(WindowingBasicsTest, WindowParamsHeaderProvidesStandaloneTypes)
{
    static_assert(std::is_enum_v<WindowStartPos>);
    static_assert(std::is_same_v<decltype(WindowParams::parent), TkWindow *>);
    static_assert(std::is_same_v<decltype(WindowParams::title), TkString>);

    WindowParams params;
    params.title = "StandaloneHeader";
    params.startPos = WindowStartPos::kCenterScreen;
    params.allowInput = false;

    EXPECT_EQ(params.title, "StandaloneHeader");
    EXPECT_EQ(params.startPos, WindowStartPos::kCenterScreen);
    EXPECT_FALSE(params.allowInput);
}

TEST(WindowingBasicsTest, DialogInlineOverloadsCompileAndReturnTypeMatches)
{
    static_assert(std::is_same_v<decltype(Dialog::msgBox("Hello")), DialogResult>);
    static_assert(std::is_same_v<decltype(Dialog::msgBox("Hello", "Title")), DialogResult>);
    static_assert(std::is_same_v<decltype(Dialog::msgBox("Hello", "Title", MsgBoxButtons::kOKCancel)), DialogResult>);
    static_assert(std::is_same_v<decltype(Dialog::msgBox("Hello", "Title", MsgBoxButtons::kYesNo, MsgBoxIcon::kQuestion)), DialogResult>);
    static_assert(std::is_same_v<decltype(Dialog::showOpenFileDialog(nullptr, "", "", false, "", std::declval<TkArray<TkString> &>())), bool>);
    static_assert(std::is_same_v<decltype(Dialog::showSaveFileDialog(nullptr, "", "", false, "", std::declval<TkArray<TkString> &>())), bool>);
    static_assert(std::is_same_v<decltype(Dialog::showBrowseFolderDialog(nullptr, "", "", std::declval<TkString &>())), bool>);
    static_assert(std::is_same_v<decltype(Dialog::showFileExplorer("")), bool>);
}

TEST(WindowingBasicsTest, WindowAndScreenApisRemainCallableWithExpectedTypes)
{
    static_assert(std::is_same_v<decltype(&TkWindow::isFirstWindow), bool (TkWindow::*)() const>);
    static_assert(std::is_same_v<decltype(&TkWindow::isFullscreen), bool (TkWindow::*)() const>);
    static_assert(std::is_same_v<decltype(&TkWindow::setFullscreen), void (TkWindow::*)(bool)>);
    static_assert(std::is_same_v<decltype(&TkWindow::isVisible), bool (TkWindow::*)() const>);
    static_assert(std::is_same_v<decltype(&TkWindow::setVisible), void (TkWindow::*)(bool)>);
    static_assert(std::is_same_v<decltype(&TkWindow::show), void (TkWindow::*)()>);
    static_assert(std::is_same_v<decltype(&TkWindow::hide), void (TkWindow::*)()>);
    static_assert(std::is_same_v<decltype(&TkWindow::minimize), void (TkWindow::*)()>);
    static_assert(std::is_same_v<decltype(&TkWindow::maximize), void (TkWindow::*)()>);
    static_assert(std::is_same_v<decltype(&TkWindow::restore), void (TkWindow::*)()>);
    static_assert(std::is_same_v<decltype(&TkWindow::close), void (TkWindow::*)(ClosingReason)>);
    static_assert(std::is_same_v<decltype(&TkWindow::getClientBounds), Rect (TkWindow::*)() const>);
    static_assert(std::is_same_v<decltype(&TkWindow::setClientBounds), void (TkWindow::*)(const Rect &)>);
    static_assert(std::is_same_v<decltype(&TkWindow::getPos), Pos (TkWindow::*)() const>);
    static_assert(std::is_same_v<decltype(&TkWindow::setPos), void (TkWindow::*)(const Pos &)>);
    static_assert(std::is_same_v<decltype(&TkWindow::screenToClient), Pos (TkWindow::*)(const Pos &) const>);
    static_assert(std::is_same_v<decltype(&TkWindow::clientToScreen), Pos (TkWindow::*)(const Pos &) const>);
    static_assert(std::is_same_v<decltype(&TkWindow::getTitle), TkString (TkWindow::*)() const>);
    static_assert(std::is_same_v<decltype(&TkWindow::setTitle), void (TkWindow::*)(const TkStringView &)>);
    static_assert(std::is_same_v<decltype(&TkWindow::getOpacity), f32 (TkWindow::*)() const>);
    static_assert(std::is_same_v<decltype(&TkWindow::setOpacity), void (TkWindow::*)(f32)>);
    static_assert(std::is_same_v<decltype(&TkWindow::focus), void (TkWindow::*)()>);
    static_assert(std::is_same_v<decltype(&TkWindow::bringToFront), void (TkWindow::*)(bool)>);
    static_assert(std::is_same_v<decltype(&TkWindow::flashWindow), void (TkWindow::*)()>);
    static_assert(std::is_same_v<decltype(&TkWindow::doDragDrop), DragDropEffect (TkWindow::*)(const TkStringView &)>);
    static_assert(std::is_same_v<decltype(&Screen::getIsFullscreen), bool (*)()>);
    static_assert(std::is_same_v<decltype(&Screen::setIsFullscreen), void (*)(bool)>);
    static_assert(std::is_same_v<decltype(&Screen::getSize), Pos (*)()>);
    static_assert(std::is_same_v<decltype(&Screen::setSize), void (*)(const Pos &)>);
    static_assert(std::is_same_v<decltype(&Screen::screenToGameViewport), Pos (*)(const Pos &)>);
    static_assert(std::is_same_v<decltype(&Screen::gameViewportToScreen), Pos (*)(const Pos &)>);
    static_assert(std::is_same_v<decltype(&Screen::getCursorVisible), bool (*)()>);
    static_assert(std::is_same_v<decltype(&Screen::setCursorVisible), void (*)(bool)>);
    static_assert(std::is_same_v<decltype(&Screen::getCursorLock), CursorLockMode (*)()>);
    static_assert(std::is_same_v<decltype(&Screen::setCursorLock), void (*)(CursorLockMode)>);
    static_assert(std::is_same_v<decltype(&Screen::updateScreen), void (*)(float)>);
    static_assert(std::is_same_v<decltype(&Screen::drawScreen), void (*)()>);
}
