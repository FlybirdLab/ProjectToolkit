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
        "${CMAKE_SOURCE_DIR}/CoreKit/src"
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

AddStb(INCS LNKS)
AddSpdlog(INCS LNKS)
AddRpmalloc(INCS LNKS)

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
        ${INCS}
)
target_link_libraries(TkCore PRIVATE
        ${LNKS}
)
target_compile_definitions(TkCore PRIVATE
        _XOPEN_SOURCE
        _LIBCPP_DRIVERKIT

        TK_DEBUG
        TK_ENABLE_SIMD
)
