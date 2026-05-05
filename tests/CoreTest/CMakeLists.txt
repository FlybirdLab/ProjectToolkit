project(CoreTest)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)
#################################################################
set(INCS
        "${CMAKE_SOURCE_DIR}/tools/CoreTest/src"
)
set(LNKS
        ""
)
set(DEPS
        ""
)
set(HEAD
        "$<$<CONFIG:Debug>:TK_DEBUG>"
)
set(SRCS
        ""
)
########################################################
# dependencies
########################################################
AddTkCore(INCS LNKS DEPS)

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
add_executable(CoreTest
        ${SRC}
        ${SRCS}
)
add_dependencies(CoreTest
        ${DEPS}
)
target_include_directories(CoreTest PRIVATE
        ${INCS}
)
target_link_libraries(CoreTest PRIVATE
        ${LNKS}
)
target_compile_definitions(CoreTest PRIVATE
        ${HEAD}
)
target_compile_options(CoreTest PRIVATE 
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)