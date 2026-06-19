# Created by cubevlmu on 2025/12/8.
# Copyright (c) 2025 Flybird Games. All rights reserved.

# CoreKit Sub-project

project(TkCore)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-deprecated-literal-operator)
endif()
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fms-extensions")
#################################################################
set(INCS
        "${CMAKE_SOURCE_DIR}/modules/CoreKit/src"
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

list(APPEND LNKS tpkg::stb)
list(APPEND LNKS tpkg::spdlog)
list(APPEND LNKS tpkg::rpmalloc)
list(APPEND LNKS tpkg::xxhash)
if(NOT TARGET tpkg::DataExtend)
        message(FATAL_ERROR "TkCore requires tpkg::DataExtend.")
endif()
list(APPEND LNKS tpkg::DataExtend)
if(NOT TARGET tpkg::tracy)
        message(FATAL_ERROR "TkCore requires tpkg::tracy.")
endif()
list(APPEND LNKS tpkg::tracy)
list(APPEND LNKS tpkg::rapidjson)
#################################################################
# project configurations
#################################################################
file(GLOB_RECURSE src
        "src/*.cpp"
        "src/*.c"
)
if(APPLE)
    file(GLOB_RECURSE objc_src
            "src/*.m"
            "src/*.mm"
    )
    list(APPEND src ${objc_src})
endif()
add_library(TkCore STATIC
        ${src}
)
target_include_directories(TkCore PRIVATE
        ${INCS}
)
target_link_libraries(TkCore PRIVATE
        ${LNKS}
)
target_compile_definitions(TkCore PRIVATE
        _XOPEN_SOURCE
        _LIBCPP_DRIVERKIT

        $<$<CONFIG:Debug>:TK_DEBUG>
        TK_ENABLE_SIMD
        TK_STATIC_PLUGINS
)
target_compile_options(TkCore PRIVATE 
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)
