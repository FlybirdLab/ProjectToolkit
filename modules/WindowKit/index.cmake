# Toolkit installing script
# TkWindow module index file

function(AddTkWindow incVar lnkVar depVar)

    list(APPEND ${depVar} "TkWindow")
    list(APPEND ${incVar}
        "${CMAKE_SOURCE_DIR}/modules/WindowKit/src"
    )

    IF(WIN32)
        list(APPEND ${lnkVar} "TkWindow")
    ELSEIF(APPLE)
        list(APPEND ${lnkVar}  "TkWindow")
    ELSEIF(LINUX)
        list(APPEND ${lnkVar} "TkWindow")
    ENDIF()

    set(${incVar} ${${incVar}} PARENT_SCOPE)
    set(${lnkVar} ${${lnkVar}} PARENT_SCOPE)
    set(${depVar} ${${depVar}} PARENT_SCOPE)
endfunction()
