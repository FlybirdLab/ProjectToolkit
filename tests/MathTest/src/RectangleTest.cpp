#include "vector_test_utils.hpp"

using namespace math_test;

TEST(RectangleTest, QueriesOperatorsAndComparison)
{
    Rectangle rect(FVec2(10.0f, 20.0f), FVec2(30.0f, 40.0f));
    EXPECT_FALSE(Rectangle::kEmpty.toString().isEmpty());
    EXPECT_FLOAT_EQ(rect.getX(), 10.0f);
    EXPECT_FLOAT_EQ(rect.getY(), 20.0f);
    EXPECT_FLOAT_EQ(rect.getWidth(), 30.0f);
    EXPECT_FLOAT_EQ(rect.getHeight(), 40.0f);
    EXPECT_FLOAT_EQ(rect.getLeft(), 10.0f);
    EXPECT_FLOAT_EQ(rect.getRight(), 40.0f);
    EXPECT_FLOAT_EQ(rect.getTop(), 20.0f);
    EXPECT_FLOAT_EQ(rect.getBottom(), 60.0f);
    expectVec2Near(rect.getUpperLeft(), FVec2(10.0f, 20.0f));
    expectVec2Near(rect.getUpperRight(), FVec2(40.0f, 20.0f));
    expectVec2Near(rect.getLowerLeft(), FVec2(10.0f, 60.0f));
    expectVec2Near(rect.getLowerRight(), FVec2(40.0f, 60.0f));
    expectVec2Near(rect.getTopLeft(), FVec2(10.0f, 20.0f));
    expectVec2Near(rect.getTopRight(), FVec2(40.0f, 20.0f));
    expectVec2Near(rect.getBottomLeft(), FVec2(10.0f, 60.0f));
    expectVec2Near(rect.getBottomRight(), FVec2(40.0f, 60.0f));
    expectVec2Near(rect.getCenter(), FVec2(25.0f, 40.0f));

    expectRectNear(rect + FVec2(1.0f, 2.0f), Rectangle(11.0f, 22.0f, 30.0f, 40.0f));
    expectRectNear(rect - FVec2(1.0f, 2.0f), Rectangle(9.0f, 18.0f, 30.0f, 40.0f));
    expectRectNear(rect * 2.0f, Rectangle(-5.0f, 0.0f, 60.0f, 80.0f));
    expectRectNear(rect / 2.0f, Rectangle(17.5f, 30.0f, 15.0f, 20.0f));

    Rectangle mutated = rect;
    mutated += FVec2(3.0f, 4.0f);
    mutated -= FVec2(1.0f, 1.0f);
    mutated *= 2.0f;
    mutated /= 2.0f;
    expectRectNear(mutated, Rectangle(12.0f, 23.0f, 30.0f, 40.0f));

    EXPECT_TRUE(rect == Rectangle(10.0f, 20.0f, 30.0f, 40.0f));
    EXPECT_TRUE(rect != Rectangle(0.0f, 0.0f, 1.0f, 1.0f));
    EXPECT_TRUE(Rectangle::nearEqual(rect, Rectangle(10.0f, 20.0f, 30.0f, 40.0f)));
}

TEST(RectangleTest, ContainsIntersectsOffsetExpandScaleAndFactories)
{
    Rectangle rect(10.0f, 20.0f, 30.0f, 40.0f);
    EXPECT_TRUE(rect.contains(FVec2(10.0f, 20.0f)));
    EXPECT_TRUE(rect.contains(FVec2(25.0f, 40.0f)));
    EXPECT_FALSE(rect.contains(FVec2(5.0f, 40.0f)));

    EXPECT_TRUE(rect.contains(Rectangle(15.0f, 25.0f, 5.0f, 5.0f)));
    EXPECT_FALSE(rect.contains(Rectangle(0.0f, 0.0f, 100.0f, 100.0f)));
    EXPECT_TRUE(rect.intersects(Rectangle(35.0f, 55.0f, 20.0f, 20.0f)));
    EXPECT_FALSE(rect.intersects(Rectangle(-100.0f, -100.0f, 5.0f, 5.0f)));

    rect.offset(5.0f, -5.0f);
    rect.offset(FVec2(-2.0f, 3.0f));
    expectRectNear(rect, Rectangle(13.0f, 18.0f, 30.0f, 40.0f));
    expectRectNear(rect.makeOffsetted(FVec2(10.0f, 20.0f)), Rectangle(23.0f, 38.0f, 30.0f, 40.0f));

    Rectangle expanded = rect.makeExpanded(2.0f);
    expectRectNear(expanded, Rectangle(11.0f, 16.0f, 34.0f, 44.0f));
    rect.expand(2.0f);
    expectRectNear(rect, expanded);

    Rectangle scaled = rect.makeScaled(2.0f);
    expectRectNear(scaled, Rectangle(-6.0f, -6.0f, 68.0f, 88.0f));
    rect.scale(0.5f);
    expectRectNear(rect, Rectangle(19.5f, 27.0f, 17.0f, 22.0f));

    expectRectNear(Rectangle::unionRectangle(Rectangle(0.0f, 0.0f, 10.0f, 10.0f), FVec2(20.0f, 5.0f)), Rectangle(0.0f, 0.0f, 20.0f, 10.0f));
    expectRectNear(Rectangle::unionRectangle(Rectangle(0.0f, 0.0f, 10.0f, 10.0f), Rectangle(5.0f, 5.0f, 20.0f, 10.0f)), Rectangle(0.0f, 0.0f, 25.0f, 15.0f));
    expectRectNear(Rectangle::shared(Rectangle(0.0f, 0.0f, 10.0f, 10.0f), Rectangle(5.0f, 5.0f, 20.0f, 10.0f)), Rectangle(5.0f, 5.0f, 5.0f, 5.0f));
    expectRectNear(Rectangle::fromPoints(FVec2(5.0f, 8.0f), FVec2(1.0f, 2.0f)), Rectangle(1.0f, 2.0f, 4.0f, 6.0f));

    const FVec2 points[] = {FVec2(1.0f, 2.0f), FVec2(5.0f, -1.0f), FVec2(-3.0f, 4.0f)};
    expectRectNear(Rectangle::fromPoints(points, 3), Rectangle(-3.0f, -1.0f, 8.0f, 5.0f));
}
