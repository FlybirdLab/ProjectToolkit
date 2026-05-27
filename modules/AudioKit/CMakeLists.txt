# AudioKit Sub-project

project(TkAudio)
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
        "${CMAKE_SOURCE_DIR}/modules/AudioKit/src"
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
# dependencies
#################################################################
SetPlatformDeps(INCS LNKS)

AddTkCore(INCS LNKS DEPS)
AddTkMath(INCS LNKS DEPS)

AddMiniMp3(INCS LNKS)
option(TK_AUDIO_ENABLE_OGG "Enable Ogg Vorbis codec" OFF)
if(TK_AUDIO_ENABLE_OGG)
    AddVorbis(INCS LNKS)
endif()
if(WIN32)
    AddMP4V2(INCS LNKS)
endif()
#################################################################
# backend option
#################################################################

set(TK_AUDIO_BACKEND_VALUES
    none
    xaudio
    openal
    alsasound
    coreaudio
)

if(WIN32)
    set(TK_AUDIO_BACKEND_DEFAULT xaudio)
elseif(APPLE)
    set(TK_AUDIO_BACKEND_DEFAULT coreaudio)
elseif(UNIX AND NOT APPLE)
    set(TK_AUDIO_BACKEND_DEFAULT alsasound)
else()
    set(TK_AUDIO_BACKEND_DEFAULT openal)
endif()

set(TK_AUDIO_BACKEND "${TK_AUDIO_BACKEND_DEFAULT}" CACHE STRING "TkAudio backend")

string(TOLOWER "${TK_AUDIO_BACKEND}" TK_AUDIO_BACKEND)
set(TK_AUDIO_BACKEND "${TK_AUDIO_BACKEND}" CACHE STRING "TkAudio backend" FORCE)
set_property(CACHE TK_AUDIO_BACKEND PROPERTY STRINGS ${TK_AUDIO_BACKEND_VALUES})

list(FIND TK_AUDIO_BACKEND_VALUES "${TK_AUDIO_BACKEND}" TK_AUDIO_BACKEND_INDEX)
if(TK_AUDIO_BACKEND_INDEX EQUAL -1)
    message(FATAL_ERROR
        "Invalid TK_AUDIO_BACKEND='${TK_AUDIO_BACKEND}'. "
        "Valid values are: ${TK_AUDIO_BACKEND_VALUES}"
    )
endif()

set(TK_AUDIO_BACKEND_NONE 0)
set(TK_AUDIO_BACKEND_XAUDIO 0)
set(TK_AUDIO_BACKEND_OPENAL 0)
set(TK_AUDIO_BACKEND_ALSASOUND 0)
set(TK_AUDIO_BACKEND_COREAUDIO 0)
set(TK_CODECS_VORBIS 0)
if(TK_AUDIO_ENABLE_OGG)
    set(TK_CODECS_VORBIS 1)
endif()
set(TK_CODECS_M4A 0)
if(WIN32)
    set(TK_CODECS_M4A 1)
endif()

if(TK_AUDIO_BACKEND STREQUAL "none")
    set(TK_AUDIO_BACKEND_NONE 1)

elseif(TK_AUDIO_BACKEND STREQUAL "xaudio")
    if(NOT WIN32)
        message(FATAL_ERROR "TkAudio backend 'xaudio' is only supported on Windows")
    endif()

    set(TK_AUDIO_BACKEND_XAUDIO 1)
    list(APPEND LNKS Xaudio2.lib)

elseif(TK_AUDIO_BACKEND STREQUAL "openal")
    set(TK_AUDIO_BACKEND_OPENAL 1)

    AddOpenAL(INCS LNKS)

elseif(TK_AUDIO_BACKEND STREQUAL "alsasound")
    if(NOT (UNIX AND NOT APPLE))
        message(FATAL_ERROR "TkAudio backend 'alsasound' is only supported on Linux/Unix")
    endif()

    set(TK_AUDIO_BACKEND_ALSASOUND 1)
    AddAlsaSound(INCS LNKS)

elseif(TK_AUDIO_BACKEND STREQUAL "coreaudio")
    if(NOT APPLE)
        message(FATAL_ERROR "TkAudio backend 'coreaudio' is only supported on Apple platforms")
    endif()

    set(TK_AUDIO_BACKEND_COREAUDIO 1)
    AddCoreAudio(INCS LNKS)

endif()

message(STATUS "TkAudio backend: ${TK_AUDIO_BACKEND}")

#################################################################
# generate environment header
#################################################################

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/toolkit/audio/environment.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/toolkit/audio/environment.hpp"
    @ONLY
)
set(TK_AUDIO_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

#################################################################
# project configurations
#################################################################
file(GLOB_RECURSE src
        "src/*.m"
        "src/*.cpp"
        "src/*.mm"
        "src/*.c"
)
add_library(TkAudio STATIC
        ${src}
)
target_include_directories(TkAudio PUBLIC
        "${TK_AUDIO_GENERATED_DIR}"
        ${INCS}
)
target_link_libraries(TkAudio PRIVATE
        ${LNKS}
)
target_compile_definitions(TkAudio PRIVATE
        TK_STATIC_PLUGINS
)
target_compile_options(TkAudio PRIVATE 
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)
