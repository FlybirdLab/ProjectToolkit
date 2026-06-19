# Created by cubevlmu on 2025/12/8.
# Copyright (c) 2025 Flybird Games. All rights reserved.

# Toolkit installing script
# TkCore module index file

function(AddTkCore incVar lnkVar depVar)
    if(NOT TARGET tpkg::DataExtend)
        message(FATAL_ERROR "TkCore requires tpkg::DataExtend.")
    endif()

    list(APPEND ${depVar} "TkCore")
    list(APPEND ${incVar}
        "${CMAKE_SOURCE_DIR}/modules/CoreKit/src"
    )
    list(APPEND ${lnkVar} tpkg::DataExtend)
    
    IF(WIN32)
        list(APPEND ${lnkVar} "TkCore")
    ELSEIF(APPLE)
        list(APPEND ${lnkVar}  "TkCore")
    ELSEIF(LINUX)
        list(APPEND ${lnkVar} "TkCore")
    ENDIF()

    set(${incVar} ${${incVar}} PARENT_SCOPE)
    set(${lnkVar} ${${lnkVar}} PARENT_SCOPE)
    set(${depVar} ${${depVar}} PARENT_SCOPE)
endfunction()
