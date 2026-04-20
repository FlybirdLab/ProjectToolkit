# Toolkit installing script
# TkLocale module index file

function(AddTkLocale incVar lnkVar depVar)

    list(APPEND ${depVar} "TkLocale")
    list(APPEND ${incVar}
        "${CMAKE_SOURCE_DIR}/modules/LocaleKit/src"
    )

    IF(WIN32)
        list(APPEND ${lnkVar} "TkLocale")
    ELSEIF(APPLE)
        list(APPEND ${lnkVar} "TkLocale")
    ELSEIF(LINUX)
        list(APPEND ${lnkVar} "TkLocale")
    ENDIF()

    set(${incVar} ${${incVar}} PARENT_SCOPE)
    set(${lnkVar} ${${lnkVar}} PARENT_SCOPE)
    set(${depVar} ${${depVar}} PARENT_SCOPE)
endfunction()
