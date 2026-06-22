project(RkGenTest)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)
#################################################################
set(INCS
        "${CMAKE_SOURCE_DIR}/tests/RkGenTest/src"
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
AddTkMath(INCS LNKS DEPS)

########################################################
# project configurations
########################################################
file(GLOB_RECURSE SRC
        "src/*.m"
        "src/*.cpp"
        "src/*.mm"
        "src/*.c"
)
add_executable(RkGenTest
        ${SRC}
        ${SRCS}
)

tk_target_rkgen(RkGenTest
    RECURSIVE
    SOURCE_DIRS
        src
    OUTPUT_DIR
        ${CMAKE_SOURCE_DIR}/tests/RkGenTest/generated
)

add_dependencies(RkGenTest
        ${DEPS}
        ShaderCompiler
)
target_include_directories(RkGenTest PRIVATE
        ${INCS}
)
target_link_libraries(RkGenTest PRIVATE
        ${LNKS}
)
target_compile_definitions(RkGenTest PRIVATE
        ${HEAD}
)
