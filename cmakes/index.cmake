# index file for cmake scripts

# platform configs
include(${CMAKE_SOURCE_DIR}/cmakes/PlatformDeps.cmake)
# multi-backend graphics configuration
include(${CMAKE_SOURCE_DIR}/cmakes/GraphicsBackends.cmake)
# module index
include(${CMAKE_SOURCE_DIR}/cmakes/Modules.cmake)
include(${CMAKE_SOURCE_DIR}/tools/RkGen/RkGen.cmake)
include(${CMAKE_SOURCE_DIR}/cmakes/Install.cmake)
