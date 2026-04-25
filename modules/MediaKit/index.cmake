# Toolkit installing script
# TkMedia module index file

function(AddTkMedia incVar lnkVar depVar)

    list(APPEND ${depVar} "TkMedia")
    list(APPEND ${incVar}
        "${CMAKE_SOURCE_DIR}/modules/MediaKit/src"
    )

    IF(WIN32)
        list(APPEND ${lnkVar} "TkMedia")
    ELSEIF(APPLE)
        list(APPEND ${lnkVar}  "TkMedia")
    ELSEIF(LINUX)
        list(APPEND ${lnkVar} "TkMedia")
    ENDIF()

    set(${incVar} ${${incVar}} PARENT_SCOPE)
    set(${lnkVar} ${${lnkVar}} PARENT_SCOPE)
    set(${depVar} ${${depVar}} PARENT_SCOPE)
endfunction()
