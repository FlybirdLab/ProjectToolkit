project(ReflectTest)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)
#################################################################
set(INCS
        "${CMAKE_SOURCE_DIR}/tests/ReflectTest/src"
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
list(APPEND LNKS tbuild::gtest)
########################################################
# project configurations
########################################################
file(GLOB_RECURSE SRC
        "src/*.m"
        "src/*.cpp"
        "src/*.mm"
        "src/*.c"
)
add_executable(ReflectTest
        ${SRC}
        ${SRCS}
)
add_dependencies(ReflectTest
        ${DEPS}
)
target_include_directories(ReflectTest PRIVATE
        ${INCS}
)
target_link_libraries(ReflectTest PRIVATE
        ${LNKS}
)
target_compile_definitions(ReflectTest PRIVATE
        ${HEAD}
)
target_compile_options(ReflectTest PRIVATE
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)
