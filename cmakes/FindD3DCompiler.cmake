include(FindPackageHandleStandardArgs)

set(_D3DCompiler_SDK_ROOT "$ENV{WindowsSdkDir}")
set(_D3DCompiler_SDK_VERSION "${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
if(NOT _D3DCompiler_SDK_VERSION)
    set(_D3DCompiler_SDK_VERSION "$ENV{WindowsSDKVersion}")
endif()

set(_D3DCompiler_ARCH_DIR x64)
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(_D3DCompiler_ARCH_DIR x86)
endif()

set(_D3DCompiler_INCLUDE_PATHS)
set(_D3DCompiler_LIBRARY_PATHS)
set(_D3DCompiler_REDIST_PATHS)

if(_D3DCompiler_SDK_ROOT)
    list(APPEND _D3DCompiler_INCLUDE_PATHS
        "${_D3DCompiler_SDK_ROOT}/Include/${_D3DCompiler_SDK_VERSION}"
        "${_D3DCompiler_SDK_ROOT}/Include"
    )
    list(APPEND _D3DCompiler_LIBRARY_PATHS
        "${_D3DCompiler_SDK_ROOT}/Lib/${_D3DCompiler_SDK_VERSION}/um/${_D3DCompiler_ARCH_DIR}"
        "${_D3DCompiler_SDK_ROOT}/Lib/um/${_D3DCompiler_ARCH_DIR}"
    )
    list(APPEND _D3DCompiler_REDIST_PATHS
        "${_D3DCompiler_SDK_ROOT}/Redist/D3D/${_D3DCompiler_ARCH_DIR}"
    )
endif()

find_path(D3DCompiler_INCLUDE_DIR
    NAMES d3dcompiler.h
    PATHS ${_D3DCompiler_INCLUDE_PATHS}
    PATH_SUFFIXES um
)

find_library(D3DCompiler_LIBRARY
    NAMES d3dcompiler
    PATHS ${_D3DCompiler_LIBRARY_PATHS}
)

find_library(D3DCompiler_DXGUID_LIBRARY
    NAMES dxguid
    PATHS ${_D3DCompiler_LIBRARY_PATHS}
)

find_file(D3DCompiler_DLL
    NAMES d3dcompiler_47.dll
    PATHS ${_D3DCompiler_REDIST_PATHS}
)

find_package_handle_standard_args(D3DCompiler
    REQUIRED_VARS
        D3DCompiler_INCLUDE_DIR
        D3DCompiler_LIBRARY
        D3DCompiler_DXGUID_LIBRARY
)

if(D3DCompiler_FOUND AND NOT TARGET D3DCompiler::D3DCompiler)
    add_library(D3DCompiler::D3DCompiler INTERFACE IMPORTED)

    target_include_directories(D3DCompiler::D3DCompiler INTERFACE
        "${D3DCompiler_INCLUDE_DIR}"
    )

    target_link_libraries(D3DCompiler::D3DCompiler INTERFACE
        "${D3DCompiler_LIBRARY}"
        "${D3DCompiler_DXGUID_LIBRARY}"
    )

    if(MSVC)
        target_link_libraries(D3DCompiler::D3DCompiler INTERFACE delayimp)
        target_link_options(D3DCompiler::D3DCompiler INTERFACE
            "/DELAYLOAD:d3dcompiler_47.dll"
        )
    endif()
endif()

mark_as_advanced(
    D3DCompiler_INCLUDE_DIR
    D3DCompiler_LIBRARY
    D3DCompiler_DXGUID_LIBRARY
    D3DCompiler_DLL
)
