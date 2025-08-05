project(Sandbox)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)
#################################################################
set(INCS
        "src"
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
add_executable(Sandbox
        ${SRC}
        ${SRCS}
)
add_dependencies(Sandbox
        ${DEPS}
)
target_include_directories(Sandbox PRIVATE
        ${INCS}
)
target_link_libraries(Sandbox PRIVATE
        ${LNKS}
)
target_compile_definitions(Sandbox PRIVATE
        ${HEAD}
)
