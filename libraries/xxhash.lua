-- xxHash - Extremely fast hash algorithm
require("xxhash", {
    source = "https://github.com/Cyan4973/xxHash.git",
    ref = "v0.8.2",
    build = "cmake",
    subdir = "cmake_unofficial",
    export_defines = false,
    cmake = {
        generator = "Ninja",
        options = {
            BUILD_SHARED_LIBS = false,
            XXHASH_BUILD_XXHSUM = false,
            -- Force static runtime library
            CMAKE_MSVC_RUNTIME_LIBRARY = "MultiThreaded$<$<CONFIG:Debug>:Debug>",
            CMAKE_POSITION_INDEPENDENT_CODE = true,
            -- Fix CMake version compatibility
            CMAKE_POLICY_VERSION_MINIMUM = "3.5"
        }
    },
    artifacts = {
        include_dirs = { "include" }
    },
    platforms = {
        windows = {
            artifacts = {
                lib_files = {
                    debug = { "lib/xxhash.lib" },
                    release = { "lib/xxhash.lib" }
                }
            }
        },
        unix = {
            artifacts = {
                lib_files = { "lib/libxxhash.a" }
            }
        }
    }
})
