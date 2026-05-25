function(tk_target_rkgen target)
    if (NOT TARGET ${target})
        message(FATAL_ERROR "tk_target_rkgen: target '${target}' does not exist.")
    endif()
    if (NOT TARGET RkGen)
        message(FATAL_ERROR "tk_target_rkgen: RkGen target does not exist. Include this helper after add_subdirectory(tools/RkGen).")
    endif()

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

    set(rkgen_filtered_inputs "")
    foreach(input IN LISTS rkgen_inputs)
        if (NOT EXISTS "${input}")
            message(FATAL_ERROR "tk_target_rkgen: input does not exist: ${input}")
        endif()
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${input}")
        file(READ "${input}" header_text)
        if (header_text MATCHES "(^|[^A-Za-z0-9_])(rkattr|rkclass|rkstruct|rkenum|rkfield|rkfunc|rkctor)([^A-Za-z0-9_]|$)")
            list(APPEND rkgen_filtered_inputs "${input}")
        endif()
    endforeach()

    if (NOT rkgen_filtered_inputs)
        return()
    endif()
    
    set(rkgen_outputs "")
    set(rkgen_output_names "")
    foreach(input IN LISTS rkgen_filtered_inputs)
        get_filename_component(stem "${input}" NAME_WE)
        list(FIND rkgen_output_names "${stem}" existing_output_name)
        if (NOT existing_output_name EQUAL -1)
            message(FATAL_ERROR "tk_target_rkgen: duplicate generated file name '${stem}.rkgen.cpp'. Rename one input or use separate OUTPUT_DIR values.")
        endif()
        list(APPEND rkgen_output_names "${stem}")

        set(output "${RKGEN_OUTPUT_DIR}/${stem}.rkgen.cpp")
        add_custom_command(
            OUTPUT "${output}"
            COMMAND $<TARGET_FILE:RkGen> --file "${input}" --out "${RKGEN_OUTPUT_DIR}"
            DEPENDS RkGen "${input}"
            COMMENT "Running RkGen for ${input}"
            VERBATIM
        )
        list(APPEND rkgen_outputs "${output}")
    endforeach()

    target_sources(${target} PRIVATE ${rkgen_outputs})
    target_include_directories(${target} PRIVATE "${RKGEN_OUTPUT_DIR}")
    add_dependencies(${target} RkGen)
    set_source_files_properties(${rkgen_outputs} PROPERTIES GENERATED TRUE)
endfunction()
