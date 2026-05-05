# GraphicsKit Sub-project

project(TkGraphics)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-deprecated-literal-operator)
endif()
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fms-extensions")
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
# backend option
#################################################################
set(TK_GRAPHICS_BACKEND_VALUES
        none
        null
        directx11
        directx12
        vulkan
        opengl
        opengles
        metal
)

if(WIN32)
    set(TK_GRAPHICS_BACKEND_DEFAULT vulkan) # directx12
elseif(APPLE)
    set(TK_GRAPHICS_BACKEND_DEFAULT metal)
elseif(UNIX AND NOT APPLE)
    set(TK_GRAPHICS_BACKEND_DEFAULT vulkan)
else()
    set(TK_GRAPHICS_BACKEND_DEFAULT null)
endif()

set(TK_GRAPHICS_BACKEND "${TK_GRAPHICS_BACKEND_DEFAULT}" CACHE STRING "TkGraphics backend")

string(TOLOWER "${TK_GRAPHICS_BACKEND}" TK_GRAPHICS_BACKEND)
set(TK_GRAPHICS_BACKEND "${TK_GRAPHICS_BACKEND}" CACHE STRING "TkGraphics backend" FORCE)
set_property(CACHE TK_GRAPHICS_BACKEND PROPERTY STRINGS ${TK_GRAPHICS_BACKEND_VALUES})

list(FIND TK_GRAPHICS_BACKEND_VALUES "${TK_GRAPHICS_BACKEND}" TK_GRAPHICS_BACKEND_INDEX)
if(TK_GRAPHICS_BACKEND_INDEX EQUAL -1)
    message(FATAL_ERROR
            "Invalid TK_GRAPHICS_BACKEND='${TK_GRAPHICS_BACKEND}'. "
            "Valid values are: ${TK_GRAPHICS_BACKEND_VALUES}"
    )
endif()

set(TK_GRAPHICS_BACKEND_NONE 0)
set(TK_GRAPHICS_BACKEND_NULL 0)
set(TK_GRAPHICS_BACKEND_DIRECTX11 0)
set(TK_GRAPHICS_BACKEND_DIRECTX12 0)
set(TK_GRAPHICS_BACKEND_VULKAN 0)
set(TK_GRAPHICS_BACKEND_OPENGL 0)
set(TK_GRAPHICS_BACKEND_OPENGLES 0)
set(TK_GRAPHICS_BACKEND_METAL 0)

if(TK_GRAPHICS_BACKEND STREQUAL "none")
    set(TK_GRAPHICS_BACKEND_NONE 1)

elseif(TK_GRAPHICS_BACKEND STREQUAL "null")
    set(TK_GRAPHICS_BACKEND_NULL 1)

elseif(TK_GRAPHICS_BACKEND STREQUAL "directx11")
    if(NOT WIN32)
        message(FATAL_ERROR "TkGraphics backend 'directx11' is only supported on Windows")
    endif()
    set(TK_GRAPHICS_BACKEND_DIRECTX11 1)
    list(APPEND LNKS d3d11.lib dxgi.lib dxguid.lib d3dcompiler.lib)

elseif(TK_GRAPHICS_BACKEND STREQUAL "directx12")
    if(NOT WIN32)
        message(FATAL_ERROR "TkGraphics backend 'directx12' is only supported on Windows")
    endif()
    set(TK_GRAPHICS_BACKEND_DIRECTX12 1)
    list(APPEND LNKS d3d12.lib dxgi.lib dxguid.lib d3dcompiler.lib)

elseif(TK_GRAPHICS_BACKEND STREQUAL "vulkan")
    set(TK_GRAPHICS_BACKEND_VULKAN 1)
    AddVulkanSDK(INCS LNKS)
    AddVolk(INCS LNKS)
    AddVMA(INCS LNKS)

elseif(TK_GRAPHICS_BACKEND STREQUAL "opengl")
    set(TK_GRAPHICS_BACKEND_OPENGL 1)
    find_package(OpenGL REQUIRED)
    list(APPEND LNKS OpenGL::GL)

elseif(TK_GRAPHICS_BACKEND STREQUAL "opengles")
    set(TK_GRAPHICS_BACKEND_OPENGLES 1)
    message(FATAL_ERROR "TkGraphics backend 'opengles' is not wired yet")

elseif(TK_GRAPHICS_BACKEND STREQUAL "metal")
    if(NOT APPLE)
        message(FATAL_ERROR "TkGraphics backend 'metal' is only supported on Apple platforms")
    endif()
    set(TK_GRAPHICS_BACKEND_METAL 1)
    find_library(METAL_FRAMEWORK Metal REQUIRED)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
    list(APPEND LNKS
            ${METAL_FRAMEWORK}
            ${QUARTZCORE_FRAMEWORK}
            ${FOUNDATION_FRAMEWORK}
    )

endif()

message(STATUS "TkGraphics backend: ${TK_GRAPHICS_BACKEND}")

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
list(FILTER src EXCLUDE REGEX ".*/backends/null/.*")
list(FILTER src EXCLUDE REGEX ".*/backends/dx11/.*")
list(FILTER src EXCLUDE REGEX ".*/backends/dx12/.*")
list(FILTER src EXCLUDE REGEX ".*/backends/vulkan/.*")
list(FILTER src EXCLUDE REGEX ".*/backends/opengl/.*")
list(FILTER src EXCLUDE REGEX ".*/backends/opengles/.*")
list(FILTER src EXCLUDE REGEX ".*/backends/metal/.*")

if(TK_GRAPHICS_BACKEND_NULL)
    file(GLOB_RECURSE backend_src "src/toolkit/graphics/backends/null/*.cpp" "src/toolkit/graphics/backends/null/*.c" "src/toolkit/graphics/backends/null/*.m" "src/toolkit/graphics/backends/null/*.mm")
    list(APPEND src ${backend_src})
elseif(TK_GRAPHICS_BACKEND_DIRECTX11)
    file(GLOB_RECURSE backend_src "src/toolkit/graphics/backends/dx11/*.cpp" "src/toolkit/graphics/backends/dx11/*.c")
    list(APPEND src ${backend_src})
elseif(TK_GRAPHICS_BACKEND_DIRECTX12)
    file(GLOB_RECURSE backend_src "src/toolkit/graphics/backends/dx12/*.cpp" "src/toolkit/graphics/backends/dx12/*.c")
    list(APPEND src ${backend_src})
elseif(TK_GRAPHICS_BACKEND_VULKAN)
    file(GLOB_RECURSE backend_src "src/toolkit/graphics/backends/vulkan/*.cpp" "src/toolkit/graphics/backends/vulkan/*.c")
    list(APPEND src ${backend_src})
elseif(TK_GRAPHICS_BACKEND_OPENGL)
    file(GLOB_RECURSE backend_src "src/toolkit/graphics/backends/opengl/*.cpp" "src/toolkit/graphics/backends/opengl/*.c")
    list(APPEND src ${backend_src})
elseif(TK_GRAPHICS_BACKEND_OPENGLES)
    file(GLOB_RECURSE backend_src "src/toolkit/graphics/backends/opengles/*.cpp" "src/toolkit/graphics/backends/opengles/*.c")
    list(APPEND src ${backend_src})
elseif(TK_GRAPHICS_BACKEND_METAL)
    file(GLOB_RECURSE backend_src "src/toolkit/graphics/backends/metal/*.cpp" "src/toolkit/graphics/backends/metal/*.m" "src/toolkit/graphics/backends/metal/*.mm")
    list(APPEND src ${backend_src})
endif()

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
