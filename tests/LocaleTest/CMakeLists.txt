project(LocaleTest)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)
#################################################################
set(INCS
        "${CMAKE_SOURCE_DIR}/tests/LocaleTest/src"
)
set(LNKS
        ""
)
set(DEPS
        ""
)
set(HEAD
        "TK_DEBUG"
        "TK_STATIC_PLUGINS"
)
set(SRCS
        ""
)
########################################################
# dependencies
########################################################
AddTkCore(INCS LNKS DEPS)
AddTkLocale(INCS LNKS DEPS)

AddGTest(INCS LNKS)
########################################################
# project configurations
########################################################
file(GLOB_RECURSE SRC
        "src/*.m"
        "src/*.cpp"
        "src/*.mm"
        "src/*.c"
)
add_executable(LocaleTest
        ${SRC}
        ${SRCS}
)
add_dependencies(LocaleTest
        ${DEPS}
)

if(WIN32 AND CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
    target_link_directories(LocaleTest PRIVATE
            "C:/Windows Kits/10/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/um/x64"
            "C:/Windows Kits/10/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/ucrt/x64"
    )
endif()

target_include_directories(LocaleTest PRIVATE
        ${INCS}
)
target_link_libraries(LocaleTest PRIVATE
        ${LNKS}
)
target_compile_definitions(LocaleTest PRIVATE
        ${HEAD}
        LOCALE_TEST_DOC_DIR=\"${CMAKE_SOURCE_DIR}/docs/testdata\"
)
target_compile_options(LocaleTest PRIVATE 
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)
