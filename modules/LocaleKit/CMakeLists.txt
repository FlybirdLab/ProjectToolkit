# LocaleKit Sub-project

project(TkLocale)
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
    "${CMAKE_SOURCE_DIR}/modules/LocaleKit/src"
)
set(LNKS)
set(DEPS)
set(HEAD)

#################################################################
# dependencies
#################################################################
SetPlatformDeps(INCS LNKS)

AddTkCore(INCS LNKS DEPS)

AddCultureInfo(INCS LNKS)
#################################################################
# project configurations
#################################################################
file(GLOB_RECURSE src
    "src/*.m"
    "src/*.cpp"
    "src/*.mm"
    "src/*.c"
)

add_library(TkLocale STATIC
    ${src}
)

target_include_directories(TkLocale PRIVATE
    ${INCS}
)

target_link_libraries(TkLocale PRIVATE
    ${LNKS}
)

target_compile_definitions(TkLocale PRIVATE
    _XOPEN_SOURCE
    _LIBCPP_DRIVERKIT
    TK_DEBUG
    TK_ENABLE_SIMD
    TK_STATIC_PLUGINS
)

target_compile_options(TkLocale PRIVATE
    "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)