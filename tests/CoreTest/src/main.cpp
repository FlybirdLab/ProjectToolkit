/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include <gtest/gtest.h>

int main1(int argc, char** argv) {
    ::testing::GTEST_FLAG(catch_exceptions) = false; // 禁止 gtest 捕获 C++ 异常
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
