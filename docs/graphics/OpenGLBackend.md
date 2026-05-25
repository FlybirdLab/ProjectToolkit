# OpenGL Backend (GraphicsKit fallback)

## Positioning

This backend is a **fallback**, not a peer of Vulkan / DirectX 12.
It exists so the engine has a working render path on hardware where:

- Vulkan or DX12 isn't available (older Intel GPUs, virtual machines, CI runners).
- The platform never had a modern API in the first place (macOS 10.14+).

It is **not** a target for new high-performance features, and it deliberately
does **not** chase API parity with the SM6 / Vulkan backends. If you need
compute, indirect draw, or modern bindless, use Vulkan/DX12 — this backend will
reject the call with `GLReportUnsupported(...)`.

## Versions

| Variant            | Profile               | Status                                |
| ------------------ | --------------------- | ------------------------------------- |
| Desktop OpenGL     | 4.1 Core Profile      | implemented (Win/macOS)               |
| OpenGL ES          | 3.0 Core              | not implemented (no EGL platform)     |

The 4.1 Core target is fixed by macOS — that is the newest profile Apple ever
shipped, and it's the lowest common denominator we still consider acceptable.
Bumping to 4.5 / DSA is **not** allowed in this backend.

## Platform matrix

| OS                | Layer        | Status                                                   |
| ----------------- | ------------ | -------------------------------------------------------- |
| Windows           | WGL          | implemented (WGL_ARB_create_context_attribs path)        |
| macOS             | NSOpenGL     | implemented (4.1 Core via NSOpenGLPixelFormat)           |
| Linux             | GLX          | not implemented — no Linux WindowKit yet                 |
| iOS / Android     | EGL          | not implemented                                          |

The macOS path is gated on WindowKit shipping a macOS native window: until
that exists, `GLPlatformMacOS::createWindowContext` is reachable but cannot be
exercised. `getWindowPtr()` is allowed to return either `NSWindow*` or
`NSView*`; the .mm resolves via `-isKindOfClass:` at runtime.

## Supported runtime features

- Window / SwapChain / Backbuffer (single RT, no MSAA)
- Vertex / Index / Constant buffers
- VS + PS shader stages, GLSL 410 Core
- GLSL shadercache format (`GLShaderSourceHeader` + GLSL bytes)
- `GfxPipelineState` link with stable attribute locations (POSITION..ATTRIBUTE3)
- `GfxVertexLayout`, including per-instance divisor
- `Texture2D` (sized internal formats only)
- `Sampler` (filter / address / anisotropy / comparison / border color)
- `SR` (shader resource view) binding to `SR0..SR15`
- `CB` (constant buffer) binding to `CB0..CB15`
- Render target texture + Depth/Stencil texture via cached FBO
- `clear` / `clearDepth` / `present`
- Feedback-loop guard (refuses binding a current RT/DS as SR)

## Not supported

The backend rejects these with `GLReportUnsupported` rather than silent no-op:

- Compute shader / `dispatch` / `dispatchIndirect`
- UAV / SSBO / image load-store / `bindUA` / `clearUA` / counter buffers
- Indirect draw (`drawInstancedIndirect`, `drawIndexedInstancedIndirect`)
- Geometry / Hull / Domain stages
- TextureCube / Texture3D / TextureArray
- Multisample textures, `resolveMultisample`
- Layered (`array_index != 0`) attachments
- MRT (multiple color render targets)
- Backbuffer + custom depth combination
- Raw / structured / argument / append / counter buffer flags
- `copyResource` / `copyBuffer` / `copyTexture` / `copySubresource`
  (these are silent no-ops in the header — see TODOs for tightening)

## Shader contract

### Attribute locations (must match GLSL `layout(location=N)`)

| Semantic       | Location |
| -------------- | -------- |
| POSITION       | 0        |
| COLOR          | 1        |
| NORMAL         | 2        |
| TANGENT        | 3        |
| BLENDINDICES   | 4        |
| BLENDWEIGHTS   | 5        |
| TEXCOORD0..7   | 6..13    |
| ATTRIBUTE0..3  | 14..17   |

### Resource bindings

- Constant buffers: GLSL block name `CB0`..`CB15`, bound via
  `glBindBufferBase(GL_UNIFORM_BUFFER, slot, ubo)` and
  `glUniformBlockBinding(program, idx, slot)` at link time.
- Sampled textures: GLSL uniform name `SR0`..`SR15`, mapped to texture units
  0..15 via `glUniform1i` once at link time.
- Sampler objects: bound via `glBindSampler(slot, sampler)` — GLSL does not
  declare standalone `SamplerN` uniforms.

### ShaderCompiler integration

`ShaderCompiler --backend opengl` (aliases: `gl`, `ogl`) emits a
`*_glsl410.shadercache` file.

The current pipeline is **passthrough + stage-define injection**: the input
source is expected to be valid GLSL 410, and the compiler:

1. Parses META_VS / META_PS markers via the existing `Parser`.
2. For each stage, locates the `#version` directive in the source.
3. Inserts a single line right after it: either
   `#define TK_SHADER_STAGE_VERTEX 1` or `#define TK_SHADER_STAGE_PIXEL 1`.
4. Wraps the transformed source in `GLShaderSourceHeader` and writes one
   shadercache entry per stage.

This lets a **single GLSL source file** drive both VS and PS via `#ifdef`
gates. See [`tools/Sandbox/shaders/Triangle.glsl.shader`](../../tools/Sandbox/shaders/Triangle.glsl.shader)
for the reference layout.

HLSL → SPIR-V → GLSL cross-compile is **not** implemented and requires
SPIRV-Cross as a dependency, which is out of scope.

## Debug / validation

`GLCheckError(const char *where)` reads `glGetError()` in a loop and logs each
error code. It runs at:

- `GLDevice::init`
- `GLSwapChain::resize`
- `GLBuffer::onInit`, `GLConstantBuffer::init`
- `GLTexture::onInit`, `GLSampler::onInit`
- `GLPipelineState::init`
- `GLFramebufferCache::createFramebuffer` (via FBO completeness check)
- `GLContext::updateTexture`
- `GLContext::drawInstanced` / `drawIndexedInstanced`

`GL_KHR_debug` (`glDebugMessageCallback`) is **not** wired in by default; it
requires GL 4.3, which lifts the macOS minimum above 4.1. If a debugging build
needs it, gate it on a CMake flag and the runtime extension presence — do not
make it a required path.

## Sandbox bring-up

The Sandbox compiles all enabled backends in and selects at runtime via
`--backend=<name>`. OpenGL is added to the choice list:

```
Sandbox --backend=opengl
```

The OpenGL path uses [`Triangle.glsl.shader`](../../tools/Sandbox/shaders/Triangle.glsl.shader)
instead of the HLSL `Triangle.shader`. The two coexist; nothing else changes.

### Manual configure

```powershell
# Windows, OpenGL-only
cmake -S . -B build-opengl -G Ninja `
  -DTK_GFX_OPENGL_ENABLE=ON `
  -DTK_GFX_OPENGLES_ENABLE=OFF `
  -DTK_GFX_VULKAN_ENABLE=OFF `
  -DTK_GFX_DIRECTX11_ENABLE=OFF `
  -DTK_GFX_DIRECTX12_ENABLE=OFF `
  -DSANDBOX_GFX_BACKEND=opengl

cmake --build build-opengl --target Sandbox
```

```bash
# macOS, OpenGL-only (deprecated 4.1 framework still ships)
cmake -S . -B build-opengl -G Ninja \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=10.14 \
  -DTK_GFX_OPENGL_ENABLE=ON \
  -DTK_GFX_OPENGLES_ENABLE=OFF \
  -DTK_GFX_VULKAN_ENABLE=OFF \
  -DTK_GFX_METAL_ENABLE=OFF \
  -DSANDBOX_GFX_BACKEND=opengl

cmake --build build-opengl --target Sandbox
```

OpenGL can also coexist with the other backends in the same build, in which
case `Sandbox --backend=opengl` selects it at runtime and `auto` still picks
DX12 > DX11 > Vulkan > OpenGL.

### Expected log on startup

```
OpenGL fallback backend initialized
GL_VENDOR: <driver>
GL_RENDERER: <GPU>
GL_VERSION: 4.1.0 <driver-specific>
GL_SHADING_LANGUAGE_VERSION: 4.10
[ShaderCompiler] compiling Triangle.glsl as glsl410
Sandbox using backend: OpenGL 4.1 (opengl)
```

If the driver reports a GL version below 4.1, `GLCheckMinimumVersion` warns
but does not abort — actual GL errors get caught by `GLCheckError`.

## Known TODOs

- `GLSwapChain::resize` on macOS does not yet read NSView `convertSizeToBacking:`,
  so Retina displays may render at logical (1x) resolution. Tracked for the
  WindowKit macOS implementation phase.
- `copyResource` / `copyBuffer` / `copyTexture` are header no-ops; either
  implement via blit FBO or convert to `GLReportUnsupported`.
- `GLContext::resetUA` is silent (UA path doesn't exist) — that's deliberate.
- Linux GLX backend is unstubbed; revisit once WindowKit Linux ships.
