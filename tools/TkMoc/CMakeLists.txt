project(TkMoc)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)
#################################################################
set(INCS
        "${CMAKE_SOURCE_DIR}/tools/TkMoc/src"
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
add_executable(TkMoc
        ${SRC}
        ${SRCS}
)
add_dependencies(TkMoc
        ${DEPS}
)
target_include_directories(TkMoc PRIVATE
        ${INCS}
)
target_link_libraries(TkMoc PRIVATE
        ${LNKS}
)
target_compile_definitions(TkMoc PRIVATE
        ${HEAD}
)
