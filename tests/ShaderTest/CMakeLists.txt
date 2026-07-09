project(ShaderTest)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)
#################################################################
set(INCS
        "${CMAKE_SOURCE_DIR}/tests/ShaderTest/src"
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
# TkShaderCompilerCore owns the lexer / parser / sema / cache / manifest
# code the tests exercise. TkGraphics alone only ships the runtime-facing
# format/ + GfxShaderAsset bits, which aren't enough by themselves.
AddTkShaderCompilerCore(INCS LNKS DEPS)

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
add_executable(ShaderTest
        ${SRC}
        ${SRCS}
)
add_dependencies(ShaderTest
        ${DEPS}
)

if(WIN32 AND CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
    target_link_directories(ShaderTest PRIVATE
            "C:/Windows Kits/10/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/um/x64"
            "C:/Windows Kits/10/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/ucrt/x64"
    )
endif()

target_include_directories(ShaderTest PRIVATE
        ${INCS}
)
target_link_libraries(ShaderTest PRIVATE
        ${LNKS}
)
target_compile_definitions(ShaderTest PRIVATE
        ${HEAD}
        # Test data root -- used by tests that read sample manifests / sources
        # from disk. Path is wrapped in quotes so it lands as a string literal.
        SHADER_TEST_DOC_DIR=\"${CMAKE_SOURCE_DIR}/tests/ShaderTest/testdata\"
        SHADER_TEST_TMP_DIR=\"${CMAKE_BINARY_DIR}/tests/ShaderTest/tmp\"
)
target_compile_options(ShaderTest PRIVATE
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)

# Pre-create the scratch directory so tests writing .jobcache / .tksd / temp
# include files don't race the test runner.
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/tests/ShaderTest/tmp")
