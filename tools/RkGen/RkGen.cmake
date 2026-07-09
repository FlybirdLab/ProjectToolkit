function(_tk_get_rkgen_target outputVar rkgenPathVar)
    if (TARGET RkGen)
        set(_rkgen_target "RkGen")
    elseif (TARGET Toolkit::RkGen)
        set(_rkgen_target "Toolkit::RkGen")
    else()
        message(FATAL_ERROR "tk_target_rkgen: RkGen target does not exist. Include ToolkitProjectToolsConfig.cmake after installing Toolkit tools, or include this helper after add_subdirectory(tools/RkGen).")
    endif()

    get_target_property(_rkgen_imported "${_rkgen_target}" IMPORTED)
    if(_rkgen_imported)
        get_target_property(_rkgen_loc "${_rkgen_target}" IMPORTED_LOCATION)
        if(_rkgen_loc)
            get_filename_component(_rkgen_loc "${_rkgen_loc}" ABSOLUTE)
        endif()
        if(NOT _rkgen_loc AND CMAKE_BUILD_TYPE)
            string(TOUPPER "${CMAKE_BUILD_TYPE}" _cfg)
            get_target_property(_rkgen_loc "${_rkgen_target}" IMPORTED_LOCATION_${_cfg})
            if(_rkgen_loc)
                get_filename_component(_rkgen_loc "${_rkgen_loc}" ABSOLUTE)
            endif()
        endif()

        if(NOT _rkgen_loc OR NOT EXISTS "${_rkgen_loc}")
            get_filename_component(_tk_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../../.." ABSOLUTE)
            set(_candidates "")
            if(WIN32)
                list(APPEND _candidates "${_tk_root}/bin/RkGen.exe" "${_tk_root}/lib/RkGen.exe" "${_tk_root}/tools/RkGen/RkGen.exe")
            else()
                list(APPEND _candidates "${_tk_root}/bin/RkGen" "${_tk_root}/lib/RkGen")
            endif()
            foreach(_c IN LISTS _candidates)
                if(EXISTS "${_c}")
                    set(_rkgen_loc "${_c}")
                    break()
                endif()
            endforeach()
        endif()
    else()
        get_target_property(_rkgen_out_dir "${_rkgen_target}" RUNTIME_OUTPUT_DIRECTORY)
        get_target_property(_rkgen_out_name "${_rkgen_target}" RUNTIME_OUTPUT_NAME)
        if(NOT _rkgen_out_name)
            set(_rkgen_out_name "${_rkgen_target}")
        endif()
        if(_rkgen_out_dir)
            get_filename_component(_rkgen_out_dir "${_rkgen_out_dir}" ABSOLUTE BASE_DIR "${CMAKE_BINARY_DIR}")
        endif()
        if(WIN32)
            set(_rkgen_loc "${_rkgen_out_dir}/${_rkgen_out_name}.exe")
        else()
            set(_rkgen_loc "${_rkgen_out_dir}/${_rkgen_out_name}")
        endif()
        if(NOT _rkgen_out_dir)
            set(_rkgen_loc "")
        endif()
    endif()

    set(${outputVar} "${_rkgen_target}" PARENT_SCOPE)
    set(${rkgenPathVar} "${_rkgen_loc}" PARENT_SCOPE)
endfunction()

function(tk_target_rkgen target)
    if (NOT TARGET ${target})
        message(FATAL_ERROR "tk_target_rkgen: target '${target}' does not exist.")
    endif()
    _tk_get_rkgen_target(rkgen_target rkgen_exe_path)

    set(options RECURSIVE)
    set(oneValueArgs OUTPUT_DIR)
    set(multiValueArgs SOURCES SOURCE_DIRS)
    cmake_parse_arguments(RKGEN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT RKGEN_SOURCES AND NOT RKGEN_SOURCE_DIRS)
        message(FATAL_ERROR "tk_target_rkgen: pass SOURCES and/or SOURCE_DIRS.")
    endif()

    if (NOT RKGEN_OUTPUT_DIR)
        set(RKGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/rkgen/${target}")
    endif()
    get_filename_component(RKGEN_OUTPUT_DIR "${RKGEN_OUTPUT_DIR}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    file(MAKE_DIRECTORY "${RKGEN_OUTPUT_DIR}")
    set(rkgen_runner_script "${CMAKE_CURRENT_BINARY_DIR}/tk_rkgen_run_if_reflected.cmake")
    file(WRITE "${rkgen_runner_script}" [=[
if(NOT DEFINED RKGEN_EXE OR RKGEN_EXE STREQUAL "")
    message(FATAL_ERROR "tk_target_rkgen: RKGEN_EXE is not set.")
endif()
if(NOT DEFINED RKGEN_INPUT OR RKGEN_INPUT STREQUAL "")
    message(FATAL_ERROR "tk_target_rkgen: RKGEN_INPUT is not set.")
endif()
if(NOT DEFINED RKGEN_OUTPUT_DIR OR RKGEN_OUTPUT_DIR STREQUAL "")
    message(FATAL_ERROR "tk_target_rkgen: RKGEN_OUTPUT_DIR is not set.")
endif()
if(NOT DEFINED RKGEN_OUTPUT OR RKGEN_OUTPUT STREQUAL "")
    message(FATAL_ERROR "tk_target_rkgen: RKGEN_OUTPUT is not set.")
endif()
if(NOT DEFINED RKGEN_OUTPUT_STEM OR RKGEN_OUTPUT_STEM STREQUAL "")
    message(FATAL_ERROR "tk_target_rkgen: RKGEN_OUTPUT_STEM is not set.")
endif()

file(READ "${RKGEN_INPUT}" header_text)
if(header_text MATCHES "(^|[^A-Za-z0-9_])(rkattr|rkclass|rkstruct|rkenum|rkfield|rkfunc|rkctor)([^A-Za-z0-9_]|$)")
    file(MAKE_DIRECTORY "${RKGEN_OUTPUT_DIR}")
    execute_process(
        COMMAND "${RKGEN_EXE}" --file "${RKGEN_INPUT}" --out "${RKGEN_OUTPUT_DIR}" --out-name "${RKGEN_OUTPUT_STEM}"
        RESULT_VARIABLE rkgen_result
    )
    if(NOT rkgen_result EQUAL 0)
        message(FATAL_ERROR "tk_target_rkgen: RkGen failed for ${RKGEN_INPUT}")
    endif()
    if(NOT EXISTS "${RKGEN_OUTPUT}")
        file(WRITE "${RKGEN_OUTPUT}" "\n")
    endif()
else()
    file(WRITE "${RKGEN_OUTPUT}" "\n")
endif()
]=])

    set(rkgen_inputs "")

    foreach(source IN LISTS RKGEN_SOURCES)
        get_filename_component(abs_source "${source}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        list(APPEND rkgen_inputs "${abs_source}")
    endforeach()

    foreach(source_dir IN LISTS RKGEN_SOURCE_DIRS)
        get_filename_component(abs_source_dir "${source_dir}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        if (RKGEN_RECURSIVE)
            file(GLOB_RECURSE dir_inputs CONFIGURE_DEPENDS "${abs_source_dir}/*.hpp" "${abs_source_dir}/*.h")
        else()
            file(GLOB dir_inputs CONFIGURE_DEPENDS "${abs_source_dir}/*.hpp" "${abs_source_dir}/*.h")
        endif()
        list(APPEND rkgen_inputs ${dir_inputs})
    endforeach()

    if (NOT rkgen_inputs)
        message(WARNING "tk_target_rkgen: no input headers found for target '${target}'.")
        return()
    endif()

    list(REMOVE_DUPLICATES rkgen_inputs)

    foreach(input IN LISTS rkgen_inputs)
        if (NOT EXISTS "${input}")
            message(FATAL_ERROR "tk_target_rkgen: input does not exist: ${input}")
        endif()
    endforeach()
    
    set(rkgen_outputs "")
    set(rkgen_output_name_keys "")
    get_target_property(rkgen_imported "${rkgen_target}" IMPORTED)
    foreach(input IN LISTS rkgen_inputs)
        get_filename_component(base_stem "${input}" NAME_WE)
        set(output_stem "${base_stem}")
        string(TOLOWER "${output_stem}" output_stem_key)
        list(FIND rkgen_output_name_keys "${output_stem_key}" existing_output_name)
        if (NOT existing_output_name EQUAL -1)
            string(SHA256 output_hash "${input}")
            string(SUBSTRING "${output_hash}" 0 8 output_hash_short)
            set(output_stem "${base_stem}_${output_hash_short}")
            string(TOLOWER "${output_stem}" output_stem_key)
            list(FIND rkgen_output_name_keys "${output_stem_key}" existing_output_name)
            set(output_suffix_index 0)
            while (NOT existing_output_name EQUAL -1)
                math(EXPR output_suffix_index "${output_suffix_index} + 1")
                set(output_stem "${base_stem}_${output_hash_short}_${output_suffix_index}")
                string(TOLOWER "${output_stem}" output_stem_key)
                list(FIND rkgen_output_name_keys "${output_stem_key}" existing_output_name)
            endwhile()
        endif()
        list(APPEND rkgen_output_name_keys "${output_stem_key}")

        set(output "${RKGEN_OUTPUT_DIR}/${output_stem}.rkgen.cpp")
        set(rkgen_depends "${input}")
        if (NOT rkgen_imported)
            list(APPEND rkgen_depends ${rkgen_target})
        endif()
        if(rkgen_exe_path)
            add_custom_command(
                OUTPUT "${output}"
                COMMAND "${CMAKE_COMMAND}"
                    "-DRKGEN_EXE=${rkgen_exe_path}"
                    "-DRKGEN_INPUT=${input}"
                    "-DRKGEN_OUTPUT_DIR=${RKGEN_OUTPUT_DIR}"
                    "-DRKGEN_OUTPUT=${output}"
                    "-DRKGEN_OUTPUT_STEM=${output_stem}"
                    -P "${rkgen_runner_script}"
                DEPENDS ${rkgen_depends}
                COMMENT "Running RkGen for ${input}"
                VERBATIM
            )
        else()
            add_custom_command(
                OUTPUT "${output}"
                COMMAND "${CMAKE_COMMAND}"
                    "-DRKGEN_EXE=$<TARGET_FILE:${rkgen_target}>"
                    "-DRKGEN_INPUT=${input}"
                    "-DRKGEN_OUTPUT_DIR=${RKGEN_OUTPUT_DIR}"
                    "-DRKGEN_OUTPUT=${output}"
                    "-DRKGEN_OUTPUT_STEM=${output_stem}"
                    -P "${rkgen_runner_script}"
                DEPENDS ${rkgen_depends}
                COMMENT "Running RkGen for ${input}"
                VERBATIM
            )
        endif()
        list(APPEND rkgen_outputs "${output}")
    endforeach()

    target_sources(${target} PRIVATE ${rkgen_outputs})
    target_include_directories(${target} PRIVATE "${RKGEN_OUTPUT_DIR}")
    if (NOT rkgen_imported)
        add_dependencies(${target} ${rkgen_target})
    endif()
    set_source_files_properties(${rkgen_outputs} PROPERTIES GENERATED TRUE)
endfunction()
