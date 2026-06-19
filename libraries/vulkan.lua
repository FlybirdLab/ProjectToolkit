--[[
Author: cubevlmu khfahqp@gmail.com
LastEditors: cubevlmu khfahqp@gmail.com
Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
--]]

-- Vulkan SDK - discovered from VULKAN_SDK or platform SDK installations.
when("windows", function()
    require_local(".", {
        name = "vulkan",
        build = "custom",
        export_defines = false,
        commands = {
            install = {
                [[powershell -NoProfile -ExecutionPolicy Bypass -Command "$sdk=$env:VULKAN_SDK; if([string]::IsNullOrWhiteSpace($sdk)){ Write-Error 'VULKAN_SDK is not set. Install the Vulkan SDK or set VULKAN_SDK to its root directory.'; exit 1 }; $include=Join-Path $sdk 'Include'; $library=Join-Path $sdk 'Lib\vulkan-1.lib'; if(-not (Test-Path -LiteralPath $include -PathType Container)){ Write-Error ('Vulkan SDK include directory is missing: ' + $include); exit 1 }; if(-not (Test-Path -LiteralPath $library -PathType Leaf)){ Write-Error ('Vulkan loader import library is missing: ' + $library); exit 1 }; $artifact='$(artifact)'; $artifactInclude=Join-Path $artifact 'include'; $artifactLib=Join-Path $artifact 'lib'; New-Item -ItemType Directory -Force -Path $artifactInclude,$artifactLib | Out-Null; Copy-Item -Path (Join-Path $include '*') -Destination $artifactInclude -Recurse -Force; Copy-Item -LiteralPath $library -Destination (Join-Path $artifactLib 'vulkan-1.lib') -Force; exit 0"]]
            }
        },
        artifacts = {
            include_dirs = { "include" },
            lib_files = { "lib/vulkan-1.lib" }
        }
    })
end)

when("linux", function()
    require_local(".", {
        name = "vulkan",
        build = "custom",
        export_defines = false,
        commands = {
            install = {
                [[sdk="${VULKAN_SDK:-}"; include=""; library=""; for path in "$sdk/include" "$sdk/x86_64/include"; do if [ -n "$sdk" ] && [ -d "$path/vulkan" ]; then include="$path"; break; fi; done; if command -v pkg-config >/dev/null 2>&1 && pkg-config --exists vulkan; then [ -n "$include" ] || include=$(pkg-config --variable=includedir vulkan); libdir=$(pkg-config --variable=libdir vulkan); [ -f "$libdir/libvulkan.so" ] && library="$libdir/libvulkan.so"; fi; [ -n "$include" ] || { [ -d /usr/include/vulkan ] && include=/usr/include; }; for path in "$sdk/lib/libvulkan.so" "$sdk/x86_64/lib/libvulkan.so" /usr/lib/x86_64-linux-gnu/libvulkan.so /usr/lib64/libvulkan.so /usr/lib/libvulkan.so; do if [ -n "$sdk" ] || [ "${path#/usr/}" != "$path" ]; then if [ -f "$path" ]; then library="$path"; break; fi; fi; done; if [ -z "$include" ] || [ ! -d "$include/vulkan" ]; then echo 'error: Vulkan SDK headers were not found. Set VULKAN_SDK or install vulkan-headers.' >&2; exit 1; fi; if [ -z "$library" ] || [ ! -f "$library" ]; then echo 'error: Vulkan loader library was not found. Set VULKAN_SDK or install the Vulkan loader development package.' >&2; exit 1; fi; mkdir -p "$(artifact)/include" "$(artifact)/lib"; cp -R "$include/vulkan" "$(artifact)/include/"; cp -L "$library" "$(artifact)/lib/libvulkan.so"]]
            }
        },
        artifacts = {
            include_dirs = { "include" },
            lib_files = { "lib/libvulkan.so" },
            system_libs = { "dl", "pthread" }
        }
    })
end)

when("macos", function()
    require_local(".", {
        name = "vulkan",
        build = "custom",
        export_defines = false,
        commands = {
            install = {
                [[sdk="${VULKAN_SDK:-}"; include=""; library=""; for path in "$sdk/include" "$sdk/macOS/include"; do if [ -n "$sdk" ] && [ -d "$path/vulkan" ]; then include="$path"; break; fi; done; for path in "$sdk/lib/libvulkan.dylib" "$sdk/macOS/lib/libvulkan.dylib"; do if [ -n "$sdk" ] && [ -f "$path" ]; then library="$path"; break; fi; done; if command -v pkg-config >/dev/null 2>&1 && pkg-config --exists vulkan; then [ -n "$include" ] || include=$(pkg-config --variable=includedir vulkan); if [ -z "$library" ]; then libdir=$(pkg-config --variable=libdir vulkan); [ -f "$libdir/libvulkan.dylib" ] && library="$libdir/libvulkan.dylib"; fi; fi; if command -v brew >/dev/null 2>&1; then if [ -z "$include" ]; then headers=$(brew --prefix vulkan-headers 2>/dev/null || true); [ -d "$headers/include/vulkan" ] && include="$headers/include"; fi; if [ -z "$library" ]; then loader=$(brew --prefix vulkan-loader 2>/dev/null || true); [ -f "$loader/lib/libvulkan.dylib" ] && library="$loader/lib/libvulkan.dylib"; fi; fi; if [ -z "$include" ] || [ ! -d "$include/vulkan" ]; then echo 'error: Vulkan SDK headers were not found. Set VULKAN_SDK or install vulkan-headers.' >&2; exit 1; fi; if [ -z "$library" ] || [ ! -f "$library" ]; then echo 'error: Vulkan loader library was not found. Set VULKAN_SDK or install vulkan-loader.' >&2; exit 1; fi; mkdir -p "$(artifact)/include" "$(artifact)/lib"; cp -R "$include/vulkan" "$(artifact)/include/"; cp -L "$library" "$(artifact)/lib/libvulkan.dylib"]]
            }
        },
        artifacts = {
            include_dirs = { "include" },
            lib_files = { "lib/libvulkan.dylib" },
            frameworks = { "Cocoa", "IOSurface", "Metal", "QuartzCore" }
        }
    })
end)

when("android", function()
    require_local(".", {
        name = "vulkan",
        build = "custom",
        export_defines = false,
        commands = {
            install = {
                [["$(cmake)" -DTPKG_ARTIFACT="$(artifact)" -DTPKG_ARCH="$(arch)" -P "$(source)/libraries/scripts/export_vulkan_android.cmake"]]
            }
        },
        artifacts = {
            include_dirs = { "include" },
            lib_files = { "lib/libvulkan.so" }
        }
    })
end)
