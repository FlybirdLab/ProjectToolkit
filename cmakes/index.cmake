# index file for cmake scripts

# platform configs
include(${CMAKE_SOURCE_DIR}/cmakes/PlatformDeps.cmake)
# module index
include(${CMAKE_SOURCE_DIR}/cmakes/Modules.cmake)
include(${CMAKE_SOURCE_DIR}/tools/RkGen/RkGen.cmake)
