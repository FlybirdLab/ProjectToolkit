cmake_minimum_required(VERSION 3.31)

# Disable external headers feature on MSVC to prevent -external: flags
# MUST be set BEFORE project()
set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "")

#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(USE_TEST ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmakes")

##########################################################
project(ToolkitProject)
##########################################################

if (MSVC)
    add_compile_options(/utf-8)
endif()

##########################################################
# ToolkitBuild integration
##########################################################
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/.tbuild/generated/cmake")
find_package(tbuild CONFIG REQUIRED)

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
