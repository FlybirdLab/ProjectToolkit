# Toolkit SDK installation helpers.

function(_tk_normalize_path inputPath outputVar)
    file(TO_CMAKE_PATH "${inputPath}" normalized)
    string(REGEX REPLACE "/$" "" normalized "${normalized}")
    set(${outputVar} "${normalized}" PARENT_SCOPE)
endfunction()

function(_tk_path_is_under path root outputVar)
    _tk_normalize_path("${path}" normalizedPath)
    _tk_normalize_path("${root}" normalizedRoot)

    if(normalizedPath STREQUAL normalizedRoot)
        set(${outputVar} TRUE PARENT_SCOPE)
        return()
    endif()

    string(FIND "${normalizedPath}" "${normalizedRoot}/" index)

    if(index EQUAL 0)
        set(${outputVar} TRUE PARENT_SCOPE)
    else()
        set(${outputVar} FALSE PARENT_SCOPE)
    endif()
endfunction()

function(_tk_collect_library_targets_from_processed_tree directory outputVar)
    set(result)

    # IMPORTANT:
    # This function must only be called with a directory that CMake has already
    # processed. CMAKE_SOURCE_DIR is safe. Arbitrary folders like libs/ may not be.
    get_property(targets DIRECTORY "${directory}" PROPERTY BUILDSYSTEM_TARGETS)

    foreach(target IN LISTS targets)
        if(TARGET "${target}")
            get_target_property(type "${target}" TYPE)

            if(type STREQUAL "STATIC_LIBRARY" OR
               type STREQUAL "SHARED_LIBRARY" OR
               type STREQUAL "MODULE_LIBRARY")
                list(APPEND result "${target}")
            endif()
        endif()
    endforeach()

    get_property(subdirectories DIRECTORY "${directory}" PROPERTY SUBDIRECTORIES)

    foreach(subdirectory IN LISTS subdirectories)
        _tk_collect_library_targets_from_processed_tree("${subdirectory}" childTargets)
        list(APPEND result ${childTargets})
    endforeach()

    set(${outputVar} "${result}" PARENT_SCOPE)
endfunction()

function(_tk_filter_targets_by_source_root inputTargets sourceRoot outputVar)
    set(result)

    foreach(target IN LISTS inputTargets)
        if(NOT TARGET "${target}")
            continue()
        endif()

        get_target_property(targetSourceDir "${target}" SOURCE_DIR)

        if(NOT targetSourceDir)
            continue()
        endif()

        _tk_path_is_under("${targetSourceDir}" "${sourceRoot}" isUnderRoot)

        if(isUnderRoot)
            list(APPEND result "${target}")
        endif()
    endforeach()

    set(${outputVar} "${result}" PARENT_SCOPE)
endfunction()

function(_tk_install_header_roots rootPattern)
    file(GLOB roots LIST_DIRECTORIES true "${rootPattern}")

    foreach(root IN LISTS roots)
        if(IS_DIRECTORY "${root}")
            install(
                DIRECTORY "${root}/"
                DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/toolkit"
                FILES_MATCHING
                    PATTERN "*.h"
                    PATTERN "*.hpp"
                    PATTERN "*.inl"
            )
        endif()
    endforeach()
endfunction()

function(_tk_install_interface_dependency_headers)
    # These headers are included directly from installed Toolkit public headers.
    # Keep this list explicit so private bundled dependencies are not published.
    set(interfaceDependencyHeaderRoots
        "${CMAKE_SOURCE_DIR}/libs/glad/include"
        "${CMAKE_SOURCE_DIR}/libs/minimp3/include"
        "${CMAKE_SOURCE_DIR}/libs/tracy/include"
        "${CMAKE_SOURCE_DIR}/libs/vorbis/include"
        "${CMAKE_SOURCE_DIR}/libs/volk/include"
    )

    foreach(headerRoot IN LISTS interfaceDependencyHeaderRoots)
        if(IS_DIRECTORY "${headerRoot}")
            install(
                DIRECTORY "${headerRoot}/"
                DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
                FILES_MATCHING
                    PATTERN "*.h"
                    PATTERN "*.hpp"
                    PATTERN "*.inl"
            )
        endif()
    endforeach()
endfunction()

function(_tk_install_prebuilt_libraries)
    if(NOT IS_DIRECTORY "${CMAKE_SOURCE_DIR}/libs")
        return()
    endif()

    file(GLOB_RECURSE prebuiltArchives CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/libs/*.lib"
        "${CMAKE_SOURCE_DIR}/libs/*.a"
    )

    file(GLOB_RECURSE prebuiltSharedLibs CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/libs/*.so"
        "${CMAKE_SOURCE_DIR}/libs/*.dylib"
    )

    file(GLOB_RECURSE prebuiltDlls CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/libs/*.dll"
    )

    if(prebuiltArchives)
        list(REMOVE_DUPLICATES prebuiltArchives)
        install(
            FILES ${prebuiltArchives}
            DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        )
    endif()

    if(prebuiltSharedLibs)
        list(REMOVE_DUPLICATES prebuiltSharedLibs)
        install(
            FILES ${prebuiltSharedLibs}
            DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        )
    endif()

    if(prebuiltDlls)
        list(REMOVE_DUPLICATES prebuiltDlls)
        install(
            FILES ${prebuiltDlls}
            DESTINATION "${CMAKE_INSTALL_BINDIR}"
        )
    endif()
endfunction()

function(_tk_install_toolkit_tools)
    set(toolTargets)

    foreach(toolTarget IN ITEMS RkGen ShaderCompiler)
        if(TARGET "${toolTarget}")
            list(APPEND toolTargets "${toolTarget}")
        endif()
    endforeach()

    if(NOT toolTargets)
        return()
    endif()

    set(toolCMakeDestination "${CMAKE_INSTALL_LIBDIR}/cmake/ToolkitProjectTools")

    install(
        TARGETS ${toolTargets}
        EXPORT ToolkitProjectToolsTargets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )

    install(
        EXPORT ToolkitProjectToolsTargets
        FILE ToolkitProjectToolsTargets.cmake
        NAMESPACE Toolkit::
        DESTINATION "${toolCMakeDestination}"
    )

    install(
        FILES
            "${CMAKE_SOURCE_DIR}/tools/RkGen/RkGen.cmake"
            "${CMAKE_SOURCE_DIR}/tools/ShaderCompiler/ShaderCompiler.cmake"
            "${CMAKE_SOURCE_DIR}/cmakes/ToolkitProjectToolsConfig.cmake"
        DESTINATION "${toolCMakeDestination}"
    )
endfunction()

function(ConfigureToolkitInstall)
    include(GNUInstallDirs)

    # Toolkit public headers.
    _tk_install_header_roots("${CMAKE_SOURCE_DIR}/modules/*/src/toolkit")

    # Generated public headers.
    _tk_install_header_roots("${CMAKE_BINARY_DIR}/modules/*/generated/toolkit")
    _tk_install_header_roots("${CMAKE_SOURCE_DIR}/modules/*/generated/toolkit")

    # Public third-party headers used by installed Toolkit headers.
    _tk_install_interface_dependency_headers()

    # Collect all CMake library targets from the already-processed CMake tree.
    # Do NOT call get_property(DIRECTORY ...) directly on libs/, because libs/
    # may exist on disk but may not be a processed CMake directory scope.
    _tk_collect_library_targets_from_processed_tree("${CMAKE_SOURCE_DIR}" allLibraryTargets)

    _tk_filter_targets_by_source_root(
        "${allLibraryTargets}"
        "${CMAKE_SOURCE_DIR}/modules"
        moduleTargets
    )

    _tk_filter_targets_by_source_root(
        "${allLibraryTargets}"
        "${CMAKE_SOURCE_DIR}/libs"
        libTargets
    )

    set(installTargets
        ${moduleTargets}
        ${libTargets}
    )

    if(installTargets)
        list(REMOVE_DUPLICATES installTargets)

        install(
            TARGETS ${installTargets}
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        )
    endif()

    # Install prebuilt .lib/.a/.so/.dylib/.dll files under libs/.
    # This covers third-party binaries that are not represented as CMake targets.
    _tk_install_prebuilt_libraries()

    # tools/ is added after ConfigureToolkitInstall() in the root CMakeLists.
    # Defer the tool install/export rules until RkGen and ShaderCompiler exist.
    cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL _tk_install_toolkit_tools)
endfunction()
