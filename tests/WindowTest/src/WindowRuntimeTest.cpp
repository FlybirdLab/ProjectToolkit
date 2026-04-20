/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "window_test_utils.hpp"
#include <toolkit/platforms/System.hpp>

TEST_F(WindowSystemFixture, RegisterCreateLookupAndDestroyWindow)
{
    WindowSystem system;
    const i32 initialCount = WindowSystem::Windows.count();
    WindowSystem::update();
    const bool hasFocus = system.getHasFocus();
    EXPECT_TRUE(hasFocus || !hasFocus);

    TkWindow *window = createTestWindow("RuntimeA", Pos(160.0, 180.0), Pos(300.0, 220.0));
    ASSERT_NE(window, nullptr);
    EXPECT_EQ(WindowSystem::Windows.count(), initialCount + 1);
    EXPECT_EQ(WindowSystem::getMainWindow(), window);
    EXPECT_EQ(WindowSystem::getByWindowPtr(window->getWindowPtr()), window);
    EXPECT_EQ(window->getParams().title, "RuntimeA");
    expectPosNear(window->getClientSize(), Pos(300.0, 220.0));
    EXPECT_FALSE(window->isVisible());
    EXPECT_FALSE(window->isClosed());
    EXPECT_FALSE(window->isFullscreen());
    EXPECT_TRUE(window->isWindowed());
    EXPECT_FALSE(window->isFirstWindow());
    EXPECT_FALSE(window->isForegroundWindow());
    EXPECT_FALSE(window->isMinimized());
    EXPECT_FALSE(window->isMaximized());
    EXPECT_EQ(window->getCursor(), CursorType::kDefault);
    EXPECT_NE(window->getWindowPtr(), nullptr);
    EXPECT_NE(window->getNativeWindow(), nullptr);
    EXPECT_GT(window->getDpi(), 0);
    EXPECT_GT(window->getDpiScale(), 0.0f);

    destroyTestWindow(window);
    EXPECT_EQ(WindowSystem::Windows.count(), initialCount);
}

TEST_F(WindowSystemFixture, MultipleWindowsMaintainLookupOrderAndParentParams)
{
    WindowSystem system;
    const i32 initialCount = WindowSystem::Windows.count();

    TkWindow *parent = createTestWindow("ParentRuntime", Pos(80.0, 90.0), Pos(300.0, 200.0));
    ASSERT_NE(parent, nullptr);

    WindowParams childParams;
    childParams.title = "ChildRuntime";
    childParams.parent = parent;
    childParams.pos = Pos(110.0, 120.0);
    childParams.size = Pos(240.0, 180.0);
    TkWindow *child = system.createWindow(childParams);
    ASSERT_NE(child, nullptr);

    EXPECT_EQ(WindowSystem::Windows.count(), initialCount + 2);
    EXPECT_EQ(WindowSystem::getMainWindow(), parent);
    EXPECT_EQ(WindowSystem::getByWindowPtr(parent->getWindowPtr()), parent);
    EXPECT_EQ(WindowSystem::getByWindowPtr(child->getWindowPtr()), child);
    EXPECT_EQ(child->getParams().parent, parent);
    EXPECT_EQ(child->getParams().title, "ChildRuntime");
    expectPosNear(child->getParams().size, Pos(240.0, 180.0));

    destroyTestWindow(child);
    destroyTestWindow(parent);
}

TEST_F(WindowSystemFixture, CenteredCreateWindowUsesResolvedCopyWithoutMutatingSourceParams)
{
    WindowParams params;
    params.title = "CenteredRuntime";
    params.pos = Pos(33.0, 44.0);
    params.size = Pos(220.0, 160.0);
    params.startPos = WindowStartPos::kCenterScreen;

    const Pos originalPos = params.pos;
    const Pos desktopSize = System::getDesktopSize();

    TkWindow *window = WindowSystem::createWindow(params);
    ASSERT_NE(window, nullptr);

    EXPECT_EQ(params.startPos, WindowStartPos::kCenterScreen);
    expectPosNear(params.pos, originalPos);

    const Pos expectedClientPos = (desktopSize - params.size) * 0.5f;
    expectPosNear(window->getParams().pos, expectedClientPos, 2.5);

    destroyTestWindow(window);
}

TEST_F(WindowSystemFixture, TitlePositionSizeCursorTrackingAndClippingWork)
{
    TkWindow *window = createTestWindow("RuntimeB", Pos(200.0, 210.0), Pos(280.0, 210.0), true);
    ASSERT_NE(window, nullptr);

    window->setTitle("UpdatedTitle");
    EXPECT_EQ(window->getTitle(), "UpdatedTitle");

    window->setCursor(CursorType::kHand);
    EXPECT_EQ(window->getCursor(), CursorType::kHand);
    window->setCursor(CursorType::kSizeWE);
    EXPECT_EQ(window->getCursor(), CursorType::kSizeWE);

    const Pos beforePos = window->getPos();
    window->setPos(beforePos + Pos(25.0, 30.0));
    expectPosNear(window->getPos(), beforePos + Pos(25.0, 30.0), 2.5);

    const Pos clientOrigin = window->getClientPos();
    window->setClientPos(clientOrigin + Pos(15.0, 10.0));
    expectPosNear(window->getClientPos(), clientOrigin + Pos(15.0, 10.0), 2.5);

    window->setClientSize(Pos(310.0, 230.0));
    expectPosNear(window->getClientSize(), Pos(310.0, 230.0), 2.5);

    window->setClientBounds(Rect(240.0, 260.0, 330.0, 240.0));
    expectRectNear(window->getClientBounds(), Rect(window->getClientPos(), window->getClientSize()), 2.5);
    expectPosNear(window->getClientSize(), Pos(330.0, 240.0), 2.5);

    const Pos roundTripInput(22.0, 35.0);
    expectPosNear(window->screenToClient(window->clientToScreen(roundTripInput)), roundTripInput, 2.5);

    EXPECT_FALSE(window->isMouseTracking());
    window->startTrackingMouse(true);
    EXPECT_TRUE(window->isMouseTracking());
    expectPosNear(window->getTrackingMouseOffset(), Pos(0.0, 0.0));
    EXPECT_FALSE(window->isMouseFlippingHorizontally());
    EXPECT_FALSE(window->isMouseFlippingVertically());
    window->endTrackingMouse();
    EXPECT_FALSE(window->isMouseTracking());

    EXPECT_FALSE(window->isCursorClipping());
    window->startClippingCursor(Rect(10.0, 20.0, 100.0, 120.0));
    EXPECT_TRUE(window->isCursorClipping());
    window->endClippingCursor();
    EXPECT_FALSE(window->isCursorClipping());

    window->setOpacity(0.25f);
    EXPECT_NEAR(window->getOpacity(), 0.25f, 0.05f);
    window->setOpacity(2.0f);
    EXPECT_NEAR(window->getOpacity(), 1.0f, 0.05f);
    window->setIcon("unused.ico");

    destroyTestWindow(window);
}

TEST_F(WindowSystemFixture, BorderlessWindowStateAndWindowManagementSmokes)
{
    TkWindow *window = createTestWindow("RuntimeStyle", Pos(220.0, 230.0), Pos(300.0, 210.0));
    ASSERT_NE(window, nullptr);

    EXPECT_TRUE(window->getParams().hasBorder);
    window->setBorderless(true, false);
    EXPECT_FALSE(window->getParams().hasBorder);
    window->setBorderless(false, false);
    EXPECT_TRUE(window->getParams().hasBorder);

    window->show();
    window->bringToFront(false);
    window->bringToFront(true);
    window->focus();
    window->flashWindow();
    window->minimize();
    window->restore();
    window->maximize();
    window->restore();
    window->setFullscreen(true);
    EXPECT_TRUE(window->isFullscreen());
    EXPECT_FALSE(window->isWindowed());
    window->setFullscreen(false);
    EXPECT_FALSE(window->isFullscreen());
    EXPECT_TRUE(window->isWindowed());

    destroyTestWindow(window);
}

TEST_F(WindowSystemFixture, VisibilityFocusTriggersAndInputGatesBehaveAsImplemented)
{
    TkWindow *window = createTestWindow("RuntimeC", Pos(260.0, 280.0), Pos(260.0, 180.0));
    ASSERT_NE(window, nullptr);

    int shownCount = 0;
    int hiddenCount = 0;
    int closedCount = 0;
    int gotFocusCount = 0;
    int lostFocusCount = 0;
    Pos resizedValue = Pos::Minimum;

    window->onShown.bind([&]() { ++shownCount; });
    window->onHidden.bind([&]() { ++hiddenCount; });
    window->onClosed.bind([&]() { ++closedCount; });
    window->onGotFocus.bind([&]() { ++gotFocusCount; });
    window->onLostFocus.bind([&]() { ++lostFocusCount; });
    window->onResized.bind([&](Pos size) { resizedValue = size; });

    EXPECT_FALSE(window->isVisible());
    window->show();
    window->setVisible(true);
    EXPECT_TRUE(window->isVisible());

    window->hide();
    window->setVisible(false);
    EXPECT_FALSE(window->isVisible());
    EXPECT_EQ(hiddenCount, 1);

    window->onShowTrigger();
    EXPECT_EQ(shownCount, 1);

    window->onResizeTrigger(444, 222);
    expectPosNear(resizedValue, Pos(444.0, 222.0));

    EXPECT_FALSE(window->isFocused());
    window->onGotFocusTrigger();
    EXPECT_TRUE(window->isFocused());
    EXPECT_EQ(gotFocusCount, 1);
    window->onGotFocusTrigger();
    EXPECT_EQ(gotFocusCount, 1);

    window->onLostFocusTrigger();
    EXPECT_FALSE(window->isFocused());
    EXPECT_EQ(lostFocusCount, 1);
    window->onLostFocusTrigger();
    EXPECT_EQ(lostFocusCount, 1);

    EXPECT_EQ(window->getInputText(), TkStringView::Empty);
    EXPECT_FALSE(window->getKey(KeyboardKeys::kA));
    EXPECT_FALSE(window->getKeyDown(KeyboardKeys::kA));
    EXPECT_FALSE(window->getKeyUp(KeyboardKeys::kA));
    EXPECT_EQ(window->getMousePos(), Pos::Minimum);
    window->setMousePos(Pos(32.0, 48.0));
    expectPosNear(window->getMousePosDelta(), Pos(0.0, 0.0));
    EXPECT_FLOAT_EQ(window->getMouseScrollDelta(), 0.0f);
    EXPECT_FALSE(window->getMouseButton(MouseButton::kLeft));
    EXPECT_FALSE(window->getMouseButtonDown(MouseButton::kLeft));
    EXPECT_FALSE(window->getMouseButtonUp(MouseButton::kLeft));
    EXPECT_EQ(window->doDragDrop("payload"), DragDropEffect::kNone);

    bool cancelClose = true;
    window->getEventGroup().onClosing.bind([&](ClosingReason, bool &cancel) { cancel = cancelClose; });

    window->show();
    EXPECT_TRUE(window->isVisible());
    window->close(ClosingReason::kUser);
    EXPECT_TRUE(window->isVisible());
    EXPECT_EQ(closedCount, 0);

    cancelClose = false;
    window->close(ClosingReason::kUser);
    EXPECT_FALSE(window->isVisible());
    EXPECT_EQ(closedCount, 1);

    destroyTestWindow(window);
}

TEST_F(WindowSystemFixture, CloseHonorsCancellationForAppExitAndUnregistersAfterClose)
{
    WindowSystem system;
    const i32 initialCount = WindowSystem::Windows.count();

    TkWindow *window = createTestWindow("RuntimeD", Pos(300.0, 320.0), Pos(240.0, 160.0));
    ASSERT_NE(window, nullptr);
    ASSERT_EQ(WindowSystem::Windows.count(), initialCount + 1);

    bool cancelClose = true;
    int closedCount = 0;
    window->getEventGroup().onClosing.bind([&](ClosingReason, bool &cancel) { cancel = cancelClose; });
    window->onClosed.bind([&]() { ++closedCount; });

    window->show();
    window->close(ClosingReason::kAppExit);
    EXPECT_TRUE(window->isVisible());
    EXPECT_EQ(closedCount, 0);
    EXPECT_EQ(WindowSystem::Windows.count(), initialCount + 1);

    cancelClose = false;
    window->close(ClosingReason::kAppExit);
    EXPECT_FALSE(window->isVisible());
    EXPECT_EQ(closedCount, 1);
    EXPECT_EQ(WindowSystem::Windows.count(), initialCount);

    destroyTestWindow(window);
}
