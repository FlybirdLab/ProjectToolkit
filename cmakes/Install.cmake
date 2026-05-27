# Toolkit SDK installation helpers.

function(_tk_collect_library_targets directory outputVar)
    get_property(targets DIRECTORY "${directory}" PROPERTY BUILDSYSTEM_TARGETS)
    set(result)

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
        _tk_collect_library_targets("${subdirectory}" childTargets)
        list(APPEND result ${childTargets})
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
        "${CMAKE_SOURCE_DIR}/libs/concurrent_queue/include"
        "${CMAKE_SOURCE_DIR}/libs/glad/include"
        "${CMAKE_SOURCE_DIR}/libs/minimp3/include"
        "${CMAKE_SOURCE_DIR}/libs/tracy/include"
        "${CMAKE_SOURCE_DIR}/libs/vorbis/include"
        "${CMAKE_SOURCE_DIR}/libs/volk/include"
        "${CMAKE_SOURCE_DIR}/libs/vma/include"
    )

    foreach(headerRoot IN LISTS interfaceDependencyHeaderRoots)
        install(
            DIRECTORY "${headerRoot}/"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
            FILES_MATCHING
                PATTERN "*.h"
                PATTERN "*.hpp"
        )
    endforeach()
endfunction()

function(ConfigureToolkitInstall)
    # This function is called after project(), when the target architecture is known.
    include(GNUInstallDirs)

    # Keep module-relative public header layout under include/toolkit.
    _tk_install_header_roots("${CMAKE_SOURCE_DIR}/modules/*/src/toolkit")

    # Include both build-tree generated headers and generators that still emit
    # public headers into a module-local generated directory.
    _tk_install_header_roots("${CMAKE_BINARY_DIR}/modules/*/generated/toolkit")
    _tk_install_header_roots("${CMAKE_SOURCE_DIR}/modules/*/generated/toolkit")

    _tk_install_interface_dependency_headers()

    _tk_collect_library_targets("${CMAKE_SOURCE_DIR}/modules" moduleTargets)
    if(moduleTargets)
        list(REMOVE_DUPLICATES moduleTargets)
        install(
            TARGETS ${moduleTargets}
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        )
    endif()
endfunction()
