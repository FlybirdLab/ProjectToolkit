project(MathTest)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)
#################################################################
set(INCS
        "${CMAKE_SOURCE_DIR}/tools/MathTest/src"
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
AddTkMath(INCS LNKS DEPS)

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
add_executable(MathTest
        ${SRC}
        ${SRCS}
)
add_dependencies(MathTest
        ${DEPS}
)
target_include_directories(MathTest PRIVATE
        ${INCS}
)
target_link_libraries(MathTest PRIVATE
        ${LNKS}
)
target_compile_definitions(MathTest PRIVATE
        ${HEAD}
)
target_compile_options(MathTest PRIVATE 
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)
