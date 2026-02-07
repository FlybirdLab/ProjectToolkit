/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */
#include "toolkit/base/Assert.hpp"

#include <thread>
#include <chrono>
#include "toolkit/types/StringView.hpp"
#include <iostream>

#include <toolkit/common.hpp>
#include <toolkit/memory/PageAllocator.hpp>
#include <toolkit/memory/ArenaAllocator.hpp>

#include <toolkit/memory/Memory.hpp>
#include <toolkit/base/Format.hpp>

#include <toolkit/memory/StackAllocator.hpp>
#include <toolkit/debug/Logger.hpp>

#include <toolkit/types/String.hpp>
#include <toolkit/collection/Array.hpp>
#include <toolkit/collection/BitArray.hpp>
#include <toolkit/collection/ChunkedArray.hpp>
#include <toolkit/types/Span.hpp>

#include <toolkit/platforms/System.hpp>

#include <toolkit/debug/Debug.hpp>

#include <toolkit/threading/JobSys.hpp>
#include <toolkit/base/Function.hpp>

using namespace toolkit::type;
using namespace toolkit;

struct TestStruct
{
    int a = 1000;
    const char *b = "awa";
};
FORMAT_CUSTOM(TestStruct, "[a:%i, b:%s]", t.a, t.b);

int main(int argc, char **argv)
{

    System::init();
    System::beforeRun();
    System::logInfo();
    TK_ASSERT(TkJobSys::init());

    TkJobSys::setJobStartingOnDispatch(false);

    std::atomic<i32> counter{0};

    TkJobSys::dispatch([&](i32)
                       { counter++; }, 10);

    // 应该还没跑
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    TK_ASSERT(counter.load() == 0);

    TkJobSys::setJobStartingOnDispatch(true);
    TkJobSys::wait();

    TK_ASSERT(counter.load() == 10);

    TkJobSys::beforeExit();
    TkJobSys::dispose();
    System::beforeQuit();
    System::quit();
    return 0;
}