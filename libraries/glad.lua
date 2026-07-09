--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- GLAD OpenGL loader. Generate the same loader shape as the previous checked-in
-- artifact, then build it with the project runtime.
require("glad", {
    source = "https://github.com/Dav1dde/glad.git",
    ref = "v0.1.36",
    build = "custom",
    linkage = "static",
    runtime = "static",
    export_defines = false,
    custom_compile = {
        windows = {
            toolchain_requirements = {
                family = "msvc-abi",
                abi = "msvc",
                binary_format = "coff"
            },
            commands = {
                configure = {
                    [[if not exist "$(build)\generated" mkdir "$(build)\generated"]]
                },
                build = {
                    [[set "PYTHONPATH=$(source)" && python -m glad --profile=compatibility --out-path="$(build)\generated" --generator=c --spec=gl --api="gl=4.1,gles1=1.0,gles2=3.0,glsc2=2.0"]],
                    [[if /I "$(config)"=="debug" ("$(cc)" /nologo /TC /I"$(build)\generated\include" /MTd /Zi /Od /c "$(build)\generated\src\glad.c" /Fo"$(build)\glad.obj") else ("$(cc)" /nologo /TC /I"$(build)\generated\include" /MT /O2 /c "$(build)\generated\src\glad.c" /Fo"$(build)\glad.obj")]]
                },
                install = {
                    [[if not exist "$(artifact)\include\glad" mkdir "$(artifact)\include\glad"]],
                    [[if not exist "$(artifact)\include\KHR" mkdir "$(artifact)\include\KHR"]],
                    [[if not exist "$(artifact)\lib" mkdir "$(artifact)\lib"]],
                    [[copy /Y "$(build)\generated\include\glad\glad.h" "$(artifact)\include\glad\glad.h"]],
                    [[copy /Y "$(build)\generated\include\KHR\khrplatform.h" "$(artifact)\include\KHR\khrplatform.h"]],
                    [["$(archiver)" /NOLOGO /OUT:"$(artifact)\lib\GLAD.lib" "$(build)\glad.obj"]]
                }
            },
            artifacts = {
                include_dirs = { "include" },
                lib_files = { "lib/GLAD.lib" },
                system_libs = { "winmm" }
            }
        },
        unix = {
            commands = {
                configure = {
                    [[mkdir -p "$(build)/generated"]]
                },
                build = {
                    [[PYTHONPATH="$(source)" python -m glad --profile=compatibility --out-path="$(build)/generated" --generator=c --spec=gl --api="gl=4.1,gles1=1.0,gles2=3.0,glsc2=2.0"]],
                    [[if [ "$(config)" = "debug" ]; then "$(cc)" -I"$(build)/generated/include" -g -O0 -fPIC -c "$(build)/generated/src/glad.c" -o "$(build)/glad.o"; else "$(cc)" -I"$(build)/generated/include" -O2 -fPIC -c "$(build)/generated/src/glad.c" -o "$(build)/glad.o"; fi]]
                },
                install = {
                    [[mkdir -p "$(artifact)/include/glad" "$(artifact)/include/KHR" "$(artifact)/lib"]],
                    [[cp "$(build)/generated/include/glad/glad.h" "$(artifact)/include/glad/glad.h"]],
                    [[cp "$(build)/generated/include/KHR/khrplatform.h" "$(artifact)/include/KHR/khrplatform.h"]],
                    [["$(archiver)" rcs "$(artifact)/lib/libglad.a" "$(build)/glad.o"]]
                }
            },
            artifacts = {
                include_dirs = { "include" },
                lib_files = { "lib/libglad.a" }
            }
        }
    }
})
