project(ShaderCompiler)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)
#################################################################
set(INCS
        "${CMAKE_SOURCE_DIR}/tools/ShaderCompiler/src"
)
set(LNKS
        ""
)
set(DEPS
        ""
)
set(HEAD
        "TK_DEBUG"
        "TK_STATIC_PLUGINS"
)
set(SRCS
        ""
)
########################################################
# dependencies
########################################################
AddTkCore(INCS LNKS DEPS)
AddTkGraphics(INCS LNKS DEPS)
########################################################
# project configurations
########################################################
file(GLOB_RECURSE SRC
        "src/*.m"
        "src/*.cpp"
        "src/*.mm"
        "src/*.c"
)
add_executable(ShaderCompiler
        ${SRC}
        ${SRCS}
)
add_dependencies(ShaderCompiler
        ${DEPS}
)
target_include_directories(ShaderCompiler PRIVATE
        ${INCS}
)
target_link_libraries(ShaderCompiler PRIVATE
        ${LNKS}
)
target_compile_definitions(ShaderCompiler PRIVATE
        ${HEAD}
)
