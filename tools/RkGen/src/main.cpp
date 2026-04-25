/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "moc/MocCtx.hpp"

int main(int argc, char** argv) {
    toolkit::MocCtx ctx {argc, argv};
    return ctx.runMoc();
}