project(TkCore)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-deprecated-literal-operator)
endif()
#################################################################
set(INCS
        "FLib/src"
)
set(LNKS
        ""
)
set(DEPS
        ""
)
set(HEAD
        ""
)
#################################################################
# dependencies
#################################################################
SetPlatformDeps(INCS LNKS)
AddSpdlog(INCS LNKS)
#################################################################
# project configurations
#################################################################
file(GLOB_RECURSE src
        "src/*.m"
        "src/*.cpp"
        "src/*.mm"
        "src/*.c"
)
add_library(TkCore STATIC
        ${src}
)
target_include_directories(TkCore PRIVATE
        ${includeList}
)
target_link_libraries(TkCore PRIVATE
        ${linkList}
)
target_compile_definitions(TkCore PRIVATE
        _XOPEN_SOURCE
        _LIBCPP_DRIVERKIT

        TK_EXTRA_CHECKS
        TK_ENABLE_SIMD
)
