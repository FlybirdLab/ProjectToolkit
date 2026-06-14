--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- SPIRV-Tools.
require("spirv-tools", {
    source = "https://github.com/KhronosGroup/SPIRV-Tools.git",
    ref = "2ec8457ab33d539b6f1fecc998360c0b8b05ed4f",
    build = "custom",
    linkage = "static",
    runtime = "static",
    export_defines = false,
    dependencies = { "spirv-headers" },
    custom_compile = {
        windows = {
            toolchain_requirements = {
                family = "msvc-abi",
                abi = "msvc",
                binary_format = "coff"
            },
            commands = {
                configure = {
                    [[powershell -NoProfile -ExecutionPolicy Bypass -Command "$root=(Resolve-Path '$(artifact)\..\..\..\..').Path; $headersRoot=$null; $lock=Join-Path $root 'tbuild.lock.toml'; if(Test-Path $lock){ $lines=Get-Content $lock; for($i=0;$i -lt $lines.Count;$i++){ if($lines[$i] -eq \"name = 'spirv-headers'\"){ for($j=$i;$j -lt $lines.Count;$j++){ if($lines[$j] -match \"artifact_id = '([^']+)'\"){ $headersRoot=Join-Path $root ('.tbuild\artifacts\' + $Matches[1]); break } }; break } } }; if(-not $headersRoot -or -not (Test-Path $headersRoot)){ $headersRoot=(Get-ChildItem (Join-Path $root '.tbuild\artifacts\spirv-headers') -Directory -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1).FullName }; if(-not $headersRoot){ exit 1 }; $headersRoot=$headersRoot.Replace('\','/'); $buildType=if('$(config)' -ieq 'debug'){'Debug'}else{'Release'}; & '$(cmake)' -S '$(source)' -B '$(build)' -G Ninja -DCMAKE_INSTALL_PREFIX='$(artifact)' ('-DCMAKE_BUILD_TYPE=' + $buildType) -DBUILD_SHARED_LIBS=OFF '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>' -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DENABLE_RTTI=OFF -DSKIP_SPIRV_TOOLS_INSTALL=OFF -DSPIRV_BUILD_FUZZER=OFF -DSPIRV_BUILD_LIBFUZZER_TARGETS=OFF -DSPIRV_SKIP_EXECUTABLES=ON -DSPIRV_SKIP_TESTS=ON -DSPIRV_TOOLS_BUILD_STATIC=ON -DSPIRV_WERROR=OFF ('-DSPIRV-Headers_SOURCE_DIR=' + $headersRoot); exit $LASTEXITCODE"]]
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
                    debug = {
                        "lib/SPIRV-Tools.lib",
                        "lib/SPIRV-Tools-shared.lib",
                        "lib/SPIRV-Tools-opt.lib",
                        "lib/SPIRV-Tools-reduce.lib",
                        "lib/SPIRV-Tools-lint.lib",
                        "lib/SPIRV-Tools-link.lib",
                        "lib/SPIRV-Tools-diff.lib"
                    },
                    release = {
                        "lib/SPIRV-Tools.lib",
                        "lib/SPIRV-Tools-shared.lib",
                        "lib/SPIRV-Tools-opt.lib",
                        "lib/SPIRV-Tools-reduce.lib",
                        "lib/SPIRV-Tools-lint.lib",
                        "lib/SPIRV-Tools-link.lib",
                        "lib/SPIRV-Tools-diff.lib"
                    }
                }
            }
        },
        unix = {
            commands = {
                configure = {
                    [[TKB_ROOT=$(cd "$(artifact)/../../../.." && pwd) && TKB_HEADERS_ARTIFACT=$(awk 'BEGIN{inpkg=0} /^\[\[package\]\]/{inpkg=0} /^name = '\''spirv-headers'\''/{inpkg=1} inpkg && /^artifact_id = /{gsub("'\''","",$3); print $3; exit}' "$TKB_ROOT/tbuild.lock.toml" 2>/dev/null || true) && if [ -n "$TKB_HEADERS_ARTIFACT" ]; then HEADERS_ROOT="$TKB_ROOT/.tbuild/artifacts/$TKB_HEADERS_ARTIFACT"; else HEADERS_ROOT=$(find "$TKB_ROOT/.tbuild/artifacts/spirv-headers" -mindepth 1 -maxdepth 1 -type d -print 2>/dev/null | sort | tail -n 1); fi && test -n "$HEADERS_ROOT" && "$(cmake)" -S "$(source)" -B "$(build)" -G Ninja -DCMAKE_INSTALL_PREFIX="$(artifact)" -DCMAKE_BUILD_TYPE=$([ "$(config)" = "debug" ] && echo Debug || echo Release) -DBUILD_SHARED_LIBS=OFF '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>' -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DENABLE_RTTI=OFF -DSKIP_SPIRV_TOOLS_INSTALL=OFF -DSPIRV_BUILD_FUZZER=OFF -DSPIRV_BUILD_LIBFUZZER_TARGETS=OFF -DSPIRV_SKIP_EXECUTABLES=ON -DSPIRV_SKIP_TESTS=ON -DSPIRV_TOOLS_BUILD_STATIC=ON -DSPIRV_WERROR=OFF -DSPIRV-Headers_SOURCE_DIR="$HEADERS_ROOT"]]
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
                    "lib/libSPIRV-Tools.a",
                    "lib/libSPIRV-Tools-opt.a",
                    "lib/libSPIRV-Tools-reduce.a",
                    "lib/libSPIRV-Tools-lint.a",
                    "lib/libSPIRV-Tools-link.a",
                    "lib/libSPIRV-Tools-diff.a"
                }
            }
        }
    }
})
