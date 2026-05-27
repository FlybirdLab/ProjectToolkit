cmake_minimum_required(VERSION 3.31)

#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
set(USE_TEST ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmakes")

if (MSVC)
    add_compile_options(/utf-8)
    add_compile_options(/Zc:char8_t-)
endif()

##########################################################
# dependencies
##########################################################
include(${CMAKE_SOURCE_DIR}/cmakes/index.cmake)
include(${CMAKE_SOURCE_DIR}/libs/index.cmake)
##########################################################
project(ToolkitProject)
##########################################################
add_subdirectory(modules)
ConfigureToolkitInstall()
add_subdirectory(tools)
IF(USE_TEST)
add_subdirectory(tests)
ENDIF()
