# Toolkit installing script
# TkVideo module index file

function(AddTkVideo incVar lnkVar depVar)

    list(APPEND ${depVar} "TkVideo")
    list(APPEND ${incVar}
        "${CMAKE_SOURCE_DIR}/modules/VideoKit/src"
    )

    IF(WIN32)
        list(APPEND ${lnkVar} "TkVideo")
    ELSEIF(APPLE)
        list(APPEND ${lnkVar}  "TkVideo")
    ELSEIF(LINUX)
        list(APPEND ${lnkVar} "TkVideo")
    ENDIF()

    set(${incVar} ${${incVar}} PARENT_SCOPE)
    set(${lnkVar} ${${lnkVar}} PARENT_SCOPE)
    set(${depVar} ${${depVar}} PARENT_SCOPE)
endfunction()
