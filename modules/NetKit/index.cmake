# Toolkit installing script
# TkNet module index file

function(AddTkNet incVar lnkVar depVar)

    list(APPEND ${depVar} "TkNet")
    list(APPEND ${incVar}
        "${CMAKE_SOURCE_DIR}/modules/NetKit/src"
    )

    IF(WIN32)
        list(APPEND ${lnkVar} "TkNet")
    ELSEIF(APPLE)
        list(APPEND ${lnkVar}  "TkNet")
    ELSEIF(LINUX)
        list(APPEND ${lnkVar} "TkNet")
    ENDIF()

    set(${incVar} ${${incVar}} PARENT_SCOPE)
    set(${lnkVar} ${${lnkVar}} PARENT_SCOPE)
    set(${depVar} ${${depVar}} PARENT_SCOPE)
endfunction()
