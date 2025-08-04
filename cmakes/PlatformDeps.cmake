# MacOS platform config

function(SetPlatformDeps incVar lnkVar)
    IF(WIN32)
        list(APPEND ${lnkVar}
                "user32"
                "gdi32"
                "shell32"
        )
    ELSEIF(APPLE)
        list(APPEND ${lnkVar}
                "-framework Foundation"
                "-framework Cocoa"
                "-framework Metal"
                "-framework IOKit"
                "-framework QuartzCore"
                "-framework AppKit"
                "-framework SystemConfiguration"
                "-ObjC"
        )
    ELSEIF(LINUX)
    ENDIF()

    set(${incVar} ${${incVar}} PARENT_SCOPE)
    set(${lnkVar} ${${lnkVar}} PARENT_SCOPE)
endfunction()