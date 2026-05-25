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
# pick the default runtime backend for the sandbox
########################################################
# TkGraphics may be compiled with several backends at once. The sandbox
# compiles them all in and selects one at runtime via --backend=<name>.
# This option only sets the *default* when --backend is not passed; users can
# still pick any compiled-in backend from the command line.
set(SANDBOX_GFX_BACKEND_VALUES auto dx12 dx11 vulkan)
set(SANDBOX_GFX_BACKEND "auto" CACHE STRING "Sandbox default graphics backend (auto|dx12|dx11|vulkan). 'auto' = first compiled-in (preference: dx12 > dx11 > vulkan).")
set_property(CACHE SANDBOX_GFX_BACKEND PROPERTY STRINGS ${SANDBOX_GFX_BACKEND_VALUES})
string(TOLOWER "${SANDBOX_GFX_BACKEND}" SANDBOX_GFX_BACKEND)

set(_sandbox_default "")
if(SANDBOX_GFX_BACKEND STREQUAL "auto")
    # Empty default - GraphicsSandbox falls back to the first compiled-in backend.
    set(_sandbox_default "")
elseif(SANDBOX_GFX_BACKEND STREQUAL "dx12")
    if(NOT TK_GFX_DIRECTX12_ENABLE)
        message(FATAL_ERROR "SANDBOX_GFX_BACKEND=dx12 requires TK_GFX_DIRECTX12_ENABLE=ON.")
    endif()
    set(_sandbox_default "dx12")
elseif(SANDBOX_GFX_BACKEND STREQUAL "dx11")
    if(NOT TK_GFX_DIRECTX11_ENABLE)
        message(FATAL_ERROR "SANDBOX_GFX_BACKEND=dx11 requires TK_GFX_DIRECTX11_ENABLE=ON.")
    endif()
    set(_sandbox_default "dx11")
elseif(SANDBOX_GFX_BACKEND STREQUAL "vulkan")
    if(NOT TK_GFX_VULKAN_ENABLE)
        message(FATAL_ERROR "SANDBOX_GFX_BACKEND=vulkan requires TK_GFX_VULKAN_ENABLE=ON.")
    endif()
    set(_sandbox_default "vulkan")
else()
    message(FATAL_ERROR "Invalid SANDBOX_GFX_BACKEND='${SANDBOX_GFX_BACKEND}'. Valid: ${SANDBOX_GFX_BACKEND_VALUES}")
endif()

if(NOT (TK_GFX_DIRECTX12_ENABLE OR TK_GFX_DIRECTX11_ENABLE OR TK_GFX_VULKAN_ENABLE))
    message(FATAL_ERROR "Sandbox needs at least one of TK_GFX_DIRECTX12_ENABLE / TK_GFX_DIRECTX11_ENABLE / TK_GFX_VULKAN_ENABLE to be ON.")
endif()

list(APPEND HEAD "SANDBOX_DEFAULT_BACKEND_NAME=\"${_sandbox_default}\"")
if(_sandbox_default STREQUAL "")
    message(STATUS "Sandbox default backend: auto (first compiled-in)")
else()
    message(STATUS "Sandbox default backend: ${_sandbox_default}")
endif()
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
