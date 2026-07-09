-- Google Test - Google Testing and Mocking Framework
require("gtest", {
    source = "https://github.com/google/googletest.git",
    mirrors = {
        "https://gitee.com/mirrors/googletest.git"
    },
    ref = "v1.14.0",
    build = "cmake",
    export_defines = false,
    cmake = {
        generator = "Ninja",
        options = {
            BUILD_GMOCK = true,
            INSTALL_GTEST = true,
            gtest_force_shared_crt = false,  -- Use static runtime
            -- Force static runtime library
            CMAKE_MSVC_RUNTIME_LIBRARY = "MultiThreaded$<$<CONFIG:Debug>:Debug>",
            CMAKE_POSITION_INDEPENDENT_CODE = true
        }
    },
    artifacts = {
        include_dirs = { "include" },
        lib_files = { "lib/gtest.lib", "lib/gtest_main.lib", "lib/gmock.lib", "lib/gmock_main.lib" }
    },
    platforms = {
        unix = {
            artifacts = {
                lib_files = { "lib/libgtest.a", "lib/libgtest_main.a", "lib/libgmock.a", "lib/libgmock_main.a" }
            }
        }
    }
})
