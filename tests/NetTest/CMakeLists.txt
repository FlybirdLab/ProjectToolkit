project(NetTest)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)
#################################################################
set(INCS
        "${CMAKE_SOURCE_DIR}/tests/NetTest/src"
)
set(LNKS
        ""
)
set(DEPS
        ""
)
set(HEAD
        "$<$<CONFIG:Debug>:TK_DEBUG>"
        "TK_STATIC_PLUGINS"
)
set(SRCS
        ""
)
########################################################
# dependencies
########################################################
AddTkCore(INCS LNKS DEPS)
AddTkReflect(INCS LNKS DEPS)
AddTkNet(INCS LNKS DEPS)
AddGTest(INCS LNKS)
########################################################
# project configurations
########################################################
file(GLOB_RECURSE SRC
        "src/*.m"
        "src/*.cpp"
        "src/*.mm"
        "src/*.c"
)
add_executable(NetTest
        ${SRC}
        ${SRCS}
)
add_dependencies(NetTest
        ${DEPS}
)
target_include_directories(NetTest PRIVATE
        ${INCS}
)
target_link_libraries(NetTest PRIVATE
        ${LNKS}
)
target_compile_definitions(NetTest PRIVATE
        ${HEAD}
)
target_compile_options(NetTest PRIVATE
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)
