#include <iostream>

#include <toolkit/globals.hpp>
#include <toolkit/memory/PoolAllocator.hpp>
#include <toolkit/memory/ArenaAllocator.hpp>

using namespace toolkit::type;

int main(int argc, char** argv) {
    u8 t = 10;
    std::printf("hello world %i\n", t);


    return 0;
}