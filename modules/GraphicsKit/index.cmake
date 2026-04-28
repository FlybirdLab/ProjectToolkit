# Toolkit installing script
# TkGraphics module index file

function(AddTkGraphics incVar lnkVar depVar)

    list(APPEND ${depVar} "TkGraphics")
    list(APPEND ${incVar}
        "${CMAKE_BINARY_DIR}/modules/GraphicsKit/generated"
        "${CMAKE_SOURCE_DIR}/modules/GraphicsKit/src"
    )

    IF(WIN32)
        list(APPEND ${lnkVar} "TkGraphics")
    ELSEIF(APPLE)
        list(APPEND ${lnkVar}  "TkGraphics")
    ELSEIF(LINUX)
        list(APPEND ${lnkVar} "TkGraphics")
    ENDIF()

    set(${incVar} ${${incVar}} PARENT_SCOPE)
    set(${lnkVar} ${${lnkVar}} PARENT_SCOPE)
    set(${depVar} ${${depVar}} PARENT_SCOPE)
endfunction()
