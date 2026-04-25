project(RkGen)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)
#################################################################
set(INCS
        "${CMAKE_SOURCE_DIR}/tools/RkGen/src"
)
set(LNKS
        ""
)
set(DEPS
        ""
)
set(HEAD
        "TK_DEBUG"
)
set(SRCS
        ""
)
########################################################
# dependencies
########################################################
AddTkCore(INCS LNKS DEPS)

########################################################
# project configurations
########################################################
file(GLOB_RECURSE SRC
        "src/*.m"
        "src/*.cpp"
        "src/*.mm"
        "src/*.c"
)
add_executable(RkGen
        ${SRC}
        ${SRCS}
)
add_dependencies(RkGen
        ${DEPS}
)
target_include_directories(RkGen PRIVATE
        ${INCS}
)
target_link_libraries(RkGen PRIVATE
        ${LNKS}
)
target_compile_definitions(RkGen PRIVATE
        ${HEAD}
)
