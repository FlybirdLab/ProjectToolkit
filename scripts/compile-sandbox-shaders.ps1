#
# @Author: cubevlmu khfahqp@gmail.com
# @LastEditors: cubevlmu khfahqp@gmail.com
# Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
#

param(
    [ValidateSet("dx12", "dx11", "vulkan", "opengl")]
    [string]$Backend = "dx12",

    [string]$BuildDir = "build",

    [string]$ShaderCompiler = "",

    [string]$OutDir = ""
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildRoot = if ([System.IO.Path]::IsPathRooted($BuildDir)) {
    $BuildDir
} else {
    Join-Path $repoRoot $BuildDir
}

if ([string]::IsNullOrWhiteSpace($ShaderCompiler)) {
    $ShaderCompiler = Join-Path $buildRoot "tools/ShaderCompiler/ShaderCompiler.exe"
}
if (-not (Test-Path $ShaderCompiler)) {
    throw "ShaderCompiler not found: $ShaderCompiler"
}

if ([string]::IsNullOrWhiteSpace($OutDir)) {
    $OutDir = Join-Path $buildRoot "tools/Sandbox/shader-output/$Backend"
}
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$compilerBackend = switch ($Backend) {
    "dx12" { "dx" }
    "dx11" { "d3d" }
    "vulkan" { "vk" }
    "opengl" { "opengl" }
}

$shaderDir = Join-Path $repoRoot "tools/Sandbox/shaders"
$inputs = @(
    Join-Path $shaderDir "Triangle.tkshader"
    Join-Path $shaderDir "VideoQuad.tkshader"
)

foreach ($input in $inputs) {
    if (-not (Test-Path $input)) {
        throw "Shader manifest not found: $input"
    }
    & $ShaderCompiler --file $input --backend $compilerBackend --out $OutDir
    if ($LASTEXITCODE -ne 0) {
        throw "ShaderCompiler failed for $input with exit code $LASTEXITCODE"
    }
}

Write-Host "Sandbox shaders compiled to $OutDir"
