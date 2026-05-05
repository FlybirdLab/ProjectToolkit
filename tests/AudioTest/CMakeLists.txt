project(AudioTest)
#################################################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OBJC_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)
#################################################################
set(INCS
        "${CMAKE_SOURCE_DIR}/tests/AudioTest/src"
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

AddTkAudio(INCS LNKS DEPS)

AddGTest(INCS LNKS)

AddMiniMp3(INCS LNKS)
# AddVorbis(INCS NULL)
########################################################
# project configurations
########################################################
file(GLOB_RECURSE SRC
        "src/*.m"
        "src/*.cpp"
        "src/*.mm"
        "src/*.c"
)
add_executable(AudioTest
        ${SRC}
        ${SRCS}
)
add_dependencies(AudioTest
        ${DEPS}
)

if(WIN32 AND CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
    target_link_directories(AudioTest PRIVATE
            "C:/Windows Kits/10/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/um/x64"
            "C:/Windows Kits/10/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/ucrt/x64"
    )
endif()

target_include_directories(AudioTest PRIVATE
        ${INCS}
)
target_link_libraries(AudioTest PRIVATE
        ${LNKS}
)
target_compile_definitions(AudioTest PRIVATE
        ${HEAD}
        AUDIO_TEST_DOC_DIR=\"${CMAKE_SOURCE_DIR}/docs/testdata/audio\"
)
target_compile_options(AudioTest PRIVATE 
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)
