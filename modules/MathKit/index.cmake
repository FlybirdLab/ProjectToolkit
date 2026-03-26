# Toolkit installing script
# TkMath module index file

function(AddTkMath incVar lnkVar depVar)

    list(APPEND ${depVar} "TkMath")
    list(APPEND ${incVar}
        "${CMAKE_SOURCE_DIR}/modules/MathKit/src"
    )

    IF(WIN32)
        list(APPEND ${lnkVar} "TkMath")
    ELSEIF(APPLE)
        list(APPEND ${lnkVar}  "TkMath")
    ELSEIF(LINUX)
        list(APPEND ${lnkVar} "TkMath")
    ENDIF()

    set(${incVar} ${${incVar}} PARENT_SCOPE)
    set(${lnkVar} ${${lnkVar}} PARENT_SCOPE)
    set(${depVar} ${${depVar}} PARENT_SCOPE)
endfunction()