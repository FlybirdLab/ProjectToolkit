# Created by cubevlmu on 2025/12/8.
# Copyright (c) 2025 Flybird Games. All rights reserved.

# Toolkit installing script
# TkCore module index file

function(AddTkCore incVar lnkVar depVar)
    set(concurrent_header "")
    AddConcurrentQueue(NULL concurrent_header)

    list(APPEND ${depVar} "TkCore")
    list(APPEND ${incVar}
        "${CMAKE_SOURCE_DIR}/modules/CoreKit/src"
        "${CMAKE_SOURCE_DIR}/libs/concurrent_queue/include"
    )
    
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