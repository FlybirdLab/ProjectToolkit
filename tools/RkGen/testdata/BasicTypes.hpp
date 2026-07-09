#pragma rkgen
#pragma once

using u8 = unsigned char;

namespace rkgen_test {

cenum class Color : u8
{
    Red,
    Green = 2,
    Blue,
};

cstruct Vec3
{
    float x;
    float y;
    float z = 0.0f;
};

}
