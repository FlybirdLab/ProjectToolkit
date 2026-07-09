cmake_minimum_required(VERSION 3.31)

# Keep MSVC diagnostic/dependency output stable for Ninja's msvc deps parser.
# Localized /showIncludes output is not reliably filtered by Ninja in every shell codepage.
if(WIN32)
    set(ENV{VSLANG} "1033")
endif()

# Disable external headers feature on MSVC to prevent -external: flags
# MUST be set BEFORE project()
set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "")

#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(USE_TEST ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmakes")

##########################################################
project(ToolkitProject)
##########################################################

if (MSVC)
    set(CMAKE_C_COMPILER_LAUNCHER "${CMAKE_COMMAND};-E;env;VSLANG=1033")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CMAKE_COMMAND};-E;env;VSLANG=1033")
    add_compile_options(/utf-8)
endif()

##########################################################
# Toolkit Package Manager integration
##########################################################
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/.tpkg/generated/cmake")
find_package(tpkg CONFIG REQUIRED)

##########################################################
# dependencies
##########################################################
include(${CMAKE_SOURCE_DIR}/cmakes/index.cmake)
if(EXISTS "${CMAKE_SOURCE_DIR}/libs/index.cmake")
    include(${CMAKE_SOURCE_DIR}/libs/index.cmake)
endif()
##########################################################
add_subdirectory(modules)
ConfigureToolkitInstall()
add_subdirectory(tools)
IF(USE_TEST)
add_subdirectory(tests)
ENDIF()
