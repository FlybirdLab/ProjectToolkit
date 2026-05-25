project(Sandbox)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)
#################################################################
set(INCS
        "${CMAKE_SOURCE_DIR}/tools/Sandbox/src"
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
        "SANDBOX_SHADER_SRC_DIR=\"${CMAKE_SOURCE_DIR}/tools/Sandbox/shaders\""
        "SANDBOX_SHADER_COMPILER_PATH=\"$<TARGET_FILE:ShaderCompiler>\""
)
set(SRCS
        ""
)
########################################################
# dependencies
########################################################
AddTkCore(INCS LNKS DEPS)
AddTkWindow(INCS LNKS DEPS)
AddTkMath(INCS LNKS DEPS)
AddTkGraphics(INCS LNKS DEPS)
AddTkReflect(INCS LNKS DEPS)
AddTkNet(INCS LNKS DEPS)

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
        ShaderCompiler
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
