/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "window_test_utils.hpp"

#include <toolkit/input/Mouse.hpp>

namespace
{
    class TestMouse final : public Mouse
    {
    public:
        TestMouse()
            : Mouse()
        {
        }

        void setMousePosition(const Pos &newPosition) override
        {
            onMouseMoved(newPosition);
        }
    };
}

TEST(InputMouseTest, ResetPreservesPositionAndClearsDelta)
{
    TestMouse mouse;
    EventQueue events;

    mouse.setMousePosition(Pos(100.0, 100.0));
    mouse.update(events);
    expectPosNear(mouse.getPos(), Pos(100.0, 100.0));
    expectPosNear(mouse.getPosDelta(), Pos(0.0, 0.0));

    mouse.resetState();
    mouse.update(events);

    expectPosNear(mouse.getPos(), Pos(100.0, 100.0));
    expectPosNear(mouse.getPosDelta(), Pos(0.0, 0.0));
}
