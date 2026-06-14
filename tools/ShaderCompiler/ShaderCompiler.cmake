function(_tk_get_shadercompiler_target outputVar)
    if (TARGET ShaderCompiler)
        set(${outputVar} "ShaderCompiler" PARENT_SCOPE)
    elseif (TARGET Toolkit::ShaderCompiler)
        set(${outputVar} "Toolkit::ShaderCompiler" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "ShaderCompiler target does not exist. Include ToolkitProjectToolsConfig.cmake after installing Toolkit tools, or include this helper after add_subdirectory(tools/ShaderCompiler).")
    endif()
endfunction()

function(_tk_append_shadercompiler_args outputVar)
    set(args)

    if (TK_SHADERCOMPILER_BACKEND)
        list(APPEND args --backend "${TK_SHADERCOMPILER_BACKEND}")
    endif()

    if (TK_SHADERCOMPILER_CACHE_DIR)
        list(APPEND args --cache-dir "${TK_SHADERCOMPILER_CACHE_DIR}")
    endif()

    if (TK_SHADERCOMPILER_JOBS)
        list(APPEND args --jobs "${TK_SHADERCOMPILER_JOBS}")
    endif()

    if (TK_SHADERCOMPILER_NO_CACHE)
        list(APPEND args --no-cache)
    endif()

    if (TK_SHADERCOMPILER_SHOW_STATS)
        list(APPEND args --show-stats)
    endif()

    foreach(includeDir IN LISTS TK_SHADERCOMPILER_INCLUDE_DIRS)
        get_filename_component(absIncludeDir "${includeDir}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        list(APPEND args -I "${absIncludeDir}")
    endforeach()

    set(${outputVar} "${args}" PARENT_SCOPE)
endfunction()

function(_tk_parse_shadercompiler_args)
    set(options ALL NO_CACHE SHOW_STATS)
    set(oneValueArgs OUTPUT_DIR BACKEND CACHE_DIR JOBS)
    set(multiValueArgs FILES SOURCE_DIRS INCLUDE_DIRS DEPENDS)
    cmake_parse_arguments(TK_SHADERCOMPILER "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT TK_SHADERCOMPILER_FILES AND NOT TK_SHADERCOMPILER_SOURCE_DIRS)
        message(FATAL_ERROR "ShaderCompiler helper: pass FILES and/or SOURCE_DIRS.")
    endif()

    if (NOT TK_SHADERCOMPILER_OUTPUT_DIR)
        set(TK_SHADERCOMPILER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shader-output")
    endif()

    get_filename_component(TK_SHADERCOMPILER_OUTPUT_DIR "${TK_SHADERCOMPILER_OUTPUT_DIR}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")

    set(TK_SHADERCOMPILER_ALL "${TK_SHADERCOMPILER_ALL}" PARENT_SCOPE)
    set(TK_SHADERCOMPILER_NO_CACHE "${TK_SHADERCOMPILER_NO_CACHE}" PARENT_SCOPE)
    set(TK_SHADERCOMPILER_SHOW_STATS "${TK_SHADERCOMPILER_SHOW_STATS}" PARENT_SCOPE)
    set(TK_SHADERCOMPILER_OUTPUT_DIR "${TK_SHADERCOMPILER_OUTPUT_DIR}" PARENT_SCOPE)
    set(TK_SHADERCOMPILER_BACKEND "${TK_SHADERCOMPILER_BACKEND}" PARENT_SCOPE)
    set(TK_SHADERCOMPILER_CACHE_DIR "${TK_SHADERCOMPILER_CACHE_DIR}" PARENT_SCOPE)
    set(TK_SHADERCOMPILER_JOBS "${TK_SHADERCOMPILER_JOBS}" PARENT_SCOPE)
    set(TK_SHADERCOMPILER_FILES "${TK_SHADERCOMPILER_FILES}" PARENT_SCOPE)
    set(TK_SHADERCOMPILER_SOURCE_DIRS "${TK_SHADERCOMPILER_SOURCE_DIRS}" PARENT_SCOPE)
    set(TK_SHADERCOMPILER_INCLUDE_DIRS "${TK_SHADERCOMPILER_INCLUDE_DIRS}" PARENT_SCOPE)
    set(TK_SHADERCOMPILER_DEPENDS "${TK_SHADERCOMPILER_DEPENDS}" PARENT_SCOPE)
endfunction()

function(tk_add_shadercompile_target name)
    if (TARGET ${name})
        message(FATAL_ERROR "tk_add_shadercompile_target: target '${name}' already exists.")
    endif()

    _tk_get_shadercompiler_target(shadercompiler_target)
    _tk_parse_shadercompiler_args(${ARGN})
    _tk_append_shadercompiler_args(common_args)

    set(commands
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TK_SHADERCOMPILER_OUTPUT_DIR}"
    )
    set(file_dependencies)
    set(target_dependencies)
    foreach(dependency IN LISTS TK_SHADERCOMPILER_DEPENDS)
        if (TARGET ${dependency})
            list(APPEND target_dependencies ${dependency})
        else()
            list(APPEND file_dependencies "${dependency}")
        endif()
    endforeach()

    foreach(shaderFile IN LISTS TK_SHADERCOMPILER_FILES)
        get_filename_component(absShaderFile "${shaderFile}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        list(APPEND commands
            COMMAND $<TARGET_FILE:${shadercompiler_target}>
                --file "${absShaderFile}"
                --out "${TK_SHADERCOMPILER_OUTPUT_DIR}"
                ${common_args}
        )
        list(APPEND file_dependencies "${absShaderFile}")
    endforeach()

    foreach(sourceDir IN LISTS TK_SHADERCOMPILER_SOURCE_DIRS)
        get_filename_component(absSourceDir "${sourceDir}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        list(APPEND commands
            COMMAND $<TARGET_FILE:${shadercompiler_target}>
                --source-dir "${absSourceDir}"
                --out "${TK_SHADERCOMPILER_OUTPUT_DIR}"
                ${common_args}
        )
        list(APPEND file_dependencies "${absSourceDir}")
    endforeach()

    if (TK_SHADERCOMPILER_ALL)
        set(all_arg ALL)
    endif()

    add_custom_target(${name} ${all_arg}
        ${commands}
        DEPENDS ${file_dependencies}
        COMMENT "Running ShaderCompiler for ${name}"
        VERBATIM
    )

    get_target_property(shadercompiler_imported "${shadercompiler_target}" IMPORTED)
    if (NOT shadercompiler_imported)
        list(APPEND target_dependencies ${shadercompiler_target})
    endif()

    if (target_dependencies)
        add_dependencies(${name} ${target_dependencies})
    endif()
endfunction()

function(tk_target_shadercompile target)
    if (NOT TARGET ${target})
        message(FATAL_ERROR "tk_target_shadercompile: target '${target}' does not exist.")
    endif()

    set(shader_target "${target}_Shaders")
    tk_add_shadercompile_target(${shader_target} ${ARGN})
    add_dependencies(${target} ${shader_target})
endfunction()
