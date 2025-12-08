cmake_minimum_required(VERSION 3.31)

#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
##########################################################
# dependencies
##########################################################
include(${CMAKE_SOURCE_DIR}/cmakes/index.cmake)
include(${CMAKE_SOURCE_DIR}/libs/index.cmake)
##########################################################
project(ToolkitProject)
##########################################################
add_subdirectory(CoreKit)
add_subdirectory(Sandbox)