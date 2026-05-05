# NetKit Sub-project

project(TkNet)
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
    "${CMAKE_SOURCE_DIR}/modules/NetKit/src"
)
set(LNKS)
set(DEPS)
set(HEAD)

#################################################################
# dependencies
#################################################################
SetPlatformDeps(INCS LNKS)

AddTkCore(INCS LNKS DEPS)
#################################################################
# backend option
#################################################################

set(TK_NET_BACKEND_VALUES
    enet
    kcp
)

set(TK_NET_BACKEND_DEFAULT enet)

if(NOT DEFINED TK_NET_BACKEND)
    set(TK_NET_BACKEND "${TK_NET_BACKEND_DEFAULT}" CACHE STRING "TkNet backend")
endif()

string(TOLOWER "${TK_NET_BACKEND}" TK_NET_BACKEND)
set(TK_NET_BACKEND "${TK_NET_BACKEND}" CACHE STRING "TkNet backend" FORCE)
set_property(CACHE TK_NET_BACKEND PROPERTY STRINGS ${TK_NET_BACKEND_VALUES})

list(FIND TK_NET_BACKEND_VALUES "${TK_NET_BACKEND}" TK_NET_BACKEND_INDEX)
if(TK_NET_BACKEND_INDEX EQUAL -1)
    message(FATAL_ERROR
        "Invalid TK_NET_BACKEND='${TK_NET_BACKEND}'. "
        "Valid values are: ${TK_NET_BACKEND_VALUES}"
    )
endif()

set(TK_NET_BACKEND_ENET 0)
set(TK_NET_BACKEND_KCP 0)

if(TK_NET_BACKEND STREQUAL "enet")
    set(TK_NET_BACKEND_ENET 1)
    AddENet(INCS LNKS)

elseif(TK_NET_BACKEND STREQUAL "kcp")
    set(TK_NET_BACKEND_KCP 1)
    AddKCP(INCS LNKS)

endif()

message(STATUS "TkNet backend: ${TK_NET_BACKEND}")

#################################################################
# generate environment header
#################################################################

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/toolkit/net/environment.hpp.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/toolkit/net/environment.hpp"
    @ONLY
)

#################################################################
# project configurations
#################################################################
file(GLOB_RECURSE src
    "src/*.m"
    "src/*.cpp"
    "src/*.mm"
    "src/*.c"
)

add_library(TkNet STATIC
    ${src}
)

target_include_directories(TkNet PRIVATE
    ${INCS}
)

target_link_libraries(TkNet PRIVATE
    ${LNKS}
)

target_compile_definitions(TkNet PRIVATE
    _XOPEN_SOURCE
    _LIBCPP_DRIVERKIT
    $<$<CONFIG:Debug>:TK_DEBUG>
    TK_ENABLE_SIMD
    TK_STATIC_PLUGINS
)

target_compile_options(TkNet PRIVATE
    "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)