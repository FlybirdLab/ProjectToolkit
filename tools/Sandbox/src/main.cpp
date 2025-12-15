#include <iostream>

#include <toolkit/common.hpp>
#include <toolkit/memory/PageAllocator.hpp>
#include <toolkit/memory/ArenaAllocator.hpp>

#include <toolkit/memory/Memory.hpp>
#include <toolkit/base/Format.hpp>

#include <toolkit/memory/StackAllocator.hpp>

#include <toolkit/types/String.hpp>
#include <toolkit/collection/Array.hpp>

using namespace toolkit::type;
using namespace toolkit;

struct TestStruct {
    int a = 1000;
    const char* b = "awa";
};
FORMAT_CUSTOM(TestStruct, "[a:%i, b:%s]", t.a, t.b);

int main(int argc, char** argv) {
	TestStruct t {
		.a = 1000,
		.b = "你好"
	};


//	toolkit::PageAllocator alloc {};
//	auto it = alloc.asInterface();
//	toolkit::FormatBuffer buf{it};
//
//    toolkit::Formatter::format(buf, "%s %s %i", t, "hello world", 114514);
//    std::puts(buf.getCurrentPointer() - sizeof(char) * buf.getUsed());

    TkString str = "";
	auto r = TkString::format("Test %s", t);
	printf("%s\n", r.getText());

    return 0;
}