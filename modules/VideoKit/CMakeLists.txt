# VideoKit Sub-project

project(TkVideo)
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
        "${CMAKE_SOURCE_DIR}/modules/VideoKit/src"
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

AddTkCore(INCS LNKS DEPS)
AddTkMath(INCS LNKS DEPS)
AddTkGraphics(INCS LNKS DEPS)
#################################################################
# project configurations
#################################################################
file(GLOB_RECURSE src
        "src/*.m"
        "src/*.cpp"
        "src/*.mm"
        "src/*.c"
)
add_library(TkVideo STATIC
        ${src}
)
target_include_directories(TkVideo PRIVATE
        ${INCS}
)
target_link_libraries(TkVideo PRIVATE
        ${LNKS}
)
target_compile_definitions(TkVideo PRIVATE
        _XOPEN_SOURCE
        _LIBCPP_DRIVERKIT

        $<$<CONFIG:Debug>:TK_DEBUG>
        TK_ENABLE_SIMD
        TK_STATIC_PLUGINS
)
target_compile_options(TkVideo PRIVATE 
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)
