--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- Ogg - container bitstream library required by Vorbis.
require("ogg", {
    source = "https://github.com/xiph/ogg.git",
    ref = "06a5e0262cdc28aa4ae6797627a783b5010440f0",
    build = "custom",
    linkage = "static",
    runtime = "static",
    export_defines = false,
    artifacts = {
        include_dirs = { "include" }
    },
    custom_compile = {
        windows = {
            toolchain_requirements = {
                family = "msvc-abi",
                abi = "msvc",
                binary_format = "coff"
            },
            commands = {
                configure = {
                    [[powershell -NoProfile -ExecutionPolicy Bypass -Command "$p='$(source)\CMakeLists.txt'; $old='set(CMAKE_MSVC_RUNTIME_LIBRARY ' + [char]34 + 'MultiThreadedDLL' + [char]34 + ')'; $new='if(NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)' + [Environment]::NewLine + '    set(CMAKE_MSVC_RUNTIME_LIBRARY ' + [char]34 + 'MultiThreadedDLL' + [char]34 + ')' + [Environment]::NewLine + 'endif()'; $s=Get-Content $p -Raw; if($s.Contains($old)){ $enc=New-Object System.Text.UTF8Encoding $false; [IO.File]::WriteAllText($p,$s.Replace($old,$new),$enc) }; $buildType=if('$(config)' -ieq 'debug'){'Debug'}else{'Release'}; & '$(cmake)' -S '$(source)' -B '$(build)' -G Ninja -DCMAKE_INSTALL_PREFIX='$(artifact)' ('-DCMAKE_BUILD_TYPE=' + $buildType) -DBUILD_SHARED_LIBS=OFF '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>' -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DINSTALL_DOCS=OFF -DINSTALL_PKG_CONFIG_MODULE=OFF -DINSTALL_CMAKE_PACKAGE_MODULE=ON; exit $LASTEXITCODE"]]
                },
                build = {
                    [["$(cmake)" --build "$(build)"]]
                },
                install = {
                    [[if /I "$(config)"=="debug" ("$(cmake)" --install "$(build)" --config Debug --prefix "$(artifact)") else ("$(cmake)" --install "$(build)" --config Release --prefix "$(artifact)")]]
                }
            },
            artifacts = {
                include_dirs = { "include" },
                lib_files = { "lib/ogg.lib" }
            }
        },
        unix = {
            commands = {
                configure = {
                    [["$(cmake)" -S "$(source)" -B "$(build)" -G Ninja -DCMAKE_INSTALL_PREFIX="$(artifact)" -DCMAKE_BUILD_TYPE=$([ "$(config)" = "debug" ] && echo Debug || echo Release) -DBUILD_SHARED_LIBS=OFF '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>' -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DINSTALL_DOCS=OFF -DINSTALL_PKG_CONFIG_MODULE=OFF -DINSTALL_CMAKE_PACKAGE_MODULE=ON]]
                },
                build = {
                    [["$(cmake)" --build "$(build)"]]
                },
                install = {
                    [[if [ "$(config)" = "debug" ]; then "$(cmake)" --install "$(build)" --config Debug --prefix "$(artifact)"; else "$(cmake)" --install "$(build)" --config Release --prefix "$(artifact)"; fi]]
                }
            },
            artifacts = {
                include_dirs = { "include" },
                lib_files = { "lib/libogg.a" }
            }
        }
    }
})
