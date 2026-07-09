# Toolkit installing script
# TkReflect module index file

function(AddTkReflect incVar lnkVar depVar)

    list(APPEND ${depVar} "TkReflect")
    list(APPEND ${incVar}
        "${CMAKE_SOURCE_DIR}/modules/ReflectKit/src"
    )

    IF(WIN32)
        list(APPEND ${lnkVar} "TkReflect")
    ELSEIF(APPLE)
        list(APPEND ${lnkVar}  "TkReflect")
    ELSEIF(LINUX)
        list(APPEND ${lnkVar} "TkReflect")
    ENDIF()

    set(${incVar} ${${incVar}} PARENT_SCOPE)
    set(${lnkVar} ${${lnkVar}} PARENT_SCOPE)
    set(${depVar} ${${depVar}} PARENT_SCOPE)
endfunction()
