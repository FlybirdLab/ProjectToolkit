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

#include "ChunkedArrayTest.hpp"
#include "HashMapTest.hpp"
#include "RingBufferTest.hpp"
#include "DataCollectionTest.hpp"
#include "HashSetTest.hpp"
#include "ThreadTest.hpp"

#include <toolkit/platforms/System.hpp>

using namespace toolkit::type;
using namespace toolkit;

struct TestStruct
{
	int a = 1000;
	const char *b = "awa";
};
FORMAT_CUSTOM(TestStruct, "[a:%i, b:%s]", t.a, t.b);

enum class SandBoxTestType : toolkit::u8
{
	kNone = 0,
	kChunkedArray,
	kHashMap,
	kRingBuffer,
	kDataContainer,
	kHashSet,
	kThread
};

constexpr SandBoxTestType TestType = SandBoxTestType::kThread;

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

	switch (TestType)
	{
	case SandBoxTestType::kNone:
		break;
	case SandBoxTestType::kChunkedArray:
		toolkit::testChunkedArray();
		break;
	case SandBoxTestType::kHashMap:
		toolkit::testHashMap();
		break;
	case SandBoxTestType::kRingBuffer:
		toolkit::testRingBuffer();
		break;
	case SandBoxTestType::kDataContainer:
		toolkit::testDataCollection();
		break;
	case SandBoxTestType::kHashSet:
		toolkit::testHashSet();
		break;
	case SandBoxTestType::kThread:
		toolkit::testThread();
		break;
	}

	TkString str = "";
	auto r = TkString::format("Test %s", t);
	logger.info("%s", r.getText());

	return 0;
}