# GraphicsKit Sub-project

project(TkGraphics)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-deprecated-literal-operator)
endif()
#################################################################
set(INCS
        "${CMAKE_SOURCE_DIR}/modules/GraphicsKit/src"
)
set(LNKS
        ""
)
set(DEPS
        ""
)
set(HEAD
        ""
)
#################################################################
# multi-backend selection (see cmakes/GraphicsBackends.cmake)
#################################################################
ConfigureGraphicsBackends(INCS LNKS)
#################################################################
# texture tool dependencies
#################################################################
set(WITH_DIRECTXTEX_DEFAULT OFF)
if(WIN32 AND TARGET tbuild::dxt)
        set(WITH_DIRECTXTEX_DEFAULT ON)
        message("Texture loader found -- DirectX Texture")
endif()
set(WITH_ASTC_DEFAULT OFF)
if(TARGET tbuild::astc)
        set(WITH_ASTC_DEFAULT ON)
        message("Texture loader found -- ASTC")
endif()

option(WITH_DIRECTXTEX "Enable DirectXTex texture import/export support." ${WITH_DIRECTXTEX_DEFAULT})
option(WITH_STB "Enable stb texture import/export support." ON)
option(WITH_ASTC "Enable ASTC texture compression support." ${WITH_ASTC_DEFAULT})

set(TK_GRAPHICS_WITH_DIRECTXTEX 0)
set(TK_GRAPHICS_WITH_STB 0)
set(TK_GRAPHICS_WITH_ASTC 0)

if(WITH_DIRECTXTEX)
        if(NOT TARGET tbuild::dxt)
                message(FATAL_ERROR "WITH_DIRECTXTEX requires tbuild::dxt.")
        endif()
        list(APPEND LNKS tbuild::dxt)
        set(TK_GRAPHICS_WITH_DIRECTXTEX 1)
endif()

if(WITH_STB)
        list(APPEND LNKS tbuild::stb)
        set(TK_GRAPHICS_WITH_STB 1)
endif()

if(WITH_ASTC)
        if(NOT TARGET tbuild::astc)
                message(FATAL_ERROR "WITH_ASTC requires tbuild::astc to be available.")
        endif()
        list(APPEND LNKS tbuild::astc)
        set(TK_GRAPHICS_WITH_ASTC 1)
endif()
#################################################################
# generate environment header
#################################################################
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/src/toolkit/graphics/environment.hpp.in"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/toolkit/graphics/environment.hpp"
        @ONLY
)
#################################################################
# dependencies
#################################################################
SetPlatformDeps(INCS LNKS)

AddTkCore(INCS LNKS DEPS)
AddTkMath(INCS LNKS DEPS)
AddTkWindow(INCS LNKS DEPS)
#################################################################
# project configurations
#################################################################
file(GLOB_RECURSE src
        "src/*.m"
        "src/*.cpp"
        "src/*.mm"
        "src/*.c"
)
# Exclude compiler-only sources owned by the TkShaderCompilerCore target.
# `src/toolkit/shader/format/` is intentionally KEPT in TkGraphics so the
# runtime can read .tksd containers without depending on the compiler library.
list(FILTER src EXCLUDE REGEX "/src/toolkit/shader/(core|backends|source|diagnostics|lexer|parser|sema|manifest|compilegraph|cache)/")

add_library(TkGraphics STATIC
        ${src}
)
option(TK_GRAPHICS_LIGHTWEIGHT "Use smaller GraphicsKit defaults for sandbox/smoke-test applications." OFF)
option(TK_GRAPHICS_ENABLE_WINDOW_SRV "Create shader-resource views for window back buffers." ON)
set(TK_GRAPHICS_UNIFORM_RING_BUFFER_SIZE "" CACHE STRING "Override Vulkan uniform ring buffer size in bytes.")

set(TK_GRAPHICS_PRIVATE_DEFINITIONS
        _XOPEN_SOURCE
        _LIBCPP_DRIVERKIT

        $<$<CONFIG:Debug>:TK_DEBUG>
        TK_ENABLE_SIMD
        TK_STATIC_PLUGINS
)

if(TK_GRAPHICS_LIGHTWEIGHT)
        list(APPEND TK_GRAPHICS_PRIVATE_DEFINITIONS
                VK_UNIFORM_RING_BUFFER_SIZE=1048576
                GFX_USE_WINDOW_SRV=0
                GFX_ALLOW_PROFILE_EVENTS=0
                GFX_USE_SHADERS_DEBUG_LAYER=0
        )
elseif(TK_GRAPHICS_UNIFORM_RING_BUFFER_SIZE)
        list(APPEND TK_GRAPHICS_PRIVATE_DEFINITIONS
                VK_UNIFORM_RING_BUFFER_SIZE=${TK_GRAPHICS_UNIFORM_RING_BUFFER_SIZE}
        )
endif()

if(NOT TK_GRAPHICS_ENABLE_WINDOW_SRV)
        list(APPEND TK_GRAPHICS_PRIVATE_DEFINITIONS
                GFX_USE_WINDOW_SRV=0
        )
endif()

target_include_directories(TkGraphics
    PUBLIC
        "${CMAKE_CURRENT_BINARY_DIR}/generated"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PRIVATE
        ${INCS}
)
target_link_libraries(TkGraphics PRIVATE
        ${LNKS}
)
target_compile_definitions(TkGraphics PRIVATE
        ${TK_GRAPHICS_PRIVATE_DEFINITIONS}
)
target_compile_options(TkGraphics PRIVATE
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)

#################################################################
# TkShaderCompilerCore -- shader compilation lib (separate target,
# physically lives under src/toolkit/shader/).
#################################################################
add_subdirectory(src/toolkit/shader)
