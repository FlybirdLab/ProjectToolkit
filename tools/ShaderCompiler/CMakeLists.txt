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
        "$<$<CONFIG:Debug>:TK_DEBUG>"
        "TK_STATIC_PLUGINS"
)
set(SRCS
        ""
)
########################################################
# dependencies -- only the public TkShaderCompilerCore facade
########################################################
AddTkCore(INCS LNKS DEPS)
AddTkGraphics(INCS LNKS DEPS)
AddTkShaderCompilerCore(INCS LNKS DEPS)
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

# Copy the D3DCompiler runtime DLL next to the executable. The DLL path is
# discovered by TkShaderCompilerCore (which already calls find_package(D3DCompiler)).
if(WIN32 AND TK_SHADER_COMPILER_CORE_D3DCOMPILER_DLL)
    add_custom_command(TARGET ShaderCompiler POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${TK_SHADER_COMPILER_CORE_D3DCOMPILER_DLL}"
            $<TARGET_FILE_DIR:ShaderCompiler>
    )

    install(FILES "${TK_SHADER_COMPILER_CORE_D3DCOMPILER_DLL}"
        DESTINATION bin
    )
endif()
