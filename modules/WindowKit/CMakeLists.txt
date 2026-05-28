# WindowKit Sub-project

project(TkWindow)
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
        "${CMAKE_SOURCE_DIR}/modules/WindowKit/src"
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

if(WIN32)
    list(APPEND LNKS
            Xinput.lib
    )
elseif(LINUX)
    find_package(X11 REQUIRED)
    list(APPEND INCS
            ${X11_INCLUDE_DIR}
    )
    list(APPEND LNKS
            ${X11_LIBRARIES}
    )
endif()
#################################################################
# project configurations
#################################################################
file(GLOB_RECURSE src
        "src/*.cpp"
        "src/*.c"
)
if(APPLE)
    file(GLOB_RECURSE objc_src
            "src/*.m"
            "src/*.mm"
    )
    list(APPEND src ${objc_src})
endif()
add_library(TkWindow STATIC
        ${src}
)
target_include_directories(TkWindow PRIVATE
        ${INCS}
)
target_link_libraries(TkWindow PRIVATE
        ${LNKS}
)
target_compile_definitions(TkWindow PRIVATE
        _XOPEN_SOURCE
        _LIBCPP_DRIVERKIT

        $<$<CONFIG:Debug>:TK_DEBUG>
        TK_ENABLE_SIMD
        TK_STATIC_PLUGINS
)
target_compile_options(TkWindow PRIVATE 
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)
