# TkShaderCompilerCore -- shader compilation library (independent of TkGraphics runtime).
# Physically lives inside GraphicsKit/src/toolkit/shader/ but built as a separate static
# library target so the runtime TkGraphics never links DXC/glslang/D3DCompiler.

project(TkShaderCompilerCore)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-deprecated-literal-operator)
endif()

set(SCC_INCS
        "${CMAKE_SOURCE_DIR}/modules/GraphicsKit/src"
)
set(SCC_LNKS
        ""
)
set(SCC_DEPS
        ""
)
set(SCC_HEAD
        "$<$<CONFIG:Debug>:TK_DEBUG>"
        "TK_STATIC_PLUGINS"
)

#################################################################
# dependencies -- runtime TkGraphics + backend compiler libs
#################################################################
AddTkCore(SCC_INCS SCC_LNKS SCC_DEPS)
AddTkGraphics(SCC_INCS SCC_LNKS SCC_DEPS)

if(NOT TARGET tbuild::glslang)
        message(FATAL_ERROR "TkShaderCompilerCore requires tbuild::glslang.")
endif()
list(APPEND SCC_LNKS tbuild::glslang)
if(NOT TARGET tbuild::spirv)
        message(FATAL_ERROR "TkShaderCompilerCore requires tbuild::spirv.")
endif()
get_target_property(TK_SPIRV_CROSS_INCLUDE_DIRS tbuild::spirv INTERFACE_INCLUDE_DIRECTORIES)
if(TK_SPIRV_CROSS_INCLUDE_DIRS)
        list(PREPEND SCC_INCS ${TK_SPIRV_CROSS_INCLUDE_DIRS})
endif()
list(APPEND SCC_LNKS tbuild::spirv)
if(NOT TARGET tbuild::dxc)
        message(FATAL_ERROR "TkShaderCompilerCore requires tbuild::dxc.")
endif()
list(APPEND SCC_LNKS tbuild::dxc)
if(NOT TARGET tbuild::vulkan)
        message(FATAL_ERROR "TkShaderCompilerCore requires tbuild::vulkan.")
endif()
list(APPEND SCC_LNKS tbuild::vulkan)
list(APPEND SCC_HEAD COMPILE_WITH_VULKAN_SHADER_COMPILER)
list(APPEND SCC_HEAD COMPILE_WITH_SPIRV_CROSS)

if(WIN32)
    find_package(D3DCompiler REQUIRED)
    list(APPEND SCC_LNKS D3DCompiler::D3DCompiler)
    list(APPEND SCC_HEAD COMPILE_WITH_D3D_SHADER_COMPILER)
endif()

#################################################################
# sources
#################################################################
file(GLOB_RECURSE SCC_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/backends/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/diagnostics/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/lexer/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/parser/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/sema/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/manifest/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/compilegraph/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/cache/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/backends/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/diagnostics/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/lexer/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/parser/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/sema/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/manifest/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/compilegraph/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/cache/*.hpp"
)

add_library(TkShaderCompilerCore STATIC
        ${SCC_SRC}
)
if(SCC_DEPS)
    add_dependencies(TkShaderCompilerCore ${SCC_DEPS})
endif()

target_include_directories(TkShaderCompilerCore
        PUBLIC
                "${CMAKE_SOURCE_DIR}/modules/GraphicsKit/src"
        PRIVATE
                ${SCC_INCS}
)
target_link_libraries(TkShaderCompilerCore
        PUBLIC
                TkGraphics
        PRIVATE
                ${SCC_LNKS}
)
target_compile_definitions(TkShaderCompilerCore PRIVATE
        ${SCC_HEAD}
)
target_compile_options(TkShaderCompilerCore PRIVATE
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)

# Re-export the D3DCompiler runtime DLL location for executables that link this
# library (the ShaderCompiler CLI needs the DLL next to the binary at run-time).
if(WIN32 AND D3DCompiler_DLL)
        set(TK_SHADER_COMPILER_CORE_D3DCOMPILER_DLL
                "${D3DCompiler_DLL}"
                CACHE INTERNAL "Path to D3DCompiler runtime DLL"
        )
endif()
