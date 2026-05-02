# ShaderCompiler

`ShaderCompiler` compiles Toolkit shader source files into `.shadercache` files. The tool accepts either one shader file or a source directory.

## Usage

```powershell
ShaderCompiler.exe --file docs/testdata/shaders/basic_color.shader --backend d3d --out build/shader-output
ShaderCompiler.exe --source-dir docs/testdata/shaders --backend d3d,dx,vk --out build/shader-output
ShaderCompiler.exe docs/testdata/shaders/basic_color.shader --type all --out build/shader-output
```

## Options

- `--file <path>` or `--input <path>`: compile one shader source file.
- `--source-dir <path>` or `--dir <path>`: compile all `.shader` and `.hlsl` files under a directory recursively.
- `--backend <list>`: comma-separated backend list.
- `--type <list>` or `--target <list>`: alias for `--backend`.
- `--out <dir>`: output directory for generated `.shadercache` files.
- `--help`: print command line help.

Supported backend names:

- `d3d`, `d3d11`, `sm5`: Direct3D shader model 5 through D3DCompiler.
- `dx`, `dxc`, `d3d12`, `sm6`: DirectX shader model 6 through DXC.
- `vk`, `vulkan`: Vulkan SPIR-V through glslang.
- `all`: all currently implemented backends for the platform.

`opengl` and `metal` are recognized by the command line layer, but compiler backends are not implemented yet.

## Test Data

Sample shaders live in:

```text
docs/testdata/shaders
```

Run all available shader compiler test inputs:

```powershell
ShaderCompiler.exe --source-dir docs/testdata/shaders --backend all --out build/shader-output
```

The output file name format is:

```text
<shader-name>_<backend-profile>.shadercache
```

## Dependencies

The Vulkan backend requires the Vulkan SDK to be installed and visible to CMake:

```powershell
echo $env:VULKAN_SDK
```

`tools/ShaderCompiler` links Vulkan through `find_package(Vulkan REQUIRED)`, wrapped by `libs/vulkan/index.cmake`.

On Windows, Direct3D Shader Model 5 uses `d3dcompiler_47.dll`, and Shader Model 6 uses DXC. Make sure the DXC runtime DLLs are available next to the executable when running the `dx` backend:

```text
dxcompiler.dll
dxil.dll
```
