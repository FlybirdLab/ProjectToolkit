# GraphicsBackends.cmake
#
# Multi-backend graphics configuration for TkGraphics.
#
# Each backend is detected independently. A backend is "available" when its
# dependencies can be located on the current host/platform. The user can
# override availability per backend with cache options of the form
# TK_GFX_<NAME>_ENABLE; by default an available backend is enabled.
#
# Public entry points:
#   DetectGraphicsBackends()                         - runs detection, populates
#                                                       per-backend AVAILABLE/ENABLE
#                                                       cache flags. Idempotent.
#   ConfigureGraphicsBackends(incVar lnkVar)         - applies all enabled
#                                                       backends to the given
#                                                       include/link lists, and
#                                                       exposes the integer
#                                                       TK_GRAPHICS_BACKEND_<NAME>
#                                                       variables expected by
#                                                       environment.hpp.in.
#
# Per-backend variables exposed after DetectGraphicsBackends():
#   TK_GFX_<NAME>_AVAILABLE  - BOOL, set by detection (cannot be overridden)
#   TK_GFX_<NAME>_ENABLE     - BOOL cache option, defaults to AVAILABLE
#
# Backend names: NULL, DIRECTX11, DIRECTX12, VULKAN, OPENGL, OPENGLES, METAL
#
# Backends whose runtime implementation is not yet wired into the source tree
# are listed in TK_GFX_UNIMPLEMENTED_BACKENDS and default to OFF even when
# their SDK is available, to avoid link errors.

if(DEFINED _TK_GRAPHICS_BACKENDS_INCLUDED)
    return()
endif()
set(_TK_GRAPHICS_BACKENDS_INCLUDED TRUE)

set(TK_GFX_ALL_BACKENDS NULL DIRECTX11 DIRECTX12 VULKAN OPENGL OPENGLES METAL)
# Backends with no source-level implementation yet. They are still detected so
# the option is visible in the cache, but they will not be enabled by default.
#
# OPENGL is implemented as a Phase-1 skeleton (no draw / no real GL context
# yet); it is removed from the unimplemented list so it can be opted into with
# -DTK_GFX_OPENGL_ENABLE=ON.
set(TK_GFX_UNIMPLEMENTED_BACKENDS NULL OPENGLES METAL)

# ----------------------------------------------------------------------------
# Detection helpers
# ----------------------------------------------------------------------------

function(_tk_gfx_set_available name value)
    set(TK_GFX_${name}_AVAILABLE ${value} CACHE INTERNAL "TkGraphics ${name} backend availability")
endfunction()

function(_tk_gfx_declare_enable name)
    set(_default ${TK_GFX_${name}_AVAILABLE})
    list(FIND TK_GFX_UNIMPLEMENTED_BACKENDS ${name} _unimpl_idx)
    set(_unimpl FALSE)
    if(NOT _unimpl_idx EQUAL -1)
        set(_unimpl TRUE)
        set(_default OFF)
    endif()
    if(NOT DEFINED TK_GFX_${name}_ENABLE)
        option(TK_GFX_${name}_ENABLE "Enable TkGraphics ${name} backend" ${_default})
    endif()
    # If user enabled a backend that is not available, force it back off.
    if(TK_GFX_${name}_ENABLE AND NOT TK_GFX_${name}_AVAILABLE)
        message(WARNING "TkGraphics backend ${name} is not available on this host; disabling.")
        set(TK_GFX_${name}_ENABLE OFF CACHE BOOL "Enable TkGraphics ${name} backend" FORCE)
    endif()
    # Unimplemented backends are gated by TK_GFX_<NAME>_ALLOW_UNIMPLEMENTED.
    # This catches stale ON values from a previous configure that pre-dated the
    # unimplemented list, and forces a clean "off until user really opts in".
    if(_unimpl AND TK_GFX_${name}_ENABLE AND NOT TK_GFX_${name}_ALLOW_UNIMPLEMENTED)
        message(STATUS "TkGraphics backend ${name} has no runtime implementation yet; forcing OFF. Set TK_GFX_${name}_ALLOW_UNIMPLEMENTED=ON to override.")
        set(TK_GFX_${name}_ENABLE OFF CACHE BOOL "Enable TkGraphics ${name} backend" FORCE)
    elseif(_unimpl AND TK_GFX_${name}_ENABLE AND TK_GFX_${name}_ALLOW_UNIMPLEMENTED)
        message(WARNING "TkGraphics backend ${name} has no runtime implementation in the source tree yet; linking may fail.")
    endif()
endfunction()

function(_tk_gfx_detect_directx11)
    if(WIN32)
        _tk_gfx_set_available(DIRECTX11 TRUE)
    else()
        _tk_gfx_set_available(DIRECTX11 FALSE)
    endif()
endfunction()

function(_tk_gfx_detect_directx12)
    if(WIN32)
        _tk_gfx_set_available(DIRECTX12 TRUE)
    else()
        _tk_gfx_set_available(DIRECTX12 FALSE)
    endif()
endfunction()

function(_tk_gfx_detect_vulkan)
    find_package(Vulkan QUIET)
    if(Vulkan_FOUND)
        _tk_gfx_set_available(VULKAN TRUE)
    else()
        _tk_gfx_set_available(VULKAN FALSE)
    endif()
endfunction()

function(_tk_gfx_detect_opengl)
    # macOS deprecated the OpenGL framework in 10.14 but still ships it with a
    # 4.1 Core profile -- which is exactly what this fallback backend targets.
    # We detect it like any other platform; the deprecation warnings are
    # silenced inside platform/macos/GLPlatformMacOS.mm via pragma diagnostic.
    find_package(OpenGL QUIET)
    if(OPENGL_FOUND OR OpenGL_FOUND)
        _tk_gfx_set_available(OPENGL TRUE)
    else()
        _tk_gfx_set_available(OPENGL FALSE)
    endif()
endfunction()

function(_tk_gfx_detect_opengles)
    # Not wired in the backend yet.
    _tk_gfx_set_available(OPENGLES FALSE)
endfunction()

function(_tk_gfx_detect_metal)
    if(APPLE)
        find_library(_TK_GFX_METAL_LIB Metal)
        if(_TK_GFX_METAL_LIB)
            _tk_gfx_set_available(METAL TRUE)
        else()
            _tk_gfx_set_available(METAL FALSE)
        endif()
    else()
        _tk_gfx_set_available(METAL FALSE)
    endif()
endfunction()

function(_tk_gfx_detect_null)
    _tk_gfx_set_available(NULL TRUE)
endfunction()

function(DetectGraphicsBackends)
    _tk_gfx_detect_null()
    _tk_gfx_detect_directx11()
    _tk_gfx_detect_directx12()
    _tk_gfx_detect_vulkan()
    _tk_gfx_detect_opengl()
    _tk_gfx_detect_opengles()
    _tk_gfx_detect_metal()

    foreach(name IN LISTS TK_GFX_ALL_BACKENDS)
        _tk_gfx_declare_enable(${name})
    endforeach()
endfunction()

# ----------------------------------------------------------------------------
# Configuration / wiring
# ----------------------------------------------------------------------------

function(ConfigureGraphicsBackends incVar lnkVar)
    DetectGraphicsBackends()

    # Mutual-exclusion: OPENGL and OPENGLES share environment macros / sources
    # and cannot both be ON at once. Fail fast at configure time rather than at
    # link time.
    if(TK_GFX_OPENGL_ENABLE AND TK_GFX_OPENGLES_ENABLE)
        message(FATAL_ERROR "TkGraphics: TK_GFX_OPENGL_ENABLE and TK_GFX_OPENGLES_ENABLE cannot both be ON. Choose one.")
    endif()

    set(_enabled "")
    set(_disabled "")

    foreach(name IN LISTS TK_GFX_ALL_BACKENDS)
        if(TK_GFX_${name}_ENABLE)
            set(TK_GRAPHICS_BACKEND_${name} 1 PARENT_SCOPE)
            list(APPEND _enabled ${name})
        else()
            set(TK_GRAPHICS_BACKEND_${name} 0 PARENT_SCOPE)
            list(APPEND _disabled ${name})
        endif()
    endforeach()

    # "None" is exposed for the environment header; it is implicit (all-off).
    if(_enabled STREQUAL "")
        set(TK_GRAPHICS_BACKEND_NONE 1 PARENT_SCOPE)
    else()
        set(TK_GRAPHICS_BACKEND_NONE 0 PARENT_SCOPE)
    endif()

    # Apply each enabled backend's dependencies.
    if(TK_GFX_DIRECTX11_ENABLE)
        AddDX11(${incVar} ${lnkVar})
    endif()
    if(TK_GFX_DIRECTX12_ENABLE)
        AddDX12(${incVar} ${lnkVar})
    endif()
    if(TK_GFX_VULKAN_ENABLE)
        AddVulkanSDK(${incVar} ${lnkVar})
        AddVolk(${incVar} ${lnkVar})
        AddVMA(${incVar} ${lnkVar})
    endif()
    if(TK_GFX_OPENGL_ENABLE)
        find_package(OpenGL REQUIRED)
        list(APPEND ${lnkVar} OpenGL::GL)
        # glad provides the function-pointer loader used by the OpenGL backend.
        AddGLAD(${incVar} ${lnkVar})
        if(APPLE)
            find_library(_TK_OGL_COCOA_FW Cocoa REQUIRED)
            list(APPEND ${lnkVar} ${_TK_OGL_COCOA_FW})
        endif()
    endif()
    if(TK_GFX_METAL_ENABLE)
        find_library(METAL_FRAMEWORK Metal REQUIRED)
        find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
        find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
        list(APPEND ${lnkVar}
            ${METAL_FRAMEWORK}
            ${QUARTZCORE_FRAMEWORK}
            ${FOUNDATION_FRAMEWORK}
        )
    endif()

    set(${incVar} ${${incVar}} PARENT_SCOPE)
    set(${lnkVar} ${${lnkVar}} PARENT_SCOPE)

    string(REPLACE ";" " " _enabled_pretty "${_enabled}")
    if(_enabled_pretty STREQUAL "")
        set(_enabled_pretty "<none>")
    endif()
    string(REPLACE ";" " " _disabled_pretty "${_disabled}")
    if(_disabled_pretty STREQUAL "")
        set(_disabled_pretty "<none>")
    endif()
    message(STATUS "TkGraphics backends enabled: ${_enabled_pretty}")
    message(STATUS "TkGraphics backends disabled: ${_disabled_pretty}")
endfunction()
