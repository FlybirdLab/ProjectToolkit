#
# @Author: cubevlmu khfahqp@gmail.com
# @LastEditors: cubevlmu khfahqp@gmail.com
# Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
#

param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug",

    [string]$BuildDir = "build",

    [string]$Target = "all",

    [string]$Generator = "Ninja",

    [string]$TBuild = "tbuild",

    [string]$CMake = "cmake",

    [string]$Toolchain = "",

    [switch]$SkipRestore,

    [switch]$SkipGenerate,

    [switch]$SkipConfigure,

    [switch]$VerboseBuild
)

$ErrorActionPreference = "Stop"

function Invoke-Step {
    param(
        [string]$Name,
        [scriptblock]$Body
    )

    Write-Host ""
    Write-Host "==> $Name"
    & $Body
    if ($LASTEXITCODE -ne 0) {
        throw "$Name failed with exit code $LASTEXITCODE"
    }
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildRoot = if ([System.IO.Path]::IsPathRooted($BuildDir)) {
    $BuildDir
} else {
    Join-Path $repoRoot $BuildDir
}

$configLower = $Config.ToLowerInvariant()
$tbuildRootArgs = @("--root", $repoRoot)
$tbuildCommandArgs = @("--config", $configLower)
if (-not [string]::IsNullOrWhiteSpace($Toolchain)) {
    $tbuildCommandArgs += @("--toolchain", $Toolchain)
}

Push-Location $repoRoot
try {
    if (-not $SkipRestore) {
        Invoke-Step "tbuild restore" {
            & $TBuild @tbuildRootArgs restore @tbuildCommandArgs
        }
    }

    if (-not $SkipGenerate) {
        Invoke-Step "tbuild generate" {
            & $TBuild @tbuildRootArgs generate @tbuildCommandArgs
        }
    }

    if (-not $SkipConfigure) {
        Invoke-Step "cmake configure" {
            & $CMake -S $repoRoot -B $buildRoot -G $Generator `
                "-DCMAKE_BUILD_TYPE=$Config" `
                "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
        }
    }

    $buildArgs = @("--build", $buildRoot, "--config", $Config, "--target", $Target)
    if ($VerboseBuild) {
        $buildArgs += "--verbose"
    }

    Invoke-Step "cmake build" {
        & $CMake @buildArgs
    }
}
finally {
    Pop-Location
}
