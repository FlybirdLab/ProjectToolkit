--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- glslang - GLSL/HLSL to SPIR-V compiler frontend.
require("glslang", {
    source = "https://github.com/KhronosGroup/glslang.git",
    ref = "458ff50a67cb69371850068a62b78f1990a1ff9a",
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
                    [[cd /D "$(source)" && python update_glslang_sources.py --dir "$(source)" --site github]],
                    [[for %%I in ("$(artifact)\..\..\..\short-build\glslang-$(config)") do if exist "%%~fI" rmdir /S /Q "%%~fI"]],
                    [[for %%I in ("$(artifact)\..\..\..\short-build\glslang-$(config)") do if /I "$(config)"=="debug" ("$(cmake)" -S "$(source)" -B "%%~fI" -G Ninja -DCMAKE_INSTALL_PREFIX="$(artifact)" -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=OFF "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>" -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DENABLE_GLSLANG_BINARIES=ON -DENABLE_GLSLANG_JS=OFF -DENABLE_HLSL=ON -DENABLE_OPT=ON -DENABLE_PCH=ON -DENABLE_RTTI=OFF -DENABLE_SPIRV=ON -DGLSLANG_ENABLE_INSTALL=ON -DGLSLANG_TESTS=OFF -DSPIRV_BUILD_COMPRESSION=OFF -DSPIRV_BUILD_FUZZER=OFF -DSPIRV_BUILD_LIBFUZZER_TARGETS=OFF -DSPIRV_HEADERS_ENABLE_INSTALL=OFF -DSPIRV_HEADERS_ENABLE_TESTS=OFF -DSPIRV_SKIP_EXECUTABLES=ON -DSPIRV_SKIP_TESTS=ON -DSPIRV_TOOLS_BUILD_STATIC=ON -DSPIRV_TOOLS_INSTALL_EMACS_HELPERS=OFF) else ("$(cmake)" -S "$(source)" -B "%%~fI" -G Ninja -DCMAKE_INSTALL_PREFIX="$(artifact)" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>" -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DENABLE_GLSLANG_BINARIES=ON -DENABLE_GLSLANG_JS=OFF -DENABLE_HLSL=ON -DENABLE_OPT=ON -DENABLE_PCH=ON -DENABLE_RTTI=OFF -DENABLE_SPIRV=ON -DGLSLANG_ENABLE_INSTALL=ON -DGLSLANG_TESTS=OFF -DSPIRV_BUILD_COMPRESSION=OFF -DSPIRV_BUILD_FUZZER=OFF -DSPIRV_BUILD_LIBFUZZER_TARGETS=OFF -DSPIRV_HEADERS_ENABLE_INSTALL=OFF -DSPIRV_HEADERS_ENABLE_TESTS=OFF -DSPIRV_SKIP_EXECUTABLES=ON -DSPIRV_SKIP_TESTS=ON -DSPIRV_TOOLS_BUILD_STATIC=ON -DSPIRV_TOOLS_INSTALL_EMACS_HELPERS=OFF)]]
                },
                build = {
                    [[for %%I in ("$(artifact)\..\..\..\short-build\glslang-$(config)") do "$(cmake)" --build "%%~fI"]]
                },
                install = {
                    [[for %%I in ("$(artifact)\..\..\..\short-build\glslang-$(config)") do if /I "$(config)"=="debug" ("$(cmake)" --install "%%~fI" --config Debug --prefix "$(artifact)") else ("$(cmake)" --install "%%~fI" --config Release --prefix "$(artifact)")]]
                }
            },
            artifacts = {
                include_dirs = { "include" },
                lib_files = {
                    debug = {
                        "lib/SPIRVd.lib",
                        "lib/SPIRV-Toolsd.lib",
                        "lib/SPIRV-Tools-sharedd.lib",
                        "lib/SPIRV-Tools-reduced.lib",
                        "lib/SPIRV-Tools-optd.lib",
                        "lib/SPIRV-Tools-lintd.lib",
                        "lib/SPIRV-Tools-linkd.lib",
                        "lib/SPIRV-Tools-diffd.lib",
                        "lib/OSDependentd.lib",
                        "lib/glslangd.lib",
                        "lib/MachineIndependentd.lib",
                        "lib/glslang-default-resource-limitsd.lib",
                        "lib/GenericCodeGend.lib"
                    },
                    release = {
                        "lib/SPIRV.lib",
                        "lib/SPIRV-Tools.lib",
                        "lib/SPIRV-Tools-shared.lib",
                        "lib/SPIRV-Tools-reduce.lib",
                        "lib/SPIRV-Tools-opt.lib",
                        "lib/SPIRV-Tools-lint.lib",
                        "lib/SPIRV-Tools-link.lib",
                        "lib/SPIRV-Tools-diff.lib",
                        "lib/OSDependent.lib",
                        "lib/glslang.lib",
                        "lib/MachineIndependent.lib",
                        "lib/glslang-default-resource-limits.lib",
                        "lib/GenericCodeGen.lib"
                    }
                }
            }
        },
        unix = {
            commands = {
                configure = {
                    [[cd "$(source)" && python update_glslang_sources.py --dir "$(source)" --site github]],
                    [[if [ "$(config)" = "debug" ]; then "$(cmake)" -S "$(source)" -B "$(build)" -G Ninja -DCMAKE_INSTALL_PREFIX="$(artifact)" -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=OFF '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>' -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DENABLE_GLSLANG_BINARIES=ON -DENABLE_GLSLANG_JS=OFF -DENABLE_HLSL=ON -DENABLE_OPT=ON -DENABLE_PCH=ON -DENABLE_RTTI=OFF -DENABLE_SPIRV=ON -DGLSLANG_ENABLE_INSTALL=ON -DGLSLANG_TESTS=OFF -DSPIRV_BUILD_COMPRESSION=OFF -DSPIRV_BUILD_FUZZER=OFF -DSPIRV_BUILD_LIBFUZZER_TARGETS=OFF -DSPIRV_HEADERS_ENABLE_INSTALL=OFF -DSPIRV_HEADERS_ENABLE_TESTS=OFF -DSPIRV_SKIP_EXECUTABLES=ON -DSPIRV_SKIP_TESTS=ON -DSPIRV_TOOLS_BUILD_STATIC=ON -DSPIRV_TOOLS_INSTALL_EMACS_HELPERS=OFF; else "$(cmake)" -S "$(source)" -B "$(build)" -G Ninja -DCMAKE_INSTALL_PREFIX="$(artifact)" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>' -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DENABLE_GLSLANG_BINARIES=ON -DENABLE_GLSLANG_JS=OFF -DENABLE_HLSL=ON -DENABLE_OPT=ON -DENABLE_PCH=ON -DENABLE_RTTI=OFF -DENABLE_SPIRV=ON -DGLSLANG_ENABLE_INSTALL=ON -DGLSLANG_TESTS=OFF -DSPIRV_BUILD_COMPRESSION=OFF -DSPIRV_BUILD_FUZZER=OFF -DSPIRV_BUILD_LIBFUZZER_TARGETS=OFF -DSPIRV_HEADERS_ENABLE_INSTALL=OFF -DSPIRV_HEADERS_ENABLE_TESTS=OFF -DSPIRV_SKIP_EXECUTABLES=ON -DSPIRV_SKIP_TESTS=ON -DSPIRV_TOOLS_BUILD_STATIC=ON -DSPIRV_TOOLS_INSTALL_EMACS_HELPERS=OFF; fi]]
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
                    "lib/libSPIRV.a",
                    "lib/libSPIRV-Tools.a",
                    "lib/libSPIRV-Tools-opt.a",
                    "lib/libOSDependent.a",
                    "lib/libglslang.a",
                    "lib/libMachineIndependent.a",
                    "lib/libglslang-default-resource-limits.a",
                    "lib/libGenericCodeGen.a"
                }
            }
        }
    }
})
