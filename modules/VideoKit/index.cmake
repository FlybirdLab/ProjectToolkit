# Toolkit installing script
# TkVideo module index file

function(AddTkVideo incVar lnkVar depVar)

    list(APPEND ${depVar} "TkVideo" "TkAudio")
    list(APPEND ${incVar}
        "${CMAKE_SOURCE_DIR}/modules/VideoKit/src"
        "${CMAKE_BINARY_DIR}/modules/AudioKit/generated"
        "${CMAKE_SOURCE_DIR}/modules/AudioKit/src"
    )

    IF(WIN32)
        list(APPEND ${lnkVar} "TkVideo" "TkAudio")
    ELSEIF(APPLE)
        list(APPEND ${lnkVar}  "TkVideo" "TkAudio")
    ELSEIF(LINUX)
        list(APPEND ${lnkVar} "TkVideo" "TkAudio")
    ENDIF()

    set(${incVar} ${${incVar}} PARENT_SCOPE)
    set(${lnkVar} ${${lnkVar}} PARENT_SCOPE)
    set(${depVar} ${${depVar}} PARENT_SCOPE)
endfunction()
