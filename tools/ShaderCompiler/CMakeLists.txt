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
# dependencies
########################################################
AddTkCore(INCS LNKS DEPS)
AddTkGraphics(INCS LNKS DEPS)

AddGlslang(INCS LNKS DEPS)
AddDXC(INCS LNKS DEPS)
AddVulkanSDK(INCS LNKS DEPS)
list(APPEND HEAD COMPILE_WITH_VULKAN_SHADER_COMPILER)

if(WIN32)
    find_package(D3DCompiler REQUIRED)
    list(APPEND LNKS D3DCompiler::D3DCompiler)
    list(APPEND HEAD COMPILE_WITH_D3D_SHADER_COMPILER)
endif()
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

if(WIN32 AND D3DCompiler_DLL)
    add_custom_command(TARGET ShaderCompiler POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${D3DCompiler_DLL}"
            $<TARGET_FILE_DIR:ShaderCompiler>
    )

    install(FILES "${D3DCompiler_DLL}"
        DESTINATION bin
    )
endif()
