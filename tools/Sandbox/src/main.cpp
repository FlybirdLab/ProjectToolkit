/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */
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

#include <toolkit/platforms/System.hpp>

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

	TestStruct t{
		.a = 1000,
		.b = "你好"};

	TkLogger logger = TkLogger::getShared();
	logger.info("Hello world");

	toolkit::StackAllocator<100> alloc{TkMemory::getDefault()};
	auto it = alloc.asInterface();

	TkString str = "";
	auto r = TkString::format("Test %s", t);
	logger.info("%s", r.getText());

	return 0;
}