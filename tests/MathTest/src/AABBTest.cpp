#include "vector_test_utils.hpp"

using namespace math_test;

TEST(AABBTest, ConstructionQueriesAndMutation)
{
    AABB bounds;
    EXPECT_TRUE(bounds.isEmpty());

    bounds.add(1.2f, 2.8f, 3.1f);
    expectAabbEq(bounds, AABB(1.0f, 2.0f, 3.0f, 2.0f, 3.0f, 4.0f));
    EXPECT_EQ(bounds.width(), 1);
    EXPECT_EQ(bounds.height(), 1);
    EXPECT_EQ(bounds.depth(), 1);
    EXPECT_EQ(bounds.x(), 1);
    EXPECT_EQ(bounds.y(), 2);
    EXPECT_EQ(bounds.z(), 3);

    AABB ctorBounds(1.0f, 2.0f, 3.0f, 7.0f, 8.0f, 9.0f);
    expectAabbEq(ctorBounds, AABB(1.0f, 2.0f, 3.0f, 7.0f, 8.0f, 9.0f));

    AABB extra;
    extra.add(-2.0f, -3.0f, -4.0f);
    extra.add(4.0f, 5.0f, 6.0f);
    bounds.add(extra);
    expectAabbEq(bounds, AABB(-2.0f, -3.0f, -4.0f, 4.0f, 5.0f, 6.0f));

    AABB copied;
    copied.set(bounds);
    expectAabbEq(copied, bounds);

    copied.clear();
    EXPECT_TRUE(copied.isEmpty());
}

TEST(AABBTest, TranslationOutsideChecksAndStability)
{
    AABB bounds;
    bounds.add(0.0f, 0.0f, 0.0f);
    bounds.add(2.0f, 4.0f, 6.0f);

    bounds.translate(1, 2, 3);
    expectAabbEq(bounds, AABB(1.0f, 2.0f, 3.0f, 3.0f, 6.0f, 9.0f));

    bounds.translate(FVec3(0.5f, -1.25f, 2.1f));
    expectAabbEq(bounds, AABB(1.0f, 0.0f, 5.0f, 4.0f, 5.0f, 12.0f));

    const AABB translated = bounds.translated(FVec3(-2.0f, 1.0f, -3.0f));
    expectAabbEq(translated, AABB(-1.0f, 1.0f, 2.0f, 2.0f, 6.0f, 9.0f));
    expectAabbEq(bounds, AABB(1.0f, 0.0f, 5.0f, 4.0f, 5.0f, 12.0f));

    AABB offsetCopy;
    offsetCopy.set(bounds, FVec3(3.0f, 4.0f, 5.0f));
    expectAabbEq(offsetCopy, AABB(4.0f, 4.0f, 10.0f, 7.0f, 9.0f, 17.0f));

    const AABB overlap(3.0f, 4.0f, 10.0f, 8.0f, 10.0f, 18.0f);
    const AABB disjoint(100.0f, 100.0f, 100.0f, 110.0f, 110.0f, 110.0f);
    EXPECT_FALSE(offsetCopy.isOutside(overlap));
    EXPECT_TRUE(offsetCopy.isOutside(disjoint));
    EXPECT_FALSE(AABB::isOutside(offsetCopy, overlap));
    EXPECT_TRUE(AABB::isOutside(offsetCopy, disjoint));
}
