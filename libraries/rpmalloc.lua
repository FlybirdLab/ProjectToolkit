--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
--]]

-- rpmalloc - lock-free thread caching allocator
require("rpmalloc", {
    source = "https://github.com/mjansson/rpmalloc.git",
    ref = "66fd705a811764035ec80f54928748d2b31a3827",
    build = "custom",
    linkage = "static",
    runtime = "static",
    pic = true,
    export_defines = false,
    custom_compile = {
        windows = {
            toolchain_requirements = {
                family = "msvc-abi",
                abi = "msvc",
                binary_format = "coff"
            },
            commands = {
                build = {
                    [[if /I "$(config)"=="debug" ("$(cc)" /nologo /std:c17 /experimental:c11atomics /D "RPMALLOC_COMPILE=1" /D "ENABLE_OVERRIDE=0" /D "_UNICODE" /D "UNICODE" /MTd /Zi /Od /c "$(source)\rpmalloc\rpmalloc.c" /Fo"$(build)\rpmalloc.obj") else ("$(cc)" /nologo /std:c17 /experimental:c11atomics /D "RPMALLOC_COMPILE=1" /D "ENABLE_OVERRIDE=0" /D "_UNICODE" /D "UNICODE" /MT /O2 /c "$(source)\rpmalloc\rpmalloc.c" /Fo"$(build)\rpmalloc.obj")]]
                },
                install = {
                    [[if not exist "$(artifact)\include\rpmalloc" mkdir "$(artifact)\include\rpmalloc"]],
                    [[if not exist "$(artifact)\lib" mkdir "$(artifact)\lib"]],
                    [[copy /Y "$(source)\rpmalloc\rpmalloc.h" "$(artifact)\include\rpmalloc\rpmalloc.h"]],
                    [[copy /Y "$(source)\rpmalloc\rpnew.h" "$(artifact)\include\rpmalloc\rpnew.h"]],
                    [["$(archiver)" /NOLOGO /OUT:"$(artifact)\lib\rpmalloc.lib" "$(build)\rpmalloc.obj"]]
                }
            },
            artifacts = {
                include_dirs = { "include" },
                lib_files = { "lib/rpmalloc.lib" }
            }
        },
        unix = {
            commands = {
                build = {
                    [[mkdir -p "$(build)"]],
                    [[if [ "$(config)" = "debug" ]; then "$(cc)" -std=c11 -D_GNU_SOURCE -DRPMALLOC_COMPILE=1 -DENABLE_OVERRIDE=0 -fPIC -g -O0 -c "$(source)/rpmalloc/rpmalloc.c" -o "$(build)/rpmalloc.o"; else "$(cc)" -std=c11 -D_GNU_SOURCE -DRPMALLOC_COMPILE=1 -DENABLE_OVERRIDE=0 -fPIC -O2 -c "$(source)/rpmalloc/rpmalloc.c" -o "$(build)/rpmalloc.o"; fi]]
                },
                install = {
                    [[mkdir -p "$(artifact)/include/rpmalloc" "$(artifact)/lib"]],
                    [[cp "$(source)/rpmalloc/rpmalloc.h" "$(artifact)/include/rpmalloc/rpmalloc.h"]],
                    [[cp "$(source)/rpmalloc/rpnew.h" "$(artifact)/include/rpmalloc/rpnew.h"]],
                    [["$(archiver)" rcs "$(artifact)/lib/librpmalloc.a" "$(build)/rpmalloc.o"]]
                }
            },
            artifacts = {
                include_dirs = { "include" },
                lib_files = { "lib/librpmalloc.a" }
            }
        }
    }
})
