# Created by cubevlmu on 2025/12/8.
# Copyright (c) 2025 Flybird Games. All rights reserved.

# Module index for ToolkitProject

add_subdirectory(CoreKit)
add_subdirectory(MathKit)
add_subdirectory(WindowKit)
add_subdirectory(AudioKit)
add_subdirectory(LocaleKit)
add_subdirectory(GraphicsKit)
add_subdirectory(ReflectKit)

if (NOT TARGET RkGen)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tools/RkGen ${CMAKE_BINARY_DIR}/tools/RkGen)
endif()

add_subdirectory(VideoKit)
add_subdirectory(NetKit)
