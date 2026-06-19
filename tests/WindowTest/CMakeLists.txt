project(WindowTest)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)
#################################################################
set(INCS
        "${CMAKE_SOURCE_DIR}/tests/WindowTest/src"
)
set(LNKS
        ""
)
set(DEPS
        ""
)
set(HEAD
        "$<$<CONFIG:Debug>:TK_DEBUG>"
        "TK_STATIC_PLUGINS"
)
set(SRCS
        ""
)
########################################################
# dependencies
########################################################
AddTkCore(INCS LNKS DEPS)
AddTkMath(INCS LNKS DEPS)
AddTkWindow(INCS LNKS DEPS)

list(APPEND LNKS tpkg::gtest)
########################################################
# project configurations
########################################################
file(GLOB_RECURSE SRC
        "src/*.m"
        "src/*.cpp"
        "src/*.mm"
        "src/*.c"
)
add_executable(WindowTest
        ${SRC}
        ${SRCS}
)
add_dependencies(WindowTest
        ${DEPS}
)

if(WIN32 AND CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
    target_link_directories(WindowTest PRIVATE
            "C:/Windows Kits/10/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/um/x64"
            "C:/Windows Kits/10/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/ucrt/x64"
    )
endif()

target_include_directories(WindowTest PRIVATE
        ${INCS}
)
target_link_libraries(WindowTest PRIVATE
        ${LNKS}
)
target_compile_definitions(WindowTest PRIVATE
        ${HEAD}
)
target_compile_options(WindowTest PRIVATE 
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)
