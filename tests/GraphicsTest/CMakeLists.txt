project(GraphicsTest)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)
#################################################################
set(INCS
        "${CMAKE_SOURCE_DIR}/tests/GraphicsTest/src"
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
AddTkGraphics(INCS LNKS DEPS)

        if(NOT TARGET tbuild::vulkan)
            message(FATAL_ERROR "GraphicsTest requires tbuild::vulkan.")
        endif()
        list(APPEND LNKS tbuild::vulkan)
        if(NOT TARGET tbuild::volk)
            message(FATAL_ERROR "GraphicsTest requires tbuild::volk.")
        endif()
        list(APPEND LNKS tbuild::volk)
        if(NOT TARGET tbuild::vma)
            message(FATAL_ERROR "GraphicsTest requires tbuild::vma.")
        endif()
        list(APPEND LNKS tbuild::vma)

list(APPEND LNKS tbuild::gtest)
########################################################
# project configurations
########################################################
file(GLOB_RECURSE SRC
        "src/*.m"
        "src/*.cpp"
        "src/*.mm"
        "src/*.c"
)
add_executable(GraphicsTest
        ${SRC}
        ${SRCS}
)
add_dependencies(GraphicsTest
        ${DEPS}
)

if(WIN32 AND CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
    target_link_directories(GraphicsTest PRIVATE
            "C:/Windows Kits/10/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/um/x64"
            "C:/Windows Kits/10/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/ucrt/x64"
    )
endif()

target_include_directories(GraphicsTest PRIVATE
        ${INCS}
)
target_link_libraries(GraphicsTest PRIVATE
        ${LNKS}
)
target_compile_definitions(GraphicsTest PRIVATE
        ${HEAD}
)
target_compile_options(GraphicsTest PRIVATE 
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)
add_test(NAME GraphicsTest COMMAND GraphicsTest)
