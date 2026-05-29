# MacOS platform config

function(SetPlatformDeps incVar lnkVar)
    IF(WIN32)
        list(APPEND ${lnkVar}
                "user32"
                "gdi32"
                "shell32"
                "Ws2_32"
        )
        add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

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
        find_package(X11 REQUIRED)
        list(APPEND ${incVar}
                ${X11_INCLUDE_DIR}
        )
        list(APPEND ${lnkVar}
                ${X11_LIBRARIES}
                dl
        )
    ENDIF()

    set(${incVar} ${${incVar}} PARENT_SCOPE)
    set(${lnkVar} ${${lnkVar}} PARENT_SCOPE)
endfunction()
