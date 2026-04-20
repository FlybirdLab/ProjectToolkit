# Toolkit installing script
# TkAudio module index file

function(AddTkAudio incVar lnkVar depVar)

    list(APPEND ${depVar} "TkAudio")
    list(APPEND ${incVar}
        "${CMAKE_SOURCE_DIR}/modules/AudioKit/src"
    )

    IF(WIN32)
        list(APPEND ${lnkVar} "TkAudio")
    ELSEIF(APPLE)
        list(APPEND ${lnkVar}  "TkAudio")
    ELSEIF(LINUX)
        list(APPEND ${lnkVar} "TkAudio")
    ENDIF()

    set(${incVar} ${${incVar}} PARENT_SCOPE)
    set(${lnkVar} ${${lnkVar}} PARENT_SCOPE)
    set(${depVar} ${${depVar}} PARENT_SCOPE)
endfunction()
