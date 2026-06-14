--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- Vorbis - Ogg Vorbis codec libraries.
require("vorbis", {
    source = "https://github.com/xiph/vorbis.git",
    ref = "68a7fc2246e734f1a311b06083fc1249551c4412",
    build = "custom",
    linkage = "static",
    runtime = "static",
    export_defines = false,
    dependencies = { "ogg" },
    custom_compile = {
        windows = {
            toolchain_requirements = {
                family = "msvc-abi",
                abi = "msvc",
                binary_format = "coff"
            },
            commands = {
                configure = {
                    [[powershell -NoProfile -ExecutionPolicy Bypass -Command "$p='$(source)\CMakeLists.txt'; $old='set(CMAKE_MSVC_RUNTIME_LIBRARY ' + [char]34 + 'MultiThreadedDLL' + [char]34 + ')'; $new='if(NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)' + [Environment]::NewLine + '    set(CMAKE_MSVC_RUNTIME_LIBRARY ' + [char]34 + 'MultiThreadedDLL' + [char]34 + ')' + [Environment]::NewLine + 'endif()'; $s=Get-Content $p -Raw; if($s.Contains($old)){ $enc=New-Object System.Text.UTF8Encoding $false; [IO.File]::WriteAllText($p,$s.Replace($old,$new),$enc) }; $root=(Resolve-Path '$(artifact)\..\..\..\..').Path; $oggRoot=$null; $lock=Join-Path $root 'tbuild.lock.toml'; if(Test-Path $lock){ $lines=Get-Content $lock; for($i=0;$i -lt $lines.Count;$i++){ if($lines[$i] -eq \"name = 'ogg'\"){ for($j=$i;$j -lt $lines.Count;$j++){ if($lines[$j] -match \"artifact_id = '([^']+)'\"){ $oggRoot=Join-Path $root ('.tbuild\artifacts\' + $Matches[1]); break } }; break } } }; if(-not $oggRoot -or -not (Test-Path $oggRoot)){ $oggRoot=(Get-ChildItem (Join-Path $root '.tbuild\artifacts\ogg') -Directory -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1).FullName }; if(-not $oggRoot){ exit 1 }; $buildType=if('$(config)' -ieq 'debug'){'Debug'}else{'Release'}; & '$(cmake)' -S '$(source)' -B '$(build)' -G Ninja -DCMAKE_INSTALL_PREFIX='$(artifact)' ('-DCMAKE_BUILD_TYPE=' + $buildType) -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>' -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DINSTALL_CMAKE_PACKAGE_MODULE=ON ('-DOGG_ROOT=' + $oggRoot) ('-DCMAKE_PREFIX_PATH=' + $oggRoot); exit $LASTEXITCODE"]]
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
                lib_files = {
                    "lib/vorbis.lib",
                    "lib/vorbisenc.lib",
                    "lib/vorbisfile.lib"
                }
            }
        },
        unix = {
            commands = {
                configure = {
                    [[TKB_ROOT=$(cd "$(artifact)/../../../.." && pwd) && TKB_OGG_ARTIFACT=$(awk 'BEGIN{inpkg=0} /^\[\[package\]\]/{inpkg=0} /^name = '\''ogg'\''/{inpkg=1} inpkg && /^artifact_id = /{gsub("'\''","",$3); print $3; exit}' "$TKB_ROOT/tbuild.lock.toml" 2>/dev/null || true) && if [ -n "$TKB_OGG_ARTIFACT" ]; then OGG_ROOT="$TKB_ROOT/.tbuild/artifacts/$TKB_OGG_ARTIFACT"; else OGG_ROOT=$(find "$TKB_ROOT/.tbuild/artifacts/ogg" -mindepth 1 -maxdepth 1 -type d -print 2>/dev/null | sort | tail -n 1); fi && test -n "$OGG_ROOT" && "$(cmake)" -S "$(source)" -B "$(build)" -G Ninja -DCMAKE_INSTALL_PREFIX="$(artifact)" -DCMAKE_BUILD_TYPE=$([ "$(config)" = "debug" ] && echo Debug || echo Release) -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>' -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DINSTALL_CMAKE_PACKAGE_MODULE=ON -DOGG_ROOT="$OGG_ROOT" -DCMAKE_PREFIX_PATH="$OGG_ROOT"]]
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
                lib_files = {
                    "lib/libvorbis.a",
                    "lib/libvorbisenc.a",
                    "lib/libvorbisfile.a"
                },
                system_libs = { "m" }
            }
        }
    }
})
